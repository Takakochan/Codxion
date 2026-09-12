/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 17:23:28 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/12 16:47:33 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_hand(t_coder *coder)
{
	int first;
	int	second;

	if (coder->left_hand->dongle_id < coder->right_hand->dongle_id)
	{
		first = dongle_take(coder->left_hand, coder);
		if (first)
			print_log(coder, " has taken a dongle");
		second = dongle_take(coder->right_hand, coder);
		if (second)
			print_log(coder, " has taken a dongle");
	}
	else
	{
		first = dongle_take(coder->right_hand, coder);
		if (first)
			print_log(coder, " has taken a dongle");
		second = dongle_take(coder->left_hand, coder);
		if (second)
			print_log(coder, " has taken a dongle");
	}
	if (first && second)
		return (1);
	dongle_release(coder->left_hand, coder);
	dongle_release(coder->right_hand, coder);
	return (0);

}

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
	coder->compile_count ++;
	pthread_mutex_unlock(&coder->session->session_mutex);
	usleep(coder->session->parameter.time_to_refactor * 1000);
}


int	should_coder_stop(t_coder *coder)
{
	int	stop;

	pthread_mutex_lock(&coder->session->session_mutex);
	if (coder->compile_count >= coder->session->parameter.number_of_compiles_required ||
		coder->session->someone_died == 1)
		stop = 1;
	else
		stop = 0;

	pthread_mutex_unlock(&coder->session->session_mutex);
	return (stop);
}

void	*coder_workflow(void *arg)
{
	t_coder *coder = arg;
	while (!should_coder_stop(coder))
	{
		if (!coder_hand(coder))
			continue;
		coder_compiling(coder);
		coder_dongle_release(coder);
		coder_debugging(coder);
		coder_refactoring(coder);
	}
	return (NULL);
}
