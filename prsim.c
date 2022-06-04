#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

int curWallTime = 1; // The clock time (ticks) start at 1

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

// TODO lower priority: implement handlingReadyQueue and handlingIOQueue
void handlingReadyQueue(queue_t ready_queue)
{
    printf("Entering and exiting ready queue function");
}
void handlingIOQueue(queue_t io_queue)
{
    printf("Entering and exiting IO queue function");
}

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

    if (strcmp(flag, "CPU"))
    {
        printf("hi io");
        generated = r % 30 + 1;
        printf("\trandom number (raw, type, value used:\t%d i %d\n", r, generated);
        return generated;
    }
    else if (flag == "cpu")
    {
        printf("hi cpu");
        generated = r % remain_running_time + 1;
        printf("\trandom number (raw, type, value used:\t%d i %d\n", r, generated);
        return generated;
    }
    else
    {
        fprintf(stderr, "ERROR: wrong flag");
        exit(EXIT_FAILURE);
    }
}

int handlingReadyQueue(struct resource *sysCPU, queue_t ready_queue, queue_t io_queue)
{
    printf("VIEW: Entering function for ready queue");
    if (sysCPU->cur_running && ready_queue->head->time_til_IO == 0) // ISSUE: CPU is running and time_til_IO > 0
    {                                                               // this means a process that previously did not block should run for the current clock cycle
        // [ready_queue -> head -> time_til_IO > 0 means it is going to block]    // ISSUE: really?
        ready_queue->head->value->time_til_completion--;

        if (ready_queue->head->value->time_til_completion == 0)
        { // it has reached 0 so it can exit the queue        // ISSUE: recheck the conditions
            sysCPU->cur_running = false;
            dequeue(ready_queue);
        }
    }
    else if (sysCPU->cur_running)
    { // this means that a process is running for a certain time period before blocking
        ready_queue->head->value->time_til_IO--;

        if (ready_queue->head->value->time_til_IO == 0)
        { // it should now block (and so it gets put in IO queue)
            sysCPU->cur_running = false;
            enqueue(io_queue, ready_queue->head->value);
            dequeue(ready_queue);
        }
    }
    else
    {
        // we will take a fresh process from ready queue
        // TODO: count as 1 dispatch in CPU stat?

        printf("VIEW: CPU idle and process %s on ready queue, load it into CPU");
        // printf("Moving process %s to CPU", process_name); // WHEN?

        // for this project, I'll just assume that the head of the queue is in CPU when CPU isn't idle
        struct process_from_input *top_process = ready_queue->head;

        bool block = false;
        if (top_process->time_til_completion > 2)
        {
            printf("VIEW: process %s, run time %d, run time remaining %d; may block for I/O", top_process->name, top_process->totalCPU, top_process->time_til_completion);
            printf("VIEW: generating floating-point random number; probability of blocking is %f", top_process->blocking_prob);
            block = to_block(top_process->blocking_prob);
        }
        else
            printf("VIEW: process %s, run time %d, run time remaining %d; will not block for I/O", top_process->name, top_process->totalCPU, top_process->time_til_completion);

        dequeue(ready_queue);

        if (block)
        {
            printf("VIEW: Process will block for I/O");
            top_process->time_til_IO = int_rng("cpu", top_process->time_til_completion);
            printf("VIEW: Process %s moved to CPU", top_process->name);
            printf("VIEW: Process %s on CPU with remaining run time %d (out of %d)", top_process->name, top_process->time_til_completion, top_process->totalCPU);
            top_process->time_til_IO--; // not too sure about this, i'm assuming it will also run for the current clock cycle: OK
            top_process->time_til_completion--;

            if (top_process->time_til_IO != 0)
            {
                sysCPU->cur_running = true;
            }
            else
            { // if the time_till_IO was initially just 1
                enqueue(io_queue, ready_queue->head->value);
                dequeue(ready_queue);
            }
        }
        else
        {
            printf("VIEW: process will not block for I/O at all);
            top_process->time_til_completion--;
            *cur_running = true;
        }
    }
}

void handlingIOQueue(int flag, queue_t ready_queue, queue_t io_queue)
{
    printf("VIEW: Entering function for I/O queue");
    struct process_from_input *io_Q_head = io_queue->head;
    struct process_from_input *io_CPU_head = ready_queue->head;

    if (io_Q_head->time_left_on_IO == -1)
    {
        io_Q_head->time_left_on_IO = int_rng("io", -1);
    }

    io_Q_head->time_left_on_IO--;

    if (io_Q_head->time_left_on_IO == 0)
    {
        enqueue(ready_queue, io_Q_head);
        dequeue(io_queue);
    }
}

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

    // editor
    struct process_from_input *p1 = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    p1->name = "editor";
    p1->totalCPU = 5;
    p1->blocking_prob = 0.87;
    p1->time_til_completion = 5;
    p1->time_til_IO = 0;
    p1->time_left_on_IO = 0;

    // compiler
    struct process_from_input *p2 = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    p2->name = "compiler";
    p2->totalCPU = 40;
    p2->blocking_prob = 0.53;
    p2->time_til_completion = 40;
    p2->time_til_IO = 0;
    p2->time_left_on_IO = 0;

    // adventure
    struct process_from_input *p3 = (struct process_from_input *)malloc(sizeof(struct process_from_input));
    p3->name = "adventure";
    p3->totalCPU = 30;
    p3->blocking_prob = 0.72;
    p3->time_til_completion = 30;
    p3->time_til_IO = 0;
    p3->time_left_on_IO = 0;

    struct process_from_input *ptr; // just to test enqueue, dequeue
    queue_t ready_queue, io_queue;
    ready_queue = queue_create();
    io_queue = queue_create();

    queue_enqueue(ready_queue, p1);
    queue_enqueue(ready_queue, p2);
    queue_enqueue(ready_queue, p3);

    // queue_dequeue(ready_queue, (void **)&ptr);
    // printf("name: %s\n", ptr->name);

    printf("%d\n", queue_length(ready_queue));
    (void)srandom(12345);
    bool value = to_block(p1->blocking_prob);
    if (value)
    {
        printf("Process will block for I/O\n");
    }
    else
    {
        printf("Process will NOT block for I/O\n");
    }
    int res = int_rng("cpu", p1->time_til_completion);
    value = to_block(p2->blocking_prob);
    res = int_rng("cpu", p2->time_til_completion);
    res = int_rng("io", -1);

    struct resource *sysCPU, *sysIO;
    sysCPU = buildResource("CPU", 0, 0, 0); // 0 time for busy, idle, and number of dispatches
    sysIO = buildResource("IO", 0, 0, 0);   // 0 time for busy, idle, and number of dispatches

    // while (queue_length(ready_queue) != 0 || queue_length(io_queue) != 0)
    // {
    //      printf("TICK = %s |||||||||||||||||||||||||||||||||||||||||||||||||||||||\n", curWallTime);
    //      printf("RUNNING READY QUEUE --------------------------------------");
    //     handlingReadyQueue(ready_queue);
    // printf("RUNNING I/O QUEUE --------------------------------------");
    //     handlingIOQueue(io_queue);
    //      curWallTime++;
    // }

    // print stat of sysCPU and sysIO

    // LATER:
    //      todo: free processes p1,p2,p3, etc above

    int error = queue_destroy(ready_queue); // LATER: check if the destruction is successful
    error = queue_destroy(io_queue);
    free(sysCPU);
    free(sysIO);

    return 0; // GG
}