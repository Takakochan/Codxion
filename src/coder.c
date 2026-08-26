/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 17:23:28 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/26 18:21:01 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <pthread.h>

typedef struct coder{
    unsigned int coder_id;
    unsigned int compile_start;
    unsigned int time_to_burnout;
    unsigned int left_hand;
    unsigned int right_hand;
}