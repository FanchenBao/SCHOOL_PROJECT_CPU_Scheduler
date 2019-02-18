/*
 * utility.cpp
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 */

#include "utility.h"


// handle multiple ready process in readyQ and ioQ
void handleSamePriorityInReadyQ(std::vector<Process>& readyQ, int priorityType, int size){
	// The top of readyQ should be pushed onto CPU, BUT if there are multiple processes with the
	// same priority and all ready to go to CPU, the processes are ordered by their arrival time,
	// with smallest arrival time getting CPU first.
	// If arrival time are the same again, then the smallest process number gets CPU first.
	// priorityType = 1 (arrival time (FCFS, RR)), 2 (CPU burst (SJF, SRF))
	// size denotes how many processes within readyQ (starting from the top) should be checked for same priority situation
	int i = 0;
	bool doneWithLoop = false;
	for (; i < size; i++){ // locate all processes that are immediately eligible for CPU
		switch (priorityType){
			case 1:
				if (readyQ[i].arrival != readyQ[0].arrival)
					doneWithLoop = true;
				break;
			case 2:
				if (readyQ[i].remainCPUBurst != readyQ[0].remainCPUBurst)
					doneWithLoop = true;
				break;
			default:
				std::cerr << "Fatal Error: Check priorityType for handling same priority in wait queue." << std::endl;
				std::exit(1);
		}
		if (doneWithLoop) {break;}

	}
	switch (priorityType){
		case 1: // when arrival time is the same, choose the smaller process number
			std::sort(readyQ.begin(), readyQ.begin()+i, ComparePNumber()); // sort these eligible processes based on process number
			break;
		case 2: // when other priority is the same, choose the earliest arrival
			std::sort(readyQ.begin(), readyQ.begin()+i, CompareArrival());
			handleSamePriorityInReadyQ(readyQ, 1, i); // check again for potential same arrival time (i.e. there are processes with same highest priority and same highest arrival time)
			break;
		default:
			std::cerr << "Fatal Error: Check priorityType for handling same priority in wait queue." << std::endl;
			std::exit(1);
	}

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
