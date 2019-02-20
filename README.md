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
* Console user interface
