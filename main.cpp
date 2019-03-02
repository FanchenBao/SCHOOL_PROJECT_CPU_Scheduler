//============================================================================
// Name        : CPU_Scheduler.cpp
// Author      : Fanchen
// Date		   : 02/10/2019
// Description : CPU scheduler assignment for COP4610
//============================================================================

#include "scheduler_algorithm.h"
#include "test.h"
#include <iomanip>



int main() {
	std::cout << std::fixed << std::setprecision(2); // output rule

	//test_9processes(); // testing for existing scheduler algorithms

	std::vector<Process> processList;
	std::vector<Process> readyQ;
	std::vector<Process> ioQ;
	std::vector<Process> complete;
	Process onCPU;
	Gantt gantt;
	std::vector<std::vector<Process> > MLQ; // multilevel queues

	// system parameters
	int sysTime = 0; // initial system time
	int sysIdle = 0; // total idle time in system
	bool CPUidle = true; // flag for whether CPU is busy or idle


	// input parameters
	int numProcess = 9; // total number of processes
//	bool hasTimeLimit = true;
	bool hasTimeLimit = false;
	int timeLimit = 150;

	loadData_9processes(processList, 1); // fast way to load the 9-process data set. 1 = (no arbitrary priority), 2 = (arbitrary priority)

	// 5-process data sets
//	int info[5][30] = {{5,6,7},{4,2,3},{2,3,4},{5,2,7},{3,2,4}};

//	int info[5][30] = {{8},{6},{12},{4},{6}};

//	int info[5][30] = {{10},{11},{12},{8},{5}};

//	int info[5][30] = {{10},{1},{2},{1},{5}};

	// initialize all processes, put them all in readyQ in the order of their number.
//	for (int i = 0; i < numProcess; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}

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

//	processList[0].priority = 3;
//	processList[1].priority = 2;
//	processList[2].priority = 1;
//	processList[3].priority = 3;
//	processList[4].priority = 2;


	// Running each scheduler algorithms
	// FCFS
	//FCFS(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// RR
//	RR(sysTime, sysIdle, CPUidle, 5, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// MLFQ
	int numSubQ = 3;
	for (int i = 0; i < numSubQ; i++) // add subqueues to MLQ
		MLQ.emplace_back(std::vector<Process>());
	std::vector<int> quantums = {4, 9, -1}; // all non-RR queues default to -1 quantum (set to -1 such that non-RR queue quantum would never reach 0 to trigger a quantum drying up event)
	MLFQ(sysTime, sysIdle, CPUidle, quantums, processList, MLQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// SJF
//	SJF(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// priority non-preemptive
//	priorityNonPreemptive(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);




	// print out final results
	printGanttChart(gantt);
	std::cout << "\nTotal number of CPU context switches: " << gantt.numCPUContextSwitch << std::endl;
//	printRT_WT_TT(readyQ, ioQ, complete, onCPU, numProcess, hasTimeLimit);
	printRT_WT_TT(MLQ, ioQ, complete, onCPU, numProcess, hasTimeLimit); // for MLFQ

	std::cout << "\nTotal Time:\t\t" << sysTime << std::endl;
	std::cout << "Idle Time:\t\t" << sysIdle << std::endl;
	std::cout << "CPU Utilization:\t" << static_cast<double>(sysTime - sysIdle) / sysTime * 100 << "%" << std::endl;

	return 0;
}
