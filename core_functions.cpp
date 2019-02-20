/*
 * core_functions.cpp
 *
 *  Created on: Feb 14, 2019
 *      Author: fanchen
 */
#include "core_functions.h"
#include "utility.h"

// constructor
Process::Process(int n, int* pt, int r, int p) : number(n), index(0), arrival(0), priority(p), remainCPUBurst(r), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0){
	int i = 0;
	while (pt[i])
		processTime.push_back(pt[i++]);
	ptSize = i;
}

// constructor, for CPU only
Process::Process(): number(0), index(0), ptSize(0), arrival(0), priority(1), remainCPUBurst(0), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0){}

// reset all parameters in the struct
void Process::reset(){
	number = 0;
	processTime.clear();
	index = 0;
	ptSize = 0;
	arrival = 0;
	priority = 1;
	remainCPUBurst = 0;
	remainIOBurst = 0;
	totalCPUBurst = 0;
	totalIOBurst = 0;
	queuePriority = 1;
	responseTime = 0;
	waitTime = 0;
	turnaroundTime = 0;
}

// CPU and I/O actions
void admitProcess(int sysTime, std::vector<Process>& processList, std::vector<Process>& readyQ){
	// admit new process based on its arrival time
	for (auto it = processList.begin(); it != processList.end();){ // only load processes whose arrival time is the same as sysTime onto readyQ
		if (it->arrival == sysTime){
			readyQ.push_back(*it); // initially all processes is in queue 1
			it = processList.erase(it);
		}
		else {return;}
	}
}

void prioritizeReadyQ(int sysTime, int priorityType, std::vector<Process>& readyQ){
	// reorder readyQ if necessary, based on priorityType
	// priorityType = 1 (arrival time (FCFS, RR)), 2 (CPU burst (SJF, SRF)), 3 (arbitrary priority)
	switch (priorityType){
		case 1: {// priority is arrival time. No major reordering needed, except when multiple processes have same arrival time at the top
			if (!readyQ.empty() && readyQ.begin()->arrival <= sysTime)
				handleSamePriorityInReadyQ(readyQ, 1, readyQ.size()); // check same arrival time situation. Comment this line if one does not want to pick the smallest process number to go first if multiple arrival times are the same
			break;
		}
		case 2: {// priority is the remaining CPU burst
			if (!readyQ.empty() && readyQ.begin()->arrival <= sysTime){
				auto it = readyQ.begin();
				for (; it != readyQ.end(); it++) // reorder based on CPU burst for all those processes eligible for CPU now
					if (it->arrival > sysTime) {break;}
				std::sort(readyQ.begin(), it, CompareRemainCPUBurst()); // sort these eligible processes based on remaining CPU burst
				handleSamePriorityInReadyQ(readyQ, 2, readyQ.size()); // for processes at the beginning having same CPU burst, reorder based on process number
			}
			break;
		}
		case 3:{ // priority is from an arbitrarily given value
			if (!readyQ.empty() && readyQ.begin()->arrival <= sysTime){
				auto it = readyQ.begin();
				for (; it != readyQ.end(); it++) // reorder based on CPU burst for all those processes eligible for CPU now
					if (it->arrival > sysTime) {break;}
				std::sort(readyQ.begin(), it, ComparePriority()); // sort these eligible processes based on remaining CPU burst
				handleSamePriorityInReadyQ(readyQ, 2, readyQ.size()); // for processes at the beginning having same CPU burst, reorder based on process number
			}
			break;
		}
		default:{
			std::cerr << "Fatal Error: Check priorityType for process admission." << std::endl;
			std::exit(1);
		}
	}
}

void popOffIO(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ){
	// I/O context switch and push all processes that finish I/O to ready queue
	std::vector<Process> targets;
	handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
	for (auto& t : targets){
		t.remainCPUBurst = t.processTime[++(t.index)]; // move index to CPU element of processTime and update remainCPUBurst
		t.arrival = sysTime; // update arrival time
	}

	readyQ.insert(readyQ.end(), targets.begin(), targets.end()); // push process on readyQ
}

void popOffIO(int sysTime, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ){
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

void popOffCPU(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch){
	// Perform CPU context switch and push the process to I/O, completion, or some readyQ.
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
		if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // push current process from CPU to I/O
		else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
		break;
	case 2: // quantum used up
		onCPU.arrival = sysTime; // reset arrival time
		readyQ.push_back(onCPU); // pushed to the back of readyQ
		break;
	default:
		std::cerr << "Fatal Error: Check reason for CPU context switch." << std::endl;
		std::exit(1);
	}
	CPUidle = true; // set CPU to idle
}

void popOffCPU(int sysTime, std::vector<int>& currQ, const std::vector<int>& quantums, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch){
	// Perform CPU context switch and push the process to I/O, completion, or some readyQ.
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
	case 3: // preempted by higher priority process
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
	// pop a process off CPU and onto I/O queue
	onCPU.remainIOBurst = onCPU.processTime[++(onCPU.index)]; // move index to I/O element of processTime and update remainIOBurst
	ioQ.push_back(onCPU);
	std::push_heap(ioQ.begin(), ioQ.end(), CompareIO());
}

void pushToCPU(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool hasTimeLimit, int timeLimit){
	// push new process to CPU
	onCPU = *readyQ.begin();
	if (onCPU.totalCPUBurst == 0) // process is serviced for the first time
		onCPU.responseTime = sysTime - onCPU.arrival;
	if (!hasTimeLimit || sysTime < timeLimit) // update wait time (this is to make sure the process loaded onto CPU at the point of timeLimit does not have its last wait time counted, because this process is considered DONE before time limit)
		onCPU.waitTime += sysTime - onCPU.arrival;
	readyQ.erase(readyQ.begin());
	CPUidle = false;
}











