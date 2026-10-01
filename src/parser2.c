/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/01 15:38:33 by tkunugi           #+#    #+#             */
/*   Updated: 2026/10/01 15:38:34 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	error_num_parameter(int argc)
{
	if (argc < 9)
	{
		fprintf(stderr, "Error: Missing some parameter\n");
		exit(EXIT_FAILURE);
	}
	if (argc > 9)
	{
		fprintf(stderr, "Error: Too many parameters\n");
		exit(EXIT_FAILURE);
	}
	return (1);
}
