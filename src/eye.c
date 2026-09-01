/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eye.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:13:36 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/01 18:47:31 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	is_he_burnedout(t_coder *coder)
{
	char	*timestamp;
	char	*coder_id_char;

	if (codexion_clock()
		- coder->last_compiled >= coder->session->parameter.time_to_burnout)
	{
		timestamp = ft_itoa(codexion_clock() - coder->session->start_time);
		coder_id_char = ft_itoa(coder->coder_id);
		write(1, timestamp, strlen(timestamp));
		write(1, " ", 1);
		write(1, coder_id_char, strlen(coder_id_char));
		write(1, " burned out\n", strlen(" burned out\n"));
		exit(EXIT_FAILURE);
	}
}

void eying_on(void *arg)
{
	t_session *session;
	int	i;
	int	N;

	session = (t_session *)arg;
	N = session->parameter.number_of_coders;

	while (1)
	{
		i = 0;
		while (i < N)
		{
			is_he_burnedout(&session->coders[i]);
			i++;
		}
		usleep(1000);
	}
}
