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


const WORD OSCAR_FAM_1		= 0x0001; // Services
const WORD OSCAR_FAM_2		= 0x0002; // Location
const WORD OSCAR_FAM_3		= 0x0003; // Contacts, adding, removal, statuschanges
const WORD OSCAR_FAM_4		= 0x0004; // ICBM, messaging
const WORD OSCAR_FAM_9		= 0x0009; // BOS, visible/invisible lists
const WORD OSCAR_FAM_11		= 0x000b; // Interval
const WORD OSCAR_FAM_19		= 0x0013; // Roster, Contactlist
const WORD OSCAR_FAM_21		= 0x0015; // icq metasearch, sms, offline messages
const WORD OSCAR_FAM_23		= 0x0017; // new user, registration


const BYTE MSG_FLAG_GETAUTO	= 0x03; // flag marking away-message request
const BYTE MSG_FLAG_MASS	= 0x80; // The message was sent to several recipients.

const BYTE MSG_AUTO		= 0x00; // An automatic message.
const BYTE MSG_NORM		= 0x01; // A plain normal message.
const BYTE MSG_CHAT		= 0x02; // A chat request.
const BYTE MSG_FILE		= 0x03; // A file transfer request.
const BYTE MSG_URL		= 0x04; // An URL message. The message consists of the description and the url.
const BYTE MSG_AUTHREQ		= 0x06; // An authorization request.
const BYTE MSG_AUTHREJ		= 0x07; // An authorization reject message.
const BYTE MSG_AUTHACC		= 0x08; // An authorization accept message.
const BYTE MSG_ADDED		= 0x0c; // You were added to the sender's contact list.
const BYTE MSG_WEB		= 0x0d; // A message sent through www.icq.com's web pager.
const BYTE MSG_EMAIL		= 0x0e; // A message sent through the email pager ([uin]@pager.icq.com).
const BYTE MSG_CONTACT		= 0x13; // A contact list message.
const BYTE MSG_EXTENDED		= 0x1a; // An extended message. The packet will contain more data.
const BYTE MSG_GET_AWAY		= 0xe8; // A message requesting the "away" auto message.
const BYTE MSG_GET_OCC		= 0xe9; // A message requesting the "occupied" auto message.
const BYTE MSG_GET_NA		= 0xea; // A message requesting the "not available" auto message.
const BYTE MSG_GET_DND		= 0xeb; // A message requesting the "do not disturb" auto message.
const BYTE MSG_GET_FFC		= 0xec; // A message requesting the "free for chat" auto message.


// taken from micq, now that's code I understand :)
const WORD MSG_FLAG_UNKNOWN	= 0x0001;
const WORD MSG_FLAG_CONTACTLIST = 0x0004;
const WORD MSG_FLAG_REAL	= 0x0010;
const WORD MSG_FLAG_LIST	= 0x0020;
const WORD MSG_FLAG_URGENT	= 0x0040;
const WORD MSG_FLAG_INV		= 0x0080;
const WORD MSG_FLAG_AWAY	= 0x0100;
const WORD MSG_FLAG_OCC		= 0x0200;
const WORD MSG_FLAG_NA		= 0x0800;
const WORD MSG_FLAG_DND		= 0x1000;


// "to contactlist" means only the contactlist flashes, no sound or other display of incoming message
const WORD P2P_ONLINE	= 0x0000; // user is online, message was received, file transfer accepted
const WORD P2P_REFUSE	= 0x0100; // refused
const WORD P2P_AWAY	= 0x0400; // accepted (to contact list) because of away
const WORD P2P_OCC	= 0x0900; // refused because of occupied (retry by sending to contact list or as urgent)
const WORD P2P_DND	= 0x0a00; // refused because of dnd (retry by sending to contact list)
const WORD P2P_NA	= 0x0e00; // accepted (to contact list) because of na


const WORD AIM_LOCINFO_GENERALINFO	= 0x0001;
const WORD AIM_LOCINFO_SHORTINFO	= 0x0002;
const WORD AIM_LOCINFO_AWAYMESSAGE	= 0x0003;
const WORD AIM_LOCINFO_CAPABILITIES	= 0x0004;


const WORD CLASS_TRIAL		= 0x0001;
const WORD CLASS_ADMINISTRATOR	= 0x0002; // AOL admin
const WORD CLASS_AOL		= 0x0004; // AOL staff user flag
const WORD CLASS_COMMERCIAL	= 0x0008; // AOL commercial account flag
const WORD CLASS_AIM		= 0x0010; // ICQ non-commercial account flag
const WORD CLASS_AWAY		= 0x0020; //  Away status flag
const WORD CLASS_ICQ		= 0x0040; //  ICQ user sign
const WORD CLASS_WIRELESS	= 0x0080; // AOL wireless user
const WORD CLASS_UNKNOWN100	= 0x0100; // Unknown bit
const WORD CLASS_UNKNOWN200	= 0x0200;  // Unknown bit
const WORD CLASS_UNKNOWN400	= 0x0400;  // Unknown bit
//const WORD CLASS_ACTIVEBUDDY	= 0x0400;
const WORD CLASS_UNKNOWN800	= 0x0800; // Unknown bit

#endif

// vim: set noet ts=4 sts=4 sw=4:
