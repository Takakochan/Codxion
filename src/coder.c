/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 17:23:28 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/01 18:16:39 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	coder_hand(t_coder *coder)
{
	if (coder->left_hand->dongle_id < coder->right_hand->dongle_id)
	{
		dongle_take(coder->left_hand, coder);
		print_log(coder, " has taken a dongle\n");
		dongle_take(coder->right_hand, coder);
		print_log(coder, " has taken a dongle\n");
	}
	else
	{
		dongle_take(coder->left_hand, coder);
		print_log(coder, " has taken a dongle\n");
		dongle_take(coder->right_hand, coder);
		print_log(coder, " has taken a dongle\n");
	}
}

static void	coder_compiling(t_coder *coder)
{
	print_log(coder, " is compiling\n");
	usleep(coder->session->parameter.time_to_compile * 1000);
}

static void	coder_debugging(t_coder *coder)
{
	print_log(coder, " is debugging\n");
	usleep(coder->session->parameter.time_to_debug);
}

static void	coder_refactoring(t_coder *coder)
{
	print_log(coder, " is refactoring\n");
	usleep(coder->session->parameter.time_to_refactor);
}

void	coder_workflow(void *arg)
{
	t_coder *coder = arg;
	coder_hand(coder);
	coder_compiling(coder);
	coder_debugging(coder);
	coder_refactoring(coder);
	is_he_burnedout(coder);
}
