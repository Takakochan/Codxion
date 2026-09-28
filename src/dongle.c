/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/28 19:19:13 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

static void	dongle_wait_remain(t_dongle *dongle)
{
	long	remaining;
	long	sleep_chunk;

	remaining = dongle->parameter.dongle_cooldown - (codexion_clock()
			- dongle->released_time);
	if (remaining < 1)
		remaining = 1;
	while (remaining > 0)
	{
		pthread_mutex_lock(&dongle->session->session_mutex);
		if (dongle->session->someone_died)
		{
			pthread_mutex_unlock(&dongle->session->session_mutex);
			return ;
		}
		pthread_mutex_unlock(&dongle->session->session_mutex);
		if (remaining > 10)
			sleep_chunk = 10;
		else
			sleep_chunk = remaining;
		pthread_mutex_unlock(&dongle->mutex);
		usleep(sleep_chunk * 1000);
		pthread_mutex_lock(&dongle->mutex);
		remaining -= sleep_chunk;
	}
}

/**
 * @brief Must be called with dongle->mutex already held.
 * 			if the dongle currently has a holder, repeatedly
 *        releases dongle->mutex, sleeps in
 *        short slices, and relocks it, re-checking
 *        someone_died each time(<=10ms). If the
 *        dongle is free, waits remains of its cooldown in helper function.
 *
 * @param dongle The dongle to wait on.
 */
static void	dongle_wait(t_dongle *dongle)
{
	while (dongle->holder != NULL)
	{
		pthread_mutex_unlock(&dongle->mutex);
		usleep(10000);
		pthread_mutex_lock(&dongle->mutex);
		pthread_mutex_lock(&dongle->session->session_mutex);
		if (dongle->session->someone_died)
		{
			pthread_mutex_unlock(&dongle->session->session_mutex);
			return ;
		}
		pthread_mutex_unlock(&dongle->session->session_mutex);
	}
	return (dongle_wait_remain(dongle));
}

static int	is_someone_dead(t_session *session)
{
	int	died;

	pthread_mutex_lock(&session->session_mutex);
	died = session->someone_died;
	pthread_mutex_unlock(&session->session_mutex);
	return (died);
}

/**
 * @brief check that no one has burned out yet. If dongle is
 * 		  already held by coder itself, let him giveup in case of
 *        one coder in the game let him burn out instead.
 *        Otherwise queues coder until coder is actually
 *        the holder (dongle->holder == coder).
 *        B only the coder actually selected by
 *        grant_next can ever exit this loop.
 * @param dongle aimed dongle.
 * @param coder coder aming the dongle.
 * @return 1 if coder now holds the dongle, 0 if already held by coder itself, or
 *         someone has burned out.
 */
int	dongle_take(t_dongle *dongle, t_coder *coder)
{
	if (is_someone_dead(coder->session))
		return (0);
	pthread_mutex_lock(&dongle->mutex);
	if (dongle->holder == coder)
	{
		dongle_wait(dongle);
		return (pthread_mutex_unlock(&dongle->mutex), 0);
	}
	if (!enqueue(&dongle->que, coder, codexion_clock()))
		return (pthread_mutex_unlock(&dongle->mutex), 0);
	while (dongle->holder != coder)
	{
		pthread_mutex_lock(&coder->session->session_mutex);
		if (coder->session->someone_died)
		{
			pthread_mutex_unlock(&coder->session->session_mutex);
			return (pthread_mutex_unlock(&dongle->mutex), 0);
		}
		pthread_mutex_unlock(&coder->session->session_mutex);
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
