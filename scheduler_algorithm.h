/*
 * scheduler_algorithm.h
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 *      Description: CPU scheduling algorithms. They are used to perform the simulation.
 */

#ifndef SCHEDULER_ALGORITHM_H_
#define SCHEDULER_ALGORITHM_H_

#include "core_functions.h"
#include "utility.h"
#include "input_output.h"

void FCFS(int& sysTime, int& sysIdle, bool& CPUidle, std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit, const bool allowOutput = true);
void RR(int& sysTime, int& sysIdle, bool& CPUidle, int quant, std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit, const bool allowOutput = true);
void MLFQ(int& sysTime, int& sysIdle, bool& CPUidle, const std::vector<int>& quantums, std::vector<Process>& processList, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit, const bool allowOutput = true);
void SJF(int& sysTime, int& sysIdle, bool& CPUidle, std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit, const bool allowOutput = true);
void priorityNonPreemptive(int& sysTime, int& sysIdle, bool& CPUidle, std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit, const bool allowOutput = true);

#endif /* SCHEDULER_ALGORITHM_H_ */
