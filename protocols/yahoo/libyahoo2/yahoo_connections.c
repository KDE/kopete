/*
 * libyahoo2: yahoo_connections.c
 *
 * Copyright (C) 2002, Philip S Tellis <philip . tellis AT gmx . net>
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
 * Linked list routines to handle multiple connections
 */

#include "yahoo_connections.h"
#include "yahoo_util.h"

struct yahoo_connection {
	struct yahoo_data *yd;
	int fd;
}; 

static YList * conn = NULL;

void add_to_list(struct yahoo_data *yd, int fd)
{
	struct yahoo_connection * c;
	
	if(!yd)
		return;
	c = y_new0(struct yahoo_connection, 1);
	c->yd = yd;
	c->fd = fd;

	conn = y_list_append(conn, c);
}

void del_from_list(struct yahoo_data *yd)
{
	YList *l;
	for(l = conn; l; l=l->next)
	{
		struct yahoo_connection * c = l->data;
		if(c->yd == yd) {
			conn = y_list_remove_link(conn, l);
			break;
		}
	}
}

void del_from_list_by_fd(int fd)
{
	YList *l;
	for(l = conn; l; l=l->next)
	{
		struct yahoo_connection * c = l->data;
		if(c->fd == fd) {
			conn = y_list_remove_link(conn, l);
			break;
		}
	}
}

struct yahoo_data * find_conn_by_id(int id)
{
	YList * l;
	for(l = conn; l; l = l->next)
	{
		struct yahoo_connection * c = l->data;
		if(c->yd->client_id == id)
			return c->yd;
	}
	return NULL;
}

struct yahoo_data * find_conn_by_fd(int fd)
{
	YList * l;
	for(l = conn; l; l = l->next)
	{
		struct yahoo_connection * c = l->data;
		if(c->fd == fd)
			return c->yd;
	}
	return NULL;
}

