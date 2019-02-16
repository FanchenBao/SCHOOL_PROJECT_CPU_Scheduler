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

// handle multiple ready process in waitQ and ioQ
void handleSameArrivalTimeInWaitQ(std::vector<Process>& waitQ);
void handleSameFinishTimeInIOQ(int sysTime, std::vector<Process>& ioQ, std::vector<Process>& targets);



#endif /* UTILITY_H_ */
