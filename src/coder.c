/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 17:23:28 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/29 18:09:03 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	coder_compiling(t_coder *coder)
{
	pthread_mutex_lock(&coder->session->session_mutex);
	coder->last_compiled = codexion_clock();
	pthread_mutex_unlock(&coder->session->session_mutex);
	print_log(coder, " is compiling");
	usleep(coder->session->parameter.time_to_compile * 1000);
}

static void	coder_dongle_release(t_coder *coder)
{
	dongle_release(coder->left_hand, coder);
	dongle_release(coder->right_hand, coder);
}

static void	coder_debugging(t_coder *coder)
{
	print_log(coder, " is debugging");
	usleep(coder->session->parameter.time_to_debug * 1000);
}

static void	coder_refactoring(t_coder *coder)
{
	pthread_mutex_lock(&coder->session->session_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->session->session_mutex);
	print_log(coder, " is refactoring");
	usleep(coder->session->parameter.time_to_refactor * 1000);
}

/**
 * @brief For a coder's infinite work rutin untill he reaches
 * required compilation time or someone die
 *
 * @param arg
 * @return void*
 */

void	*coder_workflow(void *arg)
{
	t_coder	*coder;
	int		is_done;
	int		has_someone_died;

	coder = arg;
	while (1)
	{
		pthread_mutex_lock(&coder->session->session_mutex);
		is_done = (coder->compile_count
				>= coder->session->parameter.number_of_compiles_required);
		has_someone_died = coder->session->someone_died;
		pthread_mutex_unlock(&coder->session->session_mutex);
		if (is_done || has_someone_died)
			break ;
		if (!coder_hand(coder))
			continue ;
		coder_compiling(coder);
		coder_dongle_release(coder);
		coder_debugging(coder);
		coder_refactoring(coder);
	}
	return (NULL);
}
