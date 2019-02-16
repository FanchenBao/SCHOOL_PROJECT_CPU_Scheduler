/*
 * input_output.h
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 *      Description: Handle user input and console output
 */

#ifndef INPUT_OUTPUT_H_
#define INPUT_OUTPUT_H_

#include "core_functions.h"

struct Gantt{ // for producing Gantt Chart
	std::vector<std::string> processes;
	std::vector<int> times;
	bool preIdle; // flag, true = previous Gantt entry is idle, false = previous Gantt entry is a process
};

// output
void printGanttChart(const Gantt& gantt); // print out Gantt Chart
void printRT_WT_TT(const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit); // print out RT, WT, and TT for each process
void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, const bool CPUidle); // print information of each queue when a new process is just loaded onto CPU

// Miscellaneous
void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt, bool CPUidle); // Gather info to print Gantt Chart

// for MLFQ, these functions are overloaded
void printRT_WT_TT(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit);
void printWhenNewPricessLoaded(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, const bool CPUidle);


#endif /* INPUT_OUTPUT_H_ */
