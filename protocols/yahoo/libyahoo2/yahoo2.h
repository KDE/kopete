#ifndef YAHOO2_H
#define YAHOO2_H

#include "yahoo2_types.h"

int  yahoo_get_fd(guint32 id);

guint32 yahoo_login(char *username, char *password, int initial);
void yahoo_logoff(guint32 id);
void yahoo_refresh(guint32 id);
void yahoo_keepalive(guint32 id);

void yahoo_send_im(guint32 id, char *who, char *what, int len);
void yahoo_send_typing(guint32 id, char *who, int typ);

void yahoo_set_away(guint32 id, enum yahoo_status state, char *msg, int away);

void yahoo_add_buddy(guint32 id, char *who, char *group);
void yahoo_remove_buddy(guint32 id, char *who, char *group);
void yahoo_reject_buddy(guint32 id, char *who, char *msg);
void yahoo_change_buddy_group(guint32 id, char *who, char *old_group, char *new_group);

void yahoo_conference_invite(guint32 id, char **who, char *room, char *msg);
void yahoo_conference_addinvite(guint32 id, char *who, char *room, char *msg);
void yahoo_conference_decline(guint32 id, char **who, char *room, char *msg);
void yahoo_conference_message(guint32 id, char **who, char *room, char *msg);
void yahoo_conference_logon(guint32 id, char **who, char *room);
void yahoo_conference_logoff(guint32 id, char **who, char *room);

int  yahoo_send_file(guint32 id, char *who, char *msg, char *name, long size);

int  yahoo_read_ready(guint32 id, int fd);
int  yahoo_write_ready(guint32 id, int fd);

enum yahoo_status yahoo_current_status(guint32 id);
GList * get_buddylist(guint32 id);
GList *get_identities(guint32 id);
char *get_cookie(guint32 id, char *which);

int yahoo_get_url_handle(guint32 id, char *url, char *filename, unsigned long *filesize);

#include "yahoo_httplib.h"

#endif
