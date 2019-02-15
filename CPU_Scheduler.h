/*
 * CPU_Scheduler.h
 *
 *  Created on:		Feb 14, 2019
 *  Author: 		Fanchen Bao
 *  Description:	Helper functions to perform CPU scheduling simulation
 */

#ifndef CPU_SCHEDULER_H_
#define CPU_SCHEDULER_H_

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
	int remainCPUBurst; // remaining CPU burst time, default to processTime[0].
	int remainIOBurst; // remaining I/O burst time, default to 0
	int totalCPUBurst; // total CPU burst so far, tallied at the end of a CPU burst
	int totalIOBurst; // total I/O burst so far, tallied at the end of a I/O burst
	int queuePriority; // for MLFQ only, default to 1
	int responseTime;  // RT = process first gets CPU - initial arrival time
	int waitTime; // WT = time spent in wait queue, increment each time unit when process is in wait queue
	int turnaroundTime; // TT = waitTime + totalCPUBurst + totalIOBurst (note that the wait time and I/O time that happen before a process completes its current CPU burst do NOT count towards TT

	Process(int n, int* pt, int r); // constructor
	Process();// constructor, for CPU only
};

struct Gantt{ // for producing Gantt Chart
	std::vector<std::string> processes;
	std::vector<int> times;
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

// CPU and I/O actions
void admitProcess(int sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ); // admit new process based on its arrival time
void IOContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ);
void CPUContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool exceedQuant);
void pushToIO(std::vector<Process>& ioQ, Process& onCPU);
void pushToCPU(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, bool hasTimeLimit, int timeLimit);

// handle multiple ready process in waitQ and ioQ
void handleSameArrivalTimeInWaitQ(std::vector<Process>& waitQ);
void handleSameFinishTimeInIOQ(int sysTime, std::vector<Process>& ioQ, std::vector<Process>& targets);

// output
void printGanttChart(const Gantt& gantt); // print out Gantt Chart
void printRT_WT_TT(const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit); // print out RT, WT, and TT for each process
void printRT_WT_TT_MLFQ(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool printAtEnd); // print out RT, WT, and TT for each process (MLFQ version)
void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU); // print information of each queue when a new process is just loaded onto CPU
void printWhenNewPricessLoaded_MLFQ(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, Gantt& gantt); // print information of each queue when a new process is just loaded onto CPU (MLFQ version)

// Miscellaneous
void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt); // Gather info to print Gantt Chart



#endif /* CPU_SCHEDULER_H_ */
