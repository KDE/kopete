/*
 * libyahoo2: yahoo2.h
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

#ifndef YAHOO2_H
#define YAHOO2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "yahoo2_types.h"

int  yahoo_get_fd(int id);

int  yahoo_set_log_level(enum yahoo_log_level level);

int  yahoo_login(const char *username, const char *password, int initial);
void yahoo_logoff(int id);
void yahoo_refresh(int id);
void yahoo_get_list(int id);
void yahoo_keepalive(int id);

void yahoo_send_im(int id, const char *who, const char *what);
void yahoo_send_typing(int id, const char *who, int typ);

void yahoo_set_away(int id, enum yahoo_status state, const char *msg, int away);

void yahoo_add_buddy(int id, const char *who, const char *group);
void yahoo_remove_buddy(int id, const char *who, const char *group);
void yahoo_reject_buddy(int id, const char *who, const char *msg);
void yahoo_ignore_buddy(int id, const char *who, int unignore);
void yahoo_change_buddy_group(int id, const char *who, const char *old_group, const char *new_group);

void yahoo_conference_invite(int id, YList *who, const char *room, const char *msg);
void yahoo_conference_addinvite(int id, const char *who, const char *room, const char *msg);
void yahoo_conference_decline(int id, YList *who, const char *room, const char *msg);
void yahoo_conference_message(int id, YList *who, const char *room, const char *msg);
void yahoo_conference_logon(int id, YList *who, const char *room);
void yahoo_conference_logoff(int id, YList *who, const char *room);

int  yahoo_send_file(int id, const char *who, const char *msg, const char *name, long size);

int  yahoo_read_ready(int id, int fd);
int  yahoo_write_ready(int id, int fd);

enum yahoo_status yahoo_current_status(int id);
const YList * yahoo_get_buddylist(int id);
const YList * yahoo_get_ignorelist(int id);
const YList * yahoo_get_identities(int id);
const char  * yahoo_get_cookie(int id, const char *which);

int yahoo_get_url_handle(int id, const char *url, char *filename, unsigned long *filesize);

#include "yahoo_httplib.h"

#ifdef __cplusplus
}
#endif

#endif
