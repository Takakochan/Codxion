/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 16:09:57 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/03 14:05:19 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int queue_avoid_double(t_queue *que, t_coder *coder)
{
	int i =0;
	while(i < que->size)
	{
		if (que->items[i].coder == coder)
			return (i);
		i++;
	}
	return (-1);
}


void	enqueue(t_queue *que, t_coder *coder, long timestamp)
{
	if (que->size >= 2 || queue_avoid_double(que, coder) >= 0)
		return ;
	que->items[que->size].coder = coder;
	que->items[que->size].requested_at = timestamp;
	que->items[que->size].last_compiled = coder->last_compiled;
	que->items[que->size].compile_count = coder->compile_count;
	que->size++;
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


int pick_next(t_queue *que, const char *schedular)
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


void grant_next(t_dongle *dongle)
{
	int idx;

	if (dongle->holder || dongle->que.size == 0)
		return ;
	if (codexion_clock() - dongle->released_time < dongle->parameter.dongle_cooldown)
		return ;
	idx = pick_next(&dongle->que, dongle->parameter.scheduler);
	if (idx < 0)
		return ;
	dongle->holder = dongle->que.items[idx].coder;
	dequeue(&dongle->que, idx);
	pthread_cond_broadcast(&dongle->condition);
}


void queue_remove(t_queue *que, t_coder *coder)
{
	int idx;

	idx = queue_avoid_double(que, coder);
	if (idx >= 0)
		dequeue(que, idx);
}

