/*
 * libyahoo2: yahoo_list.h
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
 */

/*
 * This is a replacement for the GList.  It only provides functions that 
 * we use in libyahoo2.  Thanks to Meredyyd from everybuddy dev for doing 
 * most of it.
 */

#ifndef __YAHOO_LIST_H__
#define __YAHOO_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _YList {
  struct _YList * next;
  struct _YList * prev;
  void * data;
} YList;

typedef int (*YListCompFunc)(const void *, const void *);

YList * y_list_append(YList * list, void * data);
YList * y_list_remove_link(YList * list, const YList * link);
YList * y_list_remove(YList * list, void * data);

YList * y_list_copy(YList * list);

void y_list_free_1(YList * list);
void y_list_free(YList * list);
int y_list_length(YList * list);

YList * y_list_find_custom(YList * list, void * data, YListCompFunc comp);

#ifdef __cplusplus
}
#endif

#endif
