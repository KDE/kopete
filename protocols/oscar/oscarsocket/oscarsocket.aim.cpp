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
}

#include "oscarsocket.h"

#include <kdebug.h>

// ----------------------------------------------------------------------------------------

const char AIM_MD5_STRING[]		= "AOL Instant Messenger (SM)";
const char AIM_CLIENTSTRING[]	= "AOL Instant Messenger (SM), version 5.1.3036/WIN32";
const WORD AIM_CLIENTID			= 0x0109;
const WORD AIM_MAJOR			= 0x0005;
const WORD AIM_MINOR			= 0x0001;
const WORD AIM_POINT			= 0x0000;
const WORD AIM_BUILD			= 0x0bdc;
const unsigned char AIM_OTHER[]	= { 0x00, 0x00, 0x00, 0xd2 };
const char AIM_COUNTRY[]		= "us";
const char AIM_LANG[] 			= "en";

// ----------------------------------------------------------------------------------------

// Sends login information, actually logs onto the server
void OscarSocket::sendLoginAIM(void)
{
	kdDebug(14150) << k_funcinfo <<  "SEND (CLI_MD5_LOGIN) sending AIM login" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0017,0x0002,0x0000,0x00000000);
	outbuf.addTLV(0x0001,getSN().length(),getSN().latin1());

	char digest[16];
	encodePassword(&digest[0]);
	digest[16] = '\0';  //do this so that addTLV sees a NULL-terminator

	outbuf.addTLV(0x0025, 16, &digest[0]);
	outbuf.addTLV(0x0003, 0x32, AIM_CLIENTSTRING);
	outbuf.addTLV16(0x0016, AIM_CLIENTID);
	outbuf.addTLV16(0x0017, AIM_MAJOR);
	outbuf.addTLV16(0x0018, AIM_MINOR);
	outbuf.addTLV16(0x0019, AIM_POINT);
	outbuf.addTLV16(0x001a, AIM_BUILD);
	outbuf.addTLV(0x0014, 0x0004, reinterpret_cast<const char *>(AIM_OTHER));
	outbuf.addTLV(0x000f, 0x0002, AIM_LANG);
	outbuf.addTLV(0x000e, 0x0002, AIM_COUNTRY);

	//if set, old-style buddy lists will not work... you will need to use SSI
	outbuf.addTLV8(0x004a,0x01);

	sendBuf(outbuf,0x02);
//	kdDebug(14150) << k_funcinfo <<  "emitting connectionChanged" << endl;
//	emit connectionChanged(3,"Sending username and password...");
}

// Parses a minityping notification from the server
void OscarSocket::parseMiniTypeNotify(Buffer &inbuf)
{
//	kdDebug(14150) << k_funcinfo <<  "RECV (SRV_TYPINGNOTIFY)" << endl;
	// Throw away 8 bytes which are all zeros
	inbuf.getBlock(8);

	// Throw away two bytes (0x0001) which are always there
	inbuf.getWord(); // notification channel

	int snlen = inbuf.getByte();
	char *sn = inbuf.getBlock(snlen);
	QString screenName = QString::fromLatin1(sn); // TODO: check if encoding is right
	delete [] sn;

	// Get the actual notification
	WORD notification = inbuf.getWord();

//	kdDebug(14150) << k_funcinfo <<
//		"Determining Minitype from user '" << screenName << "'" << endl;

	switch(notification)
	{
		case 0x0000:
			// Typing finished
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
			kdDebug(14150) << k_funcinfo << "MiniType Error: " << notification << endl;
	}
}

void OscarSocket::sendMiniTypingNotify(const QString &screenName, TypingNotify notifyType)
{
//	kdDebug(14150) << k_funcinfo << "SEND (SRV_TYPINGNOTIFY)" << endl;

	//look for direct connection before sending through server
	#if 0
	OscarConnection *dc = mDirectIMMgr->findConnection(screenName);
	if(dc)
	{
		dc->sendTypingNotify(notifyType);
		return;
	}
	#endif

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
void OscarSocket::encodePassword(char *digest)
{
	md5_state_t state;
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)key, strlen(key));
	md5_append(&state, (const md5_byte_t *)loginPassword.latin1(), loginPassword.length());
	md5_append(&state, (const md5_byte_t *)AIM_MD5_STRING, strlen(AIM_MD5_STRING));
	md5_finish(&state, (md5_byte_t *)digest);
}

/** Handles AOL's evil attempt to thwart 3rd party apps using Oscar.
 *  It requests a length and offset of aim.exe.  We can thwart it with
 *  help from the good people at Gaim */
void OscarSocket::parseMemRequest(Buffer &inbuf)
{
	/* DWORD offset = */ inbuf.getDWord();
	DWORD len = inbuf.getDWord();

	QPtrList<TLV> ql = inbuf.getTLVList();
	ql.setAutoDelete(TRUE);

//	kdDebug(14150) << k_funcinfo <<
//		"requested offset " << offset << ", length " << len << endl;

	if (len == 0)
	{
		kdDebug(14150) << k_funcinfo <<  "Length is 0, hashing null!" << endl;
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


void OscarSocket::sendAIMAway(bool away, const QString &message)
{
	kdDebug(14150) << k_funcinfo << "Called. away = " << away <<
		", message = '" << message << "'" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);

	if (away)
	{
		// user did not provide a messagetext, work around AIM-protocol
		// stupidity, it sets you online if the away message is totally empty
		QString awayText = " ";
		if (!message.isEmpty())
			awayText = message;

		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		outbuf.addTLV(0x0003, defencoding.length(), defencoding.latin1());
		outbuf.addTLV(0x0004, message.length(), message.local8Bit());
		//emit statusChanged(OSCAR_AWAY);
	}
	else //if we send it a tlv with length 0, we become unaway
	{
		outbuf.addTLV(0x0004, 0, "");
		//emit statusChanged(OSCAR_ONLINE);
	}
	sendBuf(outbuf, 0x02);

	//sendUserLocationInfoRequest(getSN(), AIM_LOCINFO_SHORTINFO)
	requestMyUserInfo();
}

void OscarSocket::parseWarningNotify(Buffer &inbuf)
{
	//aol multiplies warning % by 10, don't know why
	int newevil = inbuf.getWord() / 10;
	kdDebug(14150) << k_funcinfo <<
		"Got a warning: new warning level is " << newevil << endl;

	if (inbuf.length() != 0)
	{
		UserInfo u;
		parseUserInfo(inbuf, u);
		emit gotWarning(newevil,u.sn);
	}
	else
		emit gotWarning(newevil,QString::null);
}

void OscarSocket::sendUserLocationInfoRequest(const QString &name, WORD type)
{
	// docs: http://iserverd.khstu.ru/oscar/snac_02_05.html
	kdDebug(14150) << k_funcinfo <<
		"SEND CLI_LOCATIONINFOREQ for '" << name << "'" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0002, 0x0005, 0x0000, 0x00000000);
	outbuf.addWord(type);
	outbuf.addBUIN(name.latin1()); // TODO encoding of aim nicknames?
	sendBuf(outbuf,0x02);
}

void OscarSocket::parseUserLocationInfo(Buffer &inbuf)
{
	// SNAC(2,6)
	// docs: http://iserverd.khstu.ru/oscar/snac_02_06.html

	UserInfo u;
	parseUserInfo(inbuf, u);

	kdDebug(14150) << k_funcinfo <<
		"RECV SRV_LOCATIONINFOREQ for '" << u.sn << "'" << endl;

	QPtrList<TLV> tl = inbuf.getTLVList();
	tl.setAutoDelete(TRUE);

	QString profile;
	QString away;
	for(TLV *cur = tl.first(); cur; cur = tl.next())
	{
		switch(cur->type)
		{
			case 0x0001: //profile text encoding
//				kdDebug(14150) << k_funcinfo << "text encoding is: " << cur->data << endl;
				break;

			case 0x0002: //profile text
				/*kdDebug(14150) << k_funcinfo <<
					"The profile is: '" << cur->data << "'" << endl;*/
				profile += QString::fromAscii(cur->data); // aim always seems to use us-ascii encoding
				break;

			case 0x0003: //away message encoding
//				kdDebug(14150) << k_funcinfo <<
//					"Away message encoding is: " << cur->data << endl;
				break;

			case 0x0004: //away message
				//kdDebug(14150) << k_funcinfo << "Away message is: " << cur->data << endl;
				away += QString::fromAscii(cur->data); // aim always seems to use us-ascii encoding
				emit receivedAwayMessage(u.sn, away);
				break;

			case 0x0005: //capabilities
				//kdDebug(14150) << k_funcinfo << "Got capabilities" << endl;
				break;

			default: //unknown
				kdDebug(14150) << k_funcinfo << "Unknown user info type " << cur->type << endl;
					break;
		};
	}
	tl.clear();
	emit gotUserProfile(u, profile, away);
}

// vim: set noet ts=4 sts=4 sw=4:
