/*
 * test.h
 *
 *  Created on: Mar 1, 2019
 *      Author: fanchen
 *      Description: Functions for testing purpose
 */

#ifndef TEST_H_
#define TEST_H_

#include "scheduler_algorithm.h"

void reset(std::vector<Process>& processList, std::vector<Process>& readyQ, std::vector<Process>& ioQ, std::vector<Process>& complete, std::vector<std::vector<Process> >& MLQ, Process& onCPU, Gantt& gantt, int& sysTime, int& sysIdle, bool& CPUidle);
void loadData_9processes(std::vector<Process>& processList, int scheduler);
void test_9processes();



#endif /* TEST_H_ */
