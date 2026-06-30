/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgoulama <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/13 16:22:23 by fgoulama          #+#    #+#             */
/*   Updated: 2019/10/17 18:13:37 by fgoulama         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*new;

	new = ft_lstnew(f(lst->content));
	if (new)
	{
		while (lst->next)
		{
			ft_lstadd_back(&new, ft_lstnew(f(lst->content)));
			lst = lst->next;
		}
	}
	else
		del(new->content);
	return (new);
}
