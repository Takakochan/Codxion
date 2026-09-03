/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 17:23:28 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/03 15:48:27 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	coder_hand(t_coder *coder)
{
	if (coder->session->parameter.number_of_coders == 1)
	{
		dongle_take(coder->left_hand, coder);
		print_log(coder, " has taken a dongle");
	}
	if (coder->left_hand->dongle_id < coder->right_hand->dongle_id)
	{
		dongle_take(coder->left_hand, coder);
		print_log(coder, " has taken a dongle");
		dongle_take(coder->right_hand, coder);
		print_log(coder, " has taken a dongle");
	}
	else
	{
		dongle_take(coder->right_hand, coder);
		print_log(coder, " has taken a dongle");
		dongle_take(coder->left_hand, coder);
		print_log(coder, " has taken a dongle");
	}
}

static void	coder_compiling(t_coder *coder)
{
	coder->compile_count ++;
	print_log(coder, " is compiling");
	usleep(coder->session->parameter.time_to_compile * 1000);
	coder->last_compiled = codexion_clock();
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
	print_log(coder, " is refactoring");
	usleep(coder->session->parameter.time_to_refactor * 1000);
}

void	*coder_workflow(void *arg)
{
	t_coder *coder = arg;
	while ((int)coder->compile_count < coder->session->parameter.number_of_compiles_required)
	{
		coder_hand(coder);
		coder_compiling(coder);
		coder_dongle_release(coder);
		coder_debugging(coder);
		coder_refactoring(coder);
		is_he_burnedout(coder);
	}
	return (NULL);
}
