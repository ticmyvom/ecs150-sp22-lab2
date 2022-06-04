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
    int time_til_completion;
    int time_til_IO;     // how to long to run on CPU until blocked for IO
    int time_left_on_IO; //

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
    double throughput;  // TODO
    bool is_idle;
};

queue_t q;
struct resource *sysCPU, *sysIO;

/*
 * TODO: update with the new structure of process_from_input
 * generate a pointer to a process and enqueue it.
 */
struct process_from_input *generateProcess(char *name, int totalCPU, int completeTime, int givenCPU, int BlockedIO, int doingIO)
{
    struct process_from_input *tmp = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
    tmp->completeTime = completeTime;
    tmp->givenCPU = givenCPU;
    tmp->BlockedIO = BlockedIO;
    tmp->doingIO = doingIO;
    return tmp;
}

void displayProcess(struct process_from_input *p) // OPTIONAL/TODO: update stat to display process better
{
    printf("name\ttotalCPU\tcompleteTime\tgivenCPU\tBlockedIO\tdoingIO\n");
    printf("%s\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", p->name, p->totalCPU, p->completeTime, p->givenCPU, p->BlockedIO, p->doingIO);
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
    tmp->is_idle = true;                                      // idle after init
    return tmp;
}

// TODO: may need to edit this to print stat at the end
void displayResource(struct resource *res) // OPTIONAL: edit to update struct
{
    if (strcmp(res->name, "CPU"))
    {
        printf("%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("CPU utilization: %.2f\n", res->utilization);
        printf("Number of dispatches: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else if (strcmp(res->name, "IO"))
    {
        printf("%s:\n", res->name);
        printf("Total time spent busy: %d\n", res->busy);
        printf("Total time spent idle: %d\n", res->idle);
        printf("I/O device utilization: %.2f\n", res->utilization);
        printf("Number of times I/O was started: %d\n", res->number);
        printf("Overall throughput: %.2f\n", res->throughput);
    }
    else
    {
        printf("Error Resource Type");
    }
}
