# ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>


int main(int argc, char **argv)


int error_num_parameter(int argc);
int is_valid_num(char *str);
int is_fifo_edf(char *str);
int perser_error_handler(int argc, char **argv);
void perser(int argc, char **argv);
