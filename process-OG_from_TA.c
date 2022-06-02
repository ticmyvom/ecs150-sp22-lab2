#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

struct process
{
    char *name;
    int totalCPU;     // Its total CPU time (as read)
    int completeTime; // The wall clock time at which it was completed (this is the value of the counter, not the time of day)
    int givenCPU;     // # of times that it was given the CPU
    int BlockedIO;
    int doingIO;
};

struct resource
{
    char *name;         // CPU or IO
    int busy;           // total busy time
    int idle;           // total idle time
    double utilization; // busy / (busy + idle)
    int number;         // dispatches or IO time
    double throughput;
};

queue_t q;
struct resource *sysCPU, *sysIO;

// generate a pointer to a process and enqueue it.
struct process *generateProcess(char *name, int totalCPU, int completeTime, int givenCPU, int BlockedIO, int doingIO)
{
    struct process *tmp = (struct process *)malloc(sizeof(struct process));
    tmp->name = name;
    tmp->totalCPU = totalCPU;
    tmp->completeTime = completeTime;
    tmp->givenCPU = givenCPU;
    tmp->BlockedIO = BlockedIO;
    tmp->doingIO = doingIO;
    return tmp;
}

void displayProcess(struct process *p)
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
    tmp->throughput = (double)number / (double)(busy + idle);
    return tmp;
}

void displayResource(struct resource *res)
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

int main(void)
{
    // process
    q = queue_create();

    struct process *ptr;
    struct process *p1 = generateProcess("p1", 1, 2, 3, 4, 5);
    struct process *p2 = generateProcess("p2", 5, 4, 3, 2, 1);

    queue_enqueue(q, p1);
    queue_enqueue(q, p2);
    queue_dequeue(q, (void **)&ptr);
    displayProcess(ptr);
    queue_dequeue(q, (void **)&ptr);
    displayProcess(ptr);

    free(p1);
    free(p2);
    queue_destroy(q);

    // system resources
    sysCPU = buildResource("CPU", 2, 3, 5);
    sysIO = buildResource("IO", 3, 5, 7);

    displayResource(sysCPU);
    displayResource(sysIO);

    free(sysCPU);
    free(sysIO);
    return 0;
}