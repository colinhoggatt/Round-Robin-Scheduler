# You Spin Me Round Robin

Our goal for this lab is to build a process scheduler that imitates Round-Robin Scheduling. Using the TAILQ macro, we construct a doubly linked list in order to keep track of the processes and schedule them (add to a queue) based on arrival time. Each process runs for either the remaining burst time it has, or until the quantum length (time slice) is up. Then we add it back to the queue if it's not finished and continue cycling through the processes until we're done. We also calculate the response time for the first time a process is executed, and the waiting time over the course of the process's execution.

## Building

```shell
make
```
This command constructs our executable.

## Running

cmd for running 
```shell
./rr processes.txt 3
```
3 here is the provided quantum length. This can be changed.

results 
```shell
Average waiting time: 7.00
Average response time: 2.75
```
These are the results for a quantum length of 3.

## Cleaning up

```shell
make clean
```
This command cleans the binary (deletes the executable)