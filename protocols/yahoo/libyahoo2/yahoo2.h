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

#ifdef __MINGW32__
# include <winsock2.h>
# define write(a,b,c) send(a,b,c,0)
# define read(a,b,c)  recv(a,b,c,0)
# define HAVE_GLIB 1
#endif

#if HAVE_GLIB
# include <glib.h>
# define snprintf g_snprintf
# define vsnprintf g_vsnprintf
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "yahoo2_types.h"

/* returns the socket descriptor for a given connection. shouldn't be needed */
int  yahoo_get_fd(int id);

/* says how much logging to do */
/* see yahoo2_types.h for the different values */
int  yahoo_set_log_level(enum yahoo_log_level level);

/* these functions should be self explanatory */
/* who always means the buddy you're acting on */
/* id is the successful value returned by yahoo_login */


/* login returns a connection id used to identify the connection hereon */
/* initial is of type enum yahoo_status.  see yahoo2_types.h */
/* returns <= 0 on error - whatever ext_yahoo_connect returned */
int  yahoo_login(const char *username, const char *password, int initial);
void yahoo_logoff(int id);
/* reloads status of all buddies */
void yahoo_refresh(int id);
/* activates/deactivates an identity */
void yahoo_set_identity_status(int id, const char * identity, int active);
/* regets the entire buddy list from the server */
void yahoo_get_list(int id);
void yahoo_keepalive(int id);

/* from is the identity you're sending from.  if NULL, the default is used */
void yahoo_send_im(int id, const char *from, const char *who, const char *msg);
/* if type is true, send typing notice, else send stopped typing notice */
void yahoo_send_typing(int id, const char *from, const char *who, int typ);

/* used to set away/back status. */
/* away says whether the custom message is an away message or a sig */
void yahoo_set_away(int id, enum yahoo_status state, const char *msg, int away);

void yahoo_add_buddy(int id, const char *who, const char *group);
void yahoo_remove_buddy(int id, const char *who, const char *group);
void yahoo_reject_buddy(int id, const char *who, const char *msg);
/* if unignore is true, unignore, else ignore */
void yahoo_ignore_buddy(int id, const char *who, int unignore);
void yahoo_change_buddy_group(int id, const char *who, const char *old_group, const char *new_group);

void yahoo_conference_invite(int id, const char * from, YList *who, const char *room, const char *msg);
void yahoo_conference_addinvite(int id, const char * from, const char *who, const char *room, const YList * members, const char *msg);
void yahoo_conference_decline(int id, const char * from, YList *who, const char *room, const char *msg);
void yahoo_conference_message(int id, const char * from, YList *who, const char *room, const char *msg);
void yahoo_conference_logon(int id, const char * from, YList *who, const char *room);
void yahoo_conference_logoff(int id, const char * from, YList *who, const char *room);

/* returns a socket file descriptor to the upload stream. */
/* you should write your data to this stream when it returns */
int  yahoo_send_file(int id, const char *who, const char *msg, const char *name, long size);
/* returns a socket fd to a url for downloading a file. */
int yahoo_get_url_handle(int id, const char *url, char *filename, unsigned long *filesize);

/* these should be called when input is available on a fd */
/* registered by ext_yahoo_add_handler */
/* if these return negative values, errno may be set */
int  yahoo_read_ready(int id, int fd);
int  yahoo_write_ready(int id, int fd);

/* utility functions. these do not hit the server */
enum yahoo_status yahoo_current_status(int id);
const YList * yahoo_get_buddylist(int id);
const YList * yahoo_get_ignorelist(int id);
const YList * yahoo_get_identities(int id);
/* 'which' could be y, t, c or login.  This may change in later versions. */
const char  * yahoo_get_cookie(int id, const char *which);

/* returns the url used to get user profiles - you must append the user id */
/* as of now this is http://profiles.yahoo.com/ */
/* You'll have to do urlencoding yourself, but see yahoo_httplib.h first */
const char  * yahoo_get_profile_url( void );

#include "yahoo_httplib.h"

#ifdef __cplusplus
}
#endif

#endif
