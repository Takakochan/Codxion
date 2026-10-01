/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:15:30 by tkunugi           #+#    #+#             */
/*   Updated: 2026/10/01 15:39:53 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_num(char *str, const char *name)
{
	int	i;

	i = 0;
	if (str[i] == '\0')
	{
		fprintf(stderr, "Error: %s is empty\n", name);
		exit(EXIT_FAILURE);
	}
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
		{
			fprintf(stderr, "Error: %s has an invalid numerical value '%s'\n",
				name, str);
			exit(EXIT_FAILURE);
		}
		i++;
	}
	return (1);
}

int	is_fifo_edf(char *str)
{
	if (strcmp(str, "fifo") != 0 && strcmp(str, "edf") != 0)
	{
		fprintf(stderr,
			"Error: Scheduler(last parameter) must be 'fifo or 'edf\n");
		exit(EXIT_FAILURE);
	}
	return (1);
}

int	num_coders(char *str)
{
	if (atoi(str) == 0)
	{
		fprintf(stderr, "Error: Number of coder must be greater than 0");
		exit(EXIT_FAILURE);
	}
	return (1);
}

int	parser_error_handler(int argc, char **argv)
{
	int			i;
	const char	*names[8] = {
		"", "number_of_coders", "time_to_burnout", "time_to_compile",
		"time_to_debug", "time_to_refactor", "number_of_compiles_required",
		"dongle_cooldown"};

	error_num_parameter(argc);
	i = 1;
	while (i <= 7)
	{
		is_valid_num(argv[i], names[i]);
		i++;
	}
	num_coders(argv[1]);
	is_fifo_edf(argv[8]);
	return (1);
}

t_parameter	parser_init_param(int argc, char **argv)
{
	t_parameter	param;

	parser_error_handler(argc, argv);
	param.number_of_coders = atoi(argv[1]);
	param.time_to_burnout = atoi(argv[2]);
	param.time_to_compile = atoi(argv[3]);
	param.time_to_debug = atoi(argv[4]);
	param.time_to_refactor = atoi(argv[5]);
	param.number_of_compiles_required = atoi(argv[6]);
	param.dongle_cooldown = atoi(argv[7]);
	param.scheduler = argv[8];
	is_fifo_edf(param.scheduler);
	return (param);
}
