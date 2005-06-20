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

#ifndef STATES_H
#define STATES_H

#ifndef __GG_EVENTS
#error "this is not public header, please do not include it directly"
#endif

void gg_state_reading_reply(struct gg_session *, struct gg_event *);
void gg_state_resolving(struct gg_session *, struct gg_event *);
void gg_state_reading_key(struct gg_session *, struct gg_event *);

#ifdef __GG_LIBGADU_HAVE_OPENSSL
void gg_state_tls_negotiation(struct gg_session *, struct gg_event *);
#endif

void gg_state_connecting_hub(struct gg_session *, struct gg_event *);
void gg_state_connecting_gg(struct gg_session *, struct gg_event *);
void gg_state_state_reading(struct gg_session *, struct gg_event *);

#endif
