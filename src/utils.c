/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:11 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/28 19:23:18 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/time.h>

long codexion_clock(void)
{
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) != 0)
        return (-1);
    return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}