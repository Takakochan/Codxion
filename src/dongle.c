/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/03 14:37:00 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>


int	dongle_take(t_dongle *dongle, t_coder *coder)
{
	long			remaining;
	struct timespec	timeout;

	pthread_mutex_lock(&dongle->mutex);
	enqueue(&dongle->que, coder, codexion_clock());
	grant_next(dongle);
	while (dongle->holder != coder)
	{
		remaining = dongle->parameter.dongle_cooldown - (codexion_clock()
				- dongle->released_time);
		if (dongle->holder || remaining <= 0)
			pthread_cond_wait(&dongle->condition, &dongle->mutex);
		else
		{
			ms_to_timespec(codexion_clock() + remaining, &timeout);
			pthread_cond_timedwait(&dongle->condition, &dongle->mutex,
					&timeout);
		}
	}
	dongle->holder = coder;
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
