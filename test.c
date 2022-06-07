#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
// const char *filename = "file1";

int main(int argc, char *argv[])
{
    printf("value: %d\n", min(1, 0));
    printf("the parenthesis (5 out of 7)\n");
    // char *filename = argv[1];
    // FILE *in_file = fopen(filename, "r");
    // if (!in_file)
    // {
    //     perror("fopen");
    //     exit(EXIT_FAILURE);
    // }

    // struct stat sb;
    // if (stat(filename, &sb) == -1)
    // {
    //     perror("stat");
    //     exit(EXIT_FAILURE);
    // }

    // char *file_contents = malloc(sb.st_size);

    // while (fscanf(in_file, "%[^\n] ", file_contents) != EOF)
    // {
    //     printf("%s\n", file_contents);
    // }

    // fclose(in_file);
    // exit(EXIT_SUCCESS);
}