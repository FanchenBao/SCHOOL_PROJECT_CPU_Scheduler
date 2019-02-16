/*
 * utility.cpp
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 */

#include "utility.h"


// handle multiple ready process in waitQ and ioQ
void handleSamePriorityInWaitQ(std::vector<Process>& waitQ, int priorityType){
	// The top of waitQ should be pushed onto CPU, BUT if there are multiple processes with the
	// same priority and all ready to go to CPU, only the one with the smallest process number gets CPU first.
	// priorityType = 1 (arrival time (FCFS, RR)), 2 (CPU burst (SJF, SRF))
	auto it = ++(waitQ.begin());
	bool doneWithLoop = false;
	for (; it != waitQ.end(); it++){ // locate all processes that are immediately eligible for CPU
		switch (priorityType){
			case 1:{
				if (it->arrival != waitQ.begin()->arrival)
					doneWithLoop = true;
				break;
			}
			case 2:{
				if (it->remainCPUBurst != waitQ.begin()->remainCPUBurst)
					doneWithLoop = true;
				break;
			}
			default:{
				std::cerr << "Fatal Error: Check priorityType for handling same priority in wait queue." << std::endl;
				std::exit(1);
			}
		}
		if (doneWithLoop) {break;}

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
