/***************************************************************************
                          oscartypes.h  -  description
                             -------------------
    begin                : Fri Sep 26 2003

    Copyright (c) 2003 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARTYPES_H
#define OSCARTYPES_H

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

const BYTE MSG_FLAG_GETAUTO	=	0x03; // flag marking away-message request
const BYTE MSG_FLAG_MASS	=	0x80; // The message was sent to several recipients.

const BYTE MSG_AUTO			= 0x00; // An automatic message.
const BYTE MSG_NORM			= 0x01; // A plain normal message.
const BYTE MSG_CHAT			= 0x02; // A chat request.
const BYTE MSG_FILE			= 0x03; // A file transfer request.
const BYTE MSG_URL			= 0x04; // An URL message. The message consists of the description and the url.
const BYTE MSG_AUTHREQ		= 0x06; // An authorization request.
const BYTE MSG_AUTHREJ		= 0x07; // An authorization reject message.
const BYTE MSG_AUTHACC		= 0x08; // An authorization accept message.
const BYTE MSG_ADDED		= 0x0c; // You were added to the sender's contact list.
const BYTE MSG_WEB			= 0x0d; // A message sent through www.icq.com's web pager.
const BYTE MSG_EMAIL		= 0x0e; // A message sent through the email pager ([uin]@pager.icq.com).
const BYTE MSG_CONTACT		= 0x13; // A contact list message.
const BYTE MSG_EXTENDED		= 0x1a; // An extended message. The packet will contain more data.
const BYTE MSG_GET_AWAY		= 0xe8; // A message requesting the "away" auto message.
const BYTE MSG_GET_OCC		= 0xe9; // A message requesting the "occupied" auto message.
const BYTE MSG_GET_NA		= 0xea; // A message requesting the "not available" auto message.
const BYTE MSG_GET_DND		= 0xeb; // A message requesting the "do not disturb" auto message.
const BYTE MSG_GET_FFC		= 0xec; // A message requesting the "free for chat" auto message.


// taken from micq, now that's code I understand :)
const WORD MSG_FLAG_UNKNOWN		= 0x0001;
const WORD MSG_FLAG_CONTACTLIST = 0x0004;
const WORD MSG_FLAG_REAL		= 0x0010;
const WORD MSG_FLAG_LIST		= 0x0020;
const WORD MSG_FLAG_URGENT		= 0x0040;
const WORD MSG_FLAG_INV			= 0x0080;
const WORD MSG_FLAG_AWAY		= 0x0100;
const WORD MSG_FLAG_OCC			= 0x0200;
const WORD MSG_FLAG_NA			= 0x0800;
const WORD MSG_FLAG_DND			= 0x1000;

#endif
