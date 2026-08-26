/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   codexion.h                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: tkunugi <takakokunugi@gmail.com>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/26 15:14:50 by tkunugi       #+#    #+#                 */
/*   Updated: 2026/08/26 15:14:51 by tkunugi       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

# ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>


typedef struct coder{
    int coder_id;
    int burnout;
    int compile;
    int debug;
    int refactor;
}

typedef struct dongle{
    int dongle_id;
    int cool_down;


}


int main(int argc, char **argv);

int error_num_parameter(int argc);
int is_valid_num(char *str);
int is_fifo_edf(char *str);
int perser_error_handler(int argc, char **argv);
void perser(int argc, char **argv);
