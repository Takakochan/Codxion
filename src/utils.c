/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:11 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/22 16:29:06 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <sys/time.h>

long	codexion_clock(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	ms_to_timespec(long ms, struct timespec *ts)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000L;
}

void	print_log(t_coder *coder, char *str)
{
	long	timestamp;
	int		total_space;

	total_space = coder->coder_id * 4;
	timestamp = codexion_clock() - coder->session->start_time;
	pthread_mutex_lock(&coder->session->session_mutex);
	if (coder->session->someone_died)
	{
		pthread_mutex_unlock(&coder->session->session_mutex);
		return ;
	}
	pthread_mutex_unlock(&coder->session->session_mutex);
	pthread_mutex_lock(&coder->session->log_mutex);
	pthread_mutex_lock(&coder->session->session_mutex);
	if (!coder->session->someone_died)
	{
		pthread_mutex_unlock(&coder->session->session_mutex);
		printf("%*s", total_space, "");
		printf("%ld %u %s\n", timestamp, coder->coder_id + 1, str);
	}
	else
		pthread_mutex_unlock(&coder->session->session_mutex);
	pthread_mutex_unlock(&coder->session->log_mutex);
}
