// un needed code that I removed while developing

struct process_from_input // for printing stat at the end. TO BE REMOVED
{
    char *name;       // Process name; it is not to be longer than 10 characters. If it is, give an error and exit.
    int totalCPU;     // Its total CPU time (as read) this is the total CPU time needed for the process to complete. If it is not positive, give an error and exit
    int completeTime; // The wall clock time at which it was completed (this is the value of the counter, not the time of day)
    int givenCPU;     // # of times that it was given the CPU
    int BlockedIO;    // # of times blocked for IO
    int doingIO;      // time spent on IO
};

void line_parser(char *line);
void line_check_correctness(char *line_components[3]);

void line_parser(char *line)
{
    char *line_components[3]; // procname, runtime, blockprob

    int count = 0; // goes to 2 as we have 3 columns total
    for (int i = 0; i < strlen(line); i++)
    {
        printf("%c\n", line[i]);
    }
}

// printf("%f\n", p1->blocking_prob);

/*
 *
 */