/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:41 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/29 18:13:04 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_dongles(int num_coder, t_session *session)
{
	int	i;

	i = 0;
	while (i < num_coder)
	{
		pthread_mutex_destroy(&session->dongles[i].mutex);
		pthread_cond_destroy(&session->dongles[i].condition);
		i++;
	}
	free(session->dongles);
}

void	start_end_session(t_session *session)
{
	int			i;
	int			num_coder;
	pthread_t	monitor_thread;

	pthread_create(&monitor_thread, NULL, watching, session);
	num_coder = session->parameter.number_of_coders;
	i = 0;
	while (i < num_coder)
	{
		pthread_create(&session->coders[i].thread,
			NULL, coder_workflow, &session->coders[i]);
		i++;
	}
	i = 0;
	while (i < num_coder)
	{
		pthread_join(session->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor_thread, NULL);
	if (session->dongles)
		free_dongles(num_coder, session);
	if (session->coders)
		free(session->coders);
	free(session);
}

int	main(int argc, char **argv)
{
	t_parameter	params;
	t_session	*session;

	params = parser_init_param(argc, argv);
	session = init_session(params);
	if (!session)
		return (1);
	start_end_session(session);
	return (0);
}
