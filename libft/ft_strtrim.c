/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/09 15:47:47 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 17:44:55 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int		ft_beglen(char const *s, char const *set)
{
	int		len;
	int		j;

	len = 0;
	while (*s)
	{
		j = 0;
		while (set[j])
		{
			if (*s != set[j])
				j++;
			else
			{
				len++;
				break ;
			}
		}
		if (!set[j])
			break ;
		s++;
	}
	return (len);
}

static int		ft_endlen(char const *s, char const *set, int stop)
{
	int		len;
	int		i;
	int		j;

	len = 0;
	i = ft_strlen(s) - 1;
	while (i > stop)
	{
		j = 0;
		while (set[j])
		{
			if (s[i] != set[j])
				j++;
			else
			{
				len++;
				break ;
			}
		}
		if (!set[j])
			break ;
		i--;
	}
	return (len);
}

static int		ft_finallen(size_t len, int beglen, int endlen)
{
	return (len - beglen - endlen + 1);
}

char			*ft_strtrim(char const *s1, char const *set)
{
	int		beglen;
	int		endlen;
	char	*result;
	size_t	i;
	size_t	j;

	if (!s1 || !set)
		return (NULL);
	beglen = ft_beglen(s1, set);
	endlen = ft_endlen(s1, set, beglen);
	if (!(result = malloc(sizeof(char) *
					ft_finallen(ft_strlen(s1), beglen, endlen))))
		return (0);
	i = beglen;
	j = 0;
	while (i < (ft_strlen(s1) - endlen))
	{
		result[j] = s1[i];
		i++;
		j++;
	}
	result[j] = '\0';
	return (result);
}
