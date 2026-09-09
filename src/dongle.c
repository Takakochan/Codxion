/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/09 16:35:09 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief 
 *
 * @param dongle
 */
static void	dongle_wait(t_dongle *dongle)
{
	long			remaining;
	struct timespec	timeout;

	if (dongle->holder)
		return ((void)pthread_cond_wait(&dongle->condition, &dongle->mutex));
	remaining = dongle->parameter.dongle_cooldown
		- (codexion_clock() - dongle->released_time);
	if (remaining < 1)
		remaining = 1;
	ms_to_timespec(codexion_clock() + remaining, &timeout);
	pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);
}

int	dongle_take(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	if (dongle->holder == coder)
		return (pthread_mutex_unlock(&dongle->mutex), 1);
	if (!enqueue(&dongle->que, coder, codexion_clock()))
		return (pthread_mutex_unlock(&dongle->mutex), 0);
	grant_next(dongle);
	while (dongle->holder != coder)
	{
		dongle_wait(dongle);
		grant_next(dongle);
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

void	dongle_release(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	if (dongle->holder == coder)
	{
		dongle->holder = NULL;
		dongle->released_time = codexion_clock();
		pthread_cond_broadcast(&dongle->condition);
	}

	pthread_mutex_unlock(&dongle->mutex);
}
