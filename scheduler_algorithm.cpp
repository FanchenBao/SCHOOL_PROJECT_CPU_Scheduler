/*
 * scheduler_algorithm.cpp
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 */

#include "scheduler_algorithm.h"

void FCFS(int& sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit){
	bool CPUidle = true; // flag for whether CPU is busy or idle
	while (true){
		if (!processList.empty()){ // admit processes based on their initial arrival time
			admitProcess(sysTime, processList, waitQ);
		}

		// Do context switches first
		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0) // some process finishes I/O, I/O context switch
			IOContextSwitch(sysTime, waitQ, ioQ);

		if (!CPUidle && onCPU.remainCPUBurst == 0) // current process finishes CPU burst, CPU context switch
			CPUContextSwitch(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, 1);

		if (CPUidle){ // CPU idle, test whether okay to push process onto CPU
			if (!waitQ.empty() && waitQ.begin()->arrival <= sysTime){ // CPU and waitQ both ready to accept new process
				pushToCPU(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, hasTimeLimit, timeLimit); // load a process to CPU, if possible
				// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
				printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, CPUidle);
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
			}
			else if (!gantt.preIdle){ // CPU remains idle, nothing happens.
				// print idle condition and update Gantt Chart with it
				printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, CPUidle);
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
			}
		}

		// END POINT!!!
		if ((onCPU.remainCPUBurst == 0 && waitQ.empty() && ioQ.empty() && processList.empty()) || (hasTimeLimit && sysTime == timeLimit))
			break;

		// Update key parameters at each time tick
		if (!CPUidle) {onCPU.remainCPUBurst--;} // CPU processing
		for (auto& p : ioQ) {p.remainIOBurst--;} // ioQ updates
		sysTime++;
	}
}


void RR(int& sysTime, int quant, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit){
	int currQuant = quant;
	bool CPUidle = true; // flag for whether CPU is busy or idle
	while (true){
		if (!processList.empty()) // admit processes based on their initial arrival time
			admitProcess(sysTime, processList, waitQ);

		// Do context switches first
		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0) // some process finishes I/O, I/O context switch
			IOContextSwitch(sysTime, waitQ, ioQ);

		if (!CPUidle){ // CPU side context switch
			if (onCPU.remainCPUBurst == 0){ // current process finishes CPU burst
				CPUContextSwitch(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, 1);
				currQuant = quant; // reset quantum
			}
			else if (currQuant == 0){ // current process hasn't finished its CPU burst but used up its quantum
				CPUContextSwitch(sysTime, waitQ,ioQ, complete, onCPU, CPUidle, 2);
				currQuant = quant; // reset quantum
			}
		}

		if (CPUidle){ // CPU idle, test whether okay to push process onto CPU
			if (!waitQ.empty() && waitQ.begin()->arrival <= sysTime){ // take on new process only when CPU is idle and there is process in the waitQ
				pushToCPU(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, hasTimeLimit, timeLimit); // load a process to CPU, if possible
				// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
				printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, CPUidle);
			}
			else if (!gantt.preIdle){ // CPU remains idle, nothing happens.
				// print idle condition and update Gantt Chart with it
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
				printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, CPUidle);
			}
		}

		// END POINT!!!
		if ((onCPU.remainCPUBurst == 0 && waitQ.empty() && ioQ.empty() && processList.empty()) || (hasTimeLimit && sysTime == timeLimit))
			break;

		// Update key parameters at each time tick
		if (!CPUidle){onCPU.remainCPUBurst--; currQuant--;} // CPU processing
		for (auto& p : ioQ){p.remainIOBurst--;} // ioQ updates
		sysTime++;
	}
}

void MLFQ(int& sysTime, const std::vector<int>& quantums, std::vector<Process>& processList, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit){
	std::vector<int> currQ(quantums);
	bool CPUidle = true; // flag for whether CPU is busy or idle
	while (true){
		if (!processList.empty()) // admit processes based on their initial arrival time
			admitProcess(sysTime, processList, MLQ[0]);

		// Do context switches first.
		// I/O side context switch
		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0) // some process finishes I/O, needs to get back to waitQ
			IOContextSwitch(sysTime, MLQ, ioQ);

		if (!CPUidle){// CPU side context switch
			if (onCPU.remainCPUBurst == 0) // current process finishes CPU burst
				CPUContextSwitch(sysTime, currQ, quantums, MLQ, ioQ, complete, onCPU, CPUidle, 1);
			else if (currQ[onCPU.queuePriority - 1] == 0) // current process hasn't finished its CPU burst but used up its quantum
				CPUContextSwitch(sysTime, currQ, quantums, MLQ, ioQ, complete, onCPU, CPUidle, 2);
			else{ // current process hasn't finished its CPU burst or quantum, but the higher priority queue has process ready to preempt the current process
				for (int i = 0; i < onCPU.queuePriority - 1; i++){
					if (!MLQ[i].empty() && MLQ[i].begin()->arrival == sysTime){
						CPUContextSwitch(sysTime, currQ, quantums, MLQ, ioQ, complete, onCPU, CPUidle, 3);
						break;
					}
				}
			}

		}

		if (CPUidle){ // CPU idle, test whether okay to push process onto CPU
			for (auto& subQ : MLQ){
				if (!subQ.empty() && subQ.begin()->arrival <= sysTime){
					pushToCPU(sysTime, subQ, ioQ, complete, onCPU, CPUidle, hasTimeLimit, timeLimit);
					// Provide information to generate Gantt Chart
					updateGanttChart(sysTime, onCPU, gantt, CPUidle);
					break;
				}
			}
			if (!CPUidle) // print out waitQ, ioQ, and CPU info when a new process gets CPU,
				printWhenNewPricessLoaded(sysTime, MLQ, ioQ, complete, onCPU, CPUidle);
			else if (CPUidle && !gantt.preIdle){ // CPU remains idle, nothing happens.
				// print idle condition and update Gantt Chart with it
				printWhenNewPricessLoaded(sysTime, MLQ, ioQ, complete, onCPU, CPUidle);
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
			}
		}

		// END POINT!!!
		if (complete.size() == numProcess || (hasTimeLimit && sysTime == timeLimit)) // all processes complete or sysTime reaches timeLimit
			break;

		// Update key parameters at each time tick
		if (!CPUidle){onCPU.remainCPUBurst--; currQ[onCPU.queuePriority - 1]--;} // CPU processing
		for (auto& p : ioQ){p.remainIOBurst--;} // ioQ updates
		sysTime++;
	}
}


