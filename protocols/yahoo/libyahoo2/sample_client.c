/*
 * sample yahoo client based on libyahoo2
 *
 * $Revision$
 * $Date$
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

#include <glib.h>
#include <gtk/gtk.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <yahoo2.h>
#include <yahoo2_callbacks.h>

#define MAX_PREF_LEN 255

static int do_mail_notify = 0;
static int do_yahoo_debug = 0;
static int ignore_system = 0;
static int do_typing_notify = 1;

/* Exported to libyahoo2 */
char pager_host[MAX_PREF_LEN]="scs.yahoo.com";
char pager_port[MAX_PREF_LEN]="5050";
char filetransfer_host[MAX_PREF_LEN]="filetransfer.msg.yahoo.com";
char filetransfer_port[MAX_PREF_LEN]="80";

#define FREE(x)	if(x) { g_free(x); x=NULL; }

typedef struct {
	char yahoo_id[255];
	char password[255];
	guint32 id;
	int fd;
	gint input;
	gint ping_timeout_tag;
	int status;
	char *status_message;
} yahoo_local_account;

typedef struct {
	int id;
	char *label;
} yahoo_idlabel;

typedef struct {
	guint32 id;
	char *who;
} yahoo_authorize_data;

yahoo_idlabel yahoo_status_codes[] = {
	{YAHOO_STATUS_AVAILABLE, "Available"},
	{YAHOO_STATUS_BRB, "BRB"},
	{YAHOO_STATUS_BUSY, "Busy"},
	{YAHOO_STATUS_NOTATHOME, "Not Home"},
	{YAHOO_STATUS_NOTATDESK, "Not at Desk"},
	{YAHOO_STATUS_NOTINOFFICE, "Not in Office"},
	{YAHOO_STATUS_ONPHONE, "On Phone"},
	{YAHOO_STATUS_ONVACATION, "On Vacation"},
	{YAHOO_STATUS_OUTTOLUNCH, "Out to Lunch"},
	{YAHOO_STATUS_STEPPEDOUT, "Stepped Out"},
	{YAHOO_STATUS_INVISIBLE, "Invisible"},
	{YAHOO_STATUS_IDLE, "Idle"},
	{YAHOO_STATUS_OFFLINE, "Offline"},
	{YAHOO_STATUS_CUSTOM, "[Custom]"},
	{YAHOO_STATUS_TYPING, "Typing"},
	{0, NULL}
};

char * yahoo_status_code(enum yahoo_status s)
{
	int i;
	for(i=0; yahoo_status_codes[i].label; i++)
		if(yahoo_status_codes[i].id == s)
			return yahoo_status_codes[i].label;
	return "Unknown";
}

#define YAHOO_DEBUGLOG ext_yahoo_log

int ext_yahoo_log(char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);
	va_end(ap);
	return 0;
}

#define LOG(x) if(do_yahoo_debug) { YAHOO_DEBUGLOG("%s:%d: ", __FILE__, __LINE__); \
	YAHOO_DEBUGLOG x; \
	YAHOO_DEBUGLOG("\n"); }

#define WARNING(x) if(do_yahoo_debug) { YAHOO_DEBUGLOG("%s:%d: warning: ", __FILE__, __LINE__); \
	YAHOO_DEBUGLOG x; \
	YAHOO_DEBUGLOG("\n"); }

#define print_message(x) { printf x; printf("\n"); }

static yahoo_local_account * ylad = NULL;

gint yahoo_ping_timeout_callback(gpointer data)
{
	yahoo_local_account *ylad = data;
	yahoo_keepalive(ylad->id);
	return 1;
}

void ext_yahoo_status_changed(guint32 id, char *who, int stat, char *msg, int away)
{
	if(msg)
		print_message(("%s is now %s", who, msg))
	else
		print_message(("%s is now %s", who, 
					yahoo_status_code(stat)))
}

void ext_yahoo_got_buddies(guint32 id, GList * buds)
{
}

void ext_yahoo_got_ignore(guint32 id, GList * igns)
{
}

void ext_yahoo_got_im(guint32 id, char *who, char *msg, long tm, int stat)
{
	if(stat == 2) {
		LOG(("Error sending message to %s", who));
		return;
	}

	if(!msg)
		return;

	if(tm) {
		char timestr[255];

		strncpy(timestr, ctime(&tm), sizeof(timestr));
		timestr[strlen(timestr) - 1] = '\0';

		print_message(("[Offline message at %s from %s]: %s", 
				timestr, who, msg));
	} else {
		print_message(("%s: %s", who, msg));
	}
}

void ext_yahoo_rejected(guint32 id, char *who, char *msg)
{
	print_message(("%s has rejected you%s%s", who, 
				(msg?" with the message:\n":"."), 
				(msg?msg:"")));
}

void ext_yahoo_contact_added(guint32 id, char *myid, char *who, char *msg)
{
	char buff[1024];
	char choice;

	snprintf(buff, sizeof(buff), "%s, the yahoo user %s has added you to their contact list", myid, who);
	if(msg) {
		strcat(buff, " with the following message:\n");
		strcat(buff, msg);
		strcat(buff, "\n");
	} else {
		strcat(buff, ".  ");
	}
	strcat(buff, "Do you want to allow this [Y/N]?");

/*	print_message((buff));
	scanf("%c", &choice);
	if(choice != 'y' && choice != 'Y')
		yahoo_reject_buddy(id, who, "Thanks, but no thanks.");
*/
}

void ext_yahoo_typing_notify(guint32 id, char *who, int stat)
{
	if(stat && do_typing_notify)
		print_message(("%s is typing...", who));
}

void ext_yahoo_game_notify(guint32 id, char *who, int stat)
{
}

void ext_yahoo_mail_notify(guint32 id, char *from, char *subj, int cnt)
{
	char buff[1024] = {0};
	
	if(!do_mail_notify)
		return;

	if(from && subj)
		snprintf(buff, sizeof(buff), 
				"You have new mail from %s about %s\n", 
				from, subj);
	if(cnt) {
		char buff2[100];
		snprintf(buff2, sizeof(buff2), 
				"You have %d message%s\n", 
				cnt, cnt==1?"":"s");
		strcat(buff, buff2);
	}

	if(buff[0])
		print_message((buff));
}

void ext_yahoo_system_message(guint32 id, char *msg)
{
	if(ignore_system)
		return;

	print_message(("Yahoo System Message: %s", msg));
}

void yahoo_logout()
{
	if (ylad->id <= 0) {
		return;
	}

	if(ylad->ping_timeout_tag) {
		gtk_timeout_remove(ylad->ping_timeout_tag);
		ylad->ping_timeout_tag=0;
	}
	
	yahoo_logoff(ylad->id);

	ylad->status = YAHOO_STATUS_OFFLINE;
	ylad->id = 0;

	print_message(("logged_out"));
}

void ext_yahoo_login(yahoo_local_account * ylad, int login_mode)
{
	LOG(("ext_yahoo_login"));

	ylad->id = yahoo_login(ylad->yahoo_id, ylad->password, login_mode);
	ylad->status = YAHOO_STATUS_OFFLINE;

	if (ylad->id <= 0) {
		print_message(("Could not connect to Yahoo server.  Please verify that you are connected to the net and the pager host and port are correctly entered."));
		return;
	}

	ylad->ping_timeout_tag = gtk_timeout_add(600 * 1000, 
			(void *) yahoo_ping_timeout_callback, ylad);
}

void ext_yahoo_login_response(guint32 id, int succ, char *url)
{
	char buff[1024];

	if(succ == YAHOO_LOGIN_OK) {
		ylad->status = yahoo_current_status(id);
		print_message(("logged in"));
		return;
		
	} else if(succ == YAHOO_LOGIN_PASSWD) {

		snprintf(buff, sizeof(buff), "Could not log into Yahoo service.  Please verify that your username and password are correctly typed.");

	} else if(succ == YAHOO_LOGIN_LOCK) {
		
		snprintf(buff, sizeof(buff), "Could not log into Yahoo service.  Your account has been locked.\nVisit %s to reactivate it.", url);

	} else if(succ == YAHOO_LOGIN_DUPL) {
		snprintf(buff, sizeof(buff), "You have been logged out of the yahoo service, possibly due to a duplicate login.");
	}

	ylad->status = YAHOO_STATUS_OFFLINE;
	print_message((buff));
	yahoo_logout();
}

void ext_yahoo_error(guint32 id, char *err, int fatal)
{
	fprintf(stderr, "Yahoo Error: %s", err);
	if(fatal)
		yahoo_logout();
}

void yahoo_set_current_state(gint yahoo_state)
{
	if (ylad->status == YAHOO_STATUS_OFFLINE && yahoo_state != YAHOO_STATUS_OFFLINE) {
		ext_yahoo_login(ylad, yahoo_state);
		return;
	} else if (ylad->status != YAHOO_STATUS_OFFLINE && yahoo_state == YAHOO_STATUS_OFFLINE) {
		yahoo_logout();
		return;
	}

	ylad->status = yahoo_state;
	if(yahoo_state == YAHOO_STATUS_CUSTOM) {
		if(ylad->status_message)
			yahoo_set_away(ylad->id, yahoo_state, ylad->status_message, 1);
		else
			yahoo_set_away(ylad->id, yahoo_state, "delta p * delta x too large", 1);
	} else
		yahoo_set_away(ylad->id, yahoo_state, NULL, 1);
}

int ext_yahoo_connect(char *host, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int servfd;
	int res;
	char **p;

	if(!(server = gethostbyname(host))) {
		WARNING(("failed to look up server (%s:%d)\n%d: %s", host, port,
					h_errno, hstrerror(h_errno)));
		return -1;
	}

	if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		WARNING(("Socket create error (%d): %s", errno, 
					strerror(errno)));
		return -1;
	}

	LOG(("connecting to %s:%d\n", host, port));

	for (p = server->h_addr_list; *p; p++)
	{
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr, *p, server->h_length);
		serv_addr.sin_port = htons(port);

		res = -1;
		res = connect(servfd, (struct sockaddr *) &serv_addr, 
				sizeof(serv_addr));

		if(res == 0 ) {
			LOG(("connected"));
			return servfd;
		}
	}

	WARNING(("Could not connect to %s:%d\n%d:%s", host, port,
				errno, strerror(errno)));
	close(servfd);
	return -1;
}

/*************************************
 * Callback handling code starts here
 */
typedef struct {
	guint32 id;
	gpointer data;
	int tag;
} yahoo_callback_data;

void yahoo_callback(gpointer data, gint source, GdkInputCondition condition)
{
	yahoo_callback_data *d = data;
	int ret=1;
	char buff[1024]={0};

	if(condition & GDK_INPUT_READ) {
		LOG(("Read"));
		ret = yahoo_read_ready(d->id, source);

		if(ret == -1)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error (%d): %s", errno, 
				strerror(errno));
		else if(ret == 0)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error: Server closed socket");
	}

	if(condition & GDK_INPUT_WRITE) {
		LOG(("Write"));
		ret = yahoo_write_ready(d->id, source);

		if(ret == -1)
			snprintf(buff, sizeof(buff), 
				"Yahoo write error (%d): %s", errno, 
				strerror(errno));
		else if(ret == 0)
			snprintf(buff, sizeof(buff), 
				"Yahoo write error: Server closed socket");
	}

	if(condition & GDK_INPUT_EXCEPTION)
		LOG(("Exception"));
	if(!(condition & (GDK_INPUT_READ | GDK_INPUT_WRITE | GDK_INPUT_EXCEPTION)))
		LOG(("Unknown: %d", condition));

	if(buff[0])
		print_message((buff));
}

GList * handlers = NULL;

void ext_yahoo_add_handler(guint32 id, int fd, yahoo_input_condition cond)
{
	yahoo_callback_data *d = g_new0(yahoo_callback_data, 1);
	d->id = id;
	d->tag = gdk_input_add(fd, cond, yahoo_callback, d);

	handlers = g_list_append(handlers, d);
}

void ext_yahoo_remove_handler(guint32 id, int fd)
{
	GList * l;
	for(l = handlers; l; l = l->next)
	{
		yahoo_callback_data *d = l->data;
		if(d->id == id) {
			gdk_input_remove(d->tag);
			handlers = g_list_remove_link(handlers, l);
			break;
		}
	}
}
/*
 * Callback handling code ends here
 ***********************************/

static void process_commands(char *line)
{
	char *cmd, *to, *msg;

	char *tmp, *start;
	char *copy = strdup(line);

	enum yahoo_status state;

	start = cmd = copy;
	tmp = strchr(copy, ' ');
	if(tmp) {
		*tmp = '\0';
		copy = tmp+1;
	}

	if(!strcasecmp(cmd, "MSG")) {
		// send a message
		to = copy;
		tmp = strchr(copy, ' ');
		if(tmp) {
			*tmp = '\0';
			copy = tmp+1;
		}
		msg = copy;
		if(to && msg)
			yahoo_send_im(ylad->id, to, msg);
		g_free(start);
		return;
	} else if(!strcasecmp(cmd, "STA")) {
		if(isdigit(copy[0])) {
			state = (enum yahoo_status)atoi(copy);
			msg = NULL;
		} else {
			state = YAHOO_STATUS_CUSTOM;
			msg = copy;
		}

		yahoo_set_away(ylad->id, state, msg, 1);

		return;
	} else if(!strcasecmp(cmd, "OFF")) {
		// go offline
		g_free(start);
		gtk_main_quit();
		return;
	} else {
		fprintf(stderr, "Unknown command: %s\n", cmd);
		g_free(start);
		return;
	}
}

static void local_input_callback(gpointer data, gint source, GdkInputCondition cond)
{
	char line[1024] = {0};
	int i;
	char c;
	i=0; c=0;
	do {
		if(read(source, &c, 1) <= 0)
			c='\0';
		if(c == '\r')
			continue;
		if(c == '\n')
			break;
		if(c == '\b') {
			if(!i)
				continue;
			c = '\0';
			i--;
		}
		if(c) {
			line[i++] = c;
			line[i]='\0';
		}
	} while(i<1023 && c != '\n');

	if(line[0])
		process_commands(line);
}
int main(int argc, char * argv[])
{
	int status;
	int log_level;
	int l;

	ylad = g_new0(yahoo_local_account, 1);

	printf("Yahoo Id: ");
	scanf("%s", ylad->yahoo_id);
	printf("Password: ");
	scanf("%s", ylad->password);

	printf("Initial Status: ");
	scanf("%d", &status);

	printf("Log Level: ");
	scanf("%d", &log_level);

	yahoo_set_log_level(log_level);

	gdk_init(&argc, &argv);

	ext_yahoo_login(ylad, status);

	l = gdk_input_add(0, GDK_INPUT_READ, local_input_callback, NULL);
	gtk_main();
	gdk_input_remove(l);

	yahoo_logout();

	return 0;
}

void ext_yahoo_got_conf_invite(guint32 id, char *who, char *room, char *msg, GList *members)
{
}
void ext_yahoo_conf_userdecline(guint32 id, char *who, char *room, char *msg)
{
}
void ext_yahoo_conf_userjoin(guint32 id, char *who, char *room)
{
}
void ext_yahoo_conf_userleave(guint32 id, char *who, char *room)
{
}
void ext_yahoo_conf_message(guint32 id, char *who, char *room, char *msg)
{
}
void ext_yahoo_got_file(guint32 id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize)
{
}
