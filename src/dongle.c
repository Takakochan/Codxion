/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:01 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/26 18:35:31 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>

typedef struct dongle{
    unsigned int dongle_id;
    unsigned int cool_down;
    unsigned int in_use;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}
