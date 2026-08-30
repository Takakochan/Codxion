/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/30 16:47:05 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#include "codexion.h"

// typedef struct s_dongle{
//     unsigned int dongle_id;
//     unsigned int used;
//     long released_time;
//     t_parameter parameter;
//     pthread_mutex_t mutex;
//     pthread_cond_t condition;
// } t_dongle;

int dongle_take(t_dongle *dongle)
{
    long remaining;
    struct timespec timeout;

    pthread_mutex_lock(&dongle->mutex);
    while (dongle->used || codexion_clock() - dongle->released_time < dongle->parameter.dongle_cooldown)
    {
        remaining = dongle->parameter.dongle_cooldown = (codexion_clock() - dongle->released_time);
        if (dongle->used || remaining <= 0)
            pthread_cond_wait(&dongle->condition, &dongle->mutex);
        else
        {
            ms_to_timespec(codexion_clock() + remaining, &timeout);
            pthread_cond_timedwait(&dongle->condition, &dongle->mutex, &timeout); 
        }
    }
    dongle->used = 1;
    pthread_mutex_unlock(&dongle->mutex);
    return (1);
}

void dongle_release(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    dongle->used = 0;
    dongle->released_time = codexion_clock();
    pthread_cond_broadcast(&dongle->condition);
    pthread_mutex_unlock(&dongle->mutex);
}

