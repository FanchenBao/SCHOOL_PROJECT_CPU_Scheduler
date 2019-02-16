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

void FCFS(int& sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit);
void RR(int& sysTime, int quant, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit);
void MLFQ(int& sysTime, const std::vector<int>& quantums, std::vector<Process>& processList, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, const int numProcess, const bool hasTimeLimit, const int timeLimit);
void SJF(int& sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit);


#endif /* SCHEDULER_ALGORITHM_H_ */
