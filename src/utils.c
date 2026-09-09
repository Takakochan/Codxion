/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkunugi <takakokunugi@gmail.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 15:14:11 by tkunugi           #+#    #+#             */
/*   Updated: 2026/09/09 19:25:40 by tkunugi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>
#include <sys/time.h>

long	codexion_clock(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (-1);
	return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	ms_to_timespec(long ms, struct timespec *ts)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000L;
}

void	print_log(t_coder *coder, char *str)
{
	long	timestamp;
	int total_space;

	pthread_mutex_lock(&coder->session->log_mutex);
	total_space = coder->coder_id * 16;
	timestamp = codexion_clock() - coder->session->start_time;
	printf("%*s", total_space, "");
	printf("%ld %u %s\n", timestamp, coder->coder_id + 1, str);
	pthread_mutex_unlock(&coder->session->log_mutex);
}

void	*ft_calloc(size_t nmemb, size_t size)
{
	void	*ptr;

	if (nmemb == 0 || size == 0)
		return (malloc(0));
	ptr = (size_t *)malloc(nmemb * size);
	if (ptr == NULL)
		return (NULL);
	memset(ptr, 0, nmemb * size);
	return (ptr);
}

int	numlen(long n)
{
	int	length;

	length = 0;
	if (n <= 0)
	{
		length++;
		n = -n;
	}
	while (n != 0)
	{
		n /= 10;
		length++;
	}
	return (length);
}

char	*ft_itoa(long num)
{
	int		len;
	char	*result;

	len = numlen(num);
	result = (char *)ft_calloc(len + 1, sizeof(char));
	if (!result)
		return (NULL);
	if (num == 0)
		result[0] = '0';
	if (num < 0)
	{
		result[0] = '-';
		num = -num;
	}
	while (num > 0)
	{
		result[--len] = (num % 10) + '0';
		num /= 10;
	}
	return (result);
}
