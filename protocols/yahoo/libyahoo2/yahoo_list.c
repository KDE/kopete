/*
 * libyahoo2: yahoo_list.c
 *
 * Some code copyright (C) 2002, Philip S Tellis <philip . tellis AT gmx . net>
 * Other code copyright Meredydd Luff <meredydd AT everybuddy.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This code was borrowed from elist.c in the eb-lite sources
 *
 */

#include <stdlib.h>

#include "yahoo_list.h"

YList * y_list_append(YList * list, void * data)
{
  YList * n;
  YList * new_list=(YList *)malloc(sizeof(YList));
  YList * attach_to=NULL;

  new_list->next=NULL;
  new_list->data=data;

  for(n=list; n!=NULL; n=n->next)
  {
    attach_to=n;
  }

  if(attach_to==NULL)
  {
    new_list->prev=NULL;
    return new_list;
  } else {
    new_list->prev=attach_to;
    attach_to->next=new_list;
    return list;
  }
}

YList * y_list_remove(YList * list, void * data)
{
  YList * n;

  for(n=list; n!=NULL; n=n->next)
  {
    if(n->data==data)
    {
      return y_list_remove_link(list, n);
    }
  }

  return list;
}

/* Warning */
/* link MUST be part of list */
YList * y_list_remove_link(YList * list, YList * link)
{
  if(!link)
    return list;

  if(link->next)
    link->next->prev = link->prev;
  if(link->prev)
    link->prev->next = link->next;

  if(link == list)
    list = link->next;

  link->prev = link->next = NULL;

  return list;
}

int y_list_length(YList * list)
{
  int retval=0;
  YList * n=list;

  for(n=list; n!=NULL; n=n->next)
  { retval++; }

  return retval;
}

YList * y_list_copy(YList * list)
{
  YList * n;
  YList * copy=NULL;

  for(n=list; n!=NULL; n=n->next)
  {
    copy=y_list_append(copy, n->data);
  }

  return copy;
}

void y_list_free_1(YList * list)
{ free(list); }

void y_list_free(YList * list)
{
  YList * n=list;

  while(n!=NULL)
  {
    YList * next=n->next;
    free(n);
    n=next;
  }
}

YList * y_list_find_custom(YList * list, void * data, YListCompFunc comp)
{
	YList * l;
	for (l = list; l; l = l->next)
		if(comp(l->data, (const void *)data) == 0)
			return l;

	return NULL;
}

