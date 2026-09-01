/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 13:49:03 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/31 18:25:12 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_dongle	*init_dongles(t_parameter params, int N)
{

	t_dongle	*dongles;


	dongles = malloc(sizeof(t_dongle) * N);
	for (int i = 0; i < N; i++)
	{
		dongles[i].dongle_id = i;
		dongles[i].released_time = 0;
		dongles[i].holder = NULL;
		dongles[i].parameter = params;
		dongles[i].que.size = 0;
		pthread_mutex_init(&dongles[i].mutex, NULL); //explore!!!
		pthread_cond_init(&dongles[i].condition, NULL);
	}
	return (dongles);
}

t_coder	*init_coders(t_session *session, int N)
{
	t_coder	*coders;

	coders = malloc(sizeof(t_coder) * N);
	for (int i = 0; i < N; i++)
	{
		coders[i].coder_id = i;
		coders[i].left_hand = &session->dongles[i];
		coders[i].right_hand = &session->dongles[(i + 1) % N];
		coders[i].last_compiled = 0;
		coders[i].compile_count = 0;
		coders[i].session = session;
	}
	return (coders);
}

t_session	*init_session(t_parameter params)
{
	int			N;
	t_session	*session;

	N = params.number_of_coders;
	session = malloc(sizeof(t_session));
	if (!session)
		return (NULL);
	session->parameter = params;
	session->start_time = codexion_clock();
	session->dongles = init_dongles(params, N);
	if (!session->dongles)
		return (free(session), NULL);
	session->coders = init_coders(session, N);
	return (session);
}
