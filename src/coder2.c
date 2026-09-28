/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 20:06:00 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/28 20:12:21 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_hand_extra(t_coder *coder, int first, int second)
{
	if (first && second)
		return (1);
	dongle_release(coder->left_hand, coder);
	dongle_release(coder->right_hand, coder);
	return (0);
}

int	coder_hand(t_coder *coder)
{
	int	first;
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
	return (coder_hand_extra(coder, first, second));
}
