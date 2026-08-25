#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int error_num_parameter(int argc)
{
    if (argc != 9)
    {
        fprintf(stderr, "Error: Missing some parameter");
    }
    return (1);
}

int is_valid_num(char *str)
{
    int i;

    i = 0;
    if (str[i] == "\0")
    {
        fprintf(stderr, "Error: Empty input");
        exit(EXIT_FAILURE);
    }
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
        {
            fprintf(stderr, "Error: Invalid numerical input '%s'\n", str);
            exit(EXIT_FAILURE);
        }
        i++;
    }
    return (1);
}

int is_fifo_edf(char *str)
{
    if (strcmp(str, "fifo") != 0 && strcmp(str, "edf") != 0)
    {
        fprintf(stderr, "Error: Scheduler(last parameter) must be 'fifo or 'edf\n");
        exit(EXIT_FAILURE);
    }
    return (1);
}

int perser_error_handler(int argc, char **argv)
{
    int i;

    error_num_parameter(argc);
    i = 1;
    while (i <= 7)
    {
        is_valid_num(argv[i]);
        i++;
    }
    is_fifo_edf(argv[8]);
    return (1);
}

void perser(int argc, char **argv)
{
    perser_error_handler(argc, argv);
    int number_of_coders = atoi(argv[1]);
    int time_to_burnout = atoi(argv[2]);
    int time_to_compile = atoi(argv[3]);
    int time_to_debug = atoi(argv[4]);
    int time_to_refactor = atoi(argv[5]);
    int number_of_compiles_required = atoi(argv[6]);
    int dongle_cooldown = atoi(argv[7]);
    char    *scheduler = argv[8];
    is_fifo_edf(scheduler);

}