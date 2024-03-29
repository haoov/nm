/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsabbah <rsabbah@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 15:17:09 by rsabbah           #+#    #+#             */
/*   Updated: 2022/11/08 15:29:00 by rsabbah          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char    *ft_strchr(const char *s, int c)
{
	size_t  i;
	size_t  len;

	i = 0;
	len = ft_strlen(s);
	while (i <= len)
	{
			if (s[i] == (char)c)
					return ((char *)&s[i]);
			i++;
	}
	return (NULL);
}