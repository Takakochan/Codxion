/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 13:49:03 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/16 18:07:22 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_dongle	*init_dongles(t_parameter params, t_session *session, int num_coder)
{
	t_dongle	*dongles;
	int			i;

	i = 0;
	dongles = malloc(sizeof(t_dongle) * num_coder);
	while (i < num_coder)
	{
		dongles[i].dongle_id = i;
		dongles[i].released_time = 0;
		dongles[i].holder = NULL;
		dongles[i].parameter = params;
		dongles[i].que.size = 0;
		dongles[i].session = session;
		pthread_mutex_init(&dongles[i].mutex, NULL);
		pthread_cond_init(&dongles[i].condition, NULL);
		i++;
	}
	return (dongles);
}

t_coder	*init_coders(t_session *session, int num_coder)
{
	t_coder	*coders;
	int		i;

	i = 0;
	coders = malloc(sizeof(t_coder) * num_coder);
	while (i < num_coder)
	{
		coders[i].coder_id = i;
		coders[i].left_hand = &session->dongles[i];
		coders[i].right_hand = &session->dongles[(i + 1) % num_coder];
		coders[i].last_compiled = session->start_time;
		coders[i].compile_count = 0;
		coders[i].session = session;
		i++;
	}
	return (coders);
}

t_session	*init_session(t_parameter params)
{
	int			num_coder;
	t_session	*session;

	num_coder = params.number_of_coders;
	session = malloc(sizeof(t_session));
	if (!session)
		return (NULL);
	session->parameter = params;
	session->start_time = codexion_clock();
	session->dongles = init_dongles(params, session, num_coder);
	if (!session->dongles)
		return (free(session), NULL);
	session->coders = init_coders(session, num_coder);
	session->someone_died = 0;
	pthread_mutex_init(&session->session_mutex, NULL);
	pthread_mutex_init(&session->log_mutex, NULL);
	return (session);
}
