#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

// https://stackoverflow.com/questions/3437404/min-and-max-in-c
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int curWallTime = 0; // The clock time (ticks) start at 1

// PROC *cpu;   /* points to process on cpu */
// PROC *iodev; /* points to process on io device */

// QUEUE ready = {NULL, NULL}; /* ready queue */
// QUEUE io = {NULL, NULL};    /* i/o queue */

// PROC *palloc(char *nm, int rt, float pr);
// void addtoqueue(QUEUE *q, PROC *p);
// void movetocpu(PROC *p);
// void movetoiodev(PROC *p);
// void runfcfs(void);
// void runio(void);
// void run();
// void rfile(char *fname, FILE *fp);

// determine whether to block for I/O
bool to_block(float blocking_prob)
{
    bool res = false;
    int r = random();
    float generated = (float)r / (float)RAND_MAX;
    printf("\trandom number (raw, type, value used:\t%d f %f\n", r, generated);

    if (generated < blocking_prob)
    {
        res = true;
    }
    else
    {
        res = false;
    }
    return res;
}

// determine how many ticks to run for CPU or I/O
int int_rng(char *flag, int remain_running_time)
{
    int r = random();
    int generated;

    if (flag == "io") // generate a random number to determine how long to run on IO
    {
        printf("hi io\n");
        generated = r % 30 + 1;
        printf("\trandom number (raw, type, value used:\t%d i %d\n", r, generated);
        return generated;
    }
    else if (flag == "rr") // generate a random number to determine how long to run on RR
    {
        printf("hi Robin\n");
        int value = min(5, remain_running_time);
        generated = r % value + 1;
        printf("\trandom number (raw, type, value used:\t%d i %d\n", r, generated);
        return generated;
    }
    else if (flag == "fcfs") // generate a random number to determine how long to run before being blocked for IO
    {
        printf("hi fcfs\n");
        generated = r % remain_running_time + 1;
        printf("\trandom number (raw, type, value used:\t%d i %d\n", r, generated);
        return generated;
    }
    else
    {
        fprintf(stderr, "ERROR: wrong flag. Flag is either 'io', 'fcfs', or 'rr'");
        exit(EXIT_FAILURE);
    }
}

/**
 *
 * @param sysCPU, sysIO, ready_queue, io_queue, input_flag (whether -r or -f)
 * @return flag_from_cpu.
 *      0: handlingIOQueue can proceed normally
 *      1: indicate that we just enqueue a process to io_queue. handlingIOQueue should not run that process in the same tick
 *
 **/
int handlingReadyQueue(struct resource *sysCPU, struct resource *sysIO, queue_t ready_queue, queue_t io_queue, char *input_flag)
{
    // printf("start of handlingRQ, HEAD ready queue: %s\n", ((struct process_from_input *)ready_queue->head->value)->name);
    // printf("BACK ready queue: %s\n", ((struct process_from_input *)ready_queue->back->value)->name);
    // input_flag is either fcfs or rr
    if (queue_length(ready_queue) == 0)
    {
        // as the head of the queue is on CPU, if the ready_queue is empty then both CPU and queue are empty
        printf("VIEW: CPU and ready_queue are empty. Leaving handlingCPUQueue\n");
        return 0;
    }

    struct process_from_input *buffer = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    struct process_from_input *top_process = (struct process_from_input *)ready_queue->head->value;

    if (sysCPU->cur_running && ((struct process_from_input *)ready_queue->head->value)->time_til_IO == 0)
    { // this means a process that previously did not block should run for the current clock cycle
        // [ready_queue -> head -> time_til_IO > 0 means it is going to block]
        printf("VIEW: CPU not idle, process %s won't block for IO here\n", top_process->name);
        printf("VIEW: Process %s on CPU with remaining run time %d (out of %d).\n", top_process->name, top_process->time_til_completion, top_process->totalCPU);
        // sysCPU->busy++;

        if (((struct process_from_input *)ready_queue->head->value)->time_til_completion <= 0)
        { // it has reached 0 either just now or previous tick => exit the queue
            printf("VIEW: time_til_completion is 0. Not block for IO so exiting the queue. Leaving handlingCPUQueue\n");
            sysCPU->cur_running = false;
            queue_dequeue(ready_queue, (void **)&buffer);
            top_process->completeTime = curWallTime;
            // TODO: print stat for the current process
            displayProcess(top_process);
            // update the resources' stat after we have a complete stat on a process
            sysCPU->busy = sysCPU->busy + top_process->totalCPU;
            sysIO->busy = sysIO->busy + top_process->doingIO;
            free(buffer);
            return 0;
        }
        ((struct process_from_input *)ready_queue->head->value)->time_til_completion--;
        // top_process->givenCPU++;
    }
    else if (sysCPU->cur_running)
    { // this means that a process is running for a certain time period before blocking
        printf("VIEW: CPU is running, process %s may block for CPU after this tick.\n", top_process->name);
        // sysCPU->busy++;
        printf("VIEW: Process %s on CPU with remaining run time %d (out of %d). Decrementing it now.\n", top_process->name, top_process->time_til_completion, top_process->totalCPU);

        top_process->time_til_IO--;
        top_process->time_til_completion--;
        // top_process->givenCPU++;

        if (top_process->time_til_IO == 0)
        { // it should now block (and so it gets put in IO queue)
            printf("VIEW:\tProcess will block for IO during the next tick. Now: transfer process %s to I/O queue and leave function for ready queue.\n", top_process->name);
            sysCPU->cur_running = false;
            // queue_enqueue(io_queue, ready_queue->head->value);
            queue_dequeue(ready_queue, (void **)&buffer);
            queue_enqueue(io_queue, buffer);
            // printf("enqueue %s to io, dequeue %s from ready\n", buffer->name, buffer->name);
            // printf(" top_process->time_til_IO == 0 HEAD of ready queue then: %s\n", ((struct process_from_input *)ready_queue->head->value)->name);
            // printf("BACK of ready queue then: %s\n", ((struct process_from_input *)ready_queue->back->value)->name);
            // printf("\n");
            // printf("HEAD io queue now: %s\n", ((struct process_from_input *)io_queue->head->value)->name);
            // printf("BACK io queue now: %s\n", ((struct process_from_input *)io_queue->back->value)->name);
            printf("Leaving with flag 1\n");
            return 1;
        }
    }
    else                  // CPU is idle
    {                     // we will take a fresh process from ready queue
        sysCPU->number++; // increment CPU's # of dispatches

        // struct process_from_input *top_process = (struct process_from_input *)ready_queue->head->value; // moved to before the initial IF
        printf("VIEW: CPU is idle and process %s is on the ready queue. Loading it onto CPU.\n", top_process->name);
        // sysCPU->idle++; // count for the previous tick; same with sysCPU->busy++
        bool block = false;
        top_process->givenCPU++;

        if (top_process->time_til_completion >= 2) // has at least 2 units left to run
        {
            printf("VIEW: process %s, run time %d, run time remaining %d; may block for I/O\n", top_process->name, top_process->totalCPU, top_process->time_til_completion);
            printf("VIEW: generating floating-point random number; probability of blocking is %f\n", top_process->blocking_prob);
            block = to_block(top_process->blocking_prob);
        }

        else if (top_process->time_til_completion == 0 && top_process->time_left_on_IO == 0) // 0 time on CPU and just finished IO => terminating
        {
            printf("VIEW: time_til_completion is 0. Not block for IO so terminating. Leaving handlingCPUQueue\n");
            sysCPU->cur_running = false;
            queue_dequeue(ready_queue, (void **)&buffer);
            top_process->completeTime = curWallTime;
            // TODO: print stat for the current process as soon as it terminates
            displayProcess(top_process);
            // update the resources' stat after we have a complete stat on a process
            sysCPU->busy = sysCPU->busy + top_process->totalCPU;
            sysIO->busy = sysIO->busy + top_process->doingIO;
            return 0;
        }

        if (block)
        {
            top_process->to_be_blocked_for_IO = true;
            printf("VIEW: Process wil block for I/O\n");
            top_process->BlockedIO++; // set this once
            top_process->time_til_IO = int_rng(input_flag, top_process->time_til_completion);

            printf("VIEW: Process %s moved to CPU.\n", top_process->name);
            printf("VIEW: Process %s on CPU with remaining run time %d (out of %d). Decrementing it now.\n", top_process->name, top_process->time_til_completion, top_process->totalCPU);
            top_process->time_til_IO--;
            top_process->time_til_completion--;

            // not sure about
            if (top_process->time_til_IO != 0)
            {
                sysCPU->cur_running = true;
            }
            else
            { // if the time_till_IO was initially just 1
                // queue_enqueue(io_queue, (struct process_from_input *)ready_queue->head->value);
                queue_dequeue(ready_queue, (void **)&buffer);
                queue_enqueue(io_queue, buffer);
                // printf("enqueue %s to io, dequeue %s from ready\n", buffer->name, buffer->name);
                // printf("if the time_till_IO was initially just 1 HEAD of ready queue then: %s\n", ((struct process_from_input *)ready_queue->head->value)->name);
                // printf("BACK of ready queue then: %s\n", ((struct process_from_input *)ready_queue->back->value)->name);

                struct process_from_input *top_io = (struct process_from_input *)io_queue->head->value;
                if (top_io->name != top_process->name)
                    top_io->to_be_blocked_for_IO = false; // hande the case when io couldn't run due to flag 1 on both ticks, even tho the processes are different on the queues now

                printf("Leaving handlingCPUQueue with flag 1.\n");
                return 1; // so that IO won't run it at the same tick
            }
        }
        else
        {
            printf("Process will not block for I/O\n");
            top_process->to_be_blocked_for_IO = false;
            top_process->time_til_completion--; // if time_til_completion is 0 after this, it will terminate during the next tick
            sysCPU->cur_running = true;
        }
    } // end checking if CPU is idle

    printf("Leaving handlingCPUQueue.\n");
    return 0;
} // end handlingReadyQueue

void handlingIOQueue(struct resource *sysIO, queue_t ready_queue, queue_t io_queue, int flag_fromCPU)
{
    if (queue_length(io_queue) == 0)
    {
        printf("I/O device and queue is empty. Exiting handlingIOQueue\n");
        return;
    }

    struct process_from_input *buffer = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    struct process_from_input *top_process = (struct process_from_input *)io_queue->head->value;

    if (sysIO->cur_running == true)
    {
        printf("VIEW: IO is running.\n");
        // sysIO->busy++;
        top_process->time_left_on_IO--;
        printf("VIEW: After doing IO for this tick, process %s has %d time units on I/O device\n", top_process->name, top_process->time_left_on_IO);

        if (top_process->time_left_on_IO == 0)
        {
            printf("VIEW:\tprocess %s is being move off IO queue\n", top_process->name);
            sysIO->cur_running = false;
            queue_dequeue(io_queue, (void **)&buffer); // there maybe a problem when enqueuing and dequeuing
            queue_enqueue(ready_queue, buffer);
            printf("enqueue %s to ready, dequeue from io\n", buffer->name);
            // printf("HEAD of ready queue then: %s\n", ((struct process_from_input *)ready_queue->head->value)->name);
            // printf("BACK of ready queue then: %s\n", ((struct process_from_input *)ready_queue->back->value)->name);
            return;
        }
    } // end checking when IO is running

    else
    {
        printf("VIEW: IO is idle.\n");
        sysIO->idle++;
        // printf("VIEW: head of IO_queue is %s, head of CPU_queue is %s\n", top_process->name, ((struct process_from_input *)ready_queue->head->value)->name);
        printf("VIEW: head of IO_queue is %s\n", top_process->name);

        if (flag_fromCPU == 1 && top_process->to_be_blocked_for_IO == true)
        {
            printf("VIEW: flag is 1 and process %s will do IO in the next tick\n", top_process->name);
            printf("Exiting handlingIOQueue\n");
            return;
        }

        else
        {
            printf("VIEW: setting IO to active. Moving process %s to I/O device\n", top_process->name);
            sysIO->number++; // increment # of dispatch
            sysIO->cur_running = true;
            if (top_process->time_til_completion == 0)
            {
                printf("VIEW: remaining run time 0, so I/O block length set to 1 time unit\n");
                top_process->time_left_on_IO = 1;
                top_process->doingIO = top_process->doingIO + top_process->time_left_on_IO;
            }
            else
            {
                top_process->time_left_on_IO = int_rng("io", -1);
                top_process->doingIO = top_process->doingIO + top_process->time_left_on_IO; // update only once per IO determination
            }

            // run for 1 tick on IO
            top_process->time_left_on_IO--;
            printf("VIEW: After doing IO for this tick, process %s has %d time units on I/O device\n", top_process->name, top_process->time_left_on_IO);

            if (top_process->time_left_on_IO == 0)
            {
                sysIO->cur_running = false;
                printf("Moving process %s off io_queue and into ready_queue. Exiting handlingIOQueue\n", top_process->name);
                queue_dequeue(io_queue, (void **)&buffer);
                queue_enqueue(ready_queue, buffer);
                printf("enqueue %s to ready, dequeue from io\n", buffer->name);
                return;
            }
        }
    } // end checking when IO is idle

    printf("Exiting handlingIOQueue\n");
    return;
} // end handlingIOQueue

int main(int argc, char *argv[])
{
    // if (argc <= 2)
    // {
    //     fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
    //     return 1;
    // }

    // TODO 2nd:
    // parse input file
    // check error, exit if found
    // create a struct for each line and add to an array of struct, whose size is the # of line in the file

    /* HARDCODING
     * TODO 1st: recycle to have a function to assist with creating a struct for a process
     *          see generateProcess in process.h
     *          this will helps with hardcoding until we can parse the file
     *
     */

    // file1
    // struct process_from_input *p1 = generateTestProcess("editor", 5, 0.87);
    // struct process_from_input *p2 = generateTestProcess("compiler", 40, 0.53);
    // struct process_from_input *p3 = generateTestProcess("adventure", 30, 0.72);

    // file2
    // struct process_from_input *p1 = generateTestProcess("editor", 3, 0.87);
    // struct process_from_input *p2 = generateTestProcess("compiler", 5, 0.53);
    // struct process_from_input *p3 = generateTestProcess("adventure", 2, 0.72);

    // file3
    // struct process_from_input *p1 = generateTestProcess("editor", 3, 0.87);
    // struct process_from_input *p2 = generateTestProcess("xyzzy", 2, 0.10);

    // file4
    struct process_from_input *p1 = generateTestProcess("biggie", 20, 0.99);
    struct process_from_input *p2 = generateTestProcess("nextone", 10, 0.99);

    struct process_from_input *ptr; // just to test enqueue, dequeue
    queue_t ready_queue, io_queue;
    ready_queue = queue_create();
    io_queue = queue_create();

    queue_enqueue(ready_queue, p1);
    queue_enqueue(ready_queue, p2);
    // queue_enqueue(ready_queue, p3);

    // queue_dequeue(ready_queue, (void **)&ptr);
    // printf("name: %s\n", ptr->name);
    int processes_count = queue_length(ready_queue);
    printf("queue length: %d\n", processes_count);

    (void)srandom(12345);

    struct resource *sysCPU, *sysIO;
    sysCPU = buildResource("CPU", 0, 0, 0); // 0 time for busy, idle, and number of dispatches
    sysIO = buildResource("IO", 0, 0, 0);   // 0 time for busy, idle, and number of dispatches

    // assume input_flag is -f
    char *input_flag = "fcfs";

    printf("Processes:\n\n");
    printf("   name     CPU time  when done  cpu disp  i/o disp  i/o time\n");
    while (queue_length(ready_queue) != 0 || queue_length(io_queue) != 0)
    {
        curWallTime++;
        printf("TICK = %d |||||||||||||||||||||||||||||||||||||||||||||||||||||||\n", curWallTime);
        printf("RUNNING READY QUEUE --------------------------------------\n");
        int flag_from_cpu = handlingReadyQueue(sysCPU, sysIO, ready_queue, io_queue, input_flag);
        printf("RUNNING I/O QUEUE --------------------------------------\n");
        handlingIOQueue(sysIO, ready_queue, io_queue, flag_from_cpu);
    }

    printf("\nSystem:\n");
    printf("The wall clock time at which the simulation finished: %d\n", curWallTime);

    // TODO: print stat of sysCPU and sysIO, formulas in buildResource of process.h
    sysCPU->idle = curWallTime - sysCPU->busy;
    sysCPU->utilization = (double)sysCPU->busy / (double)(sysCPU->busy + sysCPU->idle);
    sysCPU->throughput = (double)(processes_count) / (double)(curWallTime);
    displayResource(sysCPU);

    sysIO->idle = curWallTime - sysIO->busy;
    sysIO->utilization = (double)sysIO->busy / (double)(sysIO->busy + sysIO->idle);
    sysIO->throughput = (double)(processes_count) / (double)(curWallTime);
    displayResource(sysIO);

    // LATER:
    //      todo: free processes p1,p2,p3, etc above from our hardcoding attempt

    int error = queue_destroy(ready_queue); // LATER: check if the destruction is successful
    error = queue_destroy(io_queue);
    free(sysCPU);
    free(sysIO);

    return 0; // GG
}