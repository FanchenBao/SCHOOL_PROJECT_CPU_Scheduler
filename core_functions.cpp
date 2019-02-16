/*
 * core_functions.cpp
 *
 *  Created on: Feb 14, 2019
 *      Author: fanchen
 */
#include "core_functions.h"
#include "utility.h"

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

void CPUContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch){
	// Perform CPU context switch and push the process to I/O or completion.
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
	case 1:
		if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // push current process from CPU to I/O
		else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
		break;
	case 2:
		onCPU.arrival = sysTime; // reset arrival time
		waitQ.push_back(onCPU); // pushed to the back of waitQ
		break;
	default:
		std::cerr << "Fatal Error: Check reason for CPU context switch." << std::endl;
		std::exit(1);
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











