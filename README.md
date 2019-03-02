# CPU Scheduler
Simulate CPU scheduling based on user input process information.

## Under development.
Final goal is to have user input process information into console, choose a scheduling algorithm to simulate, and get results also in the console as a Gantt Chart and a table depicting each process's response time, waiting time, turnaround time, and their averages.

To do list
* CPU scheduler algorithms:
    * FCFS (First Come First Served, non-preemptive) **Complete**
    * SJF (Shortest Job First, non-preemptive) **Complete**
    * Priority (non-preemptive) **Complete**
    * RR (Round Robin, preemptive) **Complete**
    * SRF (Shortest Remanining time First, preemptive)
    * Preemptive Priority (preemptive)
    * MLQ_noF (MultiLevel Queue non Feedback, lowest priority queue must be FCFS, other queues must be RR with higher priority having smaller quantum)
    * MLFQ (MultiLevel Feedback Queue, same rule as MLQ) **Complete**
* __Tie-break rule for multiple processes arriving at the ready queue at the same time__
    * When any non-arrival-time priorities are the same, processes are ordered according to their arrival time at the ready queue. Smaller arrival time goes first. This tie-break rule is generally desired.
    * When non-arrival-time priorities and arrival time are the same, process with smallest process number goes first. This is an arbitrary rule and can be modified if needed. If one does not wish to enforce this rule, he/she can comment out the handleSamePriorityInReadyQ() function in case 1 of prioritizeReadyQ(). Note that this is a temporary solution. Later version shall provide an option for user to decide whether they want to enforce this tie-break rule.
