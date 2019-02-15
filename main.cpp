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
		if (!processList.empty()) // admit processes based on their initial arrival time
			admitProcess(sysTime, processList, waitQ);

		// Do context switches first
		IOContextSwitch(sysTime, waitQ, ioQ); // I/O side context switch, if needed
		CPUContextSwitch(ioQ, complete, onCPU, CPUidle); // CPU side context switch, if needed
		pushToCPU(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, gantt, hasTimeLimit, timeLimit); // load a process to CPU, if possible

		// END POINT!!!
		if ((onCPU.remainCPUBurst == 0 && waitQ.empty() && ioQ.empty()) || (hasTimeLimit && sysTime == timeLimit))
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
		IOContextSwitch(sysTime, waitQ, ioQ); // I/O side context switch, if needed

		// CPU side context switch
		newProcessLoaded = false;
		if (!CPUidle && onCPU.remainCPUBurst == 0){ // current process finishes CPU burst
			if (onCPU.index < onCPU.ptSize - 2){pushToIO(ioQ, onCPU);} // still more I/O to do
			else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
			CPUidle = true; // set CPU to idle
			currQuant = quant;

			if (waitQ.empty() && ioQ.empty()) {break;} // END POINT!!!
		}
		if (!CPUidle && currQuant == 0){ // current process hasn't finished its CPU burst but used up its quantum
			onCPU.arrival = sysTime; // reset arrival time
			waitQ.push_back(onCPU); // pushed to the back of waitQ
			CPUidle = true; // set CPU to idle
			currQuant = quant; // reset quantum
		}
		if (CPUidle && !waitQ.empty() && waitQ.begin()->arrival <= sysTime){ // take on new process only when CPU is idle and there is process in the waitQ
			// the top of waitQ should be pushed onto CPU, BUT if there are multiple processes with the
			// same arrival time and all ready to go to CPU, only the one with the smallest process number
			// gets CPU first.
			handleSameArrivalTimeInWaitQ(waitQ); // check same arrival time situation
			popOnCPU(waitQ, onCPU, CPUidle, newProcessLoaded);}

		// print out waitQ, ioQ, and CPU information at each instance when a new process gets CPU
		// also provide information to generate Gantt Chart
		if (newProcessLoaded){printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, gantt);}

		// Then update key parameters at each time tick
		// CPU processing
		if (!CPUidle){
			onCPU.remainCPUBurst--;
			onCPU.turnaroundTime++;
			currQuant--;
		}
		// waitQ updates
		for (auto& p : waitQ){
			if (!p.serviced) // increase RT for the processes that haven't been serviced yet
				p.responseTime++;
			p.waitTime++;
			p.turnaroundTime++;
		}
		// ioQ updates
		for (auto& p : ioQ){
			p.remainIOBurst--;
			p.turnaroundTime++;
		}
		sysTime++;
	}
	return sysTime;

}
//
//int MLFQ(int q1, int q2, std::vector<Process>& processList, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt){
//	int sysTime = 0;
//	int currQ1 = q1;
//	int currQ2 = q2;
//	bool CPUidle = true; // flag for whether CPU is busy or idle
//	bool newProcessLoaded; // flag set true only at the iteration a new process is loaded
//
//	while (true){
//		// admit processes based on their initial arrival time
//		if (!processList.empty())
//			admitProcess(sysTime, processList, MLQ[0]);
//
//		// Do context switches first.
//		// I/O side context switch
//		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0){ // some process finishes I/O, needs to get back to waitQ
//			std::vector<Process> targets;
//			handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
//			for (auto t : targets){
//				// push process to the waiting queue corresponding to their queue priority
//				if (t.queuePriority == 1)
//					MLQ[0].push_back(t);
//				else if (t.queuePriority == 2)
//					MLQ[1].push_back(t);
//				else if (t.queuePriority == 3)
//					MLQ[2].push_back(t);
//			}
//		}
//
//		// CPU side context switch
//		// current process finishes CPU burst
//		newProcessLoaded = false;
//		if (!CPUidle && onCPU.remainCPUBurst == 0){
//			if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // still more I/O to do
//			else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
//			CPUidle = true; // set CPU to idle
//			// depending on which queue the process that has just been kicked off CPU is on, update its quantum
//			if (onCPU.queuePriority == 1){currQ1 = q1;}
//			else if (onCPU.queuePriority == 2){currQ2 = q2;}
//
//			if (MLQ[0].empty() && MLQ[1].empty() && MLQ[2].empty() && ioQ.empty()) {break;} // END POINT!!!
//		}
//		// current process hasn't finished its CPU burst but used up its quantum, ONLY for process in queue 1 and queue 2
//		if (!CPUidle && ((onCPU.queuePriority == 1 && currQ1 == 0) || (onCPU.queuePriority == 2 && currQ2 == 0))){
//			onCPU.arrival = sysTime; // reset arrival time
//			onCPU.queuePriority++; // downgrade queue priority
//			if (onCPU.queuePriority == 2){
//				MLQ[1].push_back(onCPU); // downgrade to queue 2 (note that queue2 is MLQ[1])
//				currQ1 = q1; // reset queue1 quantum
//			}
//			else if (onCPU.queuePriority == 3){
//				MLQ[2].push_back(onCPU); // downgrade to queue 3 (note that queue3 is MLQ[2])
//				currQ2 = q2; // reset queue2 quantum
//			}
//			CPUidle = true; // set CPU to idle
//		}
//		// current process hasn't finished its CPU burst or quantum, but the higher priority queue has process ready to preempt
//		// the current process
//		if (!CPUidle){
//			// process at top of queue1 is ready to get CPU and the current process on CPU has lower queue priority
//			if (!MLQ[0].empty() && MLQ[0].begin()->arrival == sysTime && onCPU.queuePriority > 1){
//				onCPU.arrival = sysTime;
//				if (onCPU.queuePriority == 2){
//					MLQ[1].push_back(onCPU); // preempted, but NOT downgraded. Still in queue2
//					currQ2 = q2; // reset queue2 quantum
//				}
//				else if (onCPU.queuePriority == 3){
//					MLQ[2].push_back(onCPU); // preempted, but NOT downgraded. Still in queue3
//				}
//				CPUidle = true; // set CPU to idle
//			}
//			// process at top of queue2 is ready to get CPU and the current process on CPU has lower queue priority
//			else if (!MLQ[1].empty() && MLQ[1].begin()->arrival == sysTime && onCPU.queuePriority > 2){
//				onCPU.arrival = sysTime;
//				MLQ[2].push_back(onCPU); // preempted, but NOT downgraded. Still in queue3
//				CPUidle = true; // set CPU to idle
//			}
//		}
//		// CPU take on new process, only when it is idle, and in the order of queue priority
//		if (CPUidle){
//			if (!MLQ[0].empty() && MLQ[0].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[0]); // check same arrival time situation
//				popOnCPU(MLQ[0], onCPU, CPUidle, newProcessLoaded);
//			}
//			else if (!MLQ[1].empty() && MLQ[1].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[1]); // check same arrival time situation
//				popOnCPU(MLQ[1], onCPU, CPUidle, newProcessLoaded);
//			}
//			else if (!MLQ[2].empty() && MLQ[2].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[2]); // check same arrival time situation
//				popOnCPU(MLQ[2], onCPU, CPUidle, newProcessLoaded);
//			}
//		}
//
//		// print out waitQ, ioQ, and CPU information at each instance when a new process gets CPU
//		// also provide information to generate Gantt Chart
//		if (newProcessLoaded)
//			{printWhenNewPricessLoaded_MLFQ(sysTime, MLQ, ioQ, complete, onCPU, gantt);}
//
//		// Then update key parameters at each time tick
//		// CPU processing
//		if (!CPUidle){
//			onCPU.remainCPUBurst--;
//			onCPU.turnaroundTime++;
//			if (onCPU.queuePriority == 1)
//				currQ1--;
//			else if (onCPU.queuePriority == 2)
//				currQ2--;
//		}
//		// MLQ updates
//		for (auto& wq : MLQ){
//			for (auto& p : wq){
//				if (!p.serviced) // increase RT for the processes that haven't been serviced yet
//					p.responseTime++;
//				p.waitTime++;
//				p.turnaroundTime++;
//			}
//		}
//		// ioQ updates
//		for (auto& p : ioQ){
//			p.remainIOBurst--;
//			p.turnaroundTime++;
//		}
//		sysTime++;
//	}
//	return sysTime;
//}



int main() {
	std::vector<Process> processList;
	std::vector<Process> waitQ;
	std::vector<Process> ioQ;
	std::vector<Process> complete;
	Process onCPU;
	Gantt gantt;
	std::vector<std::vector<Process> > MLQ; // multilevel queues
	for (int i = 0; i < 3; i++) // add three subqueues to MLQ
		MLQ.emplace_back(std::vector<Process>());

	// Testing Data
//	int info[9][30] = {{4, 27, 3, 31, 2, 43, 4, 18, 4, 22, 4, 26, 3, 24, 4},
//						{16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4},
//						{8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6},
//						{3, 35, 4, 41, 4, 45, 3, 51, 4, 61, 3, 54, 6, 82, 5, 77, 3},
//						{4, 48, 5, 44, 7, 42, 12, 37, 9, 46, 4, 41, 9, 31, 7, 43, 8},
//						{11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8},
//						{14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10},
//						{4, 14, 5, 33, 6, 51, 14, 63, 16, 87, 6, 74, 7},
//						{3, 37, 9, 41, 8, 30, 4, 29, 7, 33, 5, 22, 4, 24, 5, 29, 16}};

//	int info[5][30] = {{5,6,7},{4,2,3},{2,3,4},{5,2,7},{3,2,4}};

//	int info[5][30] = {{8},{6},{12},{4},{6}};

	int info[5][30] = {{10},{11},{12},{8},{5}};

	// initialize all processes, put them all in waitQ in the order of their number.
	for (int i = 0; i < 5; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}

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

	processList[0].arrival = 1;
	processList[1].arrival = 6;
	processList[2].arrival = 8;
	processList[3].arrival = 12;
	processList[4].arrival = 14;

//	bool hasTimeLimit = true;
	bool hasTimeLimit = false;
	int timeLimit = 150;
	int sysTime = 0; // initial system time

	// FCFS
//	FCFS(sysTime, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

	// RR
	RR(5, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

	// MLFQ
//	int totalTime = MLFQ(4, 9, processList, MLQ, ioQ, complete, onCPU, gantt);

	if (!hasTimeLimit) // if system let to finish completely, Gantt Chart needs the final system time.
		gantt.times.push_back(sysTime);

	printGanttChart(gantt);
	printRT_WT_TT(waitQ, ioQ, complete, onCPU, hasTimeLimit);

	return 0;
}
