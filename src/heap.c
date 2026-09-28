/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 16:09:57 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/28 17:04:39 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	queue_avoid_double(t_queue *que, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < que->size)
	{
		if (que->items[i].coder == coder)
			return (i);
		i++;
	}
	return (-1);
}

int	enqueue(t_queue *que, t_coder *coder, long timestamp)
{
	if (queue_avoid_double(que, coder) >= 0)
		return (1);
	else if (que->size >= 2)
		return (0);
	que->items[que->size].coder = coder;
	que->items[que->size].requested_at = timestamp;
	pthread_mutex_lock(&coder->session->session_mutex);
	que->items[que->size].last_compiled = coder->last_compiled;
	pthread_mutex_unlock(&coder->session->session_mutex);
	que->size++;
	return (1);
}

void	dequeue(t_queue *que, int idx)
{
	while (idx + 1 < que->size)
	{
		que->items[idx] = que->items[idx + 1];
		idx++;
	}
	que->size--;
}

int	pick_next(t_queue *que, const char *schedular)
{
	if (que->size == 0)
		return (-1);
	if (que->size == 1)
		return (0);
	if (!strcmp(schedular, "fifo"))
	{
		if (que->items[0].requested_at < que->items[1].requested_at)
			return (0);
		return (1);
	}
	else
	{
		if (que->items[0].last_compiled < que->items[1].last_compiled)
			return (0);
		return (1);
	}
}

void	grant_next(t_dongle *dongle)
{
	int	idx;

	if (dongle->holder || dongle->que.size == 0)
		return ;
	if (codexion_clock()
		- dongle->released_time < dongle->parameter.dongle_cooldown)
		return ;
	idx = pick_next(&dongle->que, dongle->parameter.scheduler);
	if (idx == -1)
		return ;
	dongle->holder = dongle->que.items[idx].coder;
	dequeue(&dongle->que, idx);
	pthread_cond_broadcast(&dongle->condition);
}

void	queue_remove(t_queue *que, t_coder *coder)
{
	int	idx;

	idx = queue_avoid_double(que, coder);
	if (idx >= 0)
		dequeue(que, idx);
}
