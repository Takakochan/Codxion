/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 15:25:48 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/29 15:27:40 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
