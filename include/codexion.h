/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:50 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/27 20:50:19 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include "main.c"

typedef struct s_parameter{
    int number_of_coders;
    int time_to_burnout;
    int time_to_compile;
    int time_to_debug;
    int time_to_refactor;
    int number_of_compiles_required;
    int dongle_cooldown;
    char    *scheduler;
} t_parameter;


typedef struct s_dongle{
    unsigned int dongle_id;
    long released_time;
    t_parameter s_parameter;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    
} t_dongle;

typedef struct s_coder{
    unsigned int coder_id;
    t_dongle *left_hand;
    t_dongle *right_hand;

    t_parameter parameter;
} t_coder;



int main(int argc, char **argv);

int error_num_parameter(int argc);
int is_valid_num(char *str);
int is_fifo_edf(char *str);
int parser_error_handler(int argc, char **argv);
t_parameter parser(int argc, char **argv);

#endif
