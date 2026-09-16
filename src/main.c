/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:41 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/16 17:44:42 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	start_end_session(t_session *session)
{
	int			i;
	int			num_coder;
	pthread_t	monitor_thread;

	pthread_create(&monitor_thread, NULL, eying_on, session);
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
	{
		i = 0;
		while (i < num_coder)
		{
			pthread_mutex_destroy(&session->dongles[i].mutex);
			pthread_cond_destroy(&session->dongles[i].condition);
			i++;
		}
		free(session->dongles);
	}
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
