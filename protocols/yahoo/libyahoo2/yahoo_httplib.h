#ifndef YAHOO_HTTPLIB_H
#define YAHOO_HTTPLIB_H

#include "yahoo2_types.h"

int yahoo_tcp_readline(char *ptr, int maxlen, int fd);
char *yahoo_urlencode(const char *instr);
char *yahoo_urldecode(const char *instr);
int yahoo_http_post(char *url, struct yahoo_data *yd, long size);
int yahoo_http_get(char *url, struct yahoo_data *yd);
int yahoo_get_url_fd(char *url, struct yahoo_data *yd, 
		char *filename, unsigned long *filesize);

#endif
