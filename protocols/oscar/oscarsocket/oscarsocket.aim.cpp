/*
  oscarsocket.aim.cpp  -  AIM specific part of Oscarsocket

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

extern "C"
{
#include "md5.h"
};

#include "oscarsocket.h"

#include "oscarprotocol.h"
#include "oscaraccount.h"
#include "oscardebugdialog.h"

#include <kdebug.h>

// ----------------------------------------------------------------------------------------

#define AIM_MD5_STRING 				"AOL Instant Messenger (SM)"
#define AIM_CLIENTSTRING			"AOL Instant Messenger (SM), version 4.8.2790/WIN32"

#define AIM_CLIENTID					0x0109
#define AIM_MAJOR						0x0004
#define AIM_MINOR						0x0008
#define AIM_POINT						0x0000
#define AIM_BUILD						0x0ae6
static const char AIM_OTHER[] = { 0x00, 0x00, 0x00, 0xbb };
#define AIM_COUNTRY					"us"
#define AIM_LANG						"en"

// ----------------------------------------------------------------------------------------

// Sends login information, actually logs onto the server
void OscarSocket::sendLoginAIM(void)
{
	kdDebug(14150) << k_funcinfo << "Sending AIM login info..." << endl;;
	unsigned char digest[16];
	digest[16] = '\0';  //do this so that addTLV sees a NULL-terminator

	Buffer outbuf;
	outbuf.addSnac(0x0017,0x0002,0x0000,0x00000000);
	outbuf.addTLV(0x0001,getSN().length(),getSN().latin1());

	encodePassword(digest);

	outbuf.addTLV(0x0025,16,(char *)digest);
	outbuf.addTLV(0x0003,0x32,AIM_CLIENTSTRING);
	outbuf.addTLV16(0x0016,AIM_CLIENTID);
	outbuf.addTLV16(0x0017,AIM_MAJOR);
	outbuf.addTLV16(0x0018,AIM_MINOR);
	outbuf.addTLV16(0x0019,AIM_POINT);
	outbuf.addTLV16(0x001a,AIM_BUILD);
	outbuf.addTLV(0x0014,0x0004,AIM_OTHER);
	outbuf.addTLV(0x000f,0x0002,AIM_LANG);
	outbuf.addTLV(0x000e,0x0002,AIM_COUNTRY);

	//if set, old-style buddy lists will not work... you will need to use SSI
	outbuf.addTLV8(0x004a,0x01);

	sendBuf(outbuf,0x02);
	kdDebug(14150) << k_funcinfo << "emitting connectionChanged" << endl;
//	emit connectionChanged(3,"Sending username and password...");
}

// Parses a minityping notification from the server
void OscarSocket::parseMiniTypeNotify(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_TYPINGNOTIFY) SNAC(4,20)" << endl;
	// Throw away 8 bytes which are all zeros
	inbuf.getDWord();
	inbuf.getDWord();

	// Throw away two bytes (0x0001) which are always there
	inbuf.getWord();

	// The length of the screen name
	int snlen = inbuf.getByte();
	kdDebug(14150) << "Trying to find username of length: " << snlen << endl;

	// The screen name
	char *sn = inbuf.getBlock(snlen);
	QString screenName = QString::fromLatin1(sn);
	delete [] sn;

	// Get the actual notification
	WORD notification = inbuf.getWord();

	kdDebug(14150) << "[OSCAR] Determining Minitype from user '" << screenName << "'" << endl;

	switch(notification)
	{
		case 0x0000:
			emit gotMiniTypeNotification(screenName, 0);
			break;
		case 0x0001:
			// Text Typed
			emit gotMiniTypeNotification(screenName, 1);
			break;
		case 0x0002:
			// Typing Started
			emit gotMiniTypeNotification(screenName, 2);
			break;
		default:
			kdDebug(14150) << "[OSCAR] MiniType Error: " << notification << endl;
	}
}

void OscarSocket::sendMiniTypingNotify(QString screenName, TypingNotify notifyType)
{
	kdDebug(14150) << k_funcinfo << "Sending Typing notify " << endl;

	//look for direct connection before sending through server
	OscarConnection *dc = mDirectIMMgr->findConnection(screenName);
	if(dc)
	{
		kdDebug(14150) << k_funcinfo << "Found direct connection, sending typing notify directly" << endl;
		dc->sendTypingNotify(notifyType);
		return;
	}

	Buffer outbuf;
	// This is header stuff for the SNAC
	outbuf.addSnac(0x0004,0x0014,0x0000,0x00000001);
	outbuf.addDWord(0x00000000);
	outbuf.addDWord(0x00000000);
	outbuf.addWord(0x0001);
	// Screenname length is next
	outbuf.addByte(screenName.length());
	// Then the actual screen name
	outbuf.addString(screenName.latin1(), screenName.length());
	// Then the typing status
	switch(notifyType)
	{
		case TypingFinished:
			outbuf.addWord(0x0000);
			break;
		case TextTyped:
			outbuf.addWord(0x0001);
			break;
		case TypingBegun:
			outbuf.addWord(0x0002);
			break;
		default: // error
			return;
	}
	sendBuf(outbuf, 0x02);
} // END OscarSocket::sendMiniTypingNotify()

// encodes a password, outputs to digest
int OscarSocket::encodePassword(unsigned char *digest)
{
	md5_state_t state;
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)key, strlen(key));
	md5_append(&state, (const md5_byte_t *)pass.latin1(), pass.length());
	md5_append(&state, (const md5_byte_t *)AIM_MD5_STRING, strlen(AIM_MD5_STRING));
	md5_finish(&state, (md5_byte_t *)digest);
	return 0;
}

/** Handles AOL's evil attempt to thwart 3rd party apps using Oscar.
 *  It requests a length and offset of aim.exe.  We can thwart it with
 *  help from the good people at Gaim */
void OscarSocket::parseMemRequest(Buffer &inbuf)
{
	DWORD offset = inbuf.getDWord();
	DWORD len = inbuf.getDWord();

	QPtrList<TLV> ql = inbuf.getTLVList();
	ql.setAutoDelete(TRUE);

	kdDebug(14150) << k_funcinfo << "requested offset " << offset << ", length " << len << endl;

	if (len == 0)
	{
		kdDebug(14150) << k_funcinfo << "Length is 0, hashing null!" << endl;
		md5_state_t state;
		BYTE nil = '\0';
		md5_byte_t digest[0x10];
			/*
			* These MD5 routines are stupid in that you have to have
			* at least one append.  So thats why this doesn't look
			* real logical.
			*/
		md5_init(&state);
		md5_append(&state, (const md5_byte_t *)&nil, 0);
		md5_finish(&state, digest);
		Buffer outbuf;
		outbuf.addSnac(0x0001,0x0020,0x0000,0x00000000);
		outbuf.addWord(0x0010); //hash is always 0x10 bytes
		outbuf.addString((char *)digest, 0x10);
		sendBuf(outbuf,0x02);
	}
	ql.clear();
}

// vim: set noet ts=4 sts=4 sw=4:
