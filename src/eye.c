/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eye.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:13:36 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/29 18:46:27 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_he_burnedout(t_coder *coder)
{
	pthread_mutex_lock(&coder->session->session_mutex);
	if (coder->compile_count
		>= coder->session->parameter.number_of_compiles_required)
	{
		pthread_mutex_unlock(&coder->session->session_mutex);
		return (0);
	}
	if (codexion_clock()
		- coder->last_compiled >= coder->session->parameter.time_to_burnout)
	{
		coder->session->someone_died = 1;
		pthread_mutex_unlock(&coder->session->session_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->session->session_mutex);
	return (0);
}

int	is_all_coder_done(t_session *session)
{
	int	i;
	int	count;

	i = 0;
	count = 0;
	pthread_mutex_lock(&session->session_mutex);
	while (i < session->parameter.number_of_coders)
	{
		if (session->coders[i].compile_count
			== session->parameter.number_of_compiles_required)
			count++;
		i++;
	}
	pthread_mutex_unlock(&session->session_mutex);
	if (count == session->parameter.number_of_coders)
		return (1);
	return (0);
}

static void	write_output(t_coder *coder, char *timestamp, char *coder_id_char)
{
	pthread_mutex_lock(&coder->session->log_mutex);
	write(1, timestamp, strlen(timestamp));
	write(1, " ", 1);
	write(1, coder_id_char, strlen(coder_id_char));
	write(1, " burned out\n", strlen(" burned out\n"));
	pthread_mutex_unlock(&coder->session->log_mutex);
}

void	burned_out(t_coder *coder)
{
	char	*timestamp;
	char	*coder_id_char;
	int		num_coder;
	int		i;

	timestamp = ft_itoa(codexion_clock() - coder->session->start_time);
	coder_id_char = ft_itoa(coder->coder_id + 1);
	write_output(coder, timestamp, coder_id_char);
	pthread_mutex_lock(&coder->session->session_mutex);
	coder->session->someone_died = 1;
	pthread_mutex_unlock(&coder->session->session_mutex);
	free(timestamp);
	free(coder_id_char);
	num_coder = coder->session->parameter.number_of_coders;
	i = 0;
	pthread_mutex_lock(&coder->session->log_mutex);
	while (i < num_coder)
	{
		pthread_mutex_lock(&coder->session->dongles[i].mutex);
		pthread_cond_broadcast(&coder->session->dongles[i].condition);
		pthread_mutex_unlock(&coder->session->dongles[i].mutex);
		i++;
	}
	pthread_mutex_unlock(&coder->session->log_mutex);
}

void	*watching(void *arg)
{
	t_session	*session;
	int			i;
	int			num_coder;

	session = (t_session *)arg;
	num_coder = session->parameter.number_of_coders;
	while (1)
	{
		i = 0;
		while (i < num_coder)
		{
			if (is_he_burnedout(&session->coders[i]))
				return (burned_out(&session->coders[i]), NULL);
			i++;
		}
		if (is_all_coder_done(session))
		{
			pthread_mutex_lock(&session->log_mutex);
			write(1, "finished\n", strlen("finished\n"));
			pthread_mutex_unlock(&session->log_mutex);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
