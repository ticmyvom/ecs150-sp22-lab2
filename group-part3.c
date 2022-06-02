#include<stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

int curWallTime = 0;

struct CPU { // current job in CPU
  int busy; // status, 0 = false, 1 = true
  int time; // time at which current job will stop running
};

struct IO {
  int busy; // status, 0 = false, 1 = true
  int time; // time at which current job will stop running
};

typedef struct Job {
  char* name;
  int priority;
  float probability; // probability of blocking
  int totalTime; // time it has to run
  int timeLeft; // time left to run
} Job;

typedef struct CPU_Queue_Node {
  Job* job;
  struct CPU_Queue_Node* next;
} CPU_Node;

void enqueue(CPU_Node **head, Job* job) {
  CPU_Node *new_node = malloc(sizeof(CPU_Node));

  if (!new_node) return;
  new_node->job = job;
  new_node->next = *head;

  *head = new_node;
}

Job* dequeue(CPU_Node **head) {
  CPU_Node *current, *prev = NULL;
  Job *retval = NULL;

  if (*head == NULL)
    return NULL;

  current = *head;

  while (current->next != NULL) {
    prev = current;
    current = current->next;
  }

  retval = malloc(sizeof(CPU_Node));
  retval -> name = current -> job -> name;

  free(current);

  if (prev)
    prev->next = NULL;
  else
    *head = NULL;

  return retval;
}

void print_list(CPU_Node **head) {
  CPU_Node *current = *head;
  while (current != NULL) {
    printf("%s\n", current->job->name);
    current = current->next;
  }
}

int main () {
  CPU_Node *head = NULL;

  Job* job1 = malloc(sizeof(Job));
  job1 -> name = "Job1";

  Job* job2 = malloc(sizeof(Job));
  job2 -> name = "Job2";

  enqueue(&head, job1);
  enqueue(&head, job2);

  Job* get = dequeue(&head);
  printf("%s\n", get -> name);

  get = dequeue(&head);
  printf("%s\n", get -> name);
  
  print_list(&head);
}
