/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/21 19:48:11 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

// static void	dongle_wait(t_dongle *dongle)
// {
// 	long			remaining;
// 	// struct timespec	timeout;
// 	long			sleep_chunk;

// 	pthread_mutex_lock(&dongle->session->session_mutex);
// 	if (dongle->session->someone_died)
// 	{
// 		pthread_mutex_unlock(&dongle->session->session_mutex);
// 		return ;
// 	}
// 	pthread_mutex_unlock(&dongle->session->session_mutex);
// 	while (dongle->holder != NULL)
// 	{
// 		pthread_mutex_unlock(&dongle->mutex);
// 		usleep(10000); // 10ミリ秒スリープ (10 * 1000)
// 		pthread_mutex_lock(&dongle->mutex); // 起きたら再ロック
// 		// ms_to_timespec(codexion_clock() + 10, &timeout);
// 		// pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);
// 		pthread_mutex_lock(&dongle->session->session_mutex);
// 		if (dongle->session->someone_died)
// 		{
// 			pthread_mutex_unlock(&dongle->session->session_mutex);
// 			return ;
// 		}
// 		pthread_mutex_unlock(&dongle->session->session_mutex);
// 	}
// 	remaining = dongle->parameter.dongle_cooldown - (codexion_clock()
// 			- dongle->released_time);
// 	if (remaining < 1)
// 		remaining = 1;
// 	while (remaining > 0)
// 	{
// 		pthread_mutex_lock(&dongle->session->session_mutex);
// 		if (dongle->session->someone_died)
// 		{
// 			pthread_mutex_unlock(&dongle->session->session_mutex);
// 			return ;
// 		}
// 		pthread_mutex_unlock(&dongle->session->session_mutex);
// 		if (remaining > 10)
// 			sleep_chunk = 10;
// 		else
// 			sleep_chunk = remaining;
// 		pthread_mutex_unlock(&dongle->mutex);
// 		usleep(sleep_chunk * 1000); // sleep_chunk（ミリ秒）をマイクロ秒に変換
// 		pthread_mutex_lock(&dongle->mutex); // 起きたら再ロック
// 		// ms_to_timespec(codexion_clock() + sleep_chunk, &timeout);
// 		// pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);
// 		remaining -= sleep_chunk;
// 	}
// }


static void	dongle_wait(t_dongle *dongle)
{
	long			remaining;

	pthread_mutex_lock(&dongle->session->session_mutex);
	if (dongle->session->someone_died)
	{
		pthread_mutex_unlock(&dongle->session->session_mutex);
		return;
	}
	pthread_mutex_unlock(&dongle->session->session_mutex);

	while (dongle->holder != NULL)
	{
		pthread_mutex_unlock(&dongle->mutex);
		usleep(10000);
		pthread_mutex_lock(&dongle->mutex);
		pthread_mutex_lock(&dongle->session->session_mutex);
		if (dongle->session->someone_died)
		{
			pthread_mutex_unlock(&dongle->session->session_mutex);
			return;
		}
		pthread_mutex_unlock(&dongle->session->session_mutex);
	}

	remaining = dongle->parameter.dongle_cooldown
		- (codexion_clock() - dongle->released_time);
	if (remaining < 1)
		remaining = 1;

	while (remaining > 0)
	{
		pthread_mutex_lock(&dongle->session->session_mutex);
		if (dongle->session->someone_died)
		{
			pthread_mutex_unlock(&dongle->session->session_mutex);
			return;
		}
		pthread_mutex_unlock(&dongle->session->session_mutex);

		long sleep_chunk = (remaining > 10) ? 10 : remaining;
		pthread_mutex_unlock(&dongle->mutex);
		usleep(sleep_chunk * 1000);
		pthread_mutex_lock(&dongle->mutex); 
		remaining -= sleep_chunk;
	}
}
// int	dongle_take(t_dongle *dongle, t_coder *coder)
// {
// 	pthread_mutex_lock(&coder->session->session_mutex);
// 	if (coder->session->someone_died)
// 		return (pthread_mutex_unlock(&coder->session->session_mutex), 0);
// 	pthread_mutex_unlock(&coder->session->session_mutex);
// 	pthread_mutex_lock(&dongle->mutex);
// 	if (dongle->holder != NULL)
// 	{
// 		dongle_wait(dongle);
// 		return (pthread_mutex_unlock(&dongle->mutex), 0);
// 	}
// 	if (!enqueue(&dongle->que, coder, codexion_clock()))
// 		return (pthread_mutex_unlock(&dongle->mutex), 0);
// 	while (dongle->holder != coder)
// 	{
// 		pthread_mutex_lock(&coder->session->session_mutex);
// 		if (coder->session->someone_died)
// 		{
// 			pthread_mutex_unlock(&coder->session->session_mutex);
// 			return (pthread_mutex_unlock(&dongle->mutex), 0);
// 		}
// 		pthread_mutex_unlock(&coder->session->session_mutex);
// 		dongle_wait(dongle);
// 		grant_next(dongle);
// 	}
// 	pthread_mutex_unlock(&dongle->mutex);
// 	return (1);
// }

int	dongle_take(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&coder->session->session_mutex);
	if (coder->session->someone_died)
		return (pthread_mutex_unlock(&coder->session->session_mutex), 0);
	pthread_mutex_unlock(&coder->session->session_mutex);
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
