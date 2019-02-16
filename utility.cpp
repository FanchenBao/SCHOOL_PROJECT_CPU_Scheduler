/*
 * utility.cpp
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 */

#include "utility.h"


// handle multiple ready process in waitQ and ioQ
void handleSameArrivalTimeInWaitQ(std::vector<Process>& waitQ){
	// the top of waitQ should be pushed onto CPU, BUT if there are multiple processes with the
	// same arrival time and all ready to go to CPU, only the one with the smallest process number
	// gets CPU first.
	auto it = ++(waitQ.begin());
	for (; it != waitQ.end(); it++){ // check whether more than one processes are eligible for CPU
		if (it->arrival != waitQ.begin()->arrival) {break;}
	}
	std::sort(waitQ.begin(), it, ComparePNumber()); // sort these eligible processes based on process number
}

void handleSameFinishTimeInIOQ(int sysTime, std::vector<Process>& ioQ, std::vector<Process>& targets){
	// find all processes that finish I/O and pop all of them off
	for (auto it = ioQ.begin(); it != ioQ.end();){
		if (it->remainIOBurst == 0){
			targets.push_back(*it);
			it = ioQ.erase(it);
		}
		else
			it++;
	}
	std::make_heap(ioQ.begin(), ioQ.end(), CompareIO());
	std::sort(targets.begin(), targets.end(), ComparePNumber()); // order according to process number (ascending)
}
