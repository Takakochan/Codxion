/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eye.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:13:36 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/03 17:22:34 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	is_he_burnedout(t_coder *coder)
{
	char	*timestamp;
	char	*coder_id_char;



	if (codexion_clock() - coder->last_compiled >= coder->last_compiled)
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

int is_all_coder_done(t_session *session)
{
	int i;
	int count;
	i = 0;
	count = 0;
	while(i < session->parameter.number_of_coders)
	{
		// printf("%d\n", i);
		if(session->coders[i].compile_count == session->parameter.number_of_compiles_required)
		{
			count ++;
			// printf("%d", session->coders[i].compile_count);
		}
		i++;
	}
	if (count == session->parameter.number_of_coders)
		return (1);
	return (0);
}

void *eying_on(void *arg)
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
		if (is_all_coder_done(session))
				break;
		usleep(1000);
	}
	return (NULL);
}
