#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

int curWallTime = 0;

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

int int_rng(char *flag, int remain_running_time)
{
  int r = random();
  int generated;

  if (flag == "io")
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

int handlingReadyQueue(queue_t ready_queue, queue_t io_queue, bool *cur_running)
{
  if (cur_running && ready_queue->head->time_til_IO == 0) // ISSUE: CPU is running and time_til_IO > 0
  {                                                       // this means a process that previously did not block should run for the current clock cycle
    // [ready_queue -> head -> time_til_IO > 0 means it is going to block]    // ISSUE: really?
    ready_queue->head->value->time_til_completion--;

    if (ready_queue->head->value->time_til_completion == 0)
    { // it has reached 0 so it can exit the queue        // ISSUE: recheck the conditions
      *cur_running = false;
      dequeue(ready_queue);
    }
  }
  else if (cur_running)
  { // this means that a process is running for a certain time period before blocking
    ready_queue->head->value->time_til_IO--;

    if (ready_queue->head->value->time_til_IO == 0)
    { // it should now block (and so it gets put in IO queue)
      *cur_running = false;
      enqueue(io_queue, ready_queue->head->value);
      dequeue(ready_queue);
    }
  }
  else
  { // we will take a fresh process from ready queue
    struct process_from_input *top_process = ready_queue->head->value;
    bool block = false;

    if (top_process->time_til_completion > 2)
      block = to_block(top_process->blocking_prob);

    dequeue(ready_queue);

    if (block)
    {
      top_process->time_til_IO = int_rng("cpu", top_process->time_til_completion);
      top_process->time_til_IO--; // not too sure about this, i'm assuming it will also run for the current clock cycle

      if (top_process->time_til_IO != 0)
      {
        *cur_running = true;
      }
      else
      { // if the time_till_IO was initially just 1
        enqueue(io_queue, ready_queue->head->value);
        dequeue(ready_queue);
      }
    }
    else
    {
      top_process->time_til_completion--;
      *cur_running = true;
    }
    printf("Entering and exiting ready queue function");
  }
}

void handlingIOQueue(queue_t ready_queue, queue_t io_queue)
{
  if (io_queue->head->value->time_left_on_IO == -1)
  {
    io_queue->head->value->time_left_on_IO = int_rng("io", -1);
  }

  io_queue->head->value->time_left_on_IO--;

  if (io_queue->head->value->time_left_on_IO == 0)
  {
    enqueue(ready_queue, io_queue->head->value);
    dequeue(io_queue);
  }

  printf("Entering and exiting IO queue function");
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

  struct process_from_input *cur_running_process = NULL;
  bool cur_running_process = false;

  // queue_dequeue(ready_queue, (void **)&ptr);
  // printf("name: %s\n", ptr->name);

  printf("%d\n", queue_length(ready_queue));
  (void)srandom(12345);

  int res = int_rng("cpu", p1->time_til_completion);
  value = to_block(p2->blocking_prob);
  res = int_rng("cpu", p2->time_til_completion);
  res = int_rng("io", -1);

  // ISSUE: need to have separate values for IO and CPU as either runs in parallel and independently of one another.
  bool cur_running = false;

  while (queue_length(ready_queue) != 0 || queue_length(io_queue) != 0)
  {
    handlingReadyQueue(ready_queue, cur_running_process, &cur_running);
    handlingIOQueue(ready_queue, io_queue, &cur_running);
  }

  // LATER:
  //      todo: free processes p1,p2,p3, etc above

  int error = queue_destroy(ready_queue); // LATER: check if the destruction is successful
  error = queue_destroy(io_queue);

  return 0; // GG
}
