/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eye.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:13:36 by tkunugi           #+#    #+#             */
/*   Updated: 2026/08/30 18:25:11 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void    is_he_burnedout(t_coder *coder)
{
    if (codexion_clock() - coder->last_compiled >= coder->session->parameter.time_to_burnout)
    {
        char *timestamp = itoa(codexion_clock() - coder->session->start_time);
        char *coder_id_char = itoa(coder->coder_id);
        write(1, timestamp, strlen(timestamp));
        write(1, " ", 1);
        write(1, coder_id_char, strlen(coder));
        write(1, " burned out\n", strlen(" burned out\n"));
        exit(EXIT_FAILURE);
    }
}
