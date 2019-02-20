/*
 * core_functions.h
 *
 *  Created on:		Feb 14, 2019
 *  Author: 		Fanchen Bao
 *  Description:	Core functions to perform CPU scheduling simulation
 */

#ifndef CORE_FUNCTIONS_H_
#define CORE_FUNCTIONS_H_

#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm> // make_heap(), push_heap()

struct Process{
	int number; // process number, e.g. the 1 in P1, 2 in P2, etc.
	std::vector<int> processTime; // CPU and I/O burst time, all in one vector
	int index; // index pointing to current CPU or I/O element of processTime
	int ptSize; // size of processTime
	int arrival; // arrival time, default to 0
	int priority; // process's own priority. default to 1 (smaller the priority value, the higher the priority)
	int remainCPUBurst; // remaining CPU burst time, default to processTime[0].
	int remainIOBurst; // remaining I/O burst time, default to 0
	int totalCPUBurst; // total CPU burst so far, tallied at the end of a CPU burst
	int totalIOBurst; // total I/O burst so far, tallied at the end of a I/O burst
	int queuePriority; // for MLFQ only, default to 1
	int responseTime;  // RT = process first gets CPU - initial arrival time
	int waitTime; // WT += time when process gets CPU - current arrival time
	int turnaroundTime; // TT = waitTime + totalCPUBurst + totalIOBurst (note that the wait time and I/O time that happen before a process completes its current CPU burst do NOT count towards TT

	Process(int n, int* pt, int r, int p = 1); // constructor
	Process();// constructor, for onCPU only
	void reset(); // reset all parameters
};


struct CompareIO{ // for heapify ioQ
    bool operator()(const Process &a, const Process &b) const{
        return a.remainIOBurst > b.remainIOBurst;
    }
};

struct ComparePNumber{ // for sorting based on process number
    bool operator()(const Process &a, const Process &b) const{
        return a.number < b.number;
    }
};

struct CompareArrival{ // for sorting based on arrival time
    bool operator()(const Process &a, const Process &b) const{
        return a.arrival < b.arrival;
    }
};

struct ComparePriority{ // for sorting based on arrival time
    bool operator()(const Process &a, const Process &b) const{
        return a.priority < b.priority;
    }
};

struct CompareRemainCPUBurst{ // for sorting based on remaining CPU burst
    bool operator()(const Process &a, const Process &b) const{
        return a.remainCPUBurst < b.remainCPUBurst;
    }
};

// CPU and I/O actions
void admitProcess(int sysTime, std::vector<Process>& processList, std::vector<Process>& readyQ); // admit new process based on its arrival time
void prioritizeReadyQ(int sysTime, int priorityType, std::vector<Process>& readyQ);
void popOffIO(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ);
void popOffCPU(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch);
void pushToIO(std::vector<Process>& ioQ, Process& onCPU);
void pushToCPU(int sysTime, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool hasTimeLimit, int timeLimit);

// for MLFQ, these functions are overloaded
void popOffIO(int sysTime, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ);
void popOffCPU(int sysTime, std::vector<int>& currQ, const std::vector<int>& quantums, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, int reasonForSwitch);


#endif /* CORE_FUNCTIONS_H_ */
