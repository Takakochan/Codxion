/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/12 15:57:07 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Waits for the dongle to become available or completes its cooldown.
 *
 * @param dongle
 */
// static void	dongle_wait(t_dongle *dongle)
// {
// 	long			remaining;
// 	struct timespec	timeout;

// 	if (dongle->holder > 0)
// 		return ((void)pthread_cond_wait(&dongle->condition, &dongle->mutex));
// 	remaining = dongle->parameter.dongle_cooldown
// 		- (codexion_clock() - dongle->released_time);
// 	if (remaining < 1)
// 		remaining = 1;
// 	ms_to_timespec(codexion_clock() + remaining, &timeout);
// 	pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);
// }

static void	dongle_wait(t_dongle *dongle)
{
	long			remaining;
	struct timespec	timeout;

	// 1. 待つ前に、すでに誰かがバーンアウトしていれば即座に終了する
	pthread_mutex_lock(&dongle->session->session_mutex);
	if (dongle->session->someone_died)
	{
		pthread_mutex_unlock(&dongle->session->session_mutex);
		return;
	}
	pthread_mutex_unlock(&dongle->session->session_mutex);

	// --- ケースA: 他のコーダーがドングルを所有している場合 ---
	// 所有者が解放（シグナル）するのを待ちつつ、10msごとに起きて死亡確認する
	while (dongle->holder != NULL)
	{
		ms_to_timespec(codexion_clock() + 10, &timeout);
		pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);

		pthread_mutex_lock(&dongle->session->session_mutex);
		if (dongle->session->someone_died)
		{
			pthread_mutex_unlock(&dongle->session->session_mutex);
			return;
		}
		pthread_mutex_unlock(&dongle->session->session_mutex);
	}

	// --- ケースB: クールダウン（残り時間）を待つ場合 ---
	remaining = dongle->parameter.dongle_cooldown
		- (codexion_clock() - dongle->released_time);
	if (remaining < 1)
		remaining = 1;

	// 残り時間がある間、最大10msごとに区切って眠る
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
		ms_to_timespec(codexion_clock() + sleep_chunk, &timeout);
		pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout);
		remaining -= sleep_chunk;
	}
}


int	dongle_take(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	if (dongle->holder)
	{
		dongle_wait(dongle);
		return (pthread_mutex_unlock(&dongle->mutex), 0);
	}
	if (!enqueue(&dongle->que, coder, codexion_clock()))
		return (pthread_mutex_unlock(&dongle->mutex), 0);
	// grant_next(dongle);
	while (dongle->holder == NULL)
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
