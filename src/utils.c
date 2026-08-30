/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:11 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/30 16:39:46 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/time.h>
#include "codexion.h"

long codexion_clock(void)
{
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) != 0)
        return (-1);
    return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void ms_to_timespec(long ms, struct timespec *ts)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000L;
}