/*
 * libyahoo2: yahoo_httplib.c
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

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

#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "yahoo2.h"
#include "yahoo2_callbacks.h"
#include "yahoo_httplib.h"
#include "yahoo_util.h"

#include "yahoo_debug.h"

extern enum yahoo_log_level log_level;
extern int yahoo_connect(char * host, int port);

int yahoo_tcp_readline(char *ptr, int maxlen, int fd)
{
	int n, rc;
	char c;

	for (n = 1; n < maxlen; n++) {
	again:

		if ((rc = read(fd, &c, 1)) == 1) {
			if(c == '\r')			/* get rid of \r */
				continue;
			*ptr = c;
			if (c == '\n')
				break;
			ptr++;
		} else if (rc == 0) {
			if (n == 1)
				return (0);		/* EOF, no data */
			else
				break;			/* EOF, w/ data */
		} else {
			if (errno == EINTR)
				goto again;
			return -1;
		}
	}

	*ptr = 0;
	return (n);
}

static int url_to_host_port_path(const char *url,
		char *host, int *port, char *path)
{
	char *urlcopy=NULL;
	char *slash=NULL;
	char *colon=NULL;
	
	/*
	 * http://hostname
	 * http://hostname/
	 * http://hostname/path
	 * http://hostname/path:foo
	 * http://hostname:port
	 * http://hostname:port/
	 * http://hostname:port/path
	 * http://hostname:port/path:foo
	 */

	if(strstr(url, "http://") == url) {
		urlcopy = strdup(url+7);
	} else {
		WARNING(("Weird url - unknown protocol: %s", url));
		return 0;
	}

	slash = strchr(urlcopy, '/');
	colon = strchr(urlcopy, ':');

	if(!colon || (slash && slash < colon)) {
		*port = 80;
	} else {
		*colon = 0;
		*port = atoi(colon+1);
	}

	if(!slash) {
		strcpy(path, "/");
	} else {
		strcpy(path, slash);
		*slash = 0;
	}

	strcpy(host, urlcopy);
	
	FREE(urlcopy);

	return 1;
}

static int isurlchar(unsigned char c)
{
	return (isalnum(c) || '-' == c || '_' == c);
}

char *yahoo_urlencode(const char *instr)
{
	int ipos=0, bpos=0;
	char *str = NULL;
	int len = strlen(instr);

	if(!(str = y_new(char, 3*len + 1) ))
		return "";

	while(instr[ipos]) {
		while(isurlchar(instr[ipos]))
			str[bpos++] = instr[ipos++];
		if(!instr[ipos])
			break;
		
		snprintf(&str[bpos], 3, "%%%.2x", instr[ipos]);
		bpos+=3;
		ipos++;
	}
	str[bpos]='\0';

	//free extra alloc'ed mem.
	len = strlen(str);
	str = y_renew(char, str, len+1);

	return (str);
}

char *yahoo_urldecode(const char *instr)
{
	int ipos=0, bpos=0;
	char *str = NULL;
	char entity[3]={0,0,0};
	unsigned dec;
	int len = strlen(instr);

	if(!(str = y_new(char, len+1) ))
		return "";

	while(instr[ipos]) {
		while(instr[ipos] && instr[ipos]!='%')
			if(instr[ipos]=='+') {
				str[bpos++]=' ';
				ipos++;
			} else
				str[bpos++] = instr[ipos++];
		if(!instr[ipos])
			break;
		ipos++;
		
		entity[0]=instr[ipos++];
		entity[1]=instr[ipos++];
		sscanf(entity, "%2x", &dec);
		str[bpos++] = (char)dec;
	}
	str[bpos]='\0';

	//free extra alloc'ed mem.
	len = strlen(str);
	str = y_renew(char, str, len+1);

	return (str);
}

static int yahoo_send_http_request(char *host, int port, char *request)
{
	int fd = yahoo_connect(host, port);

	if(fd > 0)
		write(fd, request, strlen(request));

	return fd;
}

int yahoo_http_post(const char *url, const struct yahoo_data *yd, long content_length)
{
	char host[255];
	int port = 80;
	char path[255];
	char buff[1024];
	
	if(!url_to_host_port_path(url, host, &port, path))
		return 0;

	snprintf(buff, sizeof(buff), 
			"POST %s HTTP/1.0\n"
			"Content-length: %ld\n"
			"User-Agent: Mozilla/4.5 [en] (" PACKAGE "/" VERSION ")\n"
			"Host: %s:%d\n"
			"Cookie: Y=%s; T=%s\n"
			"\n",
			path, content_length, 
			host, port,
			yd->cookie_y, yd->cookie_t);

	return yahoo_send_http_request(host, port, buff);
}

int yahoo_http_get(const char *url, const struct yahoo_data *yd)
{
	char host[255];
	int port = 80;
	char path[255];
	char buff[1024];
	
	if(!url_to_host_port_path(url, host, &port, path))
		return 0;

	snprintf(buff, sizeof(buff), 
			"GET %s HTTP/1.0\r\n"
			"Host: %s:%d\r\n"
			"User-Agent: Mozilla/4.6 (libyahoo/1.0)\r\n"
			"Cookie: Y=%s\r\n"
			"\r\n",
			path, host, port, yd->cookie_y);

	return yahoo_send_http_request(host, port, buff);
}

int yahoo_get_url_fd(const char *url, const struct yahoo_data *yd,
		char *filename, unsigned long *filesize)
{
	char *tmp=NULL;
	int fd=0;
	char buff[1024];
	
	fd = yahoo_http_get(url, yd);

	while(yahoo_tcp_readline(buff, 1024, fd) > 0) {
		/* read up to blank line */
		if(!strcmp(buff, ""))
			break;

		if(filesize)
			if( !strncasecmp(buff, "Content-length:", 
					strlen("Content-length:")) ) {
				tmp = strrchr(buff, ' ');
				if(tmp)
					*filesize = atol(tmp);
			}

		if(filename)
			if( !strncasecmp(buff, "Content-disposition:", 
					strlen("Content-disposition:")) ) {
				tmp = strstr(buff, "name=");
				if(tmp) {
					tmp+=strlen("name=");
					if(tmp[0] == '"') {
						char *tmp2;
						tmp++;
						tmp2 = strchr(tmp, '"');
						if(tmp2)
							*tmp2 = '\0';
					}
					strcpy(filename, tmp+strlen("name="));
				}
			}
	}
	
	/* now return the fd */

	return fd;
}

