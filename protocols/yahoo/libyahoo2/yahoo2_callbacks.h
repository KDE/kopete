/*
 * libyahoo2: yahoo2_callbacks.h
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
 * The functions in this file *must* be defined in your client program
 * If you want to use a callback structure instead of direct functions,
 * then you must define USE_STRUCT_CALLBACKS in all files that #include
 * this one.
 *
 * Register the callback structure by calling yahoo_register_callbacks -
 * declared in this file and defined in libyahoo2.c
 */



#ifndef YAHOO2_CALLBACKS_H
#define YAHOO2_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "yahoo2_types.h"

/*
 * yahoo2_callbacks.h
 *
 * Callback interface for libyahoo2
 */

typedef enum {
	YAHOO_INPUT_READ = 1 << 0,
	YAHOO_INPUT_WRITE = 1 << 1,
	YAHOO_INPUT_EXCEPTION = 1 << 2
} yahoo_input_condition;

/*
 * The following functions need to be implemented in the client
 * interface.  They will be called by the library when each
 * event occurs.
 */

/* 
 * should we use a callback structure or directly call functions
 * if you want the structure, you *must* define USE_STRUCT_CALLBACKS
 * both when you compile the library, and when you compile your code
 * that uses the library
 */

#ifdef USE_STRUCT_CALLBACKS
#define YAHOO_CALLBACK_TYPE(x)	(*x)
struct yahoo_callbacks {
#else
#define YAHOO_CALLBACK_TYPE(x)	x
#endif

/*
 * Name: ext_yahoo_login_response
 * 	Called when the login process is complete
 * Params:
 * 	id   - the id that identifies the server connection
 * 	succ - enum yahoo_login_status
 * 	url  - url to reactivate account if locked
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_login_response)(int id, int succ, char *url);




/*
 * Name: ext_yahoo_got_buddies
 * 	Called when the contact list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	buds - the buddy list
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_buddies)(int id, YList * buds);




/*
 * Name: ext_yahoo_got_ignore
 * 	Called when the ignore list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	igns - the ignore list
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_ignore)(int id, YList * igns);





/*
 * Name: ext_yahoo_got_identities
 * 	Called when the contact list is got from the server
 * Params:
 * 	id   - the id that identifies the server connection
 * 	ids  - the identity list
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_identities)(int id, YList * ids);




/*
 * Name: ext_yahoo_status_changed
 * 	Called when remote user's status changes.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	stat - status code (enum yahoo_status)
 * 	msg  - the message if stat == YAHOO_STATUS_CUSTOM
 * 	away - whether the contact is away or not (YAHOO_STATUS_CUSTOM)
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_status_changed)(int id, char *who, int stat, char *msg, int away);




/*
 * Name: ext_yahoo_got_im
 * 	Called when remote user sends you a message.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	msg  - the message - NULL if stat == 2
 * 	tm   - timestamp of message if offline
 * 	stat - message status - 0
 * 				1
 * 				2 == error sending message
 * 				5
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_im)(int id, char *who, char *msg, long tm, int stat);




/*
 * Name: ext_yahoo_got_conf_invite
 * 	Called when remote user sends you a conference invitation.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user inviting you
 * 	room - the room to join
 * 	msg  - the message
 *      members - the initial members of the conference (null terminated list)
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_conf_invite)(int id, char *who, char *room, char *msg, YList *members);




/*
 * Name: ext_yahoo_conf_userdecline
 * 	Called when someone declines to join the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has declined
 * 	room - the room
 * 	msg  - the declining message
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userdecline)(int id, char *who, char *room, char *msg);




/*
 * Name: ext_yahoo_conf_userjoin
 * 	Called when someone joins the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has joined
 * 	room - the room joined
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userjoin)(int id, char *who, char *room);




/*
 * Name: ext_yahoo_conf_userleave
 * 	Called when someone leaves the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who has left
 * 	room - the room left
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_userleave)(int id, char *who, char *room);




/*
 * Name: ext_yahoo_conf_message
 * 	Called when someone messages in the conference.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who messaged
 * 	room - the room
 * 	msg  - the message
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_conf_message)(int id, char *who, char *room, char *msg);




/*
 * Name: ext_yahoo_got_file
 * 	Called when someone sends you a file
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the user who sent the file
 * 	url  - the file url
 * 	expires  - the expiry date of the file on the server (timestamp)
 * 	msg  - the message
 * 	fname- the file name if direct transfer
 * 	fsize- the file size if direct transfer
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_got_file)(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize);




/*
 * Name: ext_yahoo_contact_added
 * 	Called when a contact is added to your list
 * Params:
 * 	id   - the id that identifies the server connection
 * 	myid - the identity he was added to
 * 	who  - who was added
 * 	msg  - any message sent
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_contact_added)(int id, char *myid, char *who, char *msg);




/*
 * Name: ext_yahoo_rejected
 * 	Called when a contact rejects your add
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - who rejected you
 * 	msg  - any message sent
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_rejected)(int id, char *who, char *msg);




/*
 * Name: ext_yahoo_typing_notify
 * 	Called when remote user starts or stops typing.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	stat - 1 if typing, 0 if stopped typing
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_typing_notify)(int id, char *who, int stat);




/*
 * Name: ext_yahoo_game_notify
 * 	Called when remote user starts or stops a game.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	who  - the handle of the remote user
 * 	stat - 1 if game, 0 if stopped gaming
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_game_notify)(int id, char *who, int stat);




/*
 * Name: ext_yahoo_mail_notify
 * 	Called when you receive mail, or with number of messages
 * Params:
 * 	id   - the id that identifies the server connection
 * 	from - who the mail is from - NULL if only mail count
 * 	subj - the subject of the mail - NULL if only mail count
 * 	cnt  - mail count - 0 if new mail notification
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_mail_notify)(int id, char *from, char *subj, int cnt);




/*
 * Name: ext_yahoo_system_message
 * 	System message
 * Params:
 * 	id   - the id that identifies the server connection
 * 	msg  - the message
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_system_message)(int id, char *msg);






/*
 * Name: ext_yahoo_error
 * 	Called on error.
 * Params:
 * 	id   - the id that identifies the server connection
 * 	err  - the error message if num == E_CUSTOM
 * 	fatal- whether this error is fatal to the connection or not
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_error)(int id, char *err, int fatal);








/*
 * Name: ext_yahoo_log
 * 	Called to log a message.
 * Params:
 * 	fmt  - the printf formatted message
 * Returns:
 * 	0
 */
int YAHOO_CALLBACK_TYPE(ext_yahoo_log)(char *fmt, ...);





/*
 * Name: ext_yahoo_add_handler
 * 	Add a listener for the fd
 * Params:
 * 	id   - the id that identifies the server connection
 * 	fd   - the fd on which to listen
 * 	cond - the condition on which to call the callback
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_add_handler)(int id, int fd, yahoo_input_condition cond);




/*
 * Name: ext_yahoo_remove_handler
 * 	Remove the listener for the fd
 * Params:
 * 	id   - the id that identifies the server connection
 * 	fd   - the fd on which to listen
 */
void YAHOO_CALLBACK_TYPE(ext_yahoo_remove_handler)(int id, int fd);





/*
 * Name: ext_yahoo_connect
 * 	Connect to a host:port
 * Params:
 * 	host - the host to connect to
 * 	port - the port to connect on
 * Returns:
 * 	a unix file descriptor to the socket
 */
int YAHOO_CALLBACK_TYPE(ext_yahoo_connect)(char *host, int port);

#ifdef USE_STRUCT_CALLBACKS
};

/*
 * if using a callback structure, call yahoo_register_callbacks
 * before doing anything else
 */
void yahoo_register_callbacks(struct yahoo_callbacks * tyc);
	
#undef YAHOO_CALLBACK_TYPE

#endif

#ifdef __cplusplus
}
#endif

#endif
