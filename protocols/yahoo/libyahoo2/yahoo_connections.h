/*
 * libyahoo2: yahoo_connections.h
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

#ifndef YAHOO_CONNECTIONS_H
#define YAHOO_CONNECTIONS_H

#include "yahoo2_types.h"

void add_to_list(struct yahoo_data *yd, int fd);
void del_from_list(struct yahoo_data *yd);
void del_from_list_by_fd(int fd);

struct yahoo_data * find_conn_by_id(int id);
struct yahoo_data * find_conn_by_fd(int fd);

#endif
