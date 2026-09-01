/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:41 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/01 18:17:26 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void start_session(t_session *session)
{
	int i;
	int N;

	N = session->parameter.number_of_coders;
	i = 0;
	while (i < N)
	{
		pthread_create(&session->coders[i].thread, NULL, coder_workflow, &session->coders[i]);
		pthread_create(&session->coders[i].thread, NULL, )
		i++;
	}
	i = 0;
	while (i < N)
	{
		pthread_join(session->coders[i].thread, NULL);
		i++;
	}
}


int	main(int argc, char **argv)
{
	t_parameter	params;
	t_session	*session;

	params = parser_init_param(argc, argv);
	session = init_session(params);
	if (!session)
		return (1);


}

