/*
    oscarsocket.cpp  -  Oscar Protocol Implementation

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

//define this if you want to get tons of packets printed out
//#define OSCAR_PACKETLOG 1

extern "C"
{
#include "md5.h"
};

#include "oscarsocket.h"

/*
#include <qapplication.h>
#include <unistd.h>*/
#include <stdlib.h>

#include "oscarprotocol.h"
#include "oscaraccount.h"
#include "oscardebugdialog.h"

#include <qdatetime.h>

#include <kfileitem.h>
#include <kdebug.h>
#include <klocale.h>

// ---------------------------------------------------------------------------------------

#define DIRECTCONNECT				0x0f1f

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

#define ICQ_CLIENTSTRING			"ICQ Inc. - Product of ICQ (TM).2002a.5.37.1.3728.85"
#define ICQ_CLIENTID					0x010A
#define ICQ_MAJOR						0x0005
#define ICQ_MINOR						0x0025
#define ICQ_POINT						0x0001
#define ICQ_BUILD						0x0e90
static const char ICQ_OTHER[] = { 0x00, 0x00, 0x00, 0x55 };
#define ICQ_COUNTRY					"us"
#define ICQ_LANG						"en"

#define AIM_CAPS_BUDDYICON			0x00000001
#define AIM_CAPS_VOICE				0x00000002
#define AIM_CAPS_IMIMAGE			0x00000004
#define AIM_CAPS_CHAT				0x00000008
#define AIM_CAPS_GETFILE			0x00000010
#define AIM_CAPS_SENDFILE			0x00000020
#define AIM_CAPS_GAMES				0x00000040
#define AIM_CAPS_SAVESTOCKS		0x00000080
#define AIM_CAPS_SENDBUDDYLIST	0x00000100
#define AIM_CAPS_GAMES2				0x00000200
#define AIM_CAPS_ISICQ				0x00000400
#define AIM_CAPS_APINFO				0x00000800
#define AIM_CAPS_ICQRTF				0x00001000
#define AIM_CAPS_EMPTY				0x00002000
#define AIM_CAPS_ICQSERVERRELAY	0x00004000
#define AIM_CAPS_IS_2001			0x00008000
#define AIM_CAPS_TRILLIANCRYPT	0x00010000
#define AIM_CAPS_LAST				0x00020000

#define KOPETE_AIM_CAPS				AIM_CAPS_IMIMAGE | AIM_CAPS_SENDFILE | AIM_CAPS_GETFILE
// AIM_CAPS_ICQSERVERRELAY commented out until we support
// type-2 icq-messages (SRV_RECVMSG with type=0x0004)
#define KOPETE_ICQ_CAPS				AIM_CAPS_ISICQ | AIM_CAPS_IS_2001 /*| AIM_CAPS_ICQSERVERRELAY*/

static const struct
{
    DWORD flag;
    char data[16];
} aim_caps[] =
{
	/*
	* Chat is oddball.
	*/
	{AIM_CAPS_CHAT,
	{0x74, 0x8f, 0x24, 0x20, 0x62, 0x87, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	/*
	* These are mostly in order.
	*/
	{AIM_CAPS_VOICE,
	{0x09, 0x46, 0x13, 0x41, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_SENDFILE,
	{0x09, 0x46, 0x13, 0x43, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	/*
	* Advertised by the EveryBuddy client.
	*/
	{AIM_CAPS_ISICQ,
	{0x09, 0x46, 0x13, 0x44, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_IMIMAGE,
	{0x09, 0x46, 0x13, 0x45, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_BUDDYICON,
	{0x09, 0x46, 0x13, 0x46, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_SAVESTOCKS,
	{0x09, 0x46, 0x13, 0x47, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_GETFILE,
	{0x09, 0x46, 0x13, 0x48, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_ICQSERVERRELAY,
	{0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	/*
	* Indeed, there are two of these.  The former appears to be correct,
	* but in some versions of winaim, the second one is set.  Either they
	* forgot to fix endianness, or they made a typo. It really doesn't
	* matter which.
	*/
	{AIM_CAPS_GAMES,
	{0x09, 0x46, 0x13, 0x4a, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
	{AIM_CAPS_GAMES2,
	{0x09, 0x46, 0x13, 0x4a, 0x4c, 0x7f, 0x11, 0xd1,
		0x22, 0x82, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_SENDBUDDYLIST,
	{0x09, 0x46, 0x13, 0x4b, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_ICQRTF,
	{0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34,
		0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92}},

	{AIM_CAPS_IS_2001,
	{0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
		0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf}},

	{AIM_CAPS_EMPTY,
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

	{AIM_CAPS_TRILLIANCRYPT,
	{0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb,
		0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00}},

	{AIM_CAPS_APINFO,
	{0xAA, 0x4A, 0x32, 0xB5, 0xF8, 0x84, 0x48, 0xc6,
		0xA3, 0xD7, 0x8C, 0x50, 0x97, 0x19, 0xFD, 0x5B}},

	{AIM_CAPS_LAST,
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
};

// TODO: replace this by some i18n() compatible method
static const char *msgerrreason[] =
{
	"Invalid error",
	"Invalid SNAC",
	"Rate to host",
	"Rate to client",
	"Not logged in",
	"Service unavailable",
	"Service not defined",
	"Obsolete SNAC",
	"Not supported by host",
	"Not supported by client",
	"Refused by client",
	"Reply too big",
	"Responses lost",
	"Request denied",
	"Busted SNAC payload",
	"Insufficient rights",
	"In local permit/deny",
	"Too evil (sender)",
	"Too evil (receiver)",
	"User temporarily unavailable",
	"No match",
	"List overflow",
	"Request ambiguous",
	"Queue full",
	"Not while on AOL"
};

static const int msgerrreasonlen = 25;

OscarSocket::OscarSocket( const QString &connName, const QByteArray &cookie,
	OscarAccount *account, QObject *parent, const char *name)
	: OscarConnection("unknown", connName, Server, cookie, parent,name)
{
	kdDebug(14150) << k_funcinfo << "connName=" << connName << endl;

	connect(this, SIGNAL(connectionClosed()), this, SLOT(OnConnectionClosed()));
	connect(this, SIGNAL(serverReady()), this, SLOT(OnServerReady()));
	key = NULL;
	mCookie = NULL;
	idle = false;
	gotAllRights=0;
	rateClasses.setAutoDelete(TRUE);
	myUserProfile = "Visit the Kopete website at <a href=""http://kopete.kde.org"">http://kopete.kde.org</a>";
	isConnected = false;
	// Save the account we're in
	m_account = account;
}

OscarSocket::~OscarSocket(void)
{
	rateClasses.clear();
}

/** This is called when a connection is established */
void OscarSocket::OnConnect(void)
{
	kdDebug(14150) << k_funcinfo << "Connected to " << peerName() << ", port " << peerPort() << endl;

	mDirectIMMgr = new OncomingSocket(this, address(), DirectIM);
	mFileTransferMgr = new OncomingSocket(this, address(), SendFile, SENDFILE_PORT);

	kdDebug(14150) << k_funcinfo << "address() is " << address().toString() <<
		" mDirectIMMgr->address() is " << mDirectIMMgr->address().toString() << endl;

	emit connectionChanged(1, QString("Connected to %2, port %1").arg(peerPort()).arg(peerName()));
}

/** This function is called when there is data to be read */
void OscarSocket::slotRead(void)
{
	FLAP fl = getFLAP();
	char *buf = new char[fl.length];
	Buffer inbuf;

	if ( fl.error ) //something went wrong, this shouldn't happen
	{
		kdDebug(14150) << k_funcinfo << "FLAP read error occured!" << endl;
		//dump packet, try to recover
		char *tmp = new char[bytesAvailable()];
		readBlock(tmp, bytesAvailable());
		inbuf.setBuf(tmp, bytesAvailable());

		inbuf.print();
		if (hasDebugDialog())
			debugDialog()->addMessageFromServer(inbuf.toString(), connectionName());

		return;
	}

	if (bytesAvailable() < fl.length)
	{
		while (waitForMore(500) < fl.length)
			kdDebug(14150) << k_funcinfo << "Not enough data read yet... waiting" << endl;
	}

	int bytesread = readBlock(buf,fl.length);
	if (bytesAvailable())
		emit readyRead(); //there is another packet waiting to be read

	inbuf.setBuf(buf,bytesread);

#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << k_funcinfo << "Input: " << endl;
	inbuf.print();
#endif
	if(hasDebugDialog())
		debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());

	switch(fl.channel)
	{
		case 0x01: //new connection negotiation channel
		{
			DWORD flapversion;
			flapversion = inbuf.getDWord();
			if (flapversion == 0x00000001)
			{
				emit connAckReceived();
			}
			else
			{
				kdDebug(14150) << k_funcinfo << "Could not read FLAP version on channel 0x01" << endl;
				return;
			}
			break;
		} // END 0x01

		case 0x02: //snac data channel
		{
			SNAC s;
			s = inbuf.getSnacHeader();

			kdDebug(14150) << k_funcinfo << "SNAC(" << s.family << "," << s.subtype << "), id=" << s.id << endl;

			switch(s.family)
			{
				case 0x0001: //generic service controls
				{
					switch(s.subtype)
					{
						case 0x0001:  //error
						{
#ifdef OSCAR_PACKETLOG
							kdDebug(14150) << k_funcinfo << "Generic service error, remaining data is:" << endl;
							inbuf.print();
#endif
							emit protocolError(
							i18n(
								"An unknown error occured. " \
								"Please report this to the Kopete development " \
								"team by visiting http://kopete.kde.org. The error " \
								"message was: \"Generic service error: SNAC(1,1)\""), 0);
							break;
						}
						case 0x0003: //server ready
							parseServerReady(inbuf);
							break;
						case 0x0005: //redirect
							parseRedirect(inbuf);
							break;
						case 0x0007: //rate info request response, SRV_RATES
							parseRateInfoResponse(inbuf);
							break;
						case 0x000f: //my user info
							parseMyUserInfo(inbuf);
							break;
						case 0x000a: //rate change
							parseRateChange(inbuf);
							break;
						case 0x0010: //warning notification
							parseWarningNotify(inbuf);
							break;
						case 0x0013: //message of the day
							parseMessageOfTheDay(inbuf);
							break;
						case 0x0018: //server versions
							parseServerVersions(inbuf);
							break;
						case 0x001f: //requests a memory segment, part of aim.exe  EVIL AOL!!!
							parseMemRequest(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				}

				case 0x0002: //locate service
				{
					switch(s.subtype)
					{
						case 0x0003: //locate rights
							parseLocateRights(inbuf);
							break;
						case 0x0006: //user profile
							parseUserProfile(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0002

				case 0x0003: //buddy services
				{
					switch(s.subtype)
					{
						case 0x0003: //buddy list rights
							parseBuddyRights(inbuf);
							break;
						case 0x000b: //buddy changed status
							parseBuddyChange(inbuf);
							break;
						case 0x000c: //offgoing buddy
							parseOffgoingBuddy(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0003

				case 0x0004: //msg services
				{
					switch(s.subtype)
					{
						case 0x0001: //msg error
							parseError(inbuf);
							break;
						case 0x0005: //msg rights
							parseMsgRights(inbuf);
							break;
						case 0x0007: //incoming IM
							parseIM(inbuf);
							break;
						case 0x000a: //missed messages
							parseMissedMessage(inbuf);
							break;
						case 0x000c: //message ack
							parseMsgAck(inbuf);
							break;
						case 0x0014: // Mini-Typing notification
							parseMiniTypeNotify(inbuf);
							break;
						default: //invalid subtype
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0004

				case 0x0009: //bos service
				{
					switch(s.subtype)
					{
						case 0x0003: //bos rights incoming
							parseBOSRights(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0009

				case 0x0013: //buddy list management
				{
					switch(s.subtype)
					{
						case 0x0003: //ssi rights
							parseSSIRights(inbuf);
							break;
						case 0x0006: //buddy list
							parseSSIData(inbuf);
							break;
						case 0x000e: //server ack
							parseSSIAck(inbuf);
							break;
						default: //invalid subtype
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0013

				case 0x0015: // ICQ CLI_META packets
				{
					switch(s.subtype)
					{
						case 0x0003:
							parseICQ_CLI_META(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					}
				}

				case 0x0017: //authorization family
				{
					switch(s.subtype)
					{
						case 0x0003: //authorization response (and hash) is being sent
							parseAuthResponse(inbuf);
							break;
						case 0x0007: //encryption key is being sent
							parsePasswordKey(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" << s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				}

				default:
					kdDebug(14150) << k_funcinfo << "Unknown SNAC(|" << s.family << "|," << s.subtype << ")" << endl;
			}; // END switch (s.family)
			break;
		} // END channel 0x02

		case 0x03: //FLAP error channel
		{
			kdDebug(14150) << "FLAP error channel, UNHANDLED" << endl;
#ifdef OSCAR_PACKETLOG
			kdDebug(14150) << "Input: " << endl;
			inbuf.print();
#endif
			if(hasDebugDialog())
				debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());
			break;
		} // END channel 0x03

		case 0x04: //close connection negotiation channel
		{
			kdDebug(14150) << "[OSCAR] Got connection close request, length=" << inbuf.getLength() << endl;
			if(hasDebugDialog())
				debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());

			// BEGIN TODO
			// This is a part of icq login procedure,
			// Move this into its own function!
			QPtrList<TLV> lst = inbuf.getTLVList();

			kdDebug(14150) << "contained TLVs:" << endl;
			TLV *t;
			for(t=lst.first(); t; t=lst.next())
			{
				kdDebug(14150) << "TLV(" << t->type << ") with length " << t->length << endl;
			}

			lst.setAutoDelete(TRUE);
			TLV *uin = findTLV(lst,0x0001);
			if(uin)
			{
				kdDebug(14150) << "found TLV(1) [UIN], uin=" << uin->data << endl;
				delete [] uin->data;
			}
			TLV *server = findTLV(lst,0x0005);
			TLV *cook = findTLV(lst,0x0006);

			TLV *err = findTLV(lst,0x0008);
			if (!err)
				err = findTLV(lst,0x0009);
			if (err)
			{
				kdDebug(14150) << "found TLV(8) [ERROR] error=" << ((err->data[0] << 8)|err->data[1]) << endl;
				delete [] err->data;
			}

			TLV *descr = findTLV(lst,0x0004);
			if(!descr)
				descr = findTLV(lst,0x000b);
			if(descr)
			{
				kdDebug(14150) << "found TLV(4) [DESCRIPTION] reason=" << descr->data << endl;
				delete [] descr->data;
			}

			if (server)
			{
				kdDebug(14150) << "found TLV(5) [SERVER]" << endl;
				QString ip = server->data;
				int index = ip.find(':');
				bosServer = ip.left(index);
				ip.remove(0,index+1); //get rid of the colon and everything before it
				bosPort = ip.toInt();
				kdDebug(14150) << "we should reconnect to server " << bosServer <<
					", ip.right(index) is " << ip <<
					", bosPort is " << bosPort << endl;
				delete[] server->data;
			}

			if (cook)
			{
				kdDebug(14150) << "found TLV(6) [COOKIE]" << endl;
				mCookie = cook->data;
				cookielen = cook->length;
				connectToBos();
			}
			// END TODO

			break;
		}

		default: //oh, crap, something's wrong
		{
			kdDebug(14150) << "[OSCAR] Error: channel " << fl.channel << " does not exist" << endl;
#ifdef OSCAR_PACKETLOG
			kdDebug(14150) << "Input: " << endl;
			inbuf.print();
#endif
		}

	} // END switch(fl.channel)

	delete [] buf;
}

/*
void OscarSocket::sendICQserverRequest(unsigned short cmd, unsigned short seq)
{
	Buffer outbuf;
	outbuf.addSnac(0x0015,0x0002,0x0000,seq);

	//message TLV (type 2)
	outbuf.addWord(0x0001);
	int tlvlen = 0;
	tlvlen += 4; // uin
	tlvlen += 2; // SUBCOMMAND
	tlvlen += 2; // SEQUENCE
	// DATA?

	outbuf.addWord(tlvlen);
	outbuf.addDWord(ouruin);
	outbuf.addWord(0x04BA); // request basic info (1210)
	outbuf.addWord(seq);
	outbuf.addDWord(otheruin);
	sendBuf(outbuf,0x02);
}
*/


/** Sends an authorization request to the server */
void OscarSocket::sendLoginRequest(void)
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0017,0x0006,0x0000,0x00000000);
	outbuf.addTLV(0x0001,getSN().length(),getSN().latin1());
	sendBuf(outbuf,0x02);
	emit connectionChanged(2,QString("Requesting login for " + getSN() + "..."));
}

/** encodes a password, outputs to digest */
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

/**
 * taken from libfaim !!!
 * encodePasswordXOR - Encode a password using old XOR method
 * @password: incoming password
 * @encoded: buffer to put encoded password
 *
 * This takes a const pointer to a (null terminated) string
 * containing the unencoded password.  It also gets passed
 * an already allocated buffer to store the encoded password.
 * This buffer should be the exact length of the password without
 * the null.  The encoded password buffer /is not %NULL terminated/.
 */
QCString OscarSocket::encodePasswordXOR()
{
	const char *password = pass.latin1();
	QCString encoded; //(strlen(password));

	kdDebug(14150) << k_funcinfo << endl;
//	kdDebug(14150) << "  unencoded pw='" << password << "'" << endl;
	// v2.1 table, also works for ICQ

	unsigned char encoding_table[] =
	{
		0xf3, 0x26, 0x81, 0xc4,
		0x39, 0x86, 0xdb, 0x92,
		0x71, 0xa3, 0xb9, 0xe6,
		0x53, 0x7a, 0x95, 0x7c
	};
	unsigned int i;
	for (i = 0; i < strlen(password); i++)
	{
		encoded += (password[i] ^ encoding_table[i]);
//		kdDebug(14150) << "adding char '" << (char) (password[i] ^ encoding_table[i]) << "'" << endl;
	}

//	kdDebug(14150) << "  encoded pw='" << encoded << "', length=" << encoded.length() << endl;
	return encoded;
}

/** adds the flap version to the buffer */
void OscarSocket::putFlapVer(Buffer &outbuf)
{
	outbuf.addDWord(0x00000001);
}

/** Called when a connection has been closed */
void OscarSocket::OnConnectionClosed(void)
{
	kdDebug(14150) << k_funcinfo << "Connection closed by server" << endl;

	rateClasses.clear();
	isConnected = false;
	if (mDirectIMMgr)
		delete mDirectIMMgr;

	if (mFileTransferMgr)
		delete mFileTransferMgr;

	emit statusChanged(OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::OFFLINE));
}

/** Called when the server aknowledges the connection */
void OscarSocket::OnConnAckReceived(void)
{
	kdDebug(14150) << k_funcinfo << endl;
	if(m_account->isICQ())
	{
		kdDebug(14150) << k_funcinfo << "ICQ-LOGIN, sending ICQ login" << endl;
		sendLoginICQ();
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "AIM-LOGIN, Sending flap version to server" << endl;
		Buffer outbuf;
		putFlapVer(outbuf);
		sendBuf(outbuf,0x01);
		sendLoginRequest();
	}
}

/** Sends the output buffer, and clears it */
void OscarSocket::sendBuf(Buffer &outbuf, BYTE chan)
{
#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << "[OSCAR] Output: " << endl;
	outbuf.print();
#endif
	if(hasDebugDialog())
		debugDialog()->addMessageFromClient(outbuf.toString(), connectionName());

	outbuf.addFlap(chan);
	if(state() != QSocket::Connected)
	{
		kdDebug(14150) << k_funcinfo << "Socket is NOT open, can't write to it right now" << endl;
	}
	else
	{
		writeBlock(outbuf.getBuf(),outbuf.getLength());
	}
	outbuf.clear();
}

// Logs in the user!
void OscarSocket::doLogin(const QString &host, int port, const QString &s, const QString &password)
{
	kdDebug(14150) << k_funcinfo << endl;
	if (isConnected)
	{
		kdDebug(14150) << k_funcinfo << "We're already connected, aborting." << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo "Connecting to '" << host << "', port=" << port << endl;

	disconnect(this, SIGNAL(connAckReceived()), this, SLOT(OnBosConnAckReceived()));
	connect(this, SIGNAL(connAckReceived()), this, SLOT(OnConnAckReceived()));

	disconnect(this, SIGNAL(connected()), this, SLOT(OnBosConnect()));
	connect(this, SIGNAL(connected()), this, SLOT(OnConnect()));

	setSN(s);
	pass = password;

	connectToHost(host,port);
}

// The program does this when a key is received
void OscarSocket::parsePasswordKey(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "Got the key" << endl;;

	WORD keylen;
	keylen = inbuf.getWord();
	if (key)
		delete [] key;
	key = inbuf.getBlock(keylen);
	sendLoginAIM();
}

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
	emit connectionChanged(3,"Sending username and password...");
}

void OscarSocket::sendLoginICQ(void)
{
	kdDebug(14150) << k_funcinfo << "Sending ICQ login info... (CLI_COOKIE)" << endl;;

	Buffer outbuf;

	putFlapVer(outbuf); // FLAP Version
	outbuf.addTLV(0x0001,getSN().length(),getSN().latin1()); // login name, i.e. ICQ UIN

	QCString encodedPassword = encodePasswordXOR();
	outbuf.addTLV(0x0002,encodedPassword.length(),encodedPassword);

	outbuf.addTLV(0x0003,strlen(ICQ_CLIENTSTRING),ICQ_CLIENTSTRING);
	outbuf.addTLV16(0x0016,ICQ_CLIENTID);
	outbuf.addTLV16(0x0017,ICQ_MAJOR);
	outbuf.addTLV16(0x0018,ICQ_MINOR);
	outbuf.addTLV16(0x0019,ICQ_POINT);
	outbuf.addTLV16(0x001a,ICQ_BUILD);
	outbuf.addTLV(0x0014,0x0004,ICQ_OTHER); // distribution chan
	outbuf.addTLV(0x000e,0x0002,ICQ_COUNTRY);
	outbuf.addTLV(0x000f,0x0002,ICQ_LANG);

//	kdDebug(14150) << "Outbuf length before flap is: " << outbuf.getLength() << endl;

	sendBuf(outbuf,0x01);
	kdDebug(14150) << k_funcinfo "emitting connectionChanged" << endl;
	emit connectionChanged(3,"Sending username and password...");
}


// Called when a cookie is received
void OscarSocket::connectToBos(void)
{
	kdDebug(14150) << k_funcinfo << "Cookie received!... preparing to connect to BOS server" << endl;

	emit connectionChanged(4,"Connecting to server...");

	disconnect(this, SIGNAL(connAckReceived()), this, SLOT(OnConnAckReceived()));
	connect(this, SIGNAL(connAckReceived()), this, SLOT(OnBosConnAckReceived()));

	disconnect(this, SIGNAL(connected()), this, SLOT(OnConnect()));
	connect(this, SIGNAL(connected()), this, SLOT(OnBosConnect()));

	connectToHost(bosServer,bosPort);
}

/** called when a conn ack is recieved for the BOS connection */
void OscarSocket::OnBosConnAckReceived()
{
	kdDebug(14150) << "[OSCAR] Bos server ack'ed us!  Sending auth cookie" << endl;
	sendCookie();
	emit connectionChanged(5,"Connected to server, authorizing...");
}

// Sends the authorization cookie to the BOS server
void OscarSocket::sendCookie(void)
{
	kdDebug(14150) << k_funcinfo << "Mhh, cookies, let's give one to the server" << endl;
	Buffer outbuf;
	putFlapVer(outbuf);
	outbuf.addTLV(0x0006,cookielen, mCookie);
	sendBuf(outbuf,0x01);
}

/** Called when the server is ready for normal commands */
void OscarSocket::OnServerReady(void)
{
	kdDebug(14150) << k_funcinfo << "What is this? [mETz] ==================" << endl;
	emit connectionChanged(6,"Authorization successful, getting info from server");
}

// Gets the rate info from the server
void OscarSocket::sendRateInfoRequest(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_RATESREQUEST)" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0001,0x0006,0x0000,0x00000006);
	sendBuf(outbuf,0x02);
}

/** Parses the rate info response */
void OscarSocket::parseRateInfoResponse(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_RATES), Parsing Rate Info Response" << endl;

	RateClass *rc = NULL;
	WORD numclasses = inbuf.getWord();
//	kdDebug(14150) << k_funcinfo << "number of Rate Classes " << numclasses << endl;

	for (unsigned int i=0;i<numclasses;i++)
	{
		rc = new RateClass;
		rc->classid = inbuf.getWord();
		rc->windowsize = inbuf.getDWord();
		rc->clear = inbuf.getDWord();
		rc->alert = inbuf.getDWord();
		rc->limit = inbuf.getDWord();
		rc->disconnect = inbuf.getDWord();
		rc->current = inbuf.getDWord();
		rc->max = inbuf.getDWord();

		//5 unknown bytes, depending on the 0x0001/0x0017 you send
		for (int j=0;j<5;j++)
				rc->unknown[j] = inbuf.getByte();

		rateClasses.append(rc);
	}

#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << k_funcinfo << "The buffer is " << inbuf.getLength() << " bytes long after reading the classes." << endl;
	kdDebug(14150) << k_funcinfo << "It looks like this: " << endl;
	inbuf.print();
#endif

	//now here come the members of each class
	for (unsigned int i=0;i<numclasses;i++)
	{
		WORD classid = inbuf.getWord();
		WORD count = inbuf.getWord();

//		kdDebug(14150) << "[OSCAR] Classid: " << classid << ", Count: " << count << endl;

		RateClass *tmp;
		for (tmp=rateClasses.first();tmp;tmp=rateClasses.next())  //find the class we're talking about
		{
			if (tmp->classid == classid)
			{
				rc = tmp;
				break;
			}
		}

		for (WORD j=0;j<count;j++)
		{
			SnacPair *s = new SnacPair;
			s->group = inbuf.getWord();
			s->type = inbuf.getWord();
//			kdDebug(14150)  << k_funcinfo << "snacpair; group=" << s->group << ", type=" << s->type << endl;
			if (rc)
				rc->members.append(s);
		}
	}

/*	kdDebug(14150) << "[OSCAR] The buffer is " << inbuf.getLength() <<
		" bytes long after reading the rate info" << endl; */
	if(inbuf.getLength() != 0)
		kdDebug(14150) << k_funcinfo << "Did not parse all Rates successfully!" << endl;
	sendRateAck();
}

// Tells the server we accept it's communist rate limits, even though I have no idea what they mean
void OscarSocket::sendRateAck()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_ACKRATES)" << endl;
	Buffer outbuf;
//	outbuf.addSnac(0x0001,0x0008,0x0000,0x00000008);
	outbuf.addSnac(0x0001,0x0008,0x0000,0x00000000);
	for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
	{
//		kdDebug(14150) << "[OSCAR] adding classid " << rc->classid << " to RateAck" << endl;

		// FIXME what to do in icq-mode?
//		if (rc->classid != 0x0015) //0x0015 is ICQ
			outbuf.addWord(rc->classid);
	}
	sendBuf(outbuf,0x02);
	emit connectionChanged(7,"Completing login...");
	requestInfo();
}

// Called on connection to bos server
void OscarSocket::OnBosConnect()
{
	kdDebug(14150) << k_funcinfo << "Connected to " << peerName() << ", port " << peerPort() << endl;
}

// Sends privacy flags to the server
void OscarSocket::sendPrivacyFlags(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0001,0x0014,0x0000,0x00000000);
	//bit 1: let others see idle time
	//bit 2: let other see member since
	outbuf.addDWord(0x00000003);
	sendBuf(outbuf,0x02);
}

// requests the current user's info
void OscarSocket::requestMyUserInfo()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQINFO)" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0001,0x000e,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

// parse my user info
void OscarSocket::parseMyUserInfo(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo "RECV (SRV_REPLYINFO) Parsing OWN user info" << endl;
	UserInfo u = parseUserInfo(inbuf);
	emit gotMyUserInfo(u);
	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** parse the server's authorization response (which hopefully contains the cookie) */
void OscarSocket::parseAuthResponse(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << endl;

	QPtrList<TLV> lst = inbuf.getTLVList();
	lst.setAutoDelete(TRUE);
	TLV *sn = findTLV(lst,0x0001);  //screen name
	TLV *url = findTLV(lst,0x0004);  //error url
	TLV *bosip = findTLV(lst,0x0005); //bos server address
	TLV *cook = findTLV(lst,0x0006); //authorization cookie
	TLV *email = findTLV(lst,0x0007); //the e-mail address attached to the account
	TLV *regstatus = findTLV(lst,0x0013); //whether the e-mail address is available to others
	TLV *err = findTLV(lst,0x0008); //whether an error occured

	if (mCookie)
		delete[] mCookie;

	if (err)
   {
		QString errorString;
		int errorCode = 0;

		switch((err->data[0] << 8)|err->data[1])
		{
			case 1: { errorString = i18n("Sign on failed because the screen name you provided is not registered on the AIM network. Please visit http://aim.aol.com to create a screen name for use on the AIM network."); errorCode = 1; break;  }
			case 5: { errorString = i18n("Sign on failed because the password supplied for this screen name is invalid. Please check your password and try again."); errorCode = 5; break; }
			case 0x11: { errorString = i18n("Sign on failed because your account is currently suspended."); errorCode = 0x11; break; }
			case 0x14: { errorString = i18n("The AOL Instant Messenger service is temporarily unavailable.  Please try again later."); errorCode = 0x14; break; }
			case 0x18: { errorString = i18n("You have been connecting and disconnecting too frequently. Wait ten minutes and try again. If you continue to try, you will need to wait even longer."); errorCode = 0x18; break; }
			case 0x1c: { errorString = i18n("The client you are using is too old.  Please upgrade."); errorCode = 0x1c; break; }
			default: { errorString = i18n("Authentication failed."); errorCode = (err->data[0] << 8) | err->data[1]; break; }
		}

		emit protocolError(errorString, errorCode);
	}

	if (bosip)
	{
		QString ip = bosip->data;
		int index;
		index = ip.find(':');
		bosServer = ip.left(index);
		ip.remove(0,index+1); //get rid of the colon and everything before it
		bosPort = ip.toInt();

		kdDebug(14150) << "[OSCAR] server is " << bosServer <<
			", ip.right(index) is " << ip <<
			", bosPort is " << bosPort << endl;

		delete[] bosip->data;
	}

	if (cook)
	{
		mCookie = cook->data;
		cookielen = cook->length;
		connectToBos();
	}

	if (sn)
		delete [] sn->data;

	if (email)
		delete [] email->data;

	if (regstatus)
		delete [] regstatus->data;

	lst.clear();

	if (url)
		delete [] url->data;
}

/** finds a tlv of type typ in the list */
TLV * OscarSocket::findTLV(QPtrList<TLV> &l, WORD typ)
{
	TLV *t;
	for(t=l.first();t;t=l.next())
	{
		if (t->type == typ)
			return t;
	}
	return NULL;
}

/** tells the server that the client is ready to receive commands & stuff */
void OscarSocket::sendClientReady(void)
{
	kdDebug(14150) << "SEND (CLI_READY) sending client ready, end of login procedure " <<
		"======================================================" << endl;

	Buffer outbuf;
//	outbuf.addSnac(0x0001,0x0002,0x0000,0x00000002);
	outbuf.addSnac(0x0001,0x0002,0x0000,0x00000000);

	for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
	{
/*		if (rc->classid == 0x0015) //0x0015 is ICQ
			kdDebug(14150) << k_funcinfo "CLI_READY containing SNAC(21,x)" << endl; */

//		if (rc->classid != 0x0015) //0x0015 is ICQ
		{
			outbuf.addWord(rc->classid);

			if (rc->classid == 0x0001 || rc->classid == 0x0013)
				outbuf.addWord(0x0003);
			else
				outbuf.addWord(0x0001);

			if (rc->classid == 0x0008 || rc->classid == 0x000b || rc->classid == 0x000c)
			{
				outbuf.addWord(0x0104);
				outbuf.addWord(0x0001);
			}
			else
			{
				outbuf.addWord(0x0110);
				if(m_account->isICQ())
					outbuf.addWord(0x047b);
				else
					outbuf.addWord(0x059b);
			}
		}
	}
	sendBuf(outbuf,0x02);

/*	KopeteOnlineStatus status;
	if (m_account->isICQ())
		status = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQONLINE);
	else
		status = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ONLINE);
	emit statusChanged( status );*/
	isConnected = true;
}

// Sends versions so that we get proper rate info
void OscarSocket::sendVersions(const WORD *families, const int len)
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0001,0x0017,0x0000,0x00000017);
	for(int i=0;i<len;i++)
	{
		outbuf.addWord(families[i]);
		if (families[i] == 0x0001 || families[i] == 0x0013)
			outbuf.addWord(0x0003);
		else
			outbuf.addWord(0x0001);
	}
	sendBuf(outbuf,0x02);
	//sendRateInfoRequest();
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
		kdDebug(14150) << "[OSCAR] Length is 0, hashing null!" << endl;
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

/** Sets idle time -- time is in minutes */
void OscarSocket::sendIdleTime(DWORD time)
{
	if (!isConnected)
		return;

	kdDebug(14150) << k_funcinfo "SEND (CLI_SNAC1_11), sending idle time, time=" << time << endl;
	bool newidle;
	if (time==0)
		newidle = false;
	else
		newidle = true;

	if (newidle != idle) //only do stuff if idle status changed
	{
		idle = newidle;
		Buffer outbuf;
		outbuf.addSnac(0x0001,0x0011,0x0000,0x00000000);
		outbuf.addDWord(time);
		sendBuf(outbuf,0x02);
	}
}

/** requests ssi data from the server */
void OscarSocket::sendBuddyListRequest(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_CHECKROSTER) Requesting SSI data" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0005,0x0000,0x00000000);
	outbuf.addDWord(0x00000000); // FIXME: contactlist timestamp
	outbuf.addWord(0x0000); // FIXME: contactlist length, same as first Word I get in parseSSIData
	sendBuf(outbuf,0x02);
}


/** parses incoming ssi data */
void OscarSocket::parseSSIData(Buffer &inbuf)
{
	AIMBuddyList blist;

	inbuf.getByte(); //get fmt version
	blist.revision = inbuf.getWord(); //gets the contactlist length

	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYROSTER) received contactlist, length=" << blist.revision << endl;

	while(inbuf.getLength() > 4) //the last 4 bytes are the timestamp
	{
		SSI *ssi = new SSI;
		WORD namelen = inbuf.getWord(); //length of name
		char *name = inbuf.getBlock(namelen); //name
		ssi->name = QString(name);
		ssi->gid = inbuf.getWord();
		ssi->bid = inbuf.getWord();
		ssi->type = inbuf.getWord(); //type of the entry
		ssi->tlvlength = inbuf.getWord(); //length of data
		if (ssi->tlvlength) //sometimes there is additional info
				ssi->tlvlist = inbuf.getBlock(ssi->tlvlength);
		ssiData.append(ssi);
/*
		kdDebug(14150) << "[OSCAR] Read server-side list-entry. name: " << ssi->name << ", group: " << ssi->gid
				<< ", id: " << ssi->bid << ", type: " << ssi->type << ", TLV length: " << ssi->tlvlength
				<< endl;
*/
		AIMBuddy *bud;
		switch (ssi->type)
		{
			case 0x0000: //buddy
			{
				bud = new AIMBuddy(ssi->bid, ssi->gid, ssi->name);
				AIMGroup *group = blist.findGroup(ssi->gid);
				QString groupName = "\"Group not found\"";
				if (group)
					groupName = group->name();
/*				kdDebug(14150) << "[OSCAR] Adding Buddy " << ssi->name <<  " to group " << ssi->gid
						<< " (" <<  groupName << ")" << endl;*/

				Buffer tmpBuf;
				tmpBuf.setBuf(ssi->tlvlist, ssi->tlvlength);
				QPtrList<TLV> lst = tmpBuf.getTLVList();
/*				kdDebug(14150) << "[OSCAR] BUDDY entry contained TLVs:" << endl;
				TLV *t;
				for(t=lst.first(); t; t=lst.next())
					kdDebug(14150) << "TLV(" << t->type << ") with length " << t->length << endl;*/
				lst.setAutoDelete(TRUE);
				TLV *nick = findTLV(lst,0x0131);
				if(nick && nick->length > 0)
				{
//					kdDebug(14150) << "[OSCAR] Buddy nickname='" << nick->data << "'" << endl;
					bud->setAlias(QString::fromLatin1(nick->data));
				}

				blist.addBuddy(bud);
				break;
			}

			case 0x0001: //group
			{
				Buffer tmpBuf;
				tmpBuf.setBuf(ssi->tlvlist, ssi->tlvlength);
/*				QPtrList<TLV> lst = tmpBuf.getTLVList();
				kdDebug(14150) << k_funcinfo << "GROUP entry contained TLVs:" << endl;
				TLV *t;
				for(t=lst.first(); t; t=lst.next())
					kdDebug(14150) << "TLV(" << t->type << ") with length " << t->length << endl;
				lst.setAutoDelete(TRUE);
*/
				if (namelen) //if it's not the master group
					blist.addGroup(ssi->gid, ssi->name);
				break;
			}

			case 0x0002: // TODO permit buddy list AKA visible list
				break;

			case 0x0003: // TODO deny buddy AKA invisible list
			{
				bud = new AIMBuddy(ssi->bid, ssi->gid, ssi->name);
				kdDebug(14150) << "[OSCAR] Adding Buddy " << ssi->name << " to deny list." << endl;
				blist.addBuddyDeny(bud);
				emit denyAdded(ssi->name);
				break;
			}

			case 0x0004: // TODO permit-deny setting
				break;

			case 0x000e: // TODO contact on ignore list
				break;
		} // END switch (ssi->type)

		if (name)
			delete [] name;
	} // END while(inbuf.getLength() > 4)

	blist.timestamp = inbuf.getDWord();
//	kdDebug(14150) << k_funcinfo << "Finished getting buddy list" << endl;
	sendSSIActivate(); // send CLI_ROSTERACK
	emit gotConfig(blist);

	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** Requests the user's BOS rights */
void OscarSocket::requestBOSRights(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQBOS) Requesting BOS rights" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}

/** Parses SSI rights data */
void OscarSocket::parseBOSRights(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYBOS) " << endl;

	QPtrList<TLV> ql = inbuf.getTLVList();
	ql.setAutoDelete(TRUE);
	TLV *t;
	WORD maxpermits = 0, maxdenies = 0;
	if ((t = findTLV(ql,0x0001))) //max permits
		maxpermits = (t->data[0] << 8) | t->data[1];
	if ((t = findTLV(ql,0x0002))) //max denies
		maxdenies = (t->data[0] << 8) | t->data[1];
//	kdDebug(14150) << k_funcinfo << "maxpermits=" << maxpermits << ", maxdenies=" << maxdenies << endl;
	ql.clear();
	//sendGroupPermissionMask();
	//sendPrivacyFlags();

	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** Parses the server ready response */
void OscarSocket::parseServerReady(Buffer &inbuf)
{
	int famcount; //the number of families received
	WORD *families = new WORD[inbuf.getLength()];
	for (famcount = 0; inbuf.getLength(); famcount++)
	{
		families[famcount] = inbuf.getWord();
	}
	sendVersions(families,famcount);
	emit serverReady();
	delete [] families;
}

/** parses server version info */
void OscarSocket::parseServerVersions(Buffer &/*inbuf*/)
{
	//The versions are not important to us at all
	//now we can request rates
	sendRateInfoRequest();
}

/** Parses Message of the day */
void OscarSocket::parseMessageOfTheDay(Buffer &inbuf)
{
	WORD id = inbuf.getWord();
	if (id < 4)
		emit protocolError(i18n("An unknown error occured. Your connection may be lost. The error was: \"AOL MOTD Error: your connection may be lost. ID: %1\"").arg(id), 0);
}

// Requests location rights (CLI_REQLOCATION)
void OscarSocket::requestLocateRights(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQLOCATION), Requesting rights for location service" << endl;
	Buffer buf;
//	buf.addSnac(0x0002,0x0002,0x0000,0x00000002);
	buf.addSnac(0x0002,0x0002,0x0000,0x00000000);
	sendBuf(buf,0x02);
}

/** Requests a bunch of information (permissions, rights, my user info, etc) from server */
void OscarSocket::requestInfo(void)
{
	requestMyUserInfo(); // CLI_REQINFO
	sendSSIRightsRequest();  // CLI_REQLISTS
	sendBuddyListRequest(); // CLI_CHECKROSTER
	requestLocateRights(); // CLI_REQLOCATION
	requestBuddyRights(); // CLI_REQBUDDY
	requestMsgRights(); // CLI_REQICBML
	requestBOSRights(); // CLI_REQBOS
	gotAllRights=0;
	// next received packet should be a SRV_REPLYINFO
}

/** adds a mask of the groups that you want to be able to see you to the buffer */
void OscarSocket::sendGroupPermissionMask(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0004,0x0000,0x00000000);
	outbuf.addDWord(0x0000001f);
	sendBuf(outbuf,0x02);
}

// adds a request for buddy list rights to the buffer
void OscarSocket::requestBuddyRights(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQBUDDY), Requesting rights for buddy service" << endl;
	Buffer outbuf;
//	outbuf.addSnac(0x0003,0x0002,0x0000,0x00000002);
	outbuf.addSnac(0x0003,0x0002,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

// adds a request for msg rights to the buffer
void OscarSocket::requestMsgRights(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQICBM), Requesting rights for ICBM (instant messages)" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0004,0x0000,0x00000004);
	sendBuf(outbuf,0x02);
}

// Parses the locate rights provided by the server (SRV_REPLYLOCATION)
void OscarSocket::parseLocateRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYLOCATION), Ignoring location rights" << endl;
	//we don't care what the locate rights are
	//and we don't know what they mean
	//  requestBuddyRights();

	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** Parses buddy list rights from the server */
void OscarSocket::parseBuddyRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYBUDDY), Ignoring Buddy Rights" << endl;
	// FIXME: write code to parse buddy rights info
	//requestMsgRights();
	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** Parses msg rights info from server */
void OscarSocket::parseMsgRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYICBM) IGNORING ICBM RIGHTS" << endl;
	// FIXME: write code to parse this
	//requestBOSRights();

	// After we get this from the server
	// we have to send some messaging parameters
	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

/** Parses an incoming IM */
void OscarSocket::parseIM(Buffer &inbuf)
{
	Buffer tmpbuf;
	QByteArray cook(8);
	WORD type = 0;
	WORD length = 0;
	//This is probably the hardest thing to do in oscar
	//first comes an 8 byte ICBM cookie (random)
	inbuf.getBlock(8);

	// Channel ID.
	//
	// Channel 0x0001 is the message channel.  There are
	// other channels for things called "rendevous"
	// which represent chat and some of the other new
	// features of AIM2/3/3.5.
	//
	// Channel 0x0002 is the Rendevous channel, which
	// is where Chat Invitiations and various client-client
	// connection negotiations come from.
	//
	// Channel 0x0004 is used for ICQ authorization, or
	// possibly any system notice.
	WORD channel = inbuf.getWord();

	// Extract the standard user info block.
	//
	// Note that although this contains TLVs that appear contiguous
	// with the TLVs read below, they are two different pieces.  The
	// userinfo block contains the number of TLVs that contain user
	// information, the rest are not even though there is no seperation.
	//
	// That also means that TLV types can be duplicated between the
	// userinfo block and the rest of the message, however there should
	// never be two TLVs of the same type in one block.
	UserInfo u = parseUserInfo(inbuf);
	TLV tlv;
	unsigned int remotePort = 0;
	QHostAddress qh;
	QString message;
	WORD msgtype = 0; //used to tell whether it is a direct IM requst, deny, or accept
	DWORD capflag = 0; //used to tell what kind of rendezvous this is
	OncomingSocket *sockToUse; //used to tell which listening socket to use
	QString fileName; //the name of the file to be transferred (if any)
	long unsigned int fileSize = 0; //the size of the file(s) to be transferred

	switch(channel)
	{
		case 0x0001: //normal IM
		{
			// Flag to indicate if there are more TLV's to parse
			bool moreTLVs = true;
			// This gets set if we are notified of an auto response
			bool isAutoResponse = false;
			while( moreTLVs )
			{
				kdDebug(14150) << k_funcinfo << "Got a normal IM block from '" << u.sn << "'" << endl;
				type = inbuf.getWord();
				switch(type)
				{
					case 0x0002: //message block
					{
						// This is the total length of the rest of this message TLV
						length = inbuf.getWord();

						//first comes 0x0501 (don't know what it is)
						inbuf.getWord();
						//next comes the features length, followed by the features
						int featureslen;
						featureslen = inbuf.getWord();
						inbuf.getBlock(featureslen);
						// Next is two bytes of static 0x0101
						inbuf.getWord();

						// Next comes the length of the message block
						WORD msglen = inbuf.getWord();
						//unicode encoding of the message, mostly 0x0000 0x0000
						inbuf.getWord();
						inbuf.getWord();

						//strip off the unicode info length
						msglen -= 4;
						// Get the message
						char *msg = inbuf.getBlock(msglen);
						message = msg;
						delete [] msg;
						kdDebug(14150) << "[OSCAR] IM text: " << message << endl;
						emit gotIM(message,u.sn,isAutoResponse);

						// Check to see if there's anything else
						if(inbuf.getLength() > 0)
							moreTLVs = true;
						else
							moreTLVs = false;
						break;
					} // END 0x0002

					case 0x0004: // Away message
					{
						// There isn't actually a message in this TLV, it just specifies
						// that the message that was send was an autoresponse
						inbuf.getWord();
						// Set the autoresponse flag
						isAutoResponse = true;

						// Check to see if there's more
						if(inbuf.getLength() > 0)
							moreTLVs = true;
						else
							moreTLVs = false;
						break;
					}

					case 0x0008: // User Icon
					{
						// TODO support this
						// The length of the TLV
						length = inbuf.getWord();
						// Get the block
						/*char *msg =*/ inbuf.getBlock(length);

						// Check to see if there are more TLVs
						if(inbuf.getLength() > 0)
							moreTLVs = true;
						else
							moreTLVs = false;
					}

					default: //unknown type
					{
						kdDebug(14150) << k_funcinfo << "Unknown message type, type=" << type << endl;
						if(inbuf.getLength() > 0)
							moreTLVs = true;
						else
							moreTLVs = false;
					}
				};
			}
			break;
		};

		case 0x0002: //rendezvous channel
		{
			kdDebug(14150) << "[OSCAR] IM received on channel 2 from " << u.sn << endl;
			tlv = inbuf.getTLV();
			kdDebug(14150) << "[OSCAR] The first TLV is of type " << tlv.type;
			if (tlv.type == 0x0005) //connection info
			{
				tmpbuf.setBuf(tlv.data,tlv.length);
				//embedded in the type 5 tlv are more tlv's
				//first 2 bytes are the request status
				// 0 - Request
				// 1 - Deny
				// 2 - Accept
				msgtype = tmpbuf.getWord();
				//next comes the cookie, which should match the ICBM cookie
				char *c = tmpbuf.getBlock(8);
				cook.duplicate(c,8);
				delete [] c;
				//the next 16 bytes are the capability block (what kind of request is this?)
				char *cap = tmpbuf.getBlock(0x10);
				int identified = 0;
				for (int i = 0; !(aim_caps[i].flag & AIM_CAPS_LAST); i++)
				{
					if (memcmp(&aim_caps[i].data, cap, 0x10) == 0)
					{
						capflag |= aim_caps[i].flag;
						identified++;
						break; /* should only match once... */
					}
				}
				delete [] cap;

				if (!identified)
				{
					printf("unknown capability: {%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
						cap[0], cap[1], cap[2], cap[3],
						cap[4], cap[5],
						cap[6], cap[7],
						cap[8], cap[9],
						cap[10], cap[11], cap[12], cap[13],
						cap[14], cap[15]);
				}

				//Next comes a big TLV chain of stuff that may or may not exist
				QPtrList<TLV> tlvlist = tmpbuf.getTLVList();
				TLV *cur;
				tlvlist.setAutoDelete(true);

				for (cur = tlvlist.first();cur;cur = tlvlist.next())
				{
					//IP address from the perspective of the client
					if (cur->type == 0x0002)
					{
						kdDebug(14150) << "[OSCAR] ClientIP1: " << cur->data[0] << "."
							<< cur->data[1] << "." << cur->data[2] << "."
							<< cur->data[3]  << endl;
					}
					//Secondary IP address from the perspective of the client
					else if (cur->type == 0x0003)
					{
							kdDebug(14150) << "[OSCAR] ClientIP2: " << cur->data[0] << "."
								<< cur->data[1] << "." << cur->data[2] << "."
								<< cur->data[3] << endl;
					}
					else if (cur->type == 0x0004) //Verified IP address (from perspective of oscar server)
					{
						DWORD tmpaddr = 0;
						for (int i=0;i<4;i++)
						{
							tmpaddr = (tmpaddr*0x100) + static_cast<unsigned char>(cur->data[i]);
						}
						qh.setAddress(tmpaddr);
						kdDebug(14150) << "[OSCAR] OscarIPRaw: " <<
							cur->data[0] << "." << cur->data[1] << "." <<
							cur->data[2] << "." << cur->data[3] << endl;
						kdDebug(14150) << "[OSCAR] OscarIP: " << qh.toString() << endl;
					}
					else if (cur->type == 0x0005) //Port number
					{
						remotePort = (cur->data[0] << 8) | cur->data[1];
						kdDebug(14150) << "[OSCAR] Port# " << remotePort << endl;
					}
					//else if (cur->type == 0x000a)
					//{
					//}
					//Error code
					else if (cur->type == 0x000b)
					{
						kdDebug(14150) << "[OSCAR] ICBM ch 2 error code " <<  ((cur->data[1] << 8) | cur->data[0]) << endl;
						emit protocolError(i18n("Rendezvous with buddy failed. Please check your internet connection or try the operation again later. Error code %1.\n").arg((cur->data[1] << 8) | cur->data[0]), 0);
					}
					//Invitation message/ chat description
					else if (cur->type == 0x000c)
					{
						message = cur->data;
						kdDebug(14150) << "[OSCAR] Invited to chat " << cur->data << endl;
					}
					//Character set
					else if (cur->type == 0x000d)
					{
						kdDebug(14150) << "[OSCAR] Using character set " << cur->data << endl;
					}
					//Language
					else if (cur->type == 0x000e)
					{
						kdDebug(14150) << "[OSCAR] Using language " << cur->data << endl;
					}
					//File transfer
					else if (cur->type == 0x2711)
					{
						Buffer thebuf;
						thebuf.setBuf(cur->data,cur->length);

						thebuf.getWord(); //more than 1 file? (0x0002 for multiple files)
						thebuf.getWord(); //number of files
						fileSize = thebuf.getDWord(); //total size
						char *fname = thebuf.getBlock(cur->length - 2 - 2 - 4 - 4); //file name
						thebuf.getDWord(); //DWord of 0x00000000
						fileName = fname;
						delete [] fname;
					} // END File transfer
					else
						kdDebug(14150) << k_funcinfo << "ICBM, unknown TLV type " << cur->type << endl;

					delete [] cur->data;
				} // END for (tlvlist...)
			}
			else
			{
				kdDebug(14150) << k_funcinfo << "IM: unknown TLV type " << type << endl;
			}

			// Set the appropriate server socket
			sockToUse = serverSocket(capflag);

			if (msgtype == 0x0000) // initiate
			{
				kdDebug(14150) << k_funcinfo << "adding " << u.sn << " to pending list." << endl;
				if ( capflag & AIM_CAPS_IMIMAGE ) //if it is a direct IM rendezvous
				{
					sockToUse->addPendingConnection(u.sn, cook, 0L, qh.toString(), 4443, DirectInfo::Outgoing);
					emit gotDirectIMRequest(u.sn);
				}
				else // file send
				{
					sockToUse->addPendingConnection(u.sn, cook, 0L, qh.toString(), remotePort, DirectInfo::Outgoing);
					emit gotFileSendRequest(u.sn, message, fileName, fileSize);
				}
			}
			else if (msgtype == 0x0001) //deny
			{
				if ( capflag & AIM_CAPS_IMIMAGE )
					emit protocolError(i18n("Direct IM request denied by %1").arg(u.sn),0);
				else
					emit protocolError(i18n("Send file request denied by %1").arg(QString(u.sn)),0);
				sockToUse->removeConnection(u.sn);
			}
			break;
		}

		case 0x0004: // non-acknowledged, server messages
		{
			kdDebug(14150) << k_funcinfo << "TODO: parse advanced oscar message used by ICQ" << endl;
			// TODO: parse that big TLV(5) inside including TLV(10001) and all the stuff
			break;
		}

		default: // unknown channel
			kdDebug(14150) << "[OSCAR] Error: unknown ICBM channel " << channel << endl;
	} // END switch(channel)
}

// parses the aim standard user info block
UserInfo OscarSocket::parseUserInfo(Buffer &inbuf)
{
	UserInfo u;
	u.userclass = 0;
	u.membersince = 0;
	u.onlinesince = 0;
	u.idletime = 0;
	u.sessionlen = 0;

	//Do some sanity checking on the length of the buffer
	if(inbuf.getLength() > 0)
	{
		BYTE len = inbuf.getByte();
//		kdDebug(14150) << "[OSCAR] Finished getting user info" << endl;

		// First comes their screen name
		char *cb = inbuf.getBlock(len);
		u.sn = cb;

		// Next comes the warning level
		//for some reason aol multiplies the warning level by 10
		u.evil = inbuf.getWord() / 10;

		//the number of TLV's that follow
		WORD tlvlen = inbuf.getWord();

/*		kdDebug(14150) << k_funcinfo << "Screenname '" << u.sn << "', length " <<
			(int)len << ", evil " << u.evil
			<< ", number of TLVs following " << tlvlen << endl;*/

		delete [] cb;
		for (int i=0;i<tlvlen;i++)
		{
			TLV t = inbuf.getTLV();
			switch(t.type)
			{
				case 0x0001: //user class
				{
					u.userclass = (((BYTE)t.data[0] << 8)) | ((BYTE)t.data[1]);
					break;
				}
				case 0x0002: //member since
				{
					u.membersince = (DWORD) (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
							| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
					break;
				}
				case 0x0003: //online since
				{
					u.onlinesince = (DWORD) (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
							| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
					break;
				}
				case 0x0004: //idle time
				{
					u.idletime = (WORD) ((((BYTE)t.data[0]) << 8) | ((BYTE)t.data[1]));
					break;
				}
				case 0x0005: // unknown time
					break;
				case 0x0006:
				{
					Buffer tmpBuf;
					tmpBuf.setBuf(t.data,t.length);
					DWORD status = tmpBuf.getDWord();
//					kdDebug(14150) << k_funcinfo << "TLV(6) [STATUS] status=" << status << endl;
/*
					if (status & 0x00000100)
						kdDebug(14150) << "INVISIBLE" << endl;

					if (status & 0x00080000)
						kdDebug(14150) << "TODAY IS BIRTHDAY" << endl;

					if (status & 0x10000000)
						kdDebug(14150) << "NEED AUTH FOR DC" << endl;

					if (status & 0x20000000)
						kdDebug(14150) << "NEED TO BE ON LIST FOR DC" << endl;
*/
					u.icqextstatus = status;
					break;
				}
				case 0x000a: // IP in a DWORD
				{
//					kdDebug(14150) << "TLV(10) [IP] data="<< t.data << endl;
					break;
				}
				case 0x000c: // CLI2CLI
				{
//					kdDebug(14150) << "TLV(12) [CLI2CLI] data left unparsed (TODO)" << endl;
					break;
				}
				case 0x000d: //capability info
				{
/*					char *cap = t.data;
					QString capstring;
					capstring.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
						cap[0], cap[1], cap[2], cap[3],cap[4], cap[5],
						cap[6], cap[7], cap[8], cap[9],
						cap[10], cap[11], cap[12], cap[13],
						cap[14], cap[15]);
					kdDebug(14150) << k_funcinfo << "TLV(13) [CAPABILITIES], " << capstring << endl;*/
					break;
				}
				case 0x000f: //session length (in seconds)
				{
					u.sessionlen = (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
							| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
					break;
				}
				case 0x0010: //session length (for AOL users)
				{
					u.sessionlen = (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
							| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
					break;
				}
				case 0x001e: // unknown, empty
					break;
				default: //unknown info type
					kdDebug(14150) << k_funcinfo << "Unknown TLV(" << t.type << ")" << endl;
			};
			delete [] t.data;
		}
/*
		kdDebug(14150) << k_funcinfo << "userclass: " << u.userclass <<
			", membersince: " << u.membersince <<
			", onlinesince: " << u.onlinesince <<
			", idletime: " << u.idletime << endl;
*/
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "ZERO sized userinfo!" << endl;
		// Buffer had length of zero for some reason, so
		u.userclass = -1;
		u.membersince = 1;
		u.onlinesince = 1;
		u.idletime = -1;
		u.sessionlen = -1;
	}

	return u;
}


/** Sends message to dest */
// FIXME: This func is just plain ugly, unreadable and incomplete! [mETz]
void OscarSocket::sendIM(const QString &message, const QString &dest, bool isAuto)
{
	//check to see if we have a direct connection to the contact
	OscarConnection *dc = mDirectIMMgr->findConnection(dest);
	if (dc)
	{
		kdDebug(14150) << k_funcinfo << "Sending direct IM " << message << " to " << dest << endl;
		dc->sendIM(message,isAuto);
		return;
	}

	kdDebug(14150) << "[OSCAR] Sending " << message << " to " << dest << endl;
	static const char deffeatures[] = { 0x01, 0x01, 0x01, 0x02 };

	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0006,0x0000,0x00000000);

	for (int i=0;i<8;i++) //generate random message cookie (MID, message ID)
		outbuf.addByte( (BYTE) rand());

	outbuf.addWord(0x0001); // message format
	// TODO: support more types
	// 2 -> special messages (also known as advanced messages)
	// 4 -> url etc.

	outbuf.addByte(dest.length()); //dest sn length
	outbuf.addString(dest.latin1(),dest.length()); //dest sn

	outbuf.addWord(0x0002); //message TLV(2)

	int tlvlen = 0;
	tlvlen += 2; // 0501
	tlvlen += 2 + sizeof(deffeatures);
	tlvlen += 2; // 0101
	tlvlen += 2; // block length
	tlvlen += 4; //charset
	tlvlen += message.length();

	outbuf.addWord(tlvlen); // add TLV(2) length

	outbuf.addWord(0x0501); // add TLV(0x0501) also known as TLV(1281)
	outbuf.addWord(sizeof(deffeatures)); // add TLV length
	outbuf.addString(deffeatures, sizeof(deffeatures)); //add deffeatures

	outbuf.addWord(0x0101); //add TLV(0x0101) also known as TLV(257)
	outbuf.addWord(message.length() + 0x04); // add TLV length
	outbuf.addDWord(0x00000000); // normal char set
	outbuf.addString(message.local8Bit(),message.length()); // the actual message

	// TODO:
	// There are a lot of other options that can go here
	// IMPLEMENT THEM!
	if(isAuto)
	{
		outbuf.addWord(0x0004);
		outbuf.addWord(0x0000);
	}
	sendBuf(outbuf,0x02);
}


void OscarSocket::sendSSIActivate(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_ROSTERACK), sending SSI Activate" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0007,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

/** Parses the oncoming buddy server notification */
void OscarSocket::parseBuddyChange(Buffer &inbuf)
{
	UserInfo u = parseUserInfo(inbuf);
//	kdDebug(14150) << "[OSCAR] Got an oncoming buddy, ScreenName '" << u.sn << "'" << endl;
	emit gotBuddyChange(u);
}

/** Parses offgoing buddy message from server */
void OscarSocket::parseOffgoingBuddy(Buffer &inbuf)
{
	UserInfo u = parseUserInfo(inbuf);
//	kdDebug(14150) << "[OSCAR] A Buddy left :-(" << endl;
	emit gotOffgoingBuddy(u.sn);
}

/** Requests sn's user info */
void OscarSocket::sendUserProfileRequest(const QString &sn)
{
    Buffer outbuf;
    outbuf.addSnac(0x0002,0x0005,0x0000,0x00000000);
    /*AIM_GETINFO_GENERALINFO 0x00001
      AIM_GETINFO_AWAYMESSAGE 0x00003
      AIM_GETINFO_CAPABILITIES 0x0004 */
    outbuf.addWord(0x0005);
    outbuf.addByte(sn.length());
    outbuf.addString(sn.latin1(),sn.length());
    sendBuf(outbuf,0x02);
}

/** Parses someone's user info */
void OscarSocket::parseUserProfile(Buffer &inbuf)
{
	UserInfo u = parseUserInfo(inbuf);
	QPtrList<TLV> tl = inbuf.getTLVList();
	tl.setAutoDelete(true);

	QString profile = "<HTML><HEAD><TITLE>User Information for %n</TITLE><HEAD><BODY BGCOLOR=#CCCCCC>";
	profile += "Username: <B>" + u.sn + "</B>";

	profile += "<IMG SRC=\"";
	if (u.userclass & 0x0004) //AOL user
		profile += "aol_icon.png";
	else if (u.userclass & 0x0010) //AIM user
		profile += "free_icon.png";
	else  //other
		profile += "dt_icon.png";

	profile += "\"><br>\n";
	profile += QString("Warning Level: <B>%1 %</B><br>\n").arg(u.evil);

	QDateTime qdt;
//  kdDebug(14150) << "ONLINE SINCE TIME IS " << u.onlinesince << endl;
	qdt.setTime_t(static_cast<uint>(u.onlinesince));
	profile += "Online Since: <B>" + qdt.toString() + "</B><br>\n";
	profile += QString("Idle Minutes: <B>%1</B><br>\n<hr><br>").arg(u.idletime);
	QString away, prof;

	for (TLV *cur = tl.first();cur;cur = tl.next())
	{
		switch(cur->type)
		{
			case 0x0001: //profile text encoding
				kdDebug(14150) << "[OSCAR] text encoding is: " << cur->data << endl;
				break;
			case 0x0002: //profile text
				kdDebug(14150) << "[OSCAR] The profile is: " << cur->data << endl;
				prof += cur->data;
				break;
			case 0x0003: //away message encoding
				kdDebug(14150) << "[OSCAR] Away message encoding is: " << cur->data << endl;
				break;
			case 0x0004: //away message
				kdDebug(14150) << "[OSCAR] Away message is: " << cur->data << endl;
				away += cur->data;
				break;
			case 0x0005: //capabilities
				kdDebug(14150) << "[OSCAR] Got capabilities" << endl;
				break;
			default: //unknown
				kdDebug(14150) << "[OSCAR] Unknown user info type " << cur->type << endl;
					break;
		};
		delete [] cur->data;
	}

	if (away.length())
		profile += "<B>Away Message:</B><br>" + away + "<br><hr>";

	if (prof.length())
		profile += prof;
	else
		profile += "<I>No user information provided</I>";

	tl.clear();

	profile += "<br><hr><I>Legend:</I><br><br><IMG SRC=\"free_icon.png\">: Normal AIM User<br>" \
		"<IMG SRC=\"aol_icon.png\">: AOL User<br><IMG SRC=\"dt_icon.png\">: Trial AIM User <br>" \
		"<IMG SRC=\"admin_icon.png\">: Administrator</HTML>";

	emit gotUserProfile(u,profile);
}

/** Sets the away message, makes user away */
void OscarSocket::sendAway(bool away, const QString &message)
{
	static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
	Buffer outbuf;
	outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);

	if (away && message)
	{ // Check to see that we're sending away
		outbuf.addTLV(0x0003,defencoding.length(),defencoding.latin1());
		outbuf.addTLV(0x0004,message.length(),message.local8Bit());
		KopeteOnlineStatus status =
			OscarProtocol::protocol()->getOnlineStatus(
				OscarProtocol::AWAY);
		emit statusChanged( status );
	}
	else //if we send it a tlv with length 0, we become unaway
	{
		outbuf.addTLV(0x0004,0,"");
		KopeteOnlineStatus status =
			OscarProtocol::protocol()->getOnlineStatus(
				OscarProtocol::ONLINE);
		emit statusChanged( status );
	}
	sendBuf(outbuf,0x02);
}

/** Sends someone a warning */
void OscarSocket::sendWarning(const QString &target, bool isAnonymous)
{
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0008,0x0000,0x00000000);
	if (isAnonymous)
		outbuf.addWord(0x0001);
	else
		outbuf.addWord(0x0000);
	outbuf.addByte(target.length());
	outbuf.addString(target.latin1(),target.length());
	sendBuf(outbuf,0x02);
}

/** Changes a user's password!!!!!! */
void OscarSocket::sendChangePassword(const QString &newpw, const QString &oldpw)
{
	// FIXME: This does not work :-(
	Buffer outbuf;
	kdDebug(14150) << "[OSCAR] Changing password from " << oldpw << " to " << newpw << endl;
	outbuf.addSnac(0x0007,0x0004,0x0000,0x00000000);
	outbuf.addTLV(0x0002,newpw.length(),newpw.latin1());
	outbuf.addTLV(0x0012,oldpw.length(),oldpw.latin1());
	sendBuf(outbuf,0x02);
}

/** Joins the given chat room */
void OscarSocket::sendChatJoin(const QString &/*name*/, const int /*exchange*/)
{
	Buffer outbuf;
	outbuf.addSnac(0x0001,0x0004,0x0000,0x00000000);
	outbuf.addWord(0x000d);
	//outbuf.addChatTLV(0x0001,exchange,name,0x0000); //instance is 0x0001 here for a test
	sendBuf(outbuf,0x02);
	kdDebug(14150) << "[OSCAR] Send chat join thingie (That's a technical term)" << endl;
}

/** Handles a redirect */
void OscarSocket::parseRedirect(Buffer &inbuf)
{
	// FIXME: this function does not work, but it is never
	// called unless you want to connect to the advertisements server
	kdDebug(14150) << "[OSCAR] Parsing redirect" << endl;

	OscarConnection *servsock = new OscarConnection(getSN(),"Redirect",Redirect,QByteArray(8));

	QPtrList<TLV> tl = inbuf.getTLVList();
//	int n;
	QString host;
	tl.setAutoDelete(true);

	if (!findTLV(tl,0x0006) && !findTLV(tl,0x0005) && !findTLV(tl,0x000e))
	{
		tl.clear();
		emit protocolError(
			i18n("An unknown error occured. Please check " \
				"your internet connection. The error message " \
				"was: \"Not enough information found in server redirect\""), 0);
		return;
	}
	for (TLV *tmp = tl.first(); tmp; tmp = tl.next())
	{
		switch (tmp->type)
		{
			case 0x0006: //auth cookie
/*				for (int i=0;i<tmp->length;i++)
					servsock->mAuthCookie[i] = tmp->data[i];*/
				break;
			case 0x000d: //service type
				//servsock->mConnType = (tmp->data[1] << 8) | tmp->data[0];
				break;
			case 0x0005: //new ip & port
			{
/*				host = tmp->data;
				n = host.find(':');
				if (n != -1)
				{
					servsock->mHost = host.left(n);
					servsock->mPort = host.right(n).toInt();
				}
				else
				{
					servsock->mHost = host;
					servsock->mPort = peerPort();
				}
				kdDebug(14150) << "[OSCAR] Set host to " << servsock->mHost <<
					", port to " << servsock->mPort << endl;*/
				break;
			}

			default: //unknown
				kdDebug(14150) << "[OSCAR] Unknown tlv type in parseredirect: " << tmp->type << endl;
				break;
		}
		delete [] tmp->data;
	}
	tl.clear();
	//sockets.append(servsock);
	delete servsock;
	kdDebug(14150) << "[OSCAR] Socket added to connection list!" << endl;
}

/** Request a direct IM session with someone
	type == 0: request
	type == 1: deny
	type == 2: accept */
void OscarSocket::sendDirectIMRequest(const QString &sn)
{
	sendRendezvous(sn,0x0000,AIM_CAPS_IMIMAGE);
}

/** Parses a message ack from the server */
void OscarSocket::parseMsgAck(Buffer &inbuf)
{
	//8 byte cookie is first
	char *ck = inbuf.getBlock(8);
	WORD typ = inbuf.getWord();
	BYTE snlen = inbuf.getByte();
	char *sn = inbuf.getBlock(snlen);
	QString nm = sn;
	delete [] sn;
	delete [] ck;
	emit gotAck(nm,typ);
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

// Parses all SNAC(15,3) Packets, these are only for ICQ!
void OscarSocket::parseICQ_CLI_META(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "START" << endl;

	QPtrList<TLV> tl = inbuf.getTLVList();
	tl.setAutoDelete(true);
	TLV *tlv = findTLV(tl,0x0001);
	if (!tlv)
   {
		kdDebug(140150) << "[OSCAR] Bad SNAC(21,3), no TLV(1) found!" << endl;
		return;
	}

	kdDebug(14150) << "[OSCAR] Got SNAC(21,3) containing TLV(1) of length=" << tlv->length << endl;

	kdDebug(14150) << "[OSCAR] data=" << tlv->data << endl;

	Buffer metadata(tlv->data, tlv->length);
	WORD commandlength = metadata.getWord();
	DWORD ourUIN = metadata.getDWord();
	WORD subcmd = metadata.getWord();
	WORD sequence = metadata.getWord();

	kdDebug(14150) << "[OSCAR] commandlength=" << commandlength <<
		", ourUIN=" << ourUIN << ", subcmd=" << subcmd << ", sequence=" << sequence << endl;

	switch(subcmd)
	{
		case 0x0041: //SRV_OFFLINEMSG
		{
			kdDebug(14150) << "SNAC(21,03) SRV_OFFLINEMSG" << endl;
			break;
		}
		case 0x0042: //SRV_DONEOFFLINEMSGS
		{
			kdDebug(14150) << "SNAC(21,03) SRV_DONEOFFLINEMSG" << endl;
			break;
		}
		case 0x07da: //SRV_DONEOFFLINEMSGS
		{
			kdDebug(14150) << "SNAC(21,03) SRV_META" << endl;
			break;
		}
		default:
			kdDebug(14150) << "Unknown SNAC(21,03) subcommand is" << subcmd << endl;
	}

	delete [] tlv->data;
	kdDebug(14150) << k_funcinfo << "END" << endl;
/*
            switch (nType){
            case ICQ_SRVxEND_OFFLINE_MSG:
                log(L_DEBUG, "End offline messages");
                serverRequest(ICQ_SRVxREQ_ACK_OFFLINE_MSG);
                sendServerRequest();
                break;
            case ICQ_SRVxOFFLINE_MSG:{
                    log(L_DEBUG, "Offline message");
                    unsigned long uin;
                    char type, flag;
                    struct tm sendTM;
                    memset(&sendTM, 0, sizeof(sendTM));
                    string message;
                    unsigned short year;
                    char month, day, hours, min;
                    msg.unpack(uin);
                    msg.unpack(year);
                    msg.unpack(month);
                    msg.unpack(day);
                    msg.unpack(hours);
                    msg.unpack(min);
                    msg.unpack(type);
                    msg.unpack(flag);
                    msg.unpack(message);
#ifndef HAVE_TM_GMTOFF
                    sendTM.tm_sec  = -__timezone;
#else
                    time_t now = time (NULL);
                    sendTM = *localtime (&now);
                    sendTM.tm_sec  = -sendTM.tm_gmtoff;
#endif
                    sendTM.tm_year = year-1900;
                    sendTM.tm_mon  = month-1;
                    sendTM.tm_mday = day;
                    sendTM.tm_hour = hours;
                    sendTM.tm_min  = min;
                    sendTM.tm_isdst = -1;
                    time_t send_time = mktime(&sendTM);
                    log(L_DEBUG, "Offline message %u [%08lX] %02X %02X %s", uin, uin, type & 0xFF, flag  & 0xFF, message.c_str());
                    ICQMessage *m = parseMessage(type, uin, message, msg, 0, 0, 0, 0);
                    if (m){
                        m->Time = (unsigned long)send_time;
                        messageReceived(m);
                    }
                    break;
                }
            case ICQ_SRVxANSWER_MORE:{
                    unsigned short nSubtype;
                    char nResult;
                    msg >> nSubtype >> nResult;
                    log(L_DEBUG, "Server answer %02X %04X", nResult & 0xFF, nSubtype);
                    if ((nResult == 0x32) || (nResult == 0x14) || (nResult == 0x1E)){
                        ICQEvent *e = findVarEvent(nId);
                        if (e == NULL){
                            log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                            break;
                        }
                        e->failAnswer(this);
                        varEvents.remove(e);
                        delete e;
                        break;
                    }
                    ICQEvent *e = findVarEvent(nId);
                    if (e == NULL){
                        log(L_WARN, "Various event ID %04X not found (%X)", nId, nResult);
                        break;
                    }
                    bool nDelete = e->processAnswer(this, msg, nSubtype);
                    if (nDelete){
                        log(L_DEBUG, "Delete event");
                        varEvents.remove(e);
                        delete e;
                    }
                    break;
                }
            default:
                log(L_WARN, "Unknown SNAC(15,03) response type %04X", nType);
            }
            break;
        }
    default:
        log(L_WARN, "Unknown various family type %04X", type);
    }

*/

}

// Sends our capabilities to the server (CLI_SETUSERINFO)
void OscarSocket::sendCapabilities(unsigned long caps)
{

	Buffer outbuf;
	outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);
	int sz = 0;
	for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
	{
		if (aim_caps[i].flag & caps)
			sz += 16;
	}

	kdDebug(14150) << "SEND (CLI_SETUSERINFO) Sending capabilities... size " << sz << endl;

	outbuf.addWord(0x0005); //TLV (type 5)
	outbuf.addWord(sz);

	for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
	{
		if (aim_caps[i].flag & caps)
			outbuf.addString(aim_caps[i].data,16);
	}
	sendBuf(outbuf,0x02);
}

// Parses a rate change
void OscarSocket::parseRateChange(Buffer &inbuf)
{
    /*WORD code = */inbuf.getWord();
    /*WORD rateclass = */inbuf.getWord();
    /*DWORD windowsize = */inbuf.getDWord();
    /*DWORD clear = */inbuf.getDWord();
    /*DWORD alert = */inbuf.getDWord();
    /*DWORD limit = */inbuf.getDWord();
    /*DWORD disconnect = */inbuf.getDWord();
    /*DWORD currentavg = */inbuf.getDWord();
    /*DWORD maxavg = */inbuf.getDWord();
    //there might be stuff that can be done w/ this crap
}

// Signs the user off
void OscarSocket::doLogoff()
{
	if(isConnected)
	{
		kdDebug(14150) << k_funcinfo << "Sending sign off request" << endl;
		Buffer outbuf;
		sendBuf(outbuf,0x04);
	}
}

/** Adds a buddy to the server side buddy list */
void OscarSocket::sendAddBuddy(const QString &name, const QString &group)
{
    kdDebug(14150) << "[OSCAR] Sending add buddy" << endl;
    SSI *newitem = ssiData.addBuddy(name,group);
    if (!newitem)
	{
	    sendAddGroup(group);
	    newitem = ssiData.addBuddy(name,group);
	}
    kdDebug(14150) << "[OSCAR] Adding " << newitem->name << ", gid " << newitem->gid
	      << ", bid " << newitem->bid << ", type " << newitem->type
	      << ", datalength " << newitem->tlvlength << endl;
    sendSSIAddModDel(newitem,0x0008);
    //now we need to modify the group our buddy is in
}

/** Renames a buddy on the server side buddy list */
/*
void OscarSocket::sendRenameBuddy(const QString &budName, const QString &budGroup, const QString &newAlias)
{
	kdDebug(14150) << k_funcinfo << "Sending rename buddy..." << endl;
	SSI *renameitem = ssiData.findBuddy(budName,budGroup);
	ssiData.print();

	if (!renameitem)
	{
		kdDebug(14150) << k_funcinfo << "Item with name " << budName << " and group "
			<< budGroup << "not found" << endl;

		emit protocolError(
			i18n("%1 in group %2 was not found on the server's " \
			"buddy list and cannot be renamed.").arg(budName).arg(budGroup),0);
		return;
	}

	Buffer buf;
	buf.setBuf(ssi->tlvlist,ssi->tlvlength);
	QPtrList<TLV> lst = buf.getTLVList();

	kdDebug(14150) << k_funcinfo << "contained TLVs:" << endl;
	TLV *t;
	for(t=lst.first(); t; t=lst.next())
	{
		kdDebug(14150) << k_funcinfo << "TLV(" << t->type << ") with length " << t->length << endl;
	}
	lst.setAutoDelete(TRUE);
	TLV *nick= findTLV(lst,0x0131);
	if(nick && nick->length > 0)
	{

    //ATÜ
	kdDebug(14150) << k_funcinfo << "Renaming " << renameitem->name << ", gid " << renameitem->gid
			<< ", bid " << renameitem->bid << ", type " << renameitem->type
			<< ", datalength " << renameitem->tlvlength << endl;

	sendSSIAddModDel(renameitem,0x0009);
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "FIXME: cannot rename a buddy without an alias set on the server!" << endl;
	}
}
*/

// Adds a group to the server side buddy list
void OscarSocket::sendAddGroup(const QString &name)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	SSI *newitem = ssiData.addGroup(name);
	if(!newitem)
	{
		kdDebug(14150) << k_funcinfo << "Null SSI returned from addGroup(), group must already exist!" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Adding group, name='" << name << "' gid=" << newitem->gid << endl;
	sendSSIAddModDel(newitem,0x0008);
}

// Changes the name of a group on the server side list
void OscarSocket::sendChangeGroupName(const QString &currentName,
									  const QString &newName)
{
	kdDebug(14150) << k_funcinfo
				   << "Renaming" << currentName << " to "
				   << newName << endl;
	// Check to make sure that the name has actually changed
	if (currentName == newName)
	{  // Name hasn't changed, don't do anything
		kdDebug(14150) << k_funcinfo
					   << "Name not actually changed, doing nothing"
					   << endl;
		return;
	}

	// Get the SSI data item to send using the
	// sendSSIAddModDel method
	SSI *updatedItem = ssiData.changeGroup(currentName,  newName);
	// Make the call to sendSSIAddModDel requesting a "modify"
	// SNAC (0x0009)
	sendSSIAddModDel(updatedItem, 0x0009);
}

// Sends SSI add, modify, or delete request, to reuse code
void OscarSocket::sendSSIAddModDel(SSI *item, WORD request_type)
{
	if (item == 0L)
	{ // The SSI Item we were given is null, do nothing
		return;
	}

	Buffer outbuf;
	outbuf.addSnac(0x0013,request_type,0x0000,0x00000000);
	//name length
	outbuf.addWord(item->name.length());
	if (item->name.length())
		outbuf.addString(item->name, item->name.length());
	outbuf.addWord(item->gid);
	outbuf.addWord(item->bid);
	outbuf.addWord(item->type);
	outbuf.addWord(item->tlvlength);
	if (item->tlvlength)
		outbuf.addString(item->tlvlist,item->tlvlength);
	sendBuf(outbuf,0x02);
}

// Parses the SSI acknowledgement
void OscarSocket::parseSSIAck(Buffer &/*inbuf*/)
{
	//there isn't much here...
	//we just need to signal to send the next item in the ssi queue
	emit SSIAck();
}

// Deletes a buddy from the server side contact list
void OscarSocket::sendDelBuddy(const QString &budName, const QString &budGroup)
{
	kdDebug(14150) << k_funcinfo << "Sending del buddy" << endl;

	SSI *delitem = ssiData.findBuddy(budName,budGroup);
	ssiData.print();
	if (!delitem)
	{
		kdDebug(14150) << "[OSCAR] Item with name " << budName << " and group "
			<< budGroup << "not found" << endl;
		emit protocolError(
			i18n("%1 in group %2 was not found on the server's " \
			"buddy list and cannot be deleted.").arg(budName).arg(budGroup),0);

		return;
	}

	kdDebug(14150) << k_funcinfo << "Deleting " << delitem->name << ", gid " << delitem->gid
			<< ", bid " << delitem->bid << ", type " << delitem->type
			<< ", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem,0x000a);

	if (!ssiData.remove(delitem))
		kdDebug(14150) << k_funcinfo << "delitem was not found in the SSI list" << endl;
}

/** Parses a warning notification */
void OscarSocket::parseWarningNotify(Buffer &inbuf)
{
	int newevil = inbuf.getWord() / 10; //aol multiplies warning % by 10, don't know why
	kdDebug(14150) << "[OSCAR} Got a warning: new warning level is " << newevil << endl;
	if (inbuf.getLength() != 0)
	{
		UserInfo u = parseUserInfo(inbuf);
		emit gotWarning(newevil,u.sn);
	}
	else
		emit gotWarning(newevil,QString::null);
}

/** Parses a message error */
void OscarSocket::parseError(Buffer &inbuf)
{
	QString msg;
	WORD reason = inbuf.getWord();
	kdDebug(14150) << k_funcinfo << "Got an error: " << QTextStream::hex << reason << endl;

	if (reason < msgerrreasonlen)
		msg = i18n("Your message did not get sent: %1").arg(msgerrreason[reason]);
	else
		msg = i18n("Your message did not get sent: Unknown reason.");

	emit protocolError(msg,reason);
}

/** Request, deny, or accept a rendezvous session with someone
type == 0: request
type == 1: deny
type == 2: accept  */
void OscarSocket::sendRendezvous(const QString &sn, WORD type, DWORD rendezvousType, const KFileItem *finfo)
{
	OncomingSocket *sockToUse = serverSocket(rendezvousType);
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0006,0x0000,0x00000000);
	QByteArray ck(8);
	//generate a random message cookie
	for (int i=0;i<8;i++)
	{
		ck[i] = static_cast<BYTE>(rand());
	}

	//add this to the list of pending connections if it is a request
	if ( type == 0 )
	{
		sockToUse->addPendingConnection(sn, ck, finfo, QString::null, 0, DirectInfo::Incoming);
	}

	outbuf.addString(ck,8);
	//channel 2
	outbuf.addWord(0x0002);
	//destination sn
	outbuf.addByte(sn.length());
	outbuf.addString(sn.latin1(),sn.length());
	//add a blank TLV of type 3
	outbuf.addTLV(0x0003,0x0000,NULL);
	//add a huge TLV of type 5
	outbuf.addWord(0x0005);
	if ( !finfo ) //this is a simple direct IM
	{
		if (type == 0x0000)
			outbuf.addWord(2+8+16+6+8+6+4);
		else
			outbuf.addWord(2+8+16);
	}
	else //this is a file transfer request
	{
		if (type == 0x0000)
			outbuf.addWord(2+8+16+6+8+6+4+2+2+2+2+4+finfo->url().fileName().length()+4);
		else
			outbuf.addWord(2+8+16);
	}
	outbuf.addWord(type); //2
	outbuf.addString(ck,8); //8
	for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
	{
		if (aim_caps[i].flag & rendezvousType)
		{
			outbuf.addString(aim_caps[i].data,0x10);
			break;
		}
	} //16

	if ( type == 0x0000 ) //if this is an initiate rendezvous command
	{
		//TLV (type a)
		outbuf.addWord(0x000a);
		outbuf.addWord(0x0002);
		outbuf.addWord(0x0001); //6
		//TLV (type 3)
		outbuf.addWord(0x0003);
		outbuf.addWord(0x0004);

		if (!sockToUse->ok()) //make sure the socket stuff is properly set up
		{
			kdDebug(14150) << "[Oscar] SERVER SOCKET NOT SET UP... returning from sendRendezvous" << endl;
			emit protocolError(i18n("Error setting up listening socket.  The request will not be send."),0);
			return;
		}

		outbuf.addDWord(static_cast<DWORD>(sockToUse->address().ip4Addr())); //8
		//TLV (type 5)
		outbuf.addWord(0x0005);
		outbuf.addWord(0x0002); //8
		outbuf.addWord(sockToUse->port()); //6
		//TLV (type f)
		outbuf.addTLV(0x000f,0x0000,NULL); //4

		if ( finfo )
		{
			outbuf.addWord(0x2711); //2
			outbuf.addWord(2+2+4+finfo->url().fileName().length()+4); //2
			outbuf.addWord(0x0001); //more than 1 file? (0x0002 for multiple -- implement later)
			outbuf.addWord(0x0001); //number of files
			outbuf.addDWord(finfo->size());
			outbuf.addString(finfo->url().fileName().latin1(),finfo->url().fileName().length());
			outbuf.addDWord(0x00000000);
		}
	}

	kdDebug(14150) << "[OSCAR] Sending direct IM, type " << type << " from " << sockToUse->address().toString() << ", port " << sockToUse->port() << endl;
	sendBuf(outbuf,0x02);
}

/** Sends a direct IM denial */
void OscarSocket::sendDirectIMDeny(const QString &sn)
{
	sendRendezvous(sn,0x0001,AIM_CAPS_IMIMAGE);
}

/** Sends a direct IM accept */
void OscarSocket::sendDirectIMAccept(const QString &sn)
{
	sendRendezvous(sn,0x0002,AIM_CAPS_IMIMAGE);
	if ( !mDirectIMMgr->establishOutgoingConnection(sn) )
		kdDebug(14150) << k_funcinfo << sn << " not found in pending connection list" << endl;
}

/** Parses a missed message notification */
void OscarSocket::parseMissedMessage(Buffer &inbuf)
{
	while (inbuf.getLength() > 0)
	{
		// get the channel (this isn't used anywhere)
		/*WORD channel =*/ inbuf.getWord();

		// get user info
		UserInfo u = parseUserInfo(inbuf);

		// get number of missed messages
		WORD nummissed = inbuf.getWord();

		//the number the aol servers report seems to be one too many
		nummissed--;

		// get reason for missed messages
		WORD reason = inbuf.getWord();

		QString errstring = i18n(
			"You missed one message from %1. Reason given:\n",
			"You missed %n messages from %1. Reason given:\n",
			nummissed).arg(u.sn);
		switch (reason)
		{
			case 0: //invalid message
				errstring += i18n("It was invalid.",
					"They were invalid.", nummissed);
				break;
			case 1: //message(s) too large
				errstring += i18n("It was too large.",
					"They were too large.", nummissed);
				break;
			case 2: //rate limit exceeded
				errstring += i18n("The client exceeded the rate limit.");
				break;
			case 3: //evil sender
				errstring += i18n("The sender's warning level is too high.");
				break;
			case 4: //evil receiver
				errstring += i18n("Your warning level is too high.");
				break;
			default: //unknown reason
				errstring += i18n("Unknown reasons.");
				break;
		};
		emit protocolError(errstring,0);
	}
}


/** Sends a 0x0013,0x0002 (requests SSI rights information) */
void OscarSocket::sendSSIRightsRequest()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQLISTS)" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}

/** Sends a 0x0013,0x0004 (requests SSI data?) */
void OscarSocket::sendSSIRequest(void)
{
	kdDebug(14150) << "SEND (CLI_REQROSTER), requesting serverside contactlist for the FIRST time" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0004,0x0000,0x00020004);
	sendBuf(outbuf,0x02);
}

/** Parses a 0x0013,0x0003 (SSI rights) from the server */
void OscarSocket::parseSSIRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYLISTS)" << endl;
	//List of TLV's
		//TLV of type 4 contains a bunch of words, representing maxmimums
		// word 0 of TLV 4 data: maxbuddies
		// word 1 of TLV 4 data: maxgroups
		// word 2 of TLV 4 data: maxpermits
		// word 3 of TLV 4 data: maxdenies
//	sendSSIRequest();
	gotAllRights++;
	if (gotAllRights==7)
		sendInfo();
}

// Sends the server lots of information about the currently logged in user
void OscarSocket::sendInfo(void)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	gotAllRights=0;

	if(!m_account->isICQ())
		sendMyProfile(); // CLI_SETUSERINFO

	if(m_account->isICQ())
		sendCapabilities(KOPETE_ICQ_CAPS); // CLI_SETUSERINFO
	else
		sendCapabilities(KOPETE_AIM_CAPS); // CLI_SETUSERINFO (again?)

	sendMsgParams(); // CLI_SETICBM

	sendIdleTime(0); // CLI_SNAC1_11, sent before CLI_SETSTATUS

	if(m_account->isICQ())
		sendStatus(0x00000000); // CLI_SETSTATUS

	if (!m_account->isICQ())
	{
		sendGroupPermissionMask(); // unknown to icq docs
		sendPrivacyFlags(); // unknown to icq docs
	}
/* // Moved upwards ;)
	if(m_account->isICQ())
		sendCapabilities(KOPETE_ICQ_CAPS);
	else
		sendCapabilities(KOPETE_AIM_CAPS);
*/
	sendClientReady(); // CLI_READY
	// TODO: CLI_REQOFFLINEMSGS
}

/** Sends the user's profile to the server */
void OscarSocket::sendMyProfile(void)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
	Buffer outbuf;
	outbuf.addSnac(0x0002,0x0004,0x0000,0x00000004);
	outbuf.addTLV(0x0001,defencoding.length(),defencoding.latin1());
	outbuf.addTLV(0x0002,myUserProfile.length(),myUserProfile.local8Bit());
	sendBuf(outbuf,0x02);
}

// Sends parameters for ICBM messages (CLI_SETICBM)
void OscarSocket::sendMsgParams(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0002,0x0000,0x00000002);

	//this is read-only, and must be set to 0 here
	outbuf.addWord(0x0000);

	//these are all read-write
	//flags
	outbuf.addDWord(0x0000000b);

	//TODO: make these parameters customizable options!
	//max message length
	outbuf.addWord(0x1f40);
	//max sender warning level
	outbuf.addWord(0x03e7);
	//max reciever warning level
	outbuf.addWord(0x03e7);
	//min message interval limit
	outbuf.addDWord(0x00000000);

	sendBuf(outbuf,0x02);
}

// Sets the user's profile
void OscarSocket::setMyProfile(const QString &profile)
{
	myUserProfile = profile;
	if (isConnected)
		sendMyProfile();
}

// Blocks user with screenname 'sname'
void OscarSocket::sendBlock(const QString &sname)
{
	kdDebug(14150) << "[OSCAR] Sending block buddy" << endl;
	SSI *newitem = ssiData.addBlock(sname);
	if (!newitem)
		return;

	kdDebug(14150) << "[OSCAR] Adding DENY:" << newitem->name << ", gid " << newitem->gid
		<< ", bid " << newitem->bid << ", type " << newitem->type
		<< ", datalength " << newitem->tlvlength << endl;
	sendSSIAddModDel(newitem,0x0008);

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	emit denyAdded(sname);
}

// Removes the block on user sname
void OscarSocket::sendRemoveBlock(const QString &sname)
{
	kdDebug(14150) << "[OSCAR] Removing block on " << sname << endl;
	SSI *delitem = ssiData.findDeny(sname);

	if (!delitem)
	{
		kdDebug(14150) << "[OSCAR] Item with name " << sname << "not found" << endl;
		emit protocolError( i18n("%1 was not found on the server's deny list and cannot be deleted.").arg(sname),0);
		return;
	}

	kdDebug(14150) << "[OSCAR] Deleting " << delitem->name << ", gid " << delitem->gid
		<< ", bid " << delitem->bid << ", type " << delitem->type
		<< ", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem,0x000a);

	if (!ssiData.remove(delitem))
		kdDebug(14150) << "[OSCAR][sendRemoveBlock] delitem was not found in the SSI list" << endl;

	ssiData.print();

	// FIXME: Use SNAC headers and SSI acks to do this more correctly
	emit denyRemoved(sname);
}

// Reads a FLAP header from the input
FLAP OscarSocket::getFLAP(void)
{
	FLAP fl;
	int theword, theword2;
	int start;
	int chan;
	fl.error = false;

	//the FLAP start byte
	if ((start = getch()) == 0x2a)
	{
		//get the channel ID
		if ( (chan = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading channel ID: nothing to be read" << endl;
			fl.error = true;
		}
		else
		{
			fl.channel = chan;
		}

		//get the sequence number
		if((theword = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading sequence number: nothing to be read" << endl;;
			fl.error = true;
		}
		else if((theword2 = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading data field length: nothing to be read" << endl;
			fl.error = true;
		}
		else
		{
			// Got both pieces of info we need...
			fl.sequence_number = (theword << 8) | theword2;
		}

		//get the data field length
		if ((theword = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading sequence number: nothing to be read" << endl;
			fl.error = true;
		}
		else if((theword2 = getch()) == -1)
		{
			kdDebug(14150) << "[OSCAR] Error reading data field length: nothing to be read" << endl;
			fl.error = true;
		}
		else
		{
			fl.length = (theword << 8) | theword2;
		}
	}
	else
	{
		kdDebug(14150) << "[OSCAR] Error reading FLAP... start byte is " << start << endl;
		fl.error = true;
		putch(start);
	}

	return fl;
}

void OscarSocket::sendMiniTypingNotify(QString screenName, TypingNotify notifyType)
{
	kdDebug(14150) << "[OSCAR] Sending Typing notify " << endl;

	//look for direct connection before sending through server
	OscarConnection *dc = mDirectIMMgr->findConnection(screenName);
	if(dc)
	{
		kdDebug(14150) << "[OSCAR] Found direct connection, sending typing notify directly" << endl;
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
}

// Called when a direct IM is received
void OscarSocket::OnDirectIMReceived(QString message, QString sender, bool isAuto)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	//for now, let's just emit a gotIM as though it came from the server
	emit gotIM(message,sender,isAuto);
}

// Called when a direct IM connection suffers an error
void OscarSocket::OnDirectIMError(QString errmsg, int num)
{
	//let's just emit a protocolError for now
	emit protocolError(errmsg, num);
}

// Called whenever a direct IM connection gets a typing notification
void OscarSocket::OnDirectMiniTypeNotification(QString screenName, int notify)
{
	//for now, just emit a regular typing notification
	emit gotMiniTypeNotification(screenName, notify);
}

// Called when a direct IM connection bites the dust
void OscarSocket::OnDirectIMConnectionClosed(QString name)
{
	emit directIMConnectionClosed(name);
}

// Called when a direct connection is set up and ready for use
void OscarSocket::OnDirectIMReady(QString name)
{
	emit connectionReady(name);
}

// Initiate a transfer of the given file to the given sn
void OscarSocket::sendFileSendRequest(const QString &sn, const KFileItem &finfo)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	sendRendezvous(sn, 0x0000, AIM_CAPS_SENDFILE, &finfo);
}

// Accepts a file transfer from sn
OscarConnection * OscarSocket::sendFileSendAccept(const QString &sn, const QString &fileName)
{
	sendRendezvous(sn, 0x0001, AIM_CAPS_SENDFILE);
	mFileTransferMgr->addFileInfo(sn,
		new KFileItem(KFileItem::Unknown, KFileItem::Unknown, fileName));

	return mFileTransferMgr->establishOutgoingConnection(sn);
}

/** Returns the appropriate server socket, based on the capability flag it is passed. */
OncomingSocket *OscarSocket::serverSocket(DWORD capflag)
{
	if ( capflag & AIM_CAPS_IMIMAGE ) //direct im
		return mDirectIMMgr;
	else  //must be a file transfer?
		return mFileTransferMgr;
}

/** Sends a file transfer deny to @sn */
void OscarSocket::sendFileSendDeny(const QString &sn)
{
	sendRendezvous(sn, 0x0002, AIM_CAPS_SENDFILE);
}

/** Called when a file transfer begins */
void OscarSocket::OnFileTransferBegun(OscarConnection *con, const QString& file,
	const unsigned long size, const QString &recipient)
{
	kdDebug(14150) << k_funcinfo << "emitting transferBegun()" << endl;
	emit transferBegun(con, file, size, recipient);
}

void OscarSocket::sendStatus(unsigned long status)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_SETSTATUS)" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0001,0x001e,0x0000,0x00000000);

	// TODO: or 'status' with values for
	// - web-aware
	// - hideip
	// - birthday
	// - direct-conn only with auth
	// - direct-conn only if on contactlist
	outbuf.addWord(0x0006);
	outbuf.addWord(0x0004);
	outbuf.addDWord(status);

	outbuf.print();

	sendBuf(outbuf, 0x2);

	KopeteOnlineStatus kStat;

	if (status & ICQ_STATUS_OFFLINE)
	{
		kdDebug(14150) << k_funcinfo << "setting to OFFLINE" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQOFFLINE);
	}
	else if (status & ICQ_STATUS_FREEFORCHAT)
	{
		kdDebug(14150) << k_funcinfo << "setting to FFC" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQONLINE);
	}
	else if (status & ICQ_STATUS_DND)
	{
		kdDebug(14150) << k_funcinfo << "setting to DND" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQDND);
	}
	else if (status & ICQ_STATUS_OCCUPIED)
	{
		kdDebug(14150) << k_funcinfo << "setting to OCC" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQOCC);
	}
	else if (status & ICQ_STATUS_NA)
	{
		kdDebug(14150) << k_funcinfo << "setting to NA" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQNA);
	}
	else if (status & ICQ_STATUS_AWAY)
	{
		kdDebug(14150) << k_funcinfo << "setting to AWAY" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQAWAY);
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "setting to ONLINE" << endl;
		kStat = OscarProtocol::protocol()->getOnlineStatus(OscarProtocol::ICQONLINE);
	}

	emit statusChanged( kStat );
}
#include "oscarsocket.moc"
