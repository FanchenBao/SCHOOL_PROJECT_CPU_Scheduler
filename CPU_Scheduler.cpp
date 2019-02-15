/*
 * CPU_Scheduler.cpp
 *
 *  Created on: Feb 14, 2019
 *      Author: fanchen
 */

#include "CPU_Scheduler.h"

// constructor
Process::Process(int n, int* pt, int r) : number(n), index(0), arrival(0), remainCPUBurst(r), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0){
	int i = 0;
	while (pt[i])
		processTime.push_back(pt[i++]);
	ptSize = i;
}

// constructor, for CPU only
Process::Process(): number(0), index(0), ptSize(0), arrival(0), remainCPUBurst(0), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0){}



// CPU and I/O actions
void admitProcess(int sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ){
	// admit new process based on its arrival time
	for (auto it = processList.begin(); it != processList.end();){ // only load processes whose arrival time is the same as sysTime onto waitQ
		if (it->arrival == sysTime){
			waitQ.push_back(*it); // initially all processes is in queue 1
			it = processList.erase(it);
		}
		else {return;}
	}
}

void IOContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ){
	// Determine whether I/O context switch is needed.
	// If needed, do the context switch and push all processes that finish I/O to wait queue
	if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0){ // some process finishes I/O, needs to get back to waitQ
		std::vector<Process> targets;
		handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
		waitQ.insert(waitQ.end(), targets.begin(), targets.end()); // push process on waitQ
	}
}

void CPUContextSwitch(std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle){
	// Determine whether CPU context switch is needed.
	// If needed, do the context switch and push the process to I/O
	if (!CPUidle && onCPU.remainCPUBurst == 0){ // current process finishes CPU burst

		onCPU.totalCPUBurst += onCPU.processTime[onCPU.index]; // update totalCPUBurst
		if (onCPU.index > 0)
			onCPU.totalIOBurst += onCPU.processTime[onCPU.index - 1]; // update totalIOBurst
		// update turnaround time. TT = wait time + CPU burst (just finished) + I/O time right before
		onCPU.turnaroundTime = onCPU.waitTime + onCPU.totalCPUBurst + onCPU.totalIOBurst;

		// still more I/O to do
		if (onCPU.index < onCPU.ptSize - 2)
			pushToIO(ioQ, onCPU); // push current process from CPU to I/O
		else
			complete.push_back(onCPU); // no more I/O, i.e. all bursts have been completed.
		CPUidle = true; // set CPU to idle
	}
}

void pushToIO(std::vector<Process>& ioQ, Process& onCPU){
	// kick a process off CPU and onto I/O queue
	onCPU.remainIOBurst = onCPU.processTime[++(onCPU.index)]; // move index to I/O element of processTime and update remainIOBurst
	ioQ.push_back(onCPU);
	std::push_heap(ioQ.begin(), ioQ.end(), CompareIO());
}

void pushToCPU(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, Gantt& gantt, bool hasTimeLimit, int timeLimit){
	// Determine whether the CPU and wait queue are both ready to push a new process to CPU
	// take on new process only when CPU is idle and there is process ready in the waitQ
	if (CPUidle && !waitQ.empty() && waitQ.begin()->arrival <= sysTime){
		handleSameArrivalTimeInWaitQ(waitQ); // check same arrival time situation
		onCPU = *waitQ.begin();
		if (onCPU.totalCPUBurst == 0) // process is serviced for the first time
			onCPU.responseTime = sysTime - onCPU.arrival;
		if (!hasTimeLimit || sysTime < timeLimit) // update wait time (this is to make sure the process loaded onto CPU at the point of timeLimit does not have its last wait time counted, because this process is considered DONE before time limit)
			onCPU.waitTime += sysTime - onCPU.arrival;
		waitQ.erase(waitQ.begin());
		CPUidle = false;

		// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
		printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU);
		updateGanttChart(sysTime, onCPU, gantt);
	}
}




// handle multiple ready process in waitQ and ioQ
void handleSameArrivalTimeInWaitQ(std::vector<Process>& waitQ){
	// the top of waitQ should be pushed onto CPU, BUT if there are multiple processes with the
	// same arrival time and all ready to go to CPU, only the one with the smallest process number
	// gets CPU first.
	auto it = ++(waitQ.begin());
	for (; it != waitQ.end(); it++){ // check whether more than one processes are eligible for CPU
		if (it->arrival != waitQ.begin()->arrival) {break;}
	}
	std::sort(waitQ.begin(), it, ComparePNumber()); // sort these eligible processes based on process number
}

void handleSameFinishTimeInIOQ(int sysTime, std::vector<Process>& ioQ, std::vector<Process>& targets){
	// find all processes that finish I/O and pop all of them off
	for (auto it = ioQ.begin(); it != ioQ.end();){
		if (it->remainIOBurst == 0){
			targets.push_back(*it);
			it = ioQ.erase(it);
		}
		else
			it++;
	}
	std::make_heap(ioQ.begin(), ioQ.end(), CompareIO());
	std::sort(targets.begin(), targets.end(), ComparePNumber()); // order according to process number (ascending)
	for (auto& t : targets){
		t.remainCPUBurst = t.processTime[++(t.index)]; // move index to CPU element of processTime and update remainCPUBurst
		t.arrival = sysTime; // update arrival time
	}
}




//output
void printGanttChart(const Gantt& gantt){
	// print out Gantt Chart
	for (auto p : gantt.processes)
		std::cout << "|" << p << "\t\t";
	std::cout <<"|\n";
	for (auto t : gantt.times)
		std::cout << t << "\t\t";
	std::cout <<"\n";
}

void printRT_WT_TT(const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit){
	// print out RT, WT, and TT for each process
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (hasTimeLimit){ // scheduling exits prematurely, processes are scattered in different places.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		forPrint.insert(forPrint.end(), waitQ.begin(), waitQ.end());
		forPrint.push_back(onCPU);
	}
	std::sort(forPrint.begin(), forPrint.end(), ComparePNumber());
	std::cout << "Process\t" << "RT\t" << "WT\t" << "TT\t" << "TT Breakdown\n";
	for (auto p : forPrint){
		std::cout << "P" << p.number << "\t"
				<< p.responseTime << "\t"
				<< p.waitTime << "\t"
				<< p.turnaroundTime << "\t"
				<< p.totalCPUBurst << "(CPU) + " << p.waitTime << "(WT) + " << p.totalIOBurst << "(I/O)" << std::endl;
	}
}

void printRT_WT_TT_MLFQ(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool printAtEnd){
	// print out RT, WT, and TT for each process
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (!printAtEnd){ // if data are to be printed mid way through the scheduling, processes are scattered in all queues.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		for (int i = 0; i < 3; i++)
			forPrint.insert(forPrint.end(), MLQ[i].begin(), MLQ[i].end());
		forPrint.push_back(onCPU);
	}
	std::sort(forPrint.begin(), forPrint.end(), ComparePNumber());
	std::cout << "Process\t" << "RT\t" << "WT\t" << "TT\n";
	for (auto p : forPrint){
		std::cout << "P" << p.number << "\t" << p.responseTime << "\t" << p.waitTime << "\t" << p.turnaroundTime << std::endl;
	}
}

void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU){
	// print information of each queue when a new process is just loaded onto CPU. Also gethering information
	// for Gantt Chart
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\n";
	if (waitQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : waitQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in I/O:\n";
	std::cout << "\tProcess\t\tRemaining I/O Time\n";
	if (ioQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : ioQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainIOBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<"*";
	std::cout << "\n\n\n";
}

void printWhenNewPricessLoaded_MLFQ(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, Gantt& gantt){
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\t\tQueue\n";
	if (MLQ[0].empty() && MLQ[1].empty() && MLQ[2].empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : MLQ[0])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
		for (auto p : MLQ[1])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
		for (auto p : MLQ[2])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in I/O:\n";
	std::cout << "\tProcess\t\tRemaining I/O Time\n";
	if (ioQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : ioQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainIOBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<"*";
	std::cout << "\n\n\n";

	// for Gantt Chart
	gantt.times.push_back(sysTime);
	gantt.processes.push_back("P" + std::to_string(onCPU.number));
}




// Miscellaneous
void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt){
	// Gather info to print Gantt Chart
	gantt.times.push_back(sysTime);
	gantt.processes.push_back("P" + std::to_string(onCPU.number));
}

