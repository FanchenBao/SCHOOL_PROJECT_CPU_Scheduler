//============================================================================
// Name        : CPU_Scheduler.cpp
// Author      : Fanchen
// Date		   : 02/10/2019
// Description : CPU scheduler assignment for COP4610
//============================================================================

#include "scheduler_algorithm.h"

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
//	FCFS(sysTime, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

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
//	printRT_WT_TT(waitQ, ioQ, complete, onCPU, hasTimeLimit);
	printRT_WT_TT(MLQ, ioQ, complete, onCPU, hasTimeLimit); // for MLFQ

	return 0;
}
