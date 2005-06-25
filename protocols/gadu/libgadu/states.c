/* $Id: events.c 427160 2005-06-19 20:47:35Z gj $ */

/*
 *  (C) Copyright 2001-2003 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Wo?ny <speedy@ziew.org>
 *                          Arkadiusz Mi?kiewicz <arekm@pld-linux.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libgadu-config.h"

#include <errno.h>
#ifdef __GG_LIBGADU_HAVE_PTHREAD
#  include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifdef __GG_LIBGADU_HAVE_OPENSSL
#  include <openssl/err.h>
#  include <openssl/x509.h>
#endif

#include "compat.h"
#include "libgadu.h"

#define __GG_EVENTS
#include "states.h"

static void failed_resolving(struct gg_session *sess, struct gg_event *e)
{
	e->type = GG_EVENT_CONN_FAILED;
	e->event.failure = GG_FAILURE_RESOLVING;
	sess->state = GG_STATE_IDLE;
}

static void failed_connecting(struct gg_session *sess, struct gg_event *e)
{
	int errno2;
	if (sess->fd != -1) {
		errno2 = errno;
		close(sess->fd);
		errno = errno2;
		sess->fd = -1;
	}
	e->type = GG_EVENT_CONN_FAILED;
	e->event.failure = GG_FAILURE_CONNECTING;
	sess->state = GG_STATE_IDLE;
}

void gg_state_state_reading(struct gg_session *sess, struct gg_event *e)
{
	char buf[1024], *tmp, *host, *foo, *sysmsg_buf = NULL;
	char tmp2[1024];
	int len = 0;
	int port = GG_DEFAULT_PORT;
	struct in_addr addr;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_reading_data(%p);\n", sess);

	/* czytamy lini? z gniazda i obcinamy \r\n. */
	gg_read_line(sess->fd, buf, sizeof(buf) - 1);
	gg_chomp(buf);
	gg_debug(GG_DEBUG_TRAFFIC, "// gg_watch_fd() received http header (%s)\n", buf);

	/* sprawdzamy, czy wszystko w porz?dku. */
	if (strncmp(buf, "HTTP/1.", 7) || strncmp(buf + 9, "200", 3)) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() that's not what we've expected, trying direct connection\n");

		close(sess->fd);

		/* je?li otrzymali?my jakie? dziwne informacje,
			* próbujemy si? ??czy? z pomini?ciem huba. */
		if (sess->proxy_addr && sess->proxy_port) {
			if ((sess->fd = gg_connect(&sess->proxy_addr, sess->proxy_port, sess->async)) == -1) {
				/* trudno. nie wysz?o. */
				gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", errno, strerror(errno));
				failed_connecting(sess, e);
				return;
			}

			sess->state = GG_STATE_CONNECTING_GG;
			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;
			return;
		}

		sess->port = GG_DEFAULT_PORT;

		/* ??czymy si? na port 8074 huba. */
		if ((sess->fd = gg_connect(&sess->hub_addr, sess->port, sess->async)) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", errno, strerror(errno));

			sess->port = GG_HTTPS_PORT;

			/* ??czymy si? na port 443. */
			if ((sess->fd = gg_connect(&sess->hub_addr, sess->port, sess->async)) == -1) {
				gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
				failed_connecting(sess, e);
				return;
			}
		}

		sess->state = GG_STATE_CONNECTING_GG;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;
		return;
	}

	/* ignorujemy reszt? nag?ówka. */
	while (strcmp(buf, "\r\n") && strcmp(buf, ""))
		gg_read_line(sess->fd, buf, sizeof(buf) - 1);

	/* czytamy pierwsz? lini? danych. */
	gg_read_line(sess->fd, buf, sizeof(buf) - 1);
	gg_chomp(buf);

	/* je?li pierwsza liczba w linii nie jest równa zeru,
		* oznacza to, ?e mamy wiadomo?? systemow?. */
	if (atoi(buf)) {

		while (gg_read_line(sess->fd, buf, sizeof(tmp2) - 1)) {
			if (!(foo = realloc(sysmsg_buf, len + strlen(tmp2) + 2))) {
				gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() out of memory for system message, ignoring\n");
				break;
			}

			sysmsg_buf = foo;

			if (!len)
				strcpy(sysmsg_buf, tmp2);
			else
				strcat(sysmsg_buf, tmp2);

			len += strlen(tmp2);
		}

		e->event.msg.msgclass = atoi(buf);
		e->type = GG_EVENT_MSG;
		e->event.msg.sender = 0;
		e->event.msg.message = sysmsg_buf;
	}

	close(sess->fd);

	gg_debug(GG_DEBUG_TRAFFIC, "// gg_watch_fd() received http data (%s)\n", buf);

	/* analizujemy otrzymane dane. */
	tmp = buf;

	while (*tmp && *tmp != ' ')
		tmp++;
	while (*tmp && *tmp == ' ')
		tmp++;
	host = tmp;
	while (*tmp && *tmp != ' ')
		tmp++;
	*tmp = 0;

	if ((tmp = strchr(host, ':'))) {
		*tmp = 0;
		port = atoi(tmp+1);
	}

	addr.s_addr = inet_addr(host);
	sess->server_addr = addr.s_addr;

	if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port) {
		/* je?li mamy proxy, ??czymy si? z nim. */
		if ((sess->fd = gg_connect(&sess->proxy_addr, sess->proxy_port, sess->async)) == -1) {
			/* nie wysz?o? trudno. */
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", errno, strerror(errno));
			failed_connecting(sess, e);
		}

		sess->state = GG_STATE_CONNECTING_GG;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;
		return;
	}

	sess->port = port;

	/* ??czymy si? z w?a?ciwym serwerem. */
	if ((sess->fd = gg_connect(&addr, sess->port, sess->async)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", errno, strerror(errno));

		sess->port = GG_HTTPS_PORT;

		/* nie wysz?o? próbujemy portu 443. */
		if ((sess->fd = gg_connect(&addr, GG_HTTPS_PORT, sess->async)) == -1) {
			/* ostatnia deska ratunku zawiod?a?
				* w takim razie zwijamy manatki. */
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
			failed_connecting(sess, e);
		}
	}

	sess->state = GG_STATE_CONNECTING_GG;
	sess->check = GG_CHECK_WRITE;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return;
}

void gg_state_connecting_gg(struct gg_session *sess, struct gg_event *e)
{
	int res = 0, res_size = sizeof(res);

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_connecting_gg(%p);\n", sess);

	/* je?li wyst?pi? b??d podczas ??czenia si?... */
	if (sess->async && (sess->timeout == 0 || getsockopt(sess->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) || res)) {
		/* je?li nie uda?o si? po??czenie z proxy,
			* nie mamy czego próbowa? wi?cej. */
		if (sess->proxy_addr && sess->proxy_port) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", res, strerror(res));
			failed_connecting(sess, e);
			return;
		}

		close(sess->fd);
		sess->fd = -1;

#ifdef ETIMEDOUT
		if (sess->timeout == 0)
			errno = ETIMEDOUT;
#endif

#ifdef __GG_LIBGADU_HAVE_OPENSSL
		/* je?li logujemy si? po TLS, nie próbujemy
			* si? ??czy? ju? z niczym innym w przypadku
			* b??du. nie do??, ?e nie ma sensu, to i
			* trzeba by si? bawi? w tworzenie na nowo
			* SSL i SSL_CTX. */

		if (sess->ssl) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", res, strerror(res));
			failed_connecting(sess, e);
			return;
		}
#endif

		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), trying https\n", res, strerror(res));

		sess->port = GG_HTTPS_PORT;

		/* próbujemy na port 443. */
		if ((sess->fd = gg_connect(&sess->server_addr, sess->port, sess->async)) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s)\n", errno, strerror(errno));
			failed_connecting(sess, e);
			return;
		}
	}

	gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connected\n");

	if (gg_proxy_http_only)
		sess->proxy_port = 0;

	/* je?li mamy proxy, wy?lijmy zapytanie. */
	if (sess->proxy_addr && sess->proxy_port) {
		char buf[100], *auth = gg_proxy_auth();
		struct in_addr addr;

		if (sess->server_addr)
			addr.s_addr = sess->server_addr;
		else
			addr.s_addr = sess->hub_addr;

		snprintf(buf, sizeof(buf), "CONNECT %s:%d HTTP/1.0\r\n", inet_ntoa(addr), sess->port);

		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() proxy request:\n//   %s", buf);

		/* wysy?amy zapytanie. jest ono na tyle krótkie,
			* ?e musi si? zmie?ci? w buforze gniazda. je?li
			* write() zawiedzie, sta?o si? co? z?ego. */
		if (write(sess->fd, buf, strlen(buf)) < (signed)strlen(buf)) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
			failed_connecting(sess, e);
			return;
		}

		if (auth) {
			gg_debug(GG_DEBUG_MISC, "//   %s", auth);
			if (write(sess->fd, auth, strlen(auth)) < (signed)strlen(auth)) {
				gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
				failed_connecting(sess, e);
				return;
			}

			free(auth);
		}

		if (write(sess->fd, "\r\n", 2) < 2) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() can't send proxy request\n");
			failed_connecting(sess, e);
			return;
		}
	}

#ifdef __GG_LIBGADU_HAVE_OPENSSL
	if (sess->ssl) {
		SSL_set_fd(sess->ssl, sess->fd);

		sess->state = GG_STATE_TLS_NEGOTIATION;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;

		return;
	}
#endif

	sess->state = GG_STATE_READING_KEY;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return;
}

void gg_state_connecting_hub(struct gg_session *sess, struct gg_event *e)
{
	char buf[1024], *client, *auth;
	int res = 0, res_size = sizeof(res);
	const char *host, *appmsg;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_connecting_hub(%p);\n", sess);

	/* je?li asynchroniczne, sprawdzamy, czy nie wyst?pi?
		* przypadkiem jaki? b??d. */
	if (sess->async && (getsockopt(sess->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) || res)) {
		/* no tak, nie uda?o si? po??czy? z proxy. nawet
			* nie próbujemy dalej. */
		if (sess->proxy_addr && sess->proxy_port) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection to proxy failed (errno=%d, %s)\n", res, strerror(res));
			failed_connecting(sess, e);
			return;
		}

		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection to hub failed (errno=%d, %s), trying direct connection\n", res, strerror(res));
		close(sess->fd);

		if ((sess->fd = gg_connect(&sess->hub_addr, GG_DEFAULT_PORT, sess->async)) == -1) {
			/* przy asynchronicznych, gg_connect()
				* zwraca -1 przy b??dach socket(),
				* ioctl(), braku routingu itd. dlatego
				* nawet nie próbujemy dalej. */
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() direct connection failed (errno=%d, %s), critical\n", errno, strerror(errno));
			failed_connecting(sess, e);
			return;
		}

		sess->state = GG_STATE_CONNECTING_GG;
		sess->check = GG_CHECK_WRITE;
		sess->timeout = GG_DEFAULT_TIMEOUT;
		return;
	}

	gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connected to hub, sending query\n");

	if (!(client = gg_urlencode((sess->client_version) ? sess->client_version : GG_DEFAULT_CLIENT_VERSION))) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() out of memory for client version\n");
		failed_connecting(sess, e);
	}

	if (!gg_proxy_http_only && sess->proxy_addr && sess->proxy_port)
		host = "http://" GG_APPMSG_HOST;
	else
		host = "";

#ifdef __GG_LIBGADU_HAVE_OPENSSL
	if (sess->ssl)
		appmsg = "appmsg3.asp";
	else
#endif
		appmsg = "appmsg2.asp";

	auth = gg_proxy_auth();

	snprintf(buf, sizeof(buf) - 1,
		"GET %s/appsvc/%s?fmnumber=%u&version=%s&lastmsg=%d HTTP/1.0\r\n"
		"Host: " GG_APPMSG_HOST "\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Pragma: no-cache\r\n"
		"%s"
		"\r\n", host, appmsg, sess->uin, client, sess->last_sysmsg, (auth) ? auth : "");

	if (auth)
		free(auth);

	free(client);

	/* zwolnij pami?? po wersji klienta. */
	if (sess->client_version) {
		free(sess->client_version);
		sess->client_version = NULL;
	}

	gg_debug(GG_DEBUG_MISC, "=> -----BEGIN-HTTP-QUERY-----\n%s\n=> -----END-HTTP-QUERY-----\n", buf);

	/* zapytanie jest krótkie, wi?c zawsze zmie?ci si?
		* do bufora gniazda. je?li write() zwróci mniej,
		* sta?o si? co? z?ego. */
	if (write(sess->fd, buf, strlen(buf)) < (signed)strlen(buf)) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() sending query failed\n");

		e->type = GG_EVENT_CONN_FAILED;
		e->event.failure = GG_FAILURE_WRITING;
		sess->state = GG_STATE_IDLE;
		close(sess->fd);
		sess->fd = -1;
		return;
	}

	sess->state = GG_STATE_READING_DATA;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return;
}

#ifdef __GG_LIBGADU_HAVE_OPENSSL
void gg_state_tls_negotiation(struct gg_session *sess, struct gg_event *e)
{
	int res;
	X509 *peer;
	char buf[1024];

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_resolving(%p);\n", sess);

	if ((res = SSL_connect(sess->ssl)) <= 0) {
		int err = SSL_get_error(sess->ssl, res);

		if (res == 0) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() disconnected during TLS negotiation\n");

			e->type = GG_EVENT_CONN_FAILED;
			e->event.failure = GG_FAILURE_TLS;
			sess->state = GG_STATE_IDLE;
			close(sess->fd);
			sess->fd = -1;
			return;
		}

		if (err == SSL_ERROR_WANT_READ) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to read\n");

			sess->state = GG_STATE_TLS_NEGOTIATION;
			sess->check = GG_CHECK_READ;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			return;
		} else if (err == SSL_ERROR_WANT_WRITE) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() wants to write\n");

			sess->state = GG_STATE_TLS_NEGOTIATION;
			sess->check = GG_CHECK_WRITE;
			sess->timeout = GG_DEFAULT_TIMEOUT;

			return;
		} else {

			ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));

			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() SSL_connect() bailed out: %s\n", buf);

			e->type = GG_EVENT_CONN_FAILED;
			e->event.failure = GG_FAILURE_TLS;
			sess->state = GG_STATE_IDLE;
			close(sess->fd);
			sess->fd = -1;
			return;
		}
	}

	gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() TLS negotiation succeded:\n//   cipher: %s\n", SSL_get_cipher_name(sess->ssl));

	peer = SSL_get_peer_certificate(sess->ssl);

	if (!peer) {
		gg_debug(GG_DEBUG_MISC, "//   WARNING! unable to get peer certificate!\n");
	} else {
		X509_NAME_oneline(X509_get_subject_name(peer), buf, sizeof(buf));
		gg_debug(GG_DEBUG_MISC, "//   cert subject: %s\n", buf);

		X509_NAME_oneline(X509_get_issuer_name(peer), buf, sizeof(buf));
		gg_debug(GG_DEBUG_MISC, "//   cert issuer: %s\n", buf);
	}

	sess->state = GG_STATE_READING_KEY;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return;
}
#endif

void gg_state_reading_key(struct gg_session *sess, struct gg_event *e)
{
	struct gg_header *h;
	struct gg_welcome *w;
	struct gg_login60 l;
	struct in_addr dcc_ip;
	unsigned int hash;
	int errno2;
	unsigned char *password = sess->password;
	int ret;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_reading_key(%p);\n", sess);

	memset(&l, 0, sizeof(l));
	l.dunno2 = 0xbe;

	/* XXX bardzo, bardzo, bardzo g?upi pomys? na pozbycie
		* si? tekstu wrzucanego przez proxy. */
	if (sess->proxy_addr && sess->proxy_port) {
		char buf[100];

		strcpy(buf, "");
		gg_read_line(sess->fd, buf, sizeof(buf) - 1);
		gg_chomp(buf);
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() proxy response:\n//   %s\n", buf);

		while (strcmp(buf, "")) {
			gg_read_line(sess->fd, buf, sizeof(buf) - 1);
			gg_chomp(buf);
			if (strcmp(buf, ""))
				gg_debug(GG_DEBUG_MISC, "//   %s\n", buf);
		}

		/* XXX niech czeka jeszcze raz w tej samej
			* fazie. g?upio, ale dzia?a. */
		sess->proxy_port = 0;

		return;
	}

	/* czytaj pierwszy pakiet. */
	if (!(h = gg_recv_packet(sess))) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() didn't receive packet (errno=%d, %s)\n", errno, strerror(errno));

		e->type = GG_EVENT_CONN_FAILED;
		e->event.failure = GG_FAILURE_READING;
		sess->state = GG_STATE_IDLE;
		errno2 = errno;
		close(sess->fd);
		errno = errno2;
		sess->fd = -1;
		return;
	}

	if (h->type != GG_WELCOME) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() invalid packet received\n");
		free(h);
		close(sess->fd);
		sess->fd = -1;
		errno = EINVAL;
		e->type = GG_EVENT_CONN_FAILED;
		e->event.failure = GG_FAILURE_INVALID;
		sess->state = GG_STATE_IDLE;
		return;
	}

	w = (struct gg_welcome*) ((char*) h + sizeof(struct gg_header));
	w->key = gg_fix32(w->key);

	hash = gg_login_hash(password, w->key);

	gg_debug(GG_DEBUG_DUMP, "// gg_watch_fd() challenge %.4x --> hash %.8x\n", w->key, hash);

	free(h);

	free(sess->password);
	sess->password = NULL;

	dcc_ip.s_addr = gg_dcc_ip;
	gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() gg_dcc_ip = %s\n", inet_ntoa(dcc_ip));

	if (gg_dcc_ip == (unsigned long) inet_addr("255.255.255.255")) {
		struct sockaddr_in sin;
		int sin_len = sizeof(sin);

		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() detecting address\n");

		if (!getsockname(sess->fd, (struct sockaddr*) &sin, &sin_len)) {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() detected address to %s\n", inet_ntoa(sin.sin_addr));
			l.local_ip = sin.sin_addr.s_addr;
		} else {
			gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() unable to detect address\n");
			l.local_ip = 0;
		}
	} else
		l.local_ip = gg_dcc_ip;

	l.uin = gg_fix32(sess->uin);
	l.hash = gg_fix32(hash);
	l.status = gg_fix32(sess->initial_status ? sess->initial_status : GG_STATUS_AVAIL);
	l.version = gg_fix32(sess->protocol_version);
	l.local_port = gg_fix16(gg_dcc_port);
	l.image_size = gg_fix32(sess->image_size);

	if (sess->external_addr && sess->external_port > 1023) {
		l.external_ip = sess->external_addr;
		l.external_port = sess->external_port;
	}

	gg_debug(GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending GG_LOGIN60 packet\n");
	ret = gg_send_packet(sess, GG_LOGIN60, &l, sizeof(l), sess->initial_descr, (sess->initial_descr) ? strlen(sess->initial_descr) : 0, NULL);

	free(sess->initial_descr);
	sess->initial_descr = NULL;

	if (ret == -1) {
		gg_debug(GG_DEBUG_TRAFFIC, "// gg_watch_fd() sending packet failed. (errno=%d, %s)\n", errno, strerror(errno));
		errno2 = errno;
		close(sess->fd);
		errno = errno2;
		sess->fd = -1;
		e->type = GG_EVENT_CONN_FAILED;
		e->event.failure = GG_FAILURE_WRITING;
		sess->state = GG_STATE_IDLE;
		return;
	}

	sess->state = GG_STATE_READING_REPLY;

	return;
}

void gg_state_resolving(struct gg_session *sess, struct gg_event *e)
{
	struct in_addr addr;
	int failed = 0;
	int port = 0;
	int errno2 = 0;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_resolving(%p);\n", sess);

	if (read(sess->fd, &addr, sizeof(addr)) < (signed)sizeof(addr) || addr.s_addr == INADDR_NONE) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() resolving failed\n");
		failed = 1;
		errno2 = errno;
	}

	close(sess->fd);
	sess->fd = -1;

#ifndef __GG_LIBGADU_HAVE_PTHREAD
	waitpid(sess->pid, NULL, 0);
	sess->pid = -1;
#else
	if (sess->resolver) {
		pthread_cancel(*((pthread_t*) sess->resolver));
		free(sess->resolver);
		sess->resolver = NULL;
	}
#endif

	if (failed) {
		errno = errno2;
		failed_resolving(sess, e);
		return;
	}

	/* je?li jeste?my w resolverze i mamy ustawiony port
		* proxy, znaczy, ?e resolvowali?my proxy. zatem
		* wpiszmy jego adres. */
	if (sess->proxy_port)
		sess->proxy_addr = addr.s_addr;

	/* zapiszmy sobie adres huba i adres serwera (do
		* bezpo?redniego po??czenia, je?li hub le?y)
		* z resolvera. */
	if (sess->proxy_addr && sess->proxy_port)
		port = sess->proxy_port;
	else {
		sess->server_addr = sess->hub_addr = addr.s_addr;
		port = GG_APPMSG_PORT;
	}

	gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() resolved, connecting to %s:%d\n", inet_ntoa(addr), port);

	/* ??czymy si? albo z hubem, albo z proxy, zale?nie
		* od tego, co resolvowali?my. */
	if ((sess->fd = gg_connect(&addr, port, sess->async)) == -1) {
		/* je?li w trybie asynchronicznym gg_connect()
			* zwróci b??d, nie ma sensu próbowa? dalej. */
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() connection failed (errno=%d, %s), critical\n", errno, strerror(errno));
		failed_connecting(sess, e);
		return;
	}

	/* je?li podano serwer i ??czmy si? przez proxy,
		* jest to bezpo?rednie po??czenie, inaczej jest
		* do huba. */
	sess->state = (sess->proxy_addr && sess->proxy_port && sess->server_addr) ? GG_STATE_CONNECTING_GG : GG_STATE_CONNECTING_HUB;
	sess->check = GG_CHECK_WRITE;
	sess->timeout = GG_DEFAULT_TIMEOUT;

	return;
}

void gg_state_reading_reply(struct gg_session *sess, struct gg_event *e)
{
	struct gg_header *h;
	int errno2;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_state_reading_reply(%p);\n", sess);

	if (!(h = gg_recv_packet(sess))) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() didn't receive packet (errno=%d, %s)\n", errno, strerror(errno));
		e->type = GG_EVENT_CONN_FAILED;
		e->event.failure = GG_FAILURE_READING;
		sess->state = GG_STATE_IDLE;
		errno2 = errno;
		close(sess->fd);
		errno = errno2;
		sess->fd = -1;
		return;
	}

	if (h->type == GG_LOGIN_OK) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() login succeded\n");
		e->type = GG_EVENT_CONN_SUCCESS;
		sess->state = GG_STATE_CONNECTED;
		sess->timeout = -1;
		sess->status = (sess->initial_status) ? sess->initial_status : GG_STATUS_AVAIL;
		free(h);
		return;
	}

	if (h->type == GG_LOGIN_FAILED) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() login failed\n");
		e->event.failure = GG_FAILURE_PASSWORD;
		errno = EACCES;
	} else if (h->type == GG_NEED_EMAIL) {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() email change needed\n");
		e->event.failure = GG_FAILURE_NEED_EMAIL;
		errno = EACCES;
	} else {
		gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() invalid packet\n");
		e->event.failure = GG_FAILURE_INVALID;
		errno = EINVAL;
	}

	e->type = GG_EVENT_CONN_FAILED;
	sess->state = GG_STATE_IDLE;
	errno2 = errno;
	close(sess->fd);
	errno = errno2;
	sess->fd = -1;
	free(h);

	return;
}
