/*
 * libyahoo2: yahoo2_types.h
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

#ifndef YAHOO2_TYPES_H
#define YAHOO2_TYPES_H

#include "yahoo_list.h"

#ifdef __cplusplus
extern "C" {
#endif

enum yahoo_status {
	YAHOO_STATUS_AVAILABLE = 0,
	YAHOO_STATUS_BRB,
	YAHOO_STATUS_BUSY,
	YAHOO_STATUS_NOTATHOME,
	YAHOO_STATUS_NOTATDESK,
	YAHOO_STATUS_NOTINOFFICE,
	YAHOO_STATUS_ONPHONE,
	YAHOO_STATUS_ONVACATION,
	YAHOO_STATUS_OUTTOLUNCH,
	YAHOO_STATUS_STEPPEDOUT,
	YAHOO_STATUS_INVISIBLE = 12,
	YAHOO_STATUS_CUSTOM = 99,
	YAHOO_STATUS_IDLE = 999,
	YAHOO_STATUS_OFFLINE = 0x5a55aa56, /* don't ask */
	YAHOO_STATUS_TYPING = 0x16
};
#define YAHOO_STATUS_GAME	0x2 		/* Games don't fit into the regular status model */

enum yahoo_login_status {
	YAHOO_LOGIN_OK = 0,
	YAHOO_LOGIN_PASSWD = 13,
	YAHOO_LOGIN_LOCK = 14,
	YAHOO_LOGIN_DUPL = 99
};

enum yahoo_error {
	E_IGNOREDUP = 2,
	E_IGNORENONE = 3,
	E_IGNORECONF = 12,
};

enum yahoo_log_level {
	YAHOO_LOG_NONE = 0,
	YAHOO_LOG_FATAL,
	YAHOO_LOG_ERR,
	YAHOO_LOG_WARNING,
	YAHOO_LOG_NOTICE,
	YAHOO_LOG_INFO,
	YAHOO_LOG_DEBUG
};


/* Yahoo style/color directives */
#define YAHOO_COLOR_BLACK "\033[30m"
#define YAHOO_COLOR_BLUE "\033[31m"
#define YAHOO_COLOR_LIGHTBLUE "\033[32m"
#define YAHOO_COLOR_GRAY "\033[33m"
#define YAHOO_COLOR_GREEN "\033[34m"
#define YAHOO_COLOR_PINK "\033[35m"
#define YAHOO_COLOR_PURPLE "\033[36m"
#define YAHOO_COLOR_ORANGE "\033[37m"
#define YAHOO_COLOR_RED "\033[38m"
#define YAHOO_COLOR_OLIVE "\033[39m"
#define YAHOO_STYLE_ITALICON "\033[2m"
#define YAHOO_STYLE_ITALICOFF "\033[x2m"
#define YAHOO_STYLE_BOLDON "\033[1m"
#define YAHOO_STYLE_BOLDOFF "\033[x1m"
#define YAHOO_STYLE_UNDERLINEON "\033[4m"
#define YAHOO_STYLE_UNDERLINEOFF "\033[x4m"
#define YAHOO_STYLE_URLON "\033[lm"
#define YAHOO_STYLE_URLOFF "\033[xlm"

enum yahoo_connection_type {
	YAHOO_CONNECTION_PAGER=0,
	YAHOO_CONNECTION_HTTP=1
};

struct yahoo_data {
	char  *user;
	char  *password;

	char  *cookie_y;
	char  *cookie_t;
	char  *cookie_c;
	char  *login_cookie;

	YList *buddies;
	YList *ignore;
	YList *identities;
	char  *login_id;

	int   fd;
	int   type;

	int   current_status;
	int   initial_status;
	int   logged_in;

	int id;

	int client_id;

	unsigned char	*rxqueue;
	int   rxlen;
	char *rawbuddylist;
	char *ignorelist;
};

struct yahoo_buddy {
	char *group;
	char *id;
	char *real_name;
};

#ifdef __cplusplus
}
#endif

#endif
