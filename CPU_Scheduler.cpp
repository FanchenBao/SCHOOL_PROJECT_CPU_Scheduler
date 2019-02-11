//============================================================================
// Name        : CPU_Scheduler.cpp
// Author      : Fanchen
// Date		   : 02/10/2019
// Description : CPU scheduler assignment for COP4610
//============================================================================

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

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

	Process(int n, int* pt, int r) : // constructor
		number(n), index(0), arrival(0), remainCPUBurst(r), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0)
		{int i = 0;
		while (pt[i])
			processTime.push_back(pt[i++]);
		ptSize = i;}

	Process(): // constructor, for CPU only
		number(0), index(0), ptSize(0), arrival(0), remainCPUBurst(0), remainIOBurst(0), totalCPUBurst(0), totalIOBurst(0), queuePriority(1), responseTime(0), waitTime(0), turnaroundTime(0){}
};

struct Gantt{
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

void printGanttChart(const Gantt& gantt){
	// print out Gantt Chart
	for (auto p : gantt.processes)
		std::cout << "|" << p << "\t\t";
	std::cout <<"|\n";
	for (auto t : gantt.times)
		std::cout << t << "\t\t";
	std::cout <<"\n";
}

void printRT_WT_TT(const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit){
	// print out RT, WT, and TT for each process
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (hasTimeLimit){ // scheduling exits prematurely, processes are scattered in different places.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		forPrint.insert(forPrint.end(), waitQ.begin(), waitQ.end());
		forPrint.push_back(onCPU);
	}
	std::sort(forPrint.begin(), forPrint.end(), ComparePNumber());
	std::cout << "Process\t" << "RT\t" << "WT\t" << "TT\t" << "TT Breakdown\n";
	for (auto p : forPrint){
		std::cout << "P" << p.number << "\t"
				<< p.responseTime << "\t"
				<< p.waitTime << "\t"
				<< p.turnaroundTime << "\t"
				<< p.totalCPUBurst << "(CPU) + " << p.waitTime << "(WT) + " << p.totalIOBurst << "(I/O)" << std::endl;
	}
}

void printRT_WT_TT_MLFQ(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool printAtEnd){
	// print out RT, WT, and TT for each process
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (!printAtEnd){ // if data are to be printed mid way through the scheduling, processes are scattered in all queues.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		for (int i = 0; i < 3; i++)
			forPrint.insert(forPrint.end(), MLQ[i].begin(), MLQ[i].end());
		forPrint.push_back(onCPU);
	}
	std::sort(forPrint.begin(), forPrint.end(), ComparePNumber());
	std::cout << "Process\t" << "RT\t" << "WT\t" << "TT\n";
	for (auto p : forPrint){
		std::cout << "P" << p.number << "\t" << p.responseTime << "\t" << p.waitTime << "\t" << p.turnaroundTime << std::endl;
	}
}

void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt){
	// Gather info to print Gantt Chart
	gantt.times.push_back(sysTime);
	gantt.processes.push_back("P" + std::to_string(onCPU.number));
}

void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU){
	// print information of each queue when a new process is just loaded onto CPU. Also gethering information
	// for Gantt Chart
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\n";
	if (waitQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : waitQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in I/O:\n";
	std::cout << "\tProcess\t\tRemaining I/O Time\n";
	if (ioQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : ioQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainIOBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<"*";
	std::cout << "\n\n\n";
}

void printWhenNewPricessLoaded_MLFQ(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, Gantt& gantt){
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\t\tQueue\n";
	if (MLQ[0].empty() && MLQ[1].empty() && MLQ[2].empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : MLQ[0])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
		for (auto p : MLQ[1])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
		for (auto p : MLQ[2])
			std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in I/O:\n";
	std::cout << "\tProcess\t\tRemaining I/O Time\n";
	if (ioQ.empty())
		std::cout << "\t[empty]\n";
	else{
		for (auto p : ioQ)
			std::cout << "\tP" << p.number << "\t\t" << p.remainIOBurst << std::endl;
	}
	for (int i = 0; i < 60; i++)
		std::cout <<"*";
	std::cout << "\n\n\n";

	// for Gantt Chart
	gantt.times.push_back(sysTime);
	gantt.processes.push_back("P" + std::to_string(onCPU.number));
}

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
	for (auto& t : targets){
		t.remainCPUBurst = t.processTime[++(t.index)]; // move index to CPU element of processTime and update remainCPUBurst
		t.arrival = sysTime; // update arrival time
	}
}

void admitProcess(int sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ){
	// admit new process based on its arrival time
	for (auto it = processList.begin(); it != processList.end();){ // only load processes whose arrival time is the same as sysTime onto waitQ
		if (it->arrival == sysTime){
			waitQ.push_back(*it); // initially all processes is in queue 1
			it = processList.erase(it);
		}
		else {return;}
	}
}

void pushToIO(std::vector<Process>& ioQ, Process& onCPU){
	// kick a process off CPU and onto I/O queue
	onCPU.remainIOBurst = onCPU.processTime[++(onCPU.index)]; // move index to I/O element of processTime and update remainIOBurst
	ioQ.push_back(onCPU);
	std::push_heap(ioQ.begin(), ioQ.end(), CompareIO());
}

void IOContextSwitch(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ){
	// Determine whether I/O context switch is needed.
	// If needed, do the context switch and push all processes that finish I/O to wait queue
	if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0){ // some process finishes I/O, needs to get back to waitQ
		std::vector<Process> targets;
		handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
		waitQ.insert(waitQ.end(), targets.begin(), targets.end()); // push process on waitQ
	}
}

void CPUContextSwitch(std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle){
	// Determine whether CPU context switch is needed.
	// If needed, do the context switch and push the process to I/O
	if (!CPUidle && onCPU.remainCPUBurst == 0){ // current process finishes CPU burst

		onCPU.totalCPUBurst += onCPU.processTime[onCPU.index]; // update totalCPUBurst
		if (onCPU.index > 0)
			onCPU.totalIOBurst += onCPU.processTime[onCPU.index - 1]; // update totalIOBurst
		// update turnaround time. TT = wait time + CPU burst (just finished) + I/O time right before
		onCPU.turnaroundTime = onCPU.waitTime + onCPU.totalCPUBurst + onCPU.totalIOBurst;

		// still more I/O to do
		if (onCPU.index < onCPU.ptSize - 2)
			pushToIO(ioQ, onCPU); // push current process from CPU to I/O
		else
			complete.push_back(onCPU); // no more I/O, i.e. all bursts have been completed.
		CPUidle = true; // set CPU to idle
	}
}

void loadProcessToCPU(int sysTime, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, bool& CPUidle, Gantt& gantt, bool hasTimeLimit, int timeLimit){
	// Determine whether the CPU and wait queue are both ready to push a new process to CPU
	// take on new process only when CPU is idle and there is process ready in the waitQ
	if (CPUidle && !waitQ.empty() && waitQ.begin()->arrival <= sysTime){
		handleSameArrivalTimeInWaitQ(waitQ); // check same arrival time situation
		onCPU = *waitQ.begin();
		if (onCPU.totalCPUBurst == 0) // process is serviced for the first time
			onCPU.responseTime = sysTime - onCPU.arrival;
//		onCPU.serviced = true;
		if (!hasTimeLimit || sysTime < timeLimit)
			onCPU.waitTime += sysTime - onCPU.arrival;
		waitQ.erase(waitQ.begin());
		CPUidle = false;

		// print out waitQ, ioQ, and CPU info when a new process gets CPU, also provide information to generate Gantt Chart
		printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU);
		updateGanttChart(sysTime, onCPU, gantt);
	}
}

void FCFS(int& sysTime, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt, bool hasTimeLimit, int timeLimit){
	bool CPUidle = true; // flag for whether CPU is busy or idle

	while (true){
		if (!processList.empty()) // admit processes based on their initial arrival time
			admitProcess(sysTime, processList, waitQ);

		// Do context switches first
		IOContextSwitch(sysTime, waitQ, ioQ); // I/O side context switch, if needed
		CPUContextSwitch(ioQ, complete, onCPU, CPUidle); // CPU side context switch, if needed
		loadProcessToCPU(sysTime, waitQ, ioQ, complete, onCPU, CPUidle, gantt, hasTimeLimit, timeLimit); // load a process to CPU, if possible

		// END POINT!!!
		if ((onCPU.remainCPUBurst == 0 && waitQ.empty() && ioQ.empty()) || (hasTimeLimit && sysTime == timeLimit))
			break;

		// Update key parameters at each time tick
		if (!CPUidle) {onCPU.remainCPUBurst--;} // CPU processing
		for (auto& p : ioQ) {p.remainIOBurst--;} // ioQ updates
		sysTime++;
	}
}


//int RR(int quant, std::vector<Process>& processList, std::vector<Process>& waitQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt){
//	int sysTime = 0;
//	int currQuant = quant;
//	bool CPUidle = true; // flag for whether CPU is busy or idle
//	bool newProcessLoaded; // flag set true only at the iteration a new process is loaded
//
//	while (true){
//		// admit processes based on their initial arrival time
//		if (!processList.empty())
//			admitProcess(sysTime, processList, waitQ);
//
//		// Do context switches first
//		// I/O side context switch
//		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0){ // some process finishes I/O, needs to get back to waitQ
//			std::vector<Process> targets;
//			handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
//			waitQ.insert(waitQ.end(), targets.begin(), targets.end()); // push process on waitQ
//		}
//
//		// CPU side context switch
//		newProcessLoaded = false;
//		if (!CPUidle && onCPU.remainCPUBurst == 0){ // current process finishes CPU burst
//			if (onCPU.index < onCPU.ptSize - 2){pushToIO(ioQ, onCPU);} // still more I/O to do
//			else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
//			CPUidle = true; // set CPU to idle
//			currQuant = quant;
//
//			if (waitQ.empty() && ioQ.empty()) {break;} // END POINT!!!
//		}
//		if (!CPUidle && currQuant == 0){ // current process hasn't finished its CPU burst but used up its quantum
//			onCPU.arrival = sysTime; // reset arrival time
//			waitQ.push_back(onCPU); // pushed to the back of waitQ
//			CPUidle = true; // set CPU to idle
//			currQuant = quant; // reset quantum
//		}
//		if (CPUidle && !waitQ.empty() && waitQ.begin()->arrival <= sysTime){ // take on new process only when CPU is idle and there is process in the waitQ
//			// the top of waitQ should be pushed onto CPU, BUT if there are multiple processes with the
//			// same arrival time and all ready to go to CPU, only the one with the smallest process number
//			// gets CPU first.
//			handleSameArrivalTimeInWaitQ(waitQ); // check same arrival time situation
//			popOnCPU(waitQ, onCPU, CPUidle, newProcessLoaded);}
//
//		// print out waitQ, ioQ, and CPU information at each instance when a new process gets CPU
//		// also provide information to generate Gantt Chart
//		if (newProcessLoaded){printWhenNewPricessLoaded(sysTime, waitQ, ioQ, complete, onCPU, gantt);}
//
//		// Then update key parameters at each time tick
//		// CPU processing
//		if (!CPUidle){
//			onCPU.remainCPUBurst--;
//			onCPU.turnaroundTime++;
//			currQuant--;
//		}
//		// waitQ updates
//		for (auto& p : waitQ){
//			if (!p.serviced) // increase RT for the processes that haven't been serviced yet
//				p.responseTime++;
//			p.waitTime++;
//			p.turnaroundTime++;
//		}
//		// ioQ updates
//		for (auto& p : ioQ){
//			p.remainIOBurst--;
//			p.turnaroundTime++;
//		}
//		sysTime++;
//	}
//	return sysTime;
//
//}
//
//int MLFQ(int q1, int q2, std::vector<Process>& processList, std::vector<std::vector<Process> >& MLQ, std::vector<Process>& ioQ, std::vector<Process>& complete, Process& onCPU, Gantt& gantt){
//	int sysTime = 0;
//	int currQ1 = q1;
//	int currQ2 = q2;
//	bool CPUidle = true; // flag for whether CPU is busy or idle
//	bool newProcessLoaded; // flag set true only at the iteration a new process is loaded
//
//	while (true){
//		// admit processes based on their initial arrival time
//		if (!processList.empty())
//			admitProcess(sysTime, processList, MLQ[0]);
//
//		// Do context switches first.
//		// I/O side context switch
//		if (!ioQ.empty() && ioQ.begin()->remainIOBurst == 0){ // some process finishes I/O, needs to get back to waitQ
//			std::vector<Process> targets;
//			handleSameFinishTimeInIOQ(sysTime, ioQ, targets);
//			for (auto t : targets){
//				// push process to the waiting queue corresponding to their queue priority
//				if (t.queuePriority == 1)
//					MLQ[0].push_back(t);
//				else if (t.queuePriority == 2)
//					MLQ[1].push_back(t);
//				else if (t.queuePriority == 3)
//					MLQ[2].push_back(t);
//			}
//		}
//
//		// CPU side context switch
//		// current process finishes CPU burst
//		newProcessLoaded = false;
//		if (!CPUidle && onCPU.remainCPUBurst == 0){
//			if (onCPU.index < onCPU.ptSize - 2) {pushToIO(ioQ, onCPU);} // still more I/O to do
//			else {complete.push_back(onCPU);} // no more I/O, i.e. all bursts have been completed.
//			CPUidle = true; // set CPU to idle
//			// depending on which queue the process that has just been kicked off CPU is on, update its quantum
//			if (onCPU.queuePriority == 1){currQ1 = q1;}
//			else if (onCPU.queuePriority == 2){currQ2 = q2;}
//
//			if (MLQ[0].empty() && MLQ[1].empty() && MLQ[2].empty() && ioQ.empty()) {break;} // END POINT!!!
//		}
//		// current process hasn't finished its CPU burst but used up its quantum, ONLY for process in queue 1 and queue 2
//		if (!CPUidle && ((onCPU.queuePriority == 1 && currQ1 == 0) || (onCPU.queuePriority == 2 && currQ2 == 0))){
//			onCPU.arrival = sysTime; // reset arrival time
//			onCPU.queuePriority++; // downgrade queue priority
//			if (onCPU.queuePriority == 2){
//				MLQ[1].push_back(onCPU); // downgrade to queue 2 (note that queue2 is MLQ[1])
//				currQ1 = q1; // reset queue1 quantum
//			}
//			else if (onCPU.queuePriority == 3){
//				MLQ[2].push_back(onCPU); // downgrade to queue 3 (note that queue3 is MLQ[2])
//				currQ2 = q2; // reset queue2 quantum
//			}
//			CPUidle = true; // set CPU to idle
//		}
//		// current process hasn't finished its CPU burst or quantum, but the higher priority queue has process ready to preempt
//		// the current process
//		if (!CPUidle){
//			// process at top of queue1 is ready to get CPU and the current process on CPU has lower queue priority
//			if (!MLQ[0].empty() && MLQ[0].begin()->arrival == sysTime && onCPU.queuePriority > 1){
//				onCPU.arrival = sysTime;
//				if (onCPU.queuePriority == 2){
//					MLQ[1].push_back(onCPU); // preempted, but NOT downgraded. Still in queue2
//					currQ2 = q2; // reset queue2 quantum
//				}
//				else if (onCPU.queuePriority == 3){
//					MLQ[2].push_back(onCPU); // preempted, but NOT downgraded. Still in queue3
//				}
//				CPUidle = true; // set CPU to idle
//			}
//			// process at top of queue2 is ready to get CPU and the current process on CPU has lower queue priority
//			else if (!MLQ[1].empty() && MLQ[1].begin()->arrival == sysTime && onCPU.queuePriority > 2){
//				onCPU.arrival = sysTime;
//				MLQ[2].push_back(onCPU); // preempted, but NOT downgraded. Still in queue3
//				CPUidle = true; // set CPU to idle
//			}
//		}
//		// CPU take on new process, only when it is idle, and in the order of queue priority
//		if (CPUidle){
//			if (!MLQ[0].empty() && MLQ[0].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[0]); // check same arrival time situation
//				popOnCPU(MLQ[0], onCPU, CPUidle, newProcessLoaded);
//			}
//			else if (!MLQ[1].empty() && MLQ[1].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[1]); // check same arrival time situation
//				popOnCPU(MLQ[1], onCPU, CPUidle, newProcessLoaded);
//			}
//			else if (!MLQ[2].empty() && MLQ[2].begin()->arrival <= sysTime){
//				handleSameArrivalTimeInWaitQ(MLQ[2]); // check same arrival time situation
//				popOnCPU(MLQ[2], onCPU, CPUidle, newProcessLoaded);
//			}
//		}
//
//		// print out waitQ, ioQ, and CPU information at each instance when a new process gets CPU
//		// also provide information to generate Gantt Chart
//		if (newProcessLoaded)
//			{printWhenNewPricessLoaded_MLFQ(sysTime, MLQ, ioQ, complete, onCPU, gantt);}
//
//		// Then update key parameters at each time tick
//		// CPU processing
//		if (!CPUidle){
//			onCPU.remainCPUBurst--;
//			onCPU.turnaroundTime++;
//			if (onCPU.queuePriority == 1)
//				currQ1--;
//			else if (onCPU.queuePriority == 2)
//				currQ2--;
//		}
//		// MLQ updates
//		for (auto& wq : MLQ){
//			for (auto& p : wq){
//				if (!p.serviced) // increase RT for the processes that haven't been serviced yet
//					p.responseTime++;
//				p.waitTime++;
//				p.turnaroundTime++;
//			}
//		}
//		// ioQ updates
//		for (auto& p : ioQ){
//			p.remainIOBurst--;
//			p.turnaroundTime++;
//		}
//		sysTime++;
//	}
//	return sysTime;
//}



int main() {
	std::vector<Process> processList;
	std::vector<Process> waitQ;
	std::vector<Process> ioQ;
	std::vector<Process> complete;
	Process onCPU;
	Gantt gantt;
	std::vector<std::vector<Process> > MLQ; // multilevel queues
	for (int i = 0; i < 3; i++) // add three subqueues to MLQ
		MLQ.emplace_back(std::vector<Process>());

	// Testing Data
//	int info[9][30] = {{4, 27, 3, 31, 2, 43, 4, 18, 4, 22, 4, 26, 3, 24, 4},
//						{16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4},
//						{8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6},
//						{3, 35, 4, 41, 4, 45, 3, 51, 4, 61, 3, 54, 6, 82, 5, 77, 3},
//						{4, 48, 5, 44, 7, 42, 12, 37, 9, 46, 4, 41, 9, 31, 7, 43, 8},
//						{11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8},
//						{14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10},
//						{4, 14, 5, 33, 6, 51, 14, 63, 16, 87, 6, 74, 7},
//						{3, 37, 9, 41, 8, 30, 4, 29, 7, 33, 5, 22, 4, 24, 5, 29, 16}};

//	int info[5][30] = {{5,6,7},{4,2,3},{2,3,4},{5,2,7},{3,2,4}};

	int info[5][30] = {{8},{6},{12},{4},{6}};

	// initialize all processes, put them all in waitQ in the order of their number.
	for (int i = 0; i < 5; i++) {processList.emplace_back(i+1, info[i], info[i][0]);}

//	processList[0].arrival = 0;
//	processList[1].arrival = 3;
//	processList[2].arrival = 4;
//	processList[3].arrival = 7;
//	processList[4].arrival = 14;

	processList[0].arrival = 0;
	processList[1].arrival = 2;
	processList[2].arrival = 5;
	processList[3].arrival = 11;
	processList[4].arrival = 17;

//	processList[0].arrival = 1;
//	processList[1].arrival = 6;
//	processList[2].arrival = 8;
//	processList[3].arrival = 12;
//	processList[4].arrival = 14;

//	bool hasTimeLimit = true;
	bool hasTimeLimit = false;
	int timeLimit = 150;
	int sysTime = 0; // initial system time

	// FCFS
	FCFS(sysTime, processList, waitQ, ioQ, complete, onCPU, gantt, hasTimeLimit, timeLimit);

	// RR
//	int totalTime = RR(5, processList, waitQ, ioQ, complete, onCPU, gantt);

	// MLFQ
//	int totalTime = MLFQ(4, 9, processList, MLQ, ioQ, complete, onCPU, gantt);

	if (!hasTimeLimit) // if system let to finish completely, Gantt Chart needs the final system time.
		gantt.times.push_back(sysTime);

	printGanttChart(gantt);
	printRT_WT_TT(waitQ, ioQ, complete, onCPU, hasTimeLimit);

	return 0;
}
