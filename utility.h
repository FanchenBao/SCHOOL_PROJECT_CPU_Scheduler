/*
 * utility.h
 *
 *  Created on: Feb 16, 2019
 *      Author: Fanchen Bao
 *      Description: Helpers to the core functions.
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include "core_functions.h"

// handle multiple ready process in readyQ and ioQ
void handleSamePriorityInReadyQ(std::vector<Process>& readyQ, int priorityType, int size);
void handleSameFinishTimeInIOQ(int sysTime, std::vector<Process>& ioQ, std::vector<Process>& targets);



#endif /* UTILITY_H_ */
