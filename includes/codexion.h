/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:50 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/30 18:35:22 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

typedef struct s_parameter
{
	int						number_of_coders;
	int						time_to_burnout;
	int						time_to_compile;
	int						time_to_debug;
	int						time_to_refactor;
	int						number_of_compiles_required;
	int						dongle_cooldown;
	char					*scheduler;
}							t_parameter;

typedef struct s_dongle
{
	unsigned int			dongle_id;
	unsigned int			used;
	long					released_time;
	t_parameter				parameter;
	pthread_mutex_t			mutex;
	pthread_cond_t			condition;
}							t_dongle;

typedef struct s_session	t_session;
typedef struct s_coder
{
	unsigned int			coder_id;
	t_dongle				*left_hand;
	t_dongle				*right_hand;
	long					last_compiled;
	unsigned int			compile_count;
	pthread_cond_t			condition;
	t_session				*session;
}							t_coder;

typedef struct s_session
{
	t_parameter				parameter;
	t_dongle				*dongles;
	t_coder					*coders;
	long					start_time;
}							t_session;

/*main*/

int							main(int argc, char **argv);

/*parser*/
int							error_num_parameter(int argc);
int							is_valid_num(char *str);
int							is_fifo_edf(char *str);
int							parser_error_handler(int argc, char **argv);

/*parser_init*/
t_parameter					parser_init_param(int argc, char **argv);

/*init*/
t_dongle					*init_dongles(t_parameter params, int N);
t_coder						*init_coders(t_session *session, int N);
t_session					*init_session(t_parameter params);

/* utils*/
long						codexion_clock(void);
void						ms_to_timespec(long ms, struct timespec *ts);
void						print_log(t_coder *coder, char *str);
void						*ft_calloc(size_t nmemb, size_t size);
int							numlen(long n);
char						*ft_itoa(long num);

/* dongle */
int							dongle_take(t_dongle *dongle);
void						dongle_release(t_dongle *dongle);

/* coder */
void						coder_workflow(t_coder *coder);

/* eye (monitor)*/
void						is_he_burnedout(t_coder *coder);

#endif