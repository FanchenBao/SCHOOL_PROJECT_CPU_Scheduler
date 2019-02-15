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
	// I/O context switch and push all processes that finish I/O to wait queue
	std::vector<Process> targets;
	handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
	for (auto& t : targets){
		t.remainCPUBurst = t.processTime[++(t.index)]; // move index to CPU element of processTime and update remainCPUBurst
		t.arrival = sysTime; // update arrival time
	}

	waitQ.insert(waitQ.end(), targets.begin(), targets.end()); // push process on waitQ
}

void IOContextSwitch(int sysTime, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ){
	// I/O context switch and push all processes that finish I/O to wait queue
	// Overloaded for MLFQ
	std::vector<Process> targets;
	handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
	for (auto& t : targets){
		t.remainCPUBurst = t.processTime[++(t.index)]; // move index to CPU element of processTime and update remainCPUBurst
		t.arrival = sysTime; // update arrival time
		MLQ[t.queuePriority - 1].push_back(t); // push process to the waiting queue corresponding to their queue priority
	}
}

void CPUContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool exceedQuant){
	// Perform CPU context switch and push the process to I/O or completion.
	// exceedQuant determines whether the context switch is initiated by reaching quantum under RR
	onCPU.totalCPUBurst += onCPU.processTime[onCPU.index] - onCPU.remainCPUBurst; // update totalCPUBurst
	onCPU.processTime[onCPU.index] = onCPU.remainCPUBurst; // update CPU burst info in processTime array
	if (onCPU.index > 0){
		onCPU.totalIOBurst += onCPU.processTime[onCPU.index - 1]; // update totalIOBurst
		onCPU.processTime[onCPU.index - 1] = 0; // once an I/O burst is counted for, update info in processTime array
	}
	// update turnaround time. TT = wait time + CPU burst (just finished) + I/O time right before
	onCPU.turnaroundTime = onCPU.waitTime + onCPU.totalCPUBurst + onCPU.totalIOBurst;

	if (exceedQuant){ // context switch due to quantum dried up.
		onCPU.arrival = sysTime; // reset arrival time
		waitQ.push_back(onCPU); // pushed to the back of waitQ
	}
	else{ // context switch due to complete previous burst
		// still more I/O to do
		if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // push current process from CPU to I/O
		else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
	}
	CPUidle = true; // set CPU to idle
}

void CPUContextSwitch(int sysTime, std::vector<int>& currQ, const std::vector<int>& quantums, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch){
	// Perform CPU context switch and push the process to I/O or completion.
	// Used specifically for MLFQ
	// reasonForSwitch = 1 (current CPU burst completes), 2 (current quantum dried up), 3 (preempted by higher priority process)
	onCPU.totalCPUBurst += onCPU.processTime[onCPU.index] - onCPU.remainCPUBurst; // update totalCPUBurst
	onCPU.processTime[onCPU.index] = onCPU.remainCPUBurst; // update CPU burst info in processTime array
	if (onCPU.index > 0){
		onCPU.totalIOBurst += onCPU.processTime[onCPU.index - 1]; // update totalIOBurst
		onCPU.processTime[onCPU.index - 1] = 0; // once an I/O burst is counted for, update info in processTime array
	}
	// update turnaround time. TT = wait time + CPU burst (just finished) + I/O time right before
	onCPU.turnaroundTime = onCPU.waitTime + onCPU.totalCPUBurst + onCPU.totalIOBurst;

	switch (reasonForSwitch){
	case 1: // context switch due to complete previous burst
		if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // still more I/O to do, push current process from CPU to I/O
		else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
		currQ[onCPU.queuePriority - 1] = quantums[onCPU.queuePriority - 1]; // reset quantum for the queue whose process just gets kicked off
		break;
	case 2: // quantum used up, current process is down-graded.
		onCPU.arrival = sysTime;
		currQ[onCPU.queuePriority - 1] = quantums[onCPU.queuePriority - 1]; // reset quantum for the queue whose process just gets kicked off (before down-grade happens)
		onCPU.queuePriority++; // downgrade queue priority
		MLQ[onCPU.queuePriority - 1].push_back(onCPU); // push process to its down-graded queue
		break;
	case 3:
		onCPU.arrival = sysTime;
		MLQ[onCPU.queuePriority - 1].push_back(onCPU); // push process to its original queue
		currQ[onCPU.queuePriority - 1] = quantums[onCPU.queuePriority - 1]; // reset quantum for the queue whose process just gets kicked off
		break;
	default:
		std::cerr << "Fatal Error: Check reason for CPU context switch." << std::endl;
		std::exit(1);
	}
	CPUidle = true; // set CPU to idle
}



void pushToIO(std::vector<Process>& ioQ, Process& onCPU){
	// kick a process off CPU and onto I/O queue
	onCPU.remainIOBurst = onCPU.processTime[++(onCPU.index)]; // move index to I/O element of processTime and update remainIOBurst
	ioQ.push_back(onCPU);
	std::push_heap(ioQ.begin(), ioQ.end(), CompareIO());
}

void pushToCPU(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool hasTimeLimit, int timeLimit){
	// push new process to CPU
	handleSameArrivalTimeInWaitQ(waitQ); // check same arrival time situation. Comment this line if one does not want to pick the smallest process number to go first if multiple arrival times are the same
	onCPU = *waitQ.begin();
	if (onCPU.totalCPUBurst == 0) // process is serviced for the first time
		onCPU.responseTime = sysTime - onCPU.arrival;
	if (!hasTimeLimit || sysTime < timeLimit) // update wait time (this is to make sure the process loaded onto CPU at the point of timeLimit does not have its last wait time counted, because this process is considered DONE before time limit)
		onCPU.waitTime += sysTime - onCPU.arrival;
	waitQ.erase(waitQ.begin());
	CPUidle = false;
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

void printRT_WT_TT(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit){
	// print out RT, WT, and TT for each process
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (hasTimeLimit){ // scheduling exits prematurely, processes are scattered in different places.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		for (auto& subQ : MLQ)
			forPrint.insert(forPrint.end(), subQ.begin(), subQ.end());
		forPrint.push_back(onCPU);
	}
	std::sort(forPrint.begin(), forPrint.end(), ComparePNumber());
	std::cout << "Process\t" << "RT\t" << "WT\t" << "TT\n";
	for (auto p : forPrint){
		std::cout << "P" << p.number << "\t"
				<< p.responseTime << "\t"
				<< p.waitTime << "\t"
				<< p.turnaroundTime << "\t"
				<< p.totalCPUBurst << "(CPU) + " << p.waitTime << "(WT) + " << p.totalIOBurst << "(I/O)" << std::endl;
	}
}

void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU){
	// print information of each queue when a new process is just loaded onto CPU.
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

void printWhenNewPricessLoaded(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU){
	// print information of each queue when a new process is just loaded onto CPU. Overloaded for MLFQ
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\t\tQueue\n";
	bool allSubQEmpty = true;
	for (auto subQ : MLQ){
		if (!subQ.empty()){
			allSubQEmpty = false;
			for (auto p : subQ)
				std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
		}
	}
	if (allSubQEmpty)
		std::cout << "\t[empty]\n";
//	else{
//		for (auto p : MLQ[0])
//			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
//		for (auto p : MLQ[1])
//			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
//		for (auto p : MLQ[2])
//			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
//	}
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




// Miscellaneous
void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt, bool CPUidle){
	// Gather info to print Gantt Chart
	gantt.times.push_back(sysTime);
	if (!CPUidle){ // a new process is currently on CPU
		gantt.processes.push_back("P" + std::to_string(onCPU.number));
		gantt.preIdle = false;
	}
	else{ // CPU in idle
		gantt.processes.push_back("IDLE");
		gantt.preIdle = true;
	}
}

