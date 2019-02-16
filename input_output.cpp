/*
 * input_output.cpp
 *
 *  Created on: Feb 16, 2019
 *      Author: fanchen
 */

#include "input_output.h"

void updateGanttChart(int sysTime, Process& onCPU, Gantt& gantt, bool CPUidle){
	// Gather info to print Gantt Chart
	gantt.times.push_back(sysTime);
	if (!CPUidle){ // a new process is currently on CPU
		gantt.processes.push_back("P" + std::to_string(onCPU.number));
		gantt.preIdle = false;
	}
	else{ // CPU in idle
		gantt.processes.push_back("IDLE");
		gantt.preIdle = true;
	}
}

//output
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

void printWhenNewPricessLoaded(int sysTime, const std::vector<Process>& waitQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, const bool CPUidle){
	// print information of each queue when a new process is just loaded onto CPU.
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: ";
	if (CPUidle)
		std::cout << "[idle]\n";
	else
		std::cout << "P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
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


// same functions overloaded for MLFQ
void printRT_WT_TT(const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, bool hasTimeLimit){
	// print out RT, WT, and TT for each process. Overloaded for MLFQ
	std::vector<Process> forPrint(complete.begin(), complete.end());
	if (hasTimeLimit){ // scheduling exits prematurely, processes are scattered in different places.
		forPrint.insert(forPrint.end(), ioQ.begin(), ioQ.end());
		for (auto& subQ : MLQ)
			forPrint.insert(forPrint.end(), subQ.begin(), subQ.end());
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

void printWhenNewPricessLoaded(int sysTime, const std::vector<std::vector<Process> >& MLQ, const std::vector<Process>& ioQ, const std::vector<Process>& complete, const Process& onCPU, const bool CPUidle){
	// print information of each queue when a new process is just loaded onto CPU. Overloaded for MLFQ
	std::cout << "Current Time = " << sysTime << std::endl;
	std::cout << "Next process on the CPU: ";
	if (CPUidle)
		std::cout << "[idle]\n";
	else
		std::cout << "P" << onCPU.number << ", Burst = "<< onCPU.remainCPUBurst << std::endl;
	for (int i = 0; i < 60; i++)
		std::cout <<".";
	std::cout << "\nList of processes in the ready queue:\n";
	std::cout << "\tProcess\t\tBurst\t\tQueue\n";
	bool allSubQEmpty = true;
	if (!CPUidle){
		for (auto subQ : MLQ){
			if (!subQ.empty()){
				allSubQEmpty = false;
				for (auto p : subQ)
					std::cout << "\tP" << p.number << "\t\t" << p.remainCPUBurst << "\t\tQ" << p.queuePriority << std::endl;
			}
		}
	}
	if (allSubQEmpty)
		std::cout << "\t[empty]\n";
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


