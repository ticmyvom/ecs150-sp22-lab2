#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include <stdbool.h>

// Based on process.c from the TA

/*
 * process info from file, simulation, and printing stats
 */
struct process_from_input
{
    char *name;          // Process name; it is not to be longer than 10 characters. If it is, give an error and exit.
    int totalCPU;        // Its total CPU time (as read) this is the total CPU time needed for the process to complete. If it is not positive, give an error and exit
    float blocking_prob; // Probability of blocking; as it is a probability, it must be between 0 and 1 inclusive. If it is not, give an error and exit.

    /* doing things per tick */
    int priority; // according to the lab2 hints, no idea the purpose
    bool to_be_blocked_for_IO;
    int time_til_completion; // remaining time on CPU
    int time_til_IO;         // how to long to run on CPU until blocked for IO
    int time_left_on_IO;     // remaining time on IO

    /* for stat printing at the end */
    int completeTime; // The wall clock time at which it was completed (this is the value of the counter, not the time of day)
    int givenCPU;     // # of times that it was given the CPU
    int BlockedIO;    // # of times blocked for IO
    int doingIO;      // time spent on IO
};

struct resource
{
    char *name;         // CPU or IO
    int busy;           // total busy time
    int idle;           // total idle time
    double utilization; // busy / (busy + idle)
    int number;         // dispatches or IO time
    double throughput;  //
    bool cur_running;   // is currently running
};

queue_t q;
struct resource *sysCPU, *sysIO;

/*
 * TODO: update with the new structure of process_from_input.
 */
struct process_from_input *generateTestProcess(char *name, int totalCPU, double blocking_prob)
{
    struct process_from_input *tmp = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
    tmp->blocking_prob = blocking_prob;
    tmp->time_til_completion = totalCPU;
    tmp->time_til_IO = 0;
    tmp->time_left_on_IO = 0;
    tmp->to_be_blocked_for_IO = false;
    return tmp;
}

struct process_from_input *generateProcess(char *name, int totalCPU, int completeTime, int givenCPU, int BlockedIO, int doingIO)
{
    struct process_from_input *tmp = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
    tmp->completeTime = completeTime;
    tmp->givenCPU = givenCPU;
    tmp->BlockedIO = BlockedIO; // should be 0 by default
    tmp->doingIO = doingIO;
    return tmp;
}

void displayProcess(struct process_from_input *p)
{
    // printf("Comment these 3 lines\n");
    // printf("name\ttotalCPU\tblocking_prob\n");
    // printf("%s\t%d\t\t%f\n", p->name, p->totalCPU, p->blocking_prob);

    printf("%-10s %6d     %6d    %6d    %6d    %6d\n", p->name, p->totalCPU, p->completeTime, p->givenCPU, p->BlockedIO, p->doingIO);
}

struct resource *buildResource(char *name, int busy, int idle, int number)
{
    struct resource *tmp = (struct resource *)malloc(sizeof(struct resource));
    tmp->name = name;
    tmp->busy = busy;
    tmp->idle = idle;
    tmp->utilization = (double)busy / (double)(busy + idle);
    tmp->number = number;
    tmp->throughput = (double)number / (double)(busy + idle); // TODO: check if this is really throughput
    tmp->cur_running = false;                                 // idle after init
    return tmp;
}

// TODO: may need to edit this to print stat at the end
void displayResource(struct resource *res) // OPTIONAL: edit to update struct (idk what i was thinking, ignore this comment)
{
    if (res->name == "CPU")
    {
        printf("\n%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("CPU utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else if (res->name == "IO")
    {
        printf("\n%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("I/O utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else
    {
        printf("Error Resource Type");
    }
}
