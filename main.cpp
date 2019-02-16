//============================================================================
// Name        : CPU_Scheduler.cpp
// Author      : Fanchen
// Date		   : 02/10/2019
// Description : CPU scheduler assignment for COP4610
//============================================================================

#include "CPU_Scheduler.h"

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
			CPUContextSwitch(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, false);

		if (CPUidle){ // CPU idle, test whether okay to push process onto CPU
			if (!waitQ.empty() && waitQ.begin()->arrival <= sysTime){ // CPU and waitQ both ready to accept new process
				pushToCPU(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, hasTimeLimit, timeLimit); // load a process to CPU, if possible
				// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
				printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, CPUidle);
				updateGanttChart(sysTime, onCPU, gantt, CPUidle);
			}
			else if (!gantt.preIdle){ // CPU remains idle, nothing happens.
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
				CPUContextSwitch(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, false);
				currQuant = quant; // reset quantum
			}
			else if (currQuant == 0){ // current process hasn't finished its CPU burst but used up its quantum
				CPUContextSwitch(sysTime, waitQ,ioQ, complete, onCPU, CPUidle, true);
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
			if (onCPU.remainCPUBurst == 0){ // current process finishes CPU burst
				CPUContextSwitch(sysTime, currQ, quantums, MLQ, ioQ, complete, onCPU, CPUidle, 1);
			}
			else if (currQ[onCPU.queuePriority - 1] == 0){ // current process hasn't finished its CPU burst but used up its quantum
				CPUContextSwitch(sysTime, currQ, quantums, MLQ, ioQ, complete, onCPU, CPUidle, 2);
			}
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
					// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
					updateGanttChart(sysTime, onCPU, gantt, CPUidle);
					break;
				}
			}
			if (!CPUidle){
				printWhenNewPricessLoaded(sysTime, MLQ, ioQ, complete, onCPU, CPUidle);
			}
			else if (CPUidle && !gantt.preIdle){ // CPU remains idle, nothing happens.
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



int main() {
	std::vector<Process> processList;
	std::vector<Process> waitQ;
	std::vector<Process> ioQ;
	std::vector<Process> complete;
	Process onCPU;
	Gantt gantt;
	std::vector<std::vector<Process> > MLQ; // multilevel queues

//	bool hasTimeLimit = true;
	bool hasTimeLimit = false;
	int timeLimit = 147;
	int sysTime = 0; // initial system time
	int numProcess = 9; // total number of processes

//	 Testing Data
	int info[9][30] = {{4, 27, 3, 31, 2, 43, 4, 18, 4, 22, 4, 26, 3, 24, 4},
						{16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4},
						{8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6},
						{3, 35, 4, 41, 4, 45, 3, 51, 4, 61, 3, 54, 6, 82, 5, 77, 3},
						{4, 48, 5, 44, 7, 42, 12, 37, 9, 46, 4, 41, 9, 31, 7, 43, 8},
						{11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8},
						{14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10},
						{4, 14, 5, 33, 6, 51, 14, 63, 16, 87, 6, 74, 7},
						{3, 37, 9, 41, 8, 30, 4, 29, 7, 33, 5, 22, 4, 24, 5, 29, 16}};

//	int info[5][30] = {{5,6,7},{4,2,3},{2,3,4},{5,2,7},{3,2,4}};

//	int info[5][30] = {{8},{6},{12},{4},{6}};

//	int info[5][30] = {{10},{11},{12},{8},{5}};

	// initialize all processes, put them all in waitQ in the order of their number.
//	for (int i = 0; i < 5; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}

	for (int i = 0; i < numProcess; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}

//	processList[0].arrival = 0;
//	processList[1].arrival = 3;
//	processList[2].arrival = 4;
//	processList[3].arrival = 7;
//	processList[4].arrival = 14;

//	processList[0].arrival = 0;
//	processList[1].arrival = 2;
//	processList[2].arrival = 5;
//	processList[3].arrival = 11;
//	processList[4].arrival = 17;

//	processList[0].arrival = 1;
//	processList[1].arrival = 6;
//	processList[2].arrival = 8;
//	processList[3].arrival = 12;
//	processList[4].arrival = 14;


	// FCFS
	FCFS(sysTime, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

	// RR
//	RR(sysTime, 5, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

	// MLFQ
	int numSubQ = 3;
	for (int i = 0; i < numSubQ; i++) // add subqueues to MLQ
		MLQ.emplace_back(std::vector<Process>());
	std::vector<int> quantums = {4, 9, -1}; // all non-RR queues default to -1 quantum (set to -1 such that non-RR queue quantum would never reach 0 to trigger a quantum drying up event)

	MLFQ(sysTime, quantums, processList, MLQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// print out final results
	printGanttChart(gantt);
	printRT_WT_TT(waitQ, ioQ, complete, onCPU, hasTimeLimit);
//	printRT_WT_TT(MLQ, ioQ, complete, onCPU, hasTimeLimit); // for MLFQ

	return 0;
}
