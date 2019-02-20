//============================================================================
// Name        : CPU_Scheduler.cpp
// Author      : Fanchen
// Date		   : 02/10/2019
// Description : CPU scheduler assignment for COP4610
//============================================================================

#include "scheduler_algorithm.h"
#include <iomanip>

void reset(std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, std::vector<std::vector<Process> >& MLQ, Process& onCPU, Gantt& gantt, int& sysTime, int& sysIdle, bool& CPUidle){
	processList.clear();
	readyQ.clear();
	ioQ.clear();
	complete.clear();
	MLQ.clear();
	onCPU.reset();
	gantt.reset();
	sysTime = 0;
	sysIdle = 0;
	CPUidle = true;
}

void loadData_9processes(std::vector<Process>& processList, int scheduler){
	// scheduler = 1 (no arbitrary priority), 2 = (arbitrary priority)
	// Testing Data
	int info[9][30] = {{4, 27, 3, 31, 2, 43, 4, 18, 4, 22, 4, 26, 3, 24, 4},
						{16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4},
						{8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6},
						{3, 35, 4, 41, 4, 45, 3, 51, 4, 61, 3, 54, 6, 82, 5, 77, 3},
						{4, 48, 5, 44, 7, 42, 12, 37, 9, 46, 4, 41, 9, 31, 7, 43, 8},
						{11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8},
						{14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10},
						{4, 14, 5, 33, 6, 51, 14, 63, 16, 87, 6, 74, 7},
						{3, 37, 9, 41, 8, 30, 4, 29, 7, 33, 5, 22, 4, 24, 5, 29, 16}};

	int arbitraryPriority[] = {1,2,3,4,5,6,7,8,9};
	// initialize all processes, put them all in readyQ in the order of their number.
	switch (scheduler){
	case 1:
		for (int i = 0; i < 9; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}
		break;
	case 2:
		for (int i = 0; i < 9; i++) {processList.emplace_back(i+1, info[i], info[i][0], arbitraryPriority[i]);}
		break;
	}
}


void test_9processes(){
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
	bool hasTimeLimit = false;
	int timeLimit = 0;

	// FCFS
	bool success = true;
	loadData_9processes(processList, 1);
	FCFS(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit, false);
	// testing
	std::sort(complete.begin(), complete.end(), ComparePNumber());
	std::vector<std::vector<int> > resFCFS = {{0,238,457},{4,264,573},{20,198,590},{28,209,690},{31,208,605},{35,270,485},{46,255,583},{60,217,597},{64,263,569}};
	for (int i = 0; i < numProcess; i++){
		if (!(complete[i].responseTime == resFCFS[i][0] && complete[i].waitTime == resFCFS[i][1] && complete[i].turnaroundTime == resFCFS[i][2]))
			{success = false; break;}
	}
	if (success)
		std::cout << "FCFS 9 processes test PASS" << std::endl;
	else
		std::cout << "FCFS 9 processes test FAIL" << std::endl;


	// RR
	success = true;
	reset(processList, readyQ, ioQ, complete, MLQ, onCPU, gantt, sysTime, sysIdle, CPUidle);
	loadData_9processes(processList, 1);
	RR(sysTime, sysIdle, CPUidle, 5, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit, false);
	// testing
	std::sort(complete.begin(), complete.end(), ComparePNumber());
	std::vector<std::vector<int> > resRR = {{0,96,315},{4,293,602},{9,259,651},{14,136,617},{17,201,598},{21,251,466},{26,294,622},{31,229,609},{35,224,530}};
	for (int i = 0; i < numProcess; i++){
		if (!(complete[i].responseTime == resRR[i][0] && complete[i].waitTime == resRR[i][1] && complete[i].turnaroundTime == resRR[i][2]))
			{success = false; break;}
	}
	if (success)
		std::cout << "RR 9 processes test PASS" << std::endl;
	else
		std::cout << "RR 9 processes test FAIL" << std::endl;


	// MLFQ
	success = true;
	reset(processList, readyQ, ioQ, complete, MLQ, onCPU, gantt, sysTime, sysIdle, CPUidle);
	loadData_9processes(processList, 1);
	int numSubQ = 3;
	for (int i = 0; i < numSubQ; i++) // add subqueues to MLQ
		MLQ.emplace_back(std::vector<Process>());
	std::vector<int> quantums = {4, 9, -1}; // all non-RR queues default to -1 quantum (set to -1 such that non-RR queue quantum would never reach 0 to trigger a quantum drying up event)
	MLFQ(sysTime, sysIdle, CPUidle, quantums, processList, MLQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit, false);
	// testing
	std::sort(complete.begin(), complete.end(), ComparePNumber());
	std::vector<std::vector<int> > resMLFQ = {{0,6,225},{4,326,635},{8,293,685},{12,12,493},{15,246,643},{19,228,443},{23,302,630},{27,217,597},{31,129,435}};
	for (int i = 0; i < numProcess; i++){
		if (!(complete[i].responseTime == resMLFQ[i][0] && complete[i].waitTime == resMLFQ[i][1] && complete[i].turnaroundTime == resMLFQ[i][2]))
			{success = false; break;}
	}
	if (success)
		std::cout << "MLFQ 9 processes test PASS" << std::endl;
	else
		std::cout << "MLFQ 9 processes test FAIL" << std::endl;


	// SJF
	success = true;
	reset(processList, readyQ, ioQ, complete, MLQ, onCPU, gantt, sysTime, sysIdle, CPUidle);
	loadData_9processes(processList, 1);
	SJF(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit, false);
	// testing
	std::sort(complete.begin(), complete.end(), ComparePNumber());
	std::vector<std::vector<int> > resSJF = {{6,58,277},{176,358,667},{18,325,717},{0,38,519},{10,88,485},{26,105,320},{58,263,591},{14,66,446},{3,79,385}};
	for (int i = 0; i < numProcess; i++){
		if (!(complete[i].responseTime == resSJF[i][0] && complete[i].waitTime == resSJF[i][1] && complete[i].turnaroundTime == resSJF[i][2]))
			{success = false; break;}
	}
	if (success)
		std::cout << "SJF 9 processes test PASS" << std::endl;
	else
		std::cout << "SJF 9 processes test FAIL" << std::endl;


	// SJF
	success = true;
	reset(processList, readyQ, ioQ, complete, MLQ, onCPU, gantt, sysTime, sysIdle, CPUidle);
	loadData_9processes(processList, 2);
	priorityNonPreemptive(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit, false);
	// testing
	std::sort(complete.begin(), complete.end(), ComparePNumber());
	std::vector<std::vector<int> > resPNP = {{0,21,240,},{4,70,379,},{20,81,473,},{28,112,593,},{34,153,550,},{38,246,461,},{103,214,542,},{308,405,785,},{312,422,728,}};
	for (int i = 0; i < numProcess; i++){
		if (!(complete[i].responseTime == resPNP[i][0] && complete[i].waitTime == resPNP[i][1] && complete[i].turnaroundTime == resPNP[i][2]))
			{success = false; break;}
	}
	if (success)
		std::cout << "priorityNonPreemptive 9 processes test PASS" << std::endl;
	else
		std::cout << "priorityNonPreemptive 9 processes test FAIL" << std::endl;


}

int main() {

	test_9processes();

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

	std::cout << std::fixed << std::setprecision(2); // output rule

	// input parameters
	int numProcess = 9; // total number of processes
//	bool hasTimeLimit = true;
	bool hasTimeLimit = false;
	int timeLimit = 150;

	loadData_9processes(processList, 2);

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


	// FCFS
//	FCFS(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// RR
//	RR(sysTime, sysIdle, CPUidle, 5, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// MLFQ
//	int numSubQ = 3;
//	for (int i = 0; i < numSubQ; i++) // add subqueues to MLQ
//		MLQ.emplace_back(std::vector<Process>());
//	std::vector<int> quantums = {4, 9, -1}; // all non-RR queues default to -1 quantum (set to -1 such that non-RR queue quantum would never reach 0 to trigger a quantum drying up event)
//	MLFQ(sysTime, sysIdle, CPUidle, quantums, processList, MLQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// SJF
//	SJF(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);

	// priority non-preemptive
//	priorityNonPreemptive(sysTime, sysIdle, CPUidle, processList, readyQ, ioQ, complete, onCPU, gantt, numProcess, hasTimeLimit, timeLimit);








	// print out final results
	printGanttChart(gantt);
	std::cout << "\nTotal number of CPU context switches: " << gantt.numCPUContextSwitch << std::endl;
	printRT_WT_TT(readyQ, ioQ, complete, onCPU, numProcess, hasTimeLimit);
//	printRT_WT_TT(MLQ, ioQ, complete, onCPU, numProcess, hasTimeLimit); // for MLFQ

	std::cout << "\nTotal Time:\t\t" << sysTime << std::endl;
	std::cout << "Idle Time:\t\t" << sysIdle << std::endl;
	std::cout << "CPU Utilization:\t" << static_cast<double>(sysTime - sysIdle) / sysTime << std::endl;

	return 0;
}
