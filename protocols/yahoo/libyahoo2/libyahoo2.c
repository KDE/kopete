/*
 * libyahoo2: libyahoo2.c
 *
 * Some code copyright (C) 2002, Philip S Tellis <philip . tellis AT gmx . net>
 *
 * Much of this code was taken and adapted from the yahoo module for
 * gaim released under the GNU GPL.  This code is also released under the 
 * GNU GPL.
 *
 * This code is derivitive of Gaim <http://gaim.sourceforge.net>
 * copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 *	       1998-1999, Adam Fritzler <afritz@marko.net>
 *	       1998-2002, Rob Flynn <rob@marko.net>
 *	       2000-2002, Eric Warmenhoven <eric@warmenhoven.org>
 *	       2001-2002, Brian Macke <macke@strangelove.net>
 *		    2001, Anand Biligiri S <abiligiri@users.sf.net>
 *		    2001, Valdis Kletnieks
 *		    2002, Sean Egan <bj91704@binghamton.edu>
 *		    2002, Toby Gray <toby.gray@ntlworld.com>
 *
 * This library also uses code from other libraries, namely:
 *     Portions from libfaim copyright 1998, 1999 Adam Fritzler
 *     <afritz@auk.cx>
 *     Portions of Sylpheed copyright 2000-2002 Hiroyuki Yamamoto
 *     <hiro-y@kcn.ne.jp>
 *
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

#if STDC_HEADERS
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <stdlib.h>
#include <ctype.h>
#include "md5.h"
#include "yahoo2.h"
#include "yahoo_connections.h"
#include "yahoo_httplib.h"
#include "yahoo_util.h"

#include "yahoo2_callbacks.h"
#include "yahoo_debug.h"

#ifdef USE_STRUCT_CALLBACKS
struct yahoo_callbacks *yc=NULL;

void yahoo_register_callbacks(struct yahoo_callbacks * tyc)
{
	yc = tyc;
}

#define YAHOO_CALLBACK(x)	yc->x
#else
#define YAHOO_CALLBACK(x)	x
#endif

int yahoo_log_message(char * fmt, ...)
{
	char out[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(out, sizeof(out), fmt, ap);
	va_end(ap);
	return YAHOO_CALLBACK(ext_yahoo_log)(out);
}

int yahoo_connect(char * host, int port)
{
	return YAHOO_CALLBACK(ext_yahoo_connect)(host, port);
}

extern char pager_host[];
extern char pager_port[];
extern char filetransfer_host[];
extern char filetransfer_port[];

static char profile_url[] = "http://profiles.yahoo.com/";

enum yahoo_service { /* these are easier to see in hex */
	YAHOO_SERVICE_LOGON = 1,
	YAHOO_SERVICE_LOGOFF,
	YAHOO_SERVICE_ISAWAY,
	YAHOO_SERVICE_ISBACK,
	YAHOO_SERVICE_IDLE, /* 5 (placemarker) */
	YAHOO_SERVICE_MESSAGE,
	YAHOO_SERVICE_IDACT,
	YAHOO_SERVICE_IDDEACT,
	YAHOO_SERVICE_MAILSTAT,
	YAHOO_SERVICE_USERSTAT, /* 0xa */
	YAHOO_SERVICE_NEWMAIL,
	YAHOO_SERVICE_CHATINVITE,
	YAHOO_SERVICE_CALENDAR,
	YAHOO_SERVICE_NEWPERSONALMAIL,
	YAHOO_SERVICE_NEWCONTACT,
	YAHOO_SERVICE_ADDIDENT, /* 0x10 */
	YAHOO_SERVICE_ADDIGNORE,
	YAHOO_SERVICE_PING,
	YAHOO_SERVICE_GROUPRENAME,
	YAHOO_SERVICE_SYSMESSAGE = 0x14,
	YAHOO_SERVICE_PASSTHROUGH2 = 0x16,
	YAHOO_SERVICE_CONFINVITE = 0x18,
	YAHOO_SERVICE_CONFLOGON,
	YAHOO_SERVICE_CONFDECLINE,
	YAHOO_SERVICE_CONFLOGOFF,
	YAHOO_SERVICE_CONFADDINVITE,
	YAHOO_SERVICE_CONFMSG,
	YAHOO_SERVICE_CHATLOGON,
	YAHOO_SERVICE_CHATLOGOFF,
	YAHOO_SERVICE_CHATMSG = 0x20,
	YAHOO_SERVICE_GAMELOGON = 0x28,
	YAHOO_SERVICE_GAMELOGOFF,
	YAHOO_SERVICE_GAMEMSG = 0x2a,
	YAHOO_SERVICE_FILETRANSFER = 0x46,
	YAHOO_SERVICE_VOICECHAT = 0x4A,
	YAHOO_SERVICE_NOTIFY = 0x4B,
	YAHOO_SERVICE_P2PFILEXFER = 0x4D,
	YAHOO_SERVICE_PEERTOPEER = 0x4F,	/* Checks if P2P possible */
	YAHOO_SERVICE_AUTHRESP = 0x54,
	YAHOO_SERVICE_LIST = 0x55,
	YAHOO_SERVICE_AUTH = 0x57,
	YAHOO_SERVICE_ADDBUDDY = 0x83,
	YAHOO_SERVICE_REMBUDDY = 0x84,
	YAHOO_SERVICE_IGNORECONTACT = 0x85,	/* > 1, 7, 13 < 1, 66, 13, 0*/
	YAHOO_SERVICE_REJECTCONTACT = 0x86
};

struct yahoo_pair {
	int key;
	char *value;
};

struct yahoo_packet {
	unsigned short int service;
	unsigned int status;
	unsigned int id;
	YList *hash;
};

static int last_id=0;

extern char *yahoo_crypt(char *, char *);

enum yahoo_log_level log_level = YAHOO_LOG_NONE;

int yahoo_set_log_level(enum yahoo_log_level level)
{
	enum yahoo_log_level l = log_level;
	log_level = level;
	return l;
}

/* Free a buddy list */
static void yahoo_free_buddies(struct yahoo_data *yd)
{
	YList *l;

	for(l = yd->buddies; l; l = l->next)
	{
		struct yahoo_buddy *bud = l->data;
		if(!bud)
			continue;

		FREE(bud->group);
		FREE(bud->id);
		FREE(bud->real_name);
		FREE(bud);
		l->data = bud = NULL;
	}

	y_list_free(yd->buddies);
	yd->buddies=NULL;
}

/* Free an identities list */
static void yahoo_free_identities(struct yahoo_data *yd)
{
	YList * l;
	for (l = yd->identities; l; l=l->next) {
		FREE(l->data);
		l->data=NULL;
	}

	y_list_free(yd->identities);
	yd->identities=NULL;
}

static void yahoo_free_data(struct yahoo_data *yd)
{
	FREE(yd->user);
	FREE(yd->password);
	FREE(yd->cookie_y);
	FREE(yd->cookie_t);
	FREE(yd->login_cookie);
	FREE(yd->login_id);

	FREE(yd->rxqueue);
	yd->rxlen = 0;

	yahoo_free_buddies(yd);
	yahoo_free_identities(yd);

	FREE(yd);
}

#define YAHOO_PACKET_HDRLEN (4 + 2 + 2 + 2 + 2 + 4 + 4)

static struct yahoo_packet *yahoo_packet_new(enum yahoo_service service, 
		enum yahoo_status status, int id)
{
	struct yahoo_packet *pkt = y_new0(struct yahoo_packet, 1);

	pkt->service = service;
	pkt->status = status;
	pkt->id = id;

	return pkt;
}

static void yahoo_packet_hash(struct yahoo_packet *pkt, int key, const char *value)
{
	struct yahoo_pair *pair = y_new0(struct yahoo_pair, 1);
	pair->key = key;
	pair->value = strdup(value);
	pkt->hash = y_list_append(pkt->hash, pair);
}

static int yahoo_packet_length(struct yahoo_packet *pkt)
{
	YList *l;

	int len = 0;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		int tmp = pair->key;
		do {
			tmp /= 10;
			len++;
		} while (tmp);
		len += 2;
		len += strlen(pair->value);
		len += 2;
	}

	return len;
}

#define yahoo_put16(buf, data) ( \
		(*(buf) = (u_char)((data)>>8)&0xff), \
		(*((buf)+1) = (u_char)(data)&0xff),  \
		2)
#define yahoo_get16(buf) ((((*(buf))<<8)&0xff00) + ((*((buf)+1)) & 0xff))
#define yahoo_put32(buf, data) ( \
		(*((buf)) = (u_char)((data)>>24)&0xff), \
		(*((buf)+1) = (u_char)((data)>>16)&0xff), \
		(*((buf)+2) = (u_char)((data)>>8)&0xff), \
		(*((buf)+3) = (u_char)(data)&0xff), \
		4)
#define yahoo_get32(buf) ((((*(buf))<<24)&0xff000000) + \
		(((*((buf)+1))<<16)&0x00ff0000) + \
		(((*((buf)+2))<< 8)&0x0000ff00) + \
		(((*((buf)+3)    )&0x000000ff)))

static void yahoo_packet_read(struct yahoo_packet *pkt, unsigned char *data, int len)
{
	int pos = 0;

	while (pos + 1 < len) {
		char *key, *value = NULL;
		int accept;
		int x;

		struct yahoo_pair *pair = y_new0(struct yahoo_pair, 1);

		key = malloc(len + 1);
		x = 0;
		while (pos + 1 < len) {
			if (data[pos] == 0xc0 && data[pos + 1] == 0x80)
				break;
			key[x++] = data[pos++];
		}
		key[x] = 0;
		pos += 2;
		pair->key = strtol(key, NULL, 10);
		free(key);

		accept = x; 
		/* if x is 0 there was no key, so don't accept it */
		if (accept)
			value = malloc(len - pos + 1);
		x = 0;
		while (pos + 1 < len) {
			if (data[pos] == 0xc0 && data[pos + 1] == 0x80)
				break;
			if (accept)
				value[x++] = data[pos++];
		}
		if (accept)
			value[x] = 0;
		pos += 2;
		if (accept) {
			pair->value = strdup(value);
			FREE(value);
			pkt->hash = y_list_append(pkt->hash, pair);
			DEBUG_MSG(("Key: %d  \tValue: %s", pair->key, pair->value));
		} else {
			FREE(pair);
		}
	}
}

static void yahoo_packet_write(struct yahoo_packet *pkt, unsigned char *data)
{
	YList *l;
	int pos = 0;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		unsigned char buf[100];

		snprintf(buf, sizeof(buf), "%d", pair->key);
		strcpy(data + pos, buf);
		pos += strlen(buf);
		data[pos++] = 0xc0;
		data[pos++] = 0x80;

		strcpy(data + pos, pair->value);
		pos += strlen(pair->value);
		data[pos++] = 0xc0;
		data[pos++] = 0x80;
	}
}

static void yahoo_dump_unhandled(struct yahoo_packet *pkt)
{
	YList *l;

	NOTICE(("Service: 0x%02x\tStatus: %d", pkt->service, pkt->status));
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		NOTICE(("\t%d => %s", pair->key, pair->value));
	}
}


static void yahoo_packet_dump(unsigned char *data, int len)
{
	if(log_level >= YAHOO_LOG_DEBUG) {
		int i;
		for (i = 0; i < len; i++) {
			if ((i % 8 == 0) && i)
				YAHOO_CALLBACK(ext_yahoo_log)(" ");
			if ((i % 16 == 0) && i)
				YAHOO_CALLBACK(ext_yahoo_log)("\n");
			YAHOO_CALLBACK(ext_yahoo_log)("%02x ", data[i]);
		}
		YAHOO_CALLBACK(ext_yahoo_log)("\n");
		for (i = 0; i < len; i++) {
			if ((i % 8 == 0) && i)
				YAHOO_CALLBACK(ext_yahoo_log)(" ");
			if ((i % 16 == 0) && i)
				YAHOO_CALLBACK(ext_yahoo_log)("\n");
			if (isprint(data[i]))
				YAHOO_CALLBACK(ext_yahoo_log)(" %c ", data[i]);
			else
				YAHOO_CALLBACK(ext_yahoo_log)(" . ");
		}
		YAHOO_CALLBACK(ext_yahoo_log)("\n");
	}
}

static char base64digits[] = 	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789._";
static void to_y64(unsigned char *out, const unsigned char *in, int inlen)
/* raw bytes in quasi-big-endian order to base 64 string (NUL-terminated) */
{
	for (; inlen >= 3; inlen -= 3)
		{
			*out++ = base64digits[in[0] >> 2];
			*out++ = base64digits[((in[0]<<4) & 0x30) | (in[1]>>4)];
			*out++ = base64digits[((in[1]<<2) & 0x3c) | (in[2]>>6)];
			*out++ = base64digits[in[2] & 0x3f];
			in += 3;
		}
	if (inlen > 0)
		{
			unsigned char fragment;

			*out++ = base64digits[in[0] >> 2];
			fragment = (in[0] << 4) & 0x30;
			if (inlen > 1)
				fragment |= in[1] >> 4;
			*out++ = base64digits[fragment];
			*out++ = (inlen < 2) ? '-' 
					: base64digits[(in[1] << 2) & 0x3c];
			*out++ = '-';
		}
	*out = '\0';
}

static int yahoo_send_packet(struct yahoo_data *yd, struct yahoo_packet *pkt, int extra_pad)
{
	int pktlen = yahoo_packet_length(pkt);
	int len = YAHOO_PACKET_HDRLEN + pktlen;
	int ret;

	unsigned char *data;
	int pos = 0;

	if (yd->fd < 0)
		return -1;

	data = y_new0(unsigned char, len + 1);

	memcpy(data + pos, "YMSG", 4); pos += 4;
	pos += yahoo_put16(data + pos, 0x0900);
	pos += yahoo_put16(data + pos, 0x0000);
	pos += yahoo_put16(data + pos, pktlen + extra_pad);
	pos += yahoo_put16(data + pos, pkt->service);
	pos += yahoo_put32(data + pos, pkt->status);
	pos += yahoo_put32(data + pos, pkt->id);

	yahoo_packet_write(pkt, data + pos);

	yahoo_packet_dump(data, len);
redo_write:
	ret = write(yd->fd, data, len);
	if(ret == -1 && errno==EINTR)
		goto redo_write;
	LOG(("wrote packet"));

	FREE(data);

	return ret;
}

static void yahoo_packet_free(struct yahoo_packet *pkt)
{
	while (pkt->hash) {
		struct yahoo_pair *pair = pkt->hash->data;
		FREE(pair->value);
		FREE(pair);
		pkt->hash = y_list_remove_link(pkt->hash, pkt->hash);
	}
	FREE(pkt);
}

static int is_same_bud(const void * a, const void * b) {
	const struct yahoo_buddy *subject = a;
	const struct yahoo_buddy *object = b;

	return strcmp(subject->id, object->id);
}

static YList * bud_str2list(char *rawlist)
{
	YList * l = NULL;

	char **lines;
	char **split;
	char **buddies;
	char **tmp, **bud;

	lines = y_strsplit(rawlist, "\n", -1);
	for (tmp = lines; *tmp; tmp++) {
		struct yahoo_buddy *newbud;

		split = y_strsplit(*tmp, ":", 2);
		if (!split)
			continue;
		if (!split[0] || !split[1]) {
			y_strfreev(split);
			continue;
		}
		buddies = y_strsplit(split[1], ",", -1);

		for (bud = buddies; bud && *bud; bud++) {
			newbud = y_new0(struct yahoo_buddy, 1);
			newbud->id = strdup(*bud);
			newbud->group = strdup(split[0]);

			if(y_list_find_custom(l, newbud, is_same_bud)) {
				FREE(newbud->id);
				FREE(newbud->group);
				FREE(newbud);
				continue;
			}

			newbud->real_name = NULL;

			l = y_list_append(l, newbud);

			NOTICE(("Added buddy %s to group %s", newbud->id, newbud->group));
		}

		y_strfreev(buddies);
		y_strfreev(split);
	}
	y_strfreev(lines);

	return l;
}

static char * getcookie(char *rawcookie)
{
	char * cookie=NULL;
	char * tmpcookie = strdup(rawcookie+2);
	char * cookieend = strchr(tmpcookie, ';');

	if(cookieend)
		*cookieend = '\0';

	cookie = strdup(tmpcookie);
	FREE(tmpcookie);
	cookieend=NULL;

	return cookie;
}

static char * getlcookie(char *cookie)
{
	char *tmp;
	char *tmpend;
	char *login_cookie = NULL;

	tmpend = strstr(cookie, "n=");
	if(tmpend) {
		tmp = strdup(tmpend+2);
		tmpend = strchr(tmp, '&');
		if(tmpend)
			*tmpend='\0';
		login_cookie = strdup(tmp);
		FREE(tmp);
	}

	return login_cookie;
}

static void yahoo_process_notify(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *msg = NULL;
	char *from = NULL;
	int stat = 0;
	char *game = NULL;
	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 4)
			from = pair->value;
		if (pair->key == 49)
			msg = pair->value;
		if (pair->key == 13)
			stat = atoi(pair->value);
		if (pair->key == 14)
			game = pair->value;
		if (pair->key == 16) {
			NOTICE((pair->value));
			return;
		}

	}

	if (!msg)
		return;
	
	if (!strncasecmp(msg, "TYPING", strlen("TYPING"))) 
		YAHOO_CALLBACK(ext_yahoo_typing_notify)(yd->client_id, from, stat);
	else if (!strncasecmp(msg, "GAME", strlen("GAME"))) 
		YAHOO_CALLBACK(ext_yahoo_game_notify)(yd->client_id, from, stat);
	else
		LOG(("Got unknown notification: %s", msg));
}

static void yahoo_process_filetransfer(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *from=NULL;
	char *to=NULL;
	char *msg=NULL;
	char *url=NULL;
	long expires=0;

	char *service=NULL;

	char *filename=NULL;
	unsigned long filesize=0L;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 4)
			from = pair->value;
		if (pair->key == 5)
			to = pair->value;
		if (pair->key == 14)
			msg = pair->value;
		if (pair->key == 20)
			url = pair->value;
		if (pair->key == 38)
			expires = atol(pair->value);

		if (pair->key == 27)
			filename = pair->value;
		if (pair->key == 28)
			filesize = atol(pair->value);

		if (pair->key == 49)
			service = pair->value;
	}

	if(pkt->service == YAHOO_SERVICE_P2PFILEXFER) {
		if(strcmp("FILEXFER", service) != 0) {
			WARNING(("unhandled service 0x%02x", pkt->service));
			yahoo_dump_unhandled(pkt);
			return;
		}
	}

	if(msg) {
		char *tmp;
		tmp = strchr(msg, '\006');
		if(tmp)
			*tmp = '\0';
	}
	if(url && from)
		YAHOO_CALLBACK(ext_yahoo_got_file)(yd->client_id, from, url, expires, msg, filename, filesize);

}

static void yahoo_process_conference(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *msg = NULL;
	char *host = NULL;
	char *who = NULL;
	char *room = NULL;
	char *id = NULL;
	YList *members = NULL;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 50)
			host = pair->value;
		
		if (pair->key == 52) {		// invite
			members = y_list_append(members, strdup(pair->value));
		}
		if (pair->key == 53)		// logon
			who = pair->value;
		if (pair->key == 54)		// decline
			who = pair->value;
		if (pair->key == 55)		// unavailable (status == 2)
			who = pair->value;
		if (pair->key == 56)		// logoff
			who = pair->value;

		if (pair->key == 57)
			room = pair->value;

		if (pair->key == 58)		// join message
			msg = pair->value;
		if (pair->key == 14)		// decline/conf message
			msg = pair->value;

		if (pair->key == 13)
			;
		if (pair->key == 16)		// error
			msg = pair->value;

		if (pair->key == 1)		// my id
			id = pair->value;
		if (pair->key == 3)		// message sender
			who = pair->value;
	}

	if(!room)
		return;

	if(host) {
		for(l = members; l; l = l->next) {
			char * w = l->data;
			if(!strcmp(w, host))
				break;
		}
		if(!l)
			members = y_list_append(members, strdup(host));
	}
	// invite, decline, join, left, message -> status == 1

	switch(pkt->service) {
	case YAHOO_SERVICE_CONFINVITE:
		if(pkt->status == 2)
			;
		else if(members)
			YAHOO_CALLBACK(ext_yahoo_got_conf_invite)(yd->client_id, host, room, msg, members);
		else if(msg)
			YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, msg, 0);
		break;
	case YAHOO_SERVICE_CONFADDINVITE:
		if(pkt->status == 2)
			;
		else
			YAHOO_CALLBACK(ext_yahoo_got_conf_invite)(yd->client_id, host, room, msg, members);
		break;
	case YAHOO_SERVICE_CONFDECLINE:
		if(who)
			YAHOO_CALLBACK(ext_yahoo_conf_userdecline)(yd->client_id, who, room, msg);
		break;
	case YAHOO_SERVICE_CONFLOGON:
		if(who)
			YAHOO_CALLBACK(ext_yahoo_conf_userjoin)(yd->client_id, who, room);
		break;
	case YAHOO_SERVICE_CONFLOGOFF:
		if(who)
			YAHOO_CALLBACK(ext_yahoo_conf_userleave)(yd->client_id, who, room);
		break;
	case YAHOO_SERVICE_CONFMSG:
		if(who)
			YAHOO_CALLBACK(ext_yahoo_conf_message)(yd->client_id, who, room, msg);
		break;
	}
}

static void yahoo_process_message(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *msg = NULL;
	char *from = NULL;
	char *real_from = NULL;
	long tm = 0L;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 0)
			real_from = pair->value;
		else if (pair->key == 1)
			from = pair->value;
		else if (pair->key == 4)
			from = pair->value;
		else if (pair->key == 14)
			msg = pair->value;
		else if (pair->key == 15)
			tm = strtol(pair->value, NULL, 10);
		else if (pair->key == 16)	// system message
			msg = pair->value;
		else
			LOG(("yahoo_process_message: status: %d, key: %d, value: %s",
					pkt->status, pair->key, pair->value));
	}

	if (pkt->service == YAHOO_SERVICE_SYSMESSAGE) {
		YAHOO_CALLBACK(ext_yahoo_system_message)(yd->client_id, msg);
	} else if (pkt->status <= 1 || pkt->status == 5) {
		char *m;
		int i, j;
		//strip_linefeed(msg);
		m = msg;
		for (i = 0, j = 0; m[i]; i++) {
			if (m[i] == 033) {
				while (m[i] && (m[i] != 'm'))
					i++;
				if (!m[i])
					i--;
				continue;
			}
			msg[j++] = m[i];
		}
		msg[j] = 0;
		YAHOO_CALLBACK(ext_yahoo_got_im)(yd->client_id, from, msg, tm, (int)pkt->status);
	} else if (pkt->status == 2) {
		YAHOO_CALLBACK(ext_yahoo_got_im)(yd->client_id, from, NULL, tm, 2);
	} else if (pkt->status == 0xffffffff) {
		YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, msg, 0);
	}
}


static void yahoo_process_status(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	YList *l;
	char *name = NULL;
	int state = 0;
	int away = 0;
	char *msg = NULL;
	
	if(pkt->service == YAHOO_SERVICE_LOGOFF && pkt->status == -1) {
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_DUPL, NULL);
		return;
	}

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;

		switch (pair->key) {
		case 0: /* we won't actually do anything with this */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 1: /* we don't get the full buddy list here. */
			if (!yd->logged_in) {
				yd->logged_in = TRUE;
				if(yd->current_status < 0)
					yd->current_status = yd->initial_status;
				YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, YAHOO_LOGIN_OK, NULL);
			}
			break;
		case 8: /* how many online buddies we have */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 7: /* the current buddy */
			name = pair->value;
			break;
		case 10: /* state */
			state = strtol(pair->value, NULL, 10);
			break;
		case 19: /* custom status message */
			msg = pair->value;
			break;
		case 47: /* is it an away message or not */
			away = atoi(pair->value);
			break;
		case 11: /* what is this? */
			NOTICE(("key %d:%s", pair->key, pair->value));
			break;
		case 17: /* in chat? */
			break;
		case 13: /* in pager? */
			if (pkt->service == YAHOO_SERVICE_LOGOFF || strtol(pair->value, NULL, 10) == 0) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, YAHOO_STATUS_OFFLINE, NULL, 1);
				break;
			}
			if (state == YAHOO_STATUS_AVAILABLE) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, state, NULL, 0);
			} else if (state == YAHOO_STATUS_CUSTOM) {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, state, msg, away);
			} else {
				YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, state, NULL, 1);
			}

			break;
		case 60: 
			/* sometimes going offline makes this 2, but invisible never sends it */
			NOTICE(("key %d:%s", pair->key, pair->value));
			 break;
		case 16: /* Custom error message */
			YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, pair->value, 0);
			break;
		default:
			WARNING(("unknown status key %d:%s", pair->key, pair->value));
			break;
		}
	}
}

static void yahoo_process_list(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;

		switch(pair->key) {
		case 87: /* buddies */
			if(!yd->rawbuddylist)
				yd->rawbuddylist = strdup(pair->value);
			else {
				yd->rawbuddylist = y_string_append(yd->rawbuddylist, pair->value);
			}
			break;

		case 88: /* ignore list */
			if(!yd->ignorelist)
				yd->ignorelist = strdup("Ignore:");
			yd->ignorelist = y_string_append(yd->ignorelist, pair->value);
			break;

		case 89: /* identities */
			{
			char **identities = y_strsplit(pair->value, ",", -1);
			int i;
			for(i=0; identities[i]; i++)
				yd->identities = y_list_append(yd->identities, 
						strdup(identities[i]));
			y_strfreev(identities);
			}
			YAHOO_CALLBACK(ext_yahoo_got_identities)(yd->client_id, yd->identities);
			break;
		case 59: /* cookies add C cookie */
			if(yd->ignorelist) {
				yd->ignore = bud_str2list(yd->ignorelist);
				FREE(yd->ignorelist);
				YAHOO_CALLBACK(ext_yahoo_got_ignore)(yd->client_id, yd->ignore);
			}
			if(yd->rawbuddylist) {
				yd->buddies = bud_str2list(yd->rawbuddylist);
				FREE(yd->rawbuddylist);
				YAHOO_CALLBACK(ext_yahoo_got_buddies)(yd->client_id, yd->buddies);
			}

			if(pair->value[0]=='Y') {
				FREE(yd->cookie_y);
				FREE(yd->login_cookie);

				yd->cookie_y = getcookie(pair->value);
				yd->login_cookie = getlcookie(yd->cookie_y);

			} else if(pair->value[0]=='T') {
				FREE(yd->cookie_t);
				yd->cookie_t = getcookie(pair->value);

			} else if(pair->value[0]=='C') {
				FREE(yd->cookie_c);
				yd->cookie_c = getcookie(pair->value);
			} 

			break;
		case 3: /* my id */
		case 90: /* 1 */
		case 100: /* 0 */
		case 101: /* NULL */
		case 102: /* NULL */
		case 93: /* 86400/1440 */
			break;
		}
	}
}

static void yahoo_process_auth(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *seed = NULL;
	char *sn   = NULL;
	YList *l;
	
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 94)
			seed = pair->value;
		if (pair->key == 1)
			sn = pair->value;
	}
	
	if (!seed)
       		return;

	/* So, Yahoo has stopped supporting its older clients in India, and 
	 * undoubtedly will soon do so in the rest of the world.
	 * 
	 * The new clients use this authentication method.  I warn you in 
	 * advance, it's bizzare, convoluted, inordinately complicated.  
	 * It's also no more secure than crypt() was.  The only purpose this 
	 * scheme could serve is to prevent third part clients from connecting 
	 * to their servers.
	 *
	 * Sorry, Yahoo.
	 */
	{

	struct yahoo_packet *pack;
	
	md5_byte_t result[16];
	md5_state_t ctx;
	char *crypt_result;
	char *password_hash = malloc(25);
	char *crypt_hash = malloc(25);
	char *hash_string_p = malloc(50 + strlen(sn));
	char *hash_string_c = malloc(50 + strlen(sn));
	
	char checksum;
	
	int sv;
	
	char *result6 = malloc(25);
	char *result96 = malloc(25);

	sv = seed[15];
	sv = sv % 8;

	md5_init(&ctx);
	md5_append(&ctx, yd->password, strlen(yd->password));
	md5_finish(&ctx, result);
	to_y64(password_hash, result, 16);
	
	md5_init(&ctx);
	crypt_result = yahoo_crypt(yd->password, "$1$_2S43d5f$");  
	md5_append(&ctx, crypt_result, strlen(crypt_result));
	md5_finish(&ctx, result);
	to_y64(crypt_hash, result, 16);

	switch (sv%5) {
	case 0:
		checksum = seed[seed[7] % 16];
		snprintf(hash_string_p, strlen(sn) + 50,
			"%c%s%s%s", checksum, password_hash, yd->user, seed);
		snprintf(hash_string_c, strlen(sn) + 50,
			"%c%s%s%s", checksum, crypt_hash, yd->user, seed);
		break;
	case 1:
		checksum = seed[seed[9] % 16];
		snprintf(hash_string_p, strlen(sn) + 50,
			"%c%s%s%s", checksum, yd->user, seed, password_hash);
		snprintf(hash_string_c, strlen(sn) + 50,
			"%c%s%s%s", checksum, yd->user, seed, crypt_hash);
		break;
	case 2:
		checksum = seed[seed[15] % 16];
		snprintf(hash_string_p, strlen(sn) + 50,
			"%c%s%s%s", checksum, seed, password_hash, yd->user);
		snprintf(hash_string_c, strlen(sn) + 50,
			"%c%s%s%s", checksum, seed, crypt_hash, yd->user);
		break;
	case 3:
		checksum = seed[seed[1] % 16];
		snprintf(hash_string_p, strlen(sn) + 50,
			"%c%s%s%s", checksum, yd->user, password_hash, seed);
		snprintf(hash_string_c, strlen(sn) + 50,
			"%c%s%s%s", checksum, yd->user, crypt_hash, seed);
		break;
	case 4:
		checksum = seed[seed[3] % 16];
		snprintf(hash_string_p, strlen(sn) + 50,
			"%c%s%s%s", checksum, password_hash, seed, yd->user);
		snprintf(hash_string_c, strlen(sn) + 50,
			"%c%s%s%s", checksum, crypt_hash, seed, yd->user);
		break;
	}
		
	md5_init(&ctx);  
	md5_append(&ctx, hash_string_p, strlen(hash_string_p));
	md5_finish(&ctx, result);
	to_y64(result6, result, 16);

	md5_init(&ctx);  
	md5_append(&ctx, hash_string_c, strlen(hash_string_c));
	md5_finish(&ctx, result);
	to_y64(result96, result, 16);

	pack = yahoo_packet_new(YAHOO_SERVICE_AUTHRESP, yd->initial_status, 0);
	yahoo_packet_hash(pack, 0, yd->user);
	yahoo_packet_hash(pack, 6, result6);
	yahoo_packet_hash(pack, 96, result96);
	yahoo_packet_hash(pack, 1, yd->user);
		
	yahoo_send_packet(yd, pack, 0);
		
	FREE(password_hash);
	FREE(crypt_hash);
	FREE(hash_string_p);
	FREE(hash_string_c);

	yahoo_packet_free(pack);

	}
}

static void yahoo_process_auth_resp(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *login_id;
	char *handle;
	char *url=NULL;
	int  login_status=0;

	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 0)
			login_id = pair->value;
		else if (pair->key == 1)
			handle = pair->value;
		else if (pair->key == 20)
			url = pair->value;
		else if (pair->key == 66)
			login_status = atoi(pair->value);
	}

	if(pkt->status == 0xffffffff) {
		YAHOO_CALLBACK(ext_yahoo_login_response)(yd->client_id, login_status, url);
		yahoo_logoff(yd->client_id);
	}
}

static void yahoo_process_mail(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *email = NULL;
	char *subj = NULL;
	int count = 0;
	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 9)
			count = strtol(pair->value, NULL, 10);
		else if (pair->key == 43)
			who = pair->value;
		else if (pair->key == 42)
			email = pair->value;
		else if (pair->key == 18)
			subj = pair->value;
		else
			LOG(("key: %d => value: %s", pair->key, pair->value));
	}

	if (who && email && subj) {
		char from[1024];
		snprintf(from, sizeof(from), "%s (%s)", who, email);
		YAHOO_CALLBACK(ext_yahoo_mail_notify)(yd->client_id, from, subj, count);
	} else 
		YAHOO_CALLBACK(ext_yahoo_mail_notify)(yd->client_id, NULL, NULL, count);
}

static void yahoo_process_contact(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *id = NULL;
	char *who = NULL;
	char *msg = NULL;
	char *name = NULL;
	int state = YAHOO_STATUS_AVAILABLE;
	int online = FALSE;
	int away = 0;

	YList *l;

	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 1)
			id = pair->value;
		else if (pair->key == 3)
			who = pair->value;
		else if (pair->key == 14)
			msg = pair->value;
		else if (pair->key == 7)
			name = pair->value;
		else if (pair->key == 10)
			state = strtol(pair->value, NULL, 10);
		else if (pair->key == 13)
			online = strtol(pair->value, NULL, 10);
		else if (pair->key == 47)
			away = strtol(pair->value, NULL, 10);
	}

	if (id)
		YAHOO_CALLBACK(ext_yahoo_contact_added)(yd->client_id, id, who, msg);
	else if (name)
		YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, name, state, msg, away);
	else if(pkt->status == 0x07)
		YAHOO_CALLBACK(ext_yahoo_rejected)(yd->client_id, who, msg);
}

static void yahoo_process_buddyadd(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *where = NULL;
	int status = 0;
	char *me = NULL;

	struct yahoo_buddy *bud=NULL;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 1)
			me = pair->value;
		if (pair->key == 7)
			who = pair->value;
		if (pair->key == 65)
			where = pair->value;
		if (pair->key == 66)
			status = strtol(pair->value, NULL, 10);
	}

	yahoo_dump_unhandled(pkt);

	if(!who)
		return;
	if(!where)
		where = "Unknown";

	bud = y_new0(struct yahoo_buddy, 1);
	bud->id = strdup(who);
	bud->group = strdup(where);
	bud->real_name = NULL;

	yd->buddies = y_list_append(yd->buddies, bud);

/*	YAHOO_CALLBACK(ext_yahoo_status_changed)(yd->client_id, who, status, NULL, (status==YAHOO_STATUS_AVAILABLE?0:1)); */
}

static void yahoo_process_buddydel(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *who = NULL;
	char *where = NULL;
	int unk_66 = 0;
	char *me = NULL;
	struct yahoo_buddy *bud;

	YList *buddy;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 1)
			me = pair->value;
		if (pair->key == 7)
			who = pair->value;
		if (pair->key == 65)
			where = pair->value;
		if (pair->key == 66)
			unk_66 = strtol(pair->value, NULL, 10);
	}
	
	bud = y_new0(struct yahoo_buddy, 1);
	bud->id = strdup(who);
	bud->group = strdup(where);

	buddy = y_list_find_custom(yd->buddies, bud, is_same_bud);

	FREE(bud->id);
	FREE(bud->group);
	FREE(bud);

	if(buddy) {
		bud = buddy->data;
		yd->buddies = y_list_remove_link(yd->buddies, buddy);

		FREE(bud->id);
		FREE(bud->group);
		FREE(bud->real_name);
		FREE(bud);

		bud=NULL;
	}
}

static void yahoo_process_ignore(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	char *who = NULL;
	int  status = 0;
	char *me = NULL;
	int  un_ignore = 0;

	YList *l;
	for (l = pkt->hash; l; l = l->next) {
		struct yahoo_pair *pair = l->data;
		if (pair->key == 0)
			who = pair->value;
		if (pair->key == 1)
			me = pair->value;
		if (pair->key == 13) /* 1 == ignore, 2 == unignore */ 
			un_ignore = strtol(pair->value, NULL, 10);
		if (pair->key == 66) 
			status = strtol(pair->value, NULL, 10);
	}


	/*
	 * status
	 * 	0  - ok
	 * 	2  - already in ignore list, could not add
	 * 	3  - not in ignore list, could not delete
	 * 	12 - is a buddy, could not add
	 */

/*	if(status)
		YAHOO_CALLBACK(ext_yahoo_error)(yd->client_id, status, who, 0);
*/	
}

static void yahoo_packet_process(struct yahoo_data *yd, struct yahoo_packet *pkt)
{
	DEBUG_MSG(("yahoo_packet_process: 0x%02x", pkt->service));
	switch (pkt->service)
	{
	case YAHOO_SERVICE_USERSTAT:
	case YAHOO_SERVICE_LOGON:
	case YAHOO_SERVICE_LOGOFF:
	case YAHOO_SERVICE_ISAWAY:
	case YAHOO_SERVICE_ISBACK:
	case YAHOO_SERVICE_GAMELOGON:
	case YAHOO_SERVICE_GAMELOGOFF:
	case YAHOO_SERVICE_IDACT:
	case YAHOO_SERVICE_IDDEACT:
		yahoo_process_status(yd, pkt);
		break;
	case YAHOO_SERVICE_NOTIFY:
		yahoo_process_notify(yd, pkt);
		break;
	case YAHOO_SERVICE_MESSAGE:
	case YAHOO_SERVICE_GAMEMSG:
	case YAHOO_SERVICE_SYSMESSAGE:
		yahoo_process_message(yd, pkt);
		break;
	case YAHOO_SERVICE_NEWMAIL:
		yahoo_process_mail(yd, pkt);
		break;
	case YAHOO_SERVICE_NEWCONTACT:
		yahoo_process_contact(yd, pkt);
		break;
	case YAHOO_SERVICE_LIST:
		yahoo_process_list(yd, pkt);
		break;
	case YAHOO_SERVICE_AUTH:
		yahoo_process_auth(yd, pkt);
		break;
	case YAHOO_SERVICE_AUTHRESP:
		yahoo_process_auth_resp(yd, pkt);
		break;
	case YAHOO_SERVICE_CONFINVITE:
	case YAHOO_SERVICE_CONFADDINVITE:
	case YAHOO_SERVICE_CONFDECLINE:
	case YAHOO_SERVICE_CONFLOGON:
	case YAHOO_SERVICE_CONFLOGOFF:
	case YAHOO_SERVICE_CONFMSG:
		yahoo_process_conference(yd, pkt);
		break;
	case YAHOO_SERVICE_P2PFILEXFER:
	case YAHOO_SERVICE_FILETRANSFER:
		yahoo_process_filetransfer(yd, pkt);
		break;
	case YAHOO_SERVICE_ADDBUDDY:
		yahoo_process_buddyadd(yd, pkt);
		break;
	case YAHOO_SERVICE_REMBUDDY:
		yahoo_process_buddydel(yd, pkt);
		break;
	case YAHOO_SERVICE_IGNORECONTACT:
		yahoo_process_ignore(yd, pkt);
		break;
	case YAHOO_SERVICE_IDLE:
	case YAHOO_SERVICE_MAILSTAT:
	case YAHOO_SERVICE_CHATINVITE:
	case YAHOO_SERVICE_CALENDAR:
	case YAHOO_SERVICE_NEWPERSONALMAIL:
	case YAHOO_SERVICE_ADDIDENT:
	case YAHOO_SERVICE_ADDIGNORE:
	case YAHOO_SERVICE_PING:
	case YAHOO_SERVICE_GROUPRENAME:
	case YAHOO_SERVICE_PASSTHROUGH2:
	case YAHOO_SERVICE_CHATLOGON:
	case YAHOO_SERVICE_CHATLOGOFF:
	case YAHOO_SERVICE_CHATMSG:
	case YAHOO_SERVICE_VOICECHAT:
	case YAHOO_SERVICE_REJECTCONTACT:
	case YAHOO_SERVICE_PEERTOPEER:
		WARNING(("unhandled service 0x%02x", pkt->service));
		yahoo_dump_unhandled(pkt);
		break;
	default:
		WARNING(("unknown service 0x%02x", pkt->service));
		yahoo_dump_unhandled(pkt);
		break;
	}
}

static struct yahoo_packet * yahoo_getdata(struct yahoo_data * yd)
{
	struct yahoo_packet *pkt;
	int pos = 0;
	int pktlen;

	DEBUG_MSG(("rxlen is %d", yd->rxlen));
	if (yd->rxlen < YAHOO_PACKET_HDRLEN) {
		DEBUG_MSG(("len < YAHOO_PACKET_HDRLEN"));
		return NULL;
	}

	pos += 4; /* YMSG */
	pos += 2;
	pos += 2;

	pktlen = yahoo_get16(yd->rxqueue + pos); pos += 2;
	DEBUG_MSG(("%d bytes to read, rxlen is %d", 
			pktlen, yd->rxlen));

	if (yd->rxlen < (YAHOO_PACKET_HDRLEN + pktlen)) {
		DEBUG_MSG(("len < YAHOO_PACKET_HDRLEN + pktlen"));
		return NULL;
	}

	LOG(("reading packet"));
	yahoo_packet_dump(yd->rxqueue, YAHOO_PACKET_HDRLEN + pktlen);

	pkt = yahoo_packet_new(0, 0, 0);

	pkt->service = yahoo_get16(yd->rxqueue + pos); pos += 2;
	pkt->status = yahoo_get32(yd->rxqueue + pos); pos += 4;
	DEBUG_MSG(("Yahoo Service: 0x%02x Status: %d", pkt->service,
		       pkt->status));
	pkt->id = yahoo_get32(yd->rxqueue + pos); pos += 4;

	yd->id = pkt->id;

	yahoo_packet_read(pkt, yd->rxqueue + pos, pktlen);

	yd->rxlen -= YAHOO_PACKET_HDRLEN + pktlen;
	DEBUG_MSG(("rxlen == %d, rxqueue == %p", yd->rxlen, yd->rxqueue));
	if (yd->rxlen>0) {
		char *tmp = y_memdup(yd->rxqueue + YAHOO_PACKET_HDRLEN 
				+ pktlen, yd->rxlen);
		FREE(yd->rxqueue);
		yd->rxqueue = tmp;
		DEBUG_MSG(("new rxlen == %d, rxqueue == %p", yd->rxlen, yd->rxqueue));
	} else {
		DEBUG_MSG(("freed rxqueue == %p", yd->rxqueue));
		FREE(yd->rxqueue);
	}

	return pkt;
}

int yahoo_write_ready(int id, int fd)
{
	return 1;
}

int yahoo_read_ready(int id, int fd)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
	char buf[1024];
	int len;

	DEBUG_MSG(("callback"));

	if(!yd)
		return -2;
	
redo:
	len = read(fd, buf, sizeof(buf));

	if(len == -1 && errno == EINTR)
		goto redo;

	if (len <= 0) {
		int e = errno;
		DEBUG_MSG(("len == %d (<= 0)", len));

		yd->current_status = -1;
		YAHOO_CALLBACK(ext_yahoo_remove_handler)(id, fd);

		close(fd);

		/* no need to return an error, because we've already fixed it */
		if(len == 0)
			return 1;

		errno=e;
		return -1;
	}

	yd->rxqueue = realloc(yd->rxqueue, len + yd->rxlen);
	memcpy(yd->rxqueue + yd->rxlen, buf, len);
	yd->rxlen += len;

	while ((pkt = yahoo_getdata(yd)) != NULL) {

		yahoo_packet_process(yd, pkt);

		yahoo_packet_free(pkt);
	}

	return len;
}

int yahoo_login(const char *username, const char *password, int initial)
{
	struct yahoo_data *yd;
	struct yahoo_packet *pkt;
	int fd;

	fd = YAHOO_CALLBACK(ext_yahoo_connect)(pager_host, atoi(pager_port));

	if(fd <= 0)
		return fd;

	yd = y_new0(struct yahoo_data, 1);
	yd->fd = fd;

	yd->user = strdup(username);
	yd->password = strdup(password);

	yd->initial_status = initial;
	yd->current_status = -1;

	yd->client_id = ++last_id;

	add_to_list(yd, yd->fd);

	pkt = yahoo_packet_new(YAHOO_SERVICE_AUTH, YAHOO_STATUS_AVAILABLE, 0);

	yahoo_packet_hash(pkt, 1, username);
	NOTICE(("Sending initial packet"));
	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);

	YAHOO_CALLBACK(ext_yahoo_add_handler)(yd->client_id, yd->fd, YAHOO_INPUT_READ);

	return yd->client_id;
}


int yahoo_get_fd(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return 0;
	else
		return yd->fd;
}

static void yahoo_close(struct yahoo_data *yd) 
{
	del_from_list(yd);

	if (yd->fd >= 0)
		close(yd->fd);
	FREE(yd->rxqueue);
	yd->rxlen = 0;
	yahoo_free_data(yd);
}

void yahoo_send_im(int id, const char *from, const char *who, const char *what)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_MESSAGE, YAHOO_STATUS_OFFLINE, yd->id);

	yahoo_packet_hash(pkt, 0, yd->user);
	yahoo_packet_hash(pkt, 1, from?from:yd->user);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt, 14, what);

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_send_typing(int id, const char *from, const char *who, int typ)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_NOTIFY, YAHOO_STATUS_TYPING, yd->id);

	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt, 4, from?from:yd->user);
	yahoo_packet_hash(pkt, 14, " ");
	yahoo_packet_hash(pkt, 13, typ ? "1" : "0");
	yahoo_packet_hash(pkt, 49, "TYPING");

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_set_away(int id, enum yahoo_status state, const char *msg, int away)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;
	int service;
	char s[4];

	if(!yd)
		return;

	if (msg) {
		yd->current_status = YAHOO_STATUS_CUSTOM;
	} else {
		yd->current_status = state;
	}

	if (yd->current_status == YAHOO_STATUS_AVAILABLE)
		service = YAHOO_SERVICE_ISBACK;
	else
		service = YAHOO_SERVICE_ISAWAY;
	pkt = yahoo_packet_new(service, yd->current_status, yd->id);
	snprintf(s, sizeof(s), "%d", yd->current_status);
	yahoo_packet_hash(pkt, 10, s);
	if (yd->current_status == YAHOO_STATUS_CUSTOM) {
		yahoo_packet_hash(pkt, 19, msg);
		yahoo_packet_hash(pkt, 47, away?"1":"0");
	}

	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_logoff(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	if(yd->current_status != -1) {

		pkt = yahoo_packet_new(YAHOO_SERVICE_LOGOFF, YAHOO_STATUS_AVAILABLE, yd->id);
		yd->current_status = -1;

		if (pkt) {
			yahoo_send_packet(yd, pkt, 0);
			yahoo_packet_free(pkt);
		}
	}

	YAHOO_CALLBACK(ext_yahoo_remove_handler)(id, yd->fd);
	yahoo_close(yd);
}

void yahoo_get_list(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_LIST, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	if (pkt) {
		yahoo_send_packet(yd, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

void yahoo_set_identity_status(int id, const char * identity, int active)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	pkt = yahoo_packet_new(active?YAHOO_SERVICE_IDACT:YAHOO_SERVICE_IDDEACT,
			YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 3, identity);
	if (pkt) {
		yahoo_send_packet(yd, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

void yahoo_refresh(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_USERSTAT, YAHOO_STATUS_AVAILABLE, yd->id);
	if (pkt) {
		yahoo_send_packet(yd, pkt, 0);
		yahoo_packet_free(pkt);
	}
}

void yahoo_keepalive(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt=NULL;
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_PING, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_add_buddy(int id, const char *who, const char *group)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;

	if(!yd)
		return;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_ADDBUDDY, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, group);
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_remove_buddy(int id, const char *who, const char *group)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_REMBUDDY, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, group);
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_reject_buddy(int id, const char *who, const char *msg)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;

	if(!yd)
		return;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_REJECTCONTACT, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 14, msg);
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_ignore_buddy(int id, const char *who, int unignore)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;

	if(!yd)
		return;

	if (!yd->logged_in)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_IGNORECONTACT, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 13, unignore?"2":"1");
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_change_buddy_group(int id, const char *who, const char *old_group, const char *new_group)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt = NULL;

	if(!yd)
		return;

	/* 
	 * this seems to work by adding the buddy to the new group, then 
	 * removing from the old group.  there's an additional field - 14 
	 * that probably says that this is a group rename.
	 * I wonder what YAHOO_SERVICE_GROUPRENAME is for then
	 */
	pkt = yahoo_packet_new(YAHOO_SERVICE_ADDBUDDY, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, new_group);
	yahoo_packet_hash(pkt, 14, " ");

	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);

	pkt = yahoo_packet_new(YAHOO_SERVICE_REMBUDDY, YAHOO_STATUS_AVAILABLE, yd->id);
	yahoo_packet_hash(pkt, 1, yd->user);
	yahoo_packet_hash(pkt, 7, who);
	yahoo_packet_hash(pkt, 65, old_group);
	yahoo_send_packet(yd, pkt, 0);
	yahoo_packet_free(pkt);
}

void yahoo_conference_addinvite(int id, const char * from, const char *who, const char *room, const YList * members, const char *msg)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFADDINVITE, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 51, who);
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 58, msg);
	yahoo_packet_hash(pkt, 13, "0");
	for(; members; members = members->next) {
		yahoo_packet_hash(pkt, 52, (char *)members->data);
		yahoo_packet_hash(pkt, 53, (char *)members->data);
	}
	// 52, 53 -> other members?

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_invite(int id, const char * from, YList *who, const char *room, const char *msg)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFINVITE, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	yahoo_packet_hash(pkt, 50, yd->user);
	for(; who; who = who->next) {
		yahoo_packet_hash(pkt, 52, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 58, msg);
	yahoo_packet_hash(pkt, 13, "0");

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_logon(int id, const char *from, YList *who, const char *room)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFLOGON, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for(; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_decline(int id, const char * from, YList *who, const char *room, const char *msg)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFDECLINE, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for(; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 14, msg);

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_logoff(int id, const char * from, YList *who, const char *room)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFLOGOFF, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for(; who; who = who->next) {
		yahoo_packet_hash(pkt, 3, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

void yahoo_conference_message(int id, const char * from, YList *who, const char *room, const char *msg)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_packet *pkt;
		
	if(!yd)
		return;

	pkt = yahoo_packet_new(YAHOO_SERVICE_CONFMSG, YAHOO_STATUS_AVAILABLE, yd->id);

	yahoo_packet_hash(pkt, 1, (from?from:yd->user));
	for(; who; who = who->next) {
		yahoo_packet_hash(pkt, 53, (char *)who->data);
	}
	yahoo_packet_hash(pkt, 57, room);
	yahoo_packet_hash(pkt, 14, msg);

	yahoo_send_packet(yd, pkt, 0);

	yahoo_packet_free(pkt);
}

int yahoo_send_file(int id, const char *who, const char *msg, const char *name, long size)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	struct yahoo_data *nyd;
	struct yahoo_packet *pkt = NULL;
	char size_str[10];
	long content_length=0;
	char buff[1024];
	char url[255];

	if(!yd)
		return -1;

	nyd = y_new0(struct yahoo_data, 1);
	nyd->id = yd->id;
	nyd->client_id = ++last_id;
	nyd->user = strdup(yd->user);
	nyd->cookie_y = strdup(yd->cookie_y);
	nyd->cookie_t = strdup(yd->cookie_t);
	nyd->type = YAHOO_CONNECTION_HTTP;

	pkt = yahoo_packet_new(YAHOO_SERVICE_FILETRANSFER, YAHOO_STATUS_AVAILABLE, nyd->id);

	snprintf(size_str, sizeof(size_str), "%ld", size);

	yahoo_packet_hash(pkt, 0, nyd->user);
	yahoo_packet_hash(pkt, 5, who);
	yahoo_packet_hash(pkt, 14, msg);
	yahoo_packet_hash(pkt, 27, name);
	yahoo_packet_hash(pkt, 28, size_str);

	content_length = YAHOO_PACKET_HDRLEN + yahoo_packet_length(pkt);

	snprintf(url, sizeof(url), "http://%s:%s/notifyft", 
			filetransfer_host, filetransfer_port);
	nyd->fd = yahoo_http_post(url, nyd, content_length + 4 + size);

	add_to_list(nyd, nyd->fd);

	yahoo_send_packet(nyd, pkt, 8);
	yahoo_packet_free(pkt);

	snprintf(buff, sizeof(buff), "29");
	buff[2] = 0xc0;
	buff[3] = 0x80;
	
	write(nyd->fd, buff, 4);

/*	YAHOO_CALLBACK(ext_yahoo_add_handler)(nyd->client_id, nyd->fd, YAHOO_INPUT_READ); */

	return nyd->fd;

	/*
	while(yahoo_tcp_readline(buff, sizeof(buff), nyd->fd) > 0) {
		if(!strcmp(buff, ""))
			break;
	}

	yahoo_close(nyd);
	*/
}


enum yahoo_status yahoo_current_status(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return YAHOO_STATUS_OFFLINE;
	return yd->current_status;
}

const YList * yahoo_get_buddylist(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return NULL;
	return yd->buddies;
}

const YList * yahoo_get_ignorelist(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return NULL;
	return yd->ignore;
}

const YList * yahoo_get_identities(int id)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return NULL;
	return yd->identities;
}

const char * yahoo_get_cookie(int id, const char *which)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return NULL;
	if(!strncasecmp(which, "y", 1))
		return yd->cookie_y;
	if(!strncasecmp(which, "t", 1))
		return yd->cookie_t;
	if(!strncasecmp(which, "c", 1))
		return yd->cookie_c;
	if(!strncasecmp(which, "login", 5))
		return yd->login_cookie;
	return NULL;
}

int yahoo_get_url_handle(int id, const char *url, char *filename, unsigned long *filesize)
{
	struct yahoo_data *yd = find_conn_by_id(id);
	if(!yd)
		return 0;

	return yahoo_get_url_fd(url, yd, filename, filesize);
}

const char * yahoo_get_profile_url( void )
{
	return profile_url;
}

