/*
  oscarsocket.icq.cpp  -  ICQ specific part of Oscarsocket

  Copyright (c) 2003 by Stefan Gehn <sgehn@gmx.net>
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

#include "oscarsocket.h"
#include "oscarsocket.icq.h"

#include "oscaraccount.h"
#include <qdatetime.h>
#include <qtimer.h>

#include <kdebug.h>

#define ICQ_CLIENTSTRING			"ICQ Inc. - Product of ICQ (TM).2002a.5.37.1.3728.85"
#define ICQ_CLIENTID					0x010A
#define ICQ_MAJOR						0x0005
#define ICQ_MINOR						0x0025
#define ICQ_POINT						0x0001
#define ICQ_BUILD						0x0e90
static const char ICQ_OTHER[] = { 0x00, 0x00, 0x00, 0x55 };
#define ICQ_COUNTRY					"us"
#define ICQ_LANG						"en"

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

	kdDebug(14150) << k_funcinfo <<  endl;
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

void OscarSocket::sendLoginICQ(void)
{
	kdDebug(14150) << k_funcinfo <<  "Sending ICQ login info... (CLI_COOKIE)" << endl;;

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
//	emit connectionChanged(3,"Sending username and password...");
}

// Parses all SNAC(15,3) Packets, these are only for ICQ!
void OscarSocket::parseICQ_CLI_META(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo <<  "START" << endl;

	QPtrList<TLV> tl = inbuf.getTLVList();
	tl.setAutoDelete(true);
	TLV *tlv = findTLV(tl,0x0001);
	if (!tlv)
   {
		kdDebug(140150) << k_funcinfo <<  "Bad SNAC(21,3), no TLV(1) found!" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo <<  "Got SNAC(21,3) containing TLV(1) of length=" << tlv->length << endl;

	kdDebug(14150) << k_funcinfo <<  "data=" << tlv->data << endl;

	Buffer metadata(tlv->data, tlv->length);
	WORD commandlength = metadata.getWord();
	DWORD ourUIN = metadata.getDWord();
	WORD subcmd = metadata.getWord();
	WORD sequence = metadata.getWord();

	kdDebug(14150) << k_funcinfo <<  "commandlength=" << commandlength <<
		", ourUIN=" << ourUIN << ", subcmd=" << subcmd << ", sequence=" << sequence << endl;

	switch(subcmd)
	{
		case 0x0041: //SRV_OFFLINEMSG
		{
			kdDebug(14150) << k_funcinfo <<  "SNAC(21,03) SRV_OFFLINEMSG" << endl;

			DWORD UIN = metadata.getDWord();
			WORD year = metadata.getWord();
			BYTE month = metadata.getByte();
			BYTE day = metadata.getByte();
			BYTE hour = metadata.getByte();
			BYTE minute = metadata.getByte();
			WORD type = metadata.getWord();
			WORD msglen = inbuf.getWord();
			char *msg = inbuf.getBlock(msglen); // Get the message
			QString message = msg;
			delete [] msg;

			kdDebug(14150) << k_funcinfo <<   "Offline message from '" << UIN <<
				"' type=" << (type & 0xFF) << ", message='" << message << "'" << endl;

			emit gotIM(message, QString::number(UIN), false);
			break;
		}
		case 0x0042: //SRV_DONEOFFLINEMSGS
		{
			kdDebug(14150) << k_funcinfo <<  "SNAC(21,03) SRV_DONEOFFLINEMSG" << endl;
			sendAckOfflineMessages();
			break;
		}
		case 0x07da: //SRV_META
		{
			kdDebug(14150) << "SNAC(21,03) SRV_META" << endl;
			break;
		}
		default:
			kdDebug(14150) << "Unknown SNAC(21,03) subcommand is" << subcmd << endl;
	}

	delete [] tlv->data;
	kdDebug(14150) << k_funcinfo <<  "END" << endl;

// BEGIN Code from libicq, just as a mark for me to see what I have to add here [mETz]
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
} // END OscarSocket::parseICQ_CLI_META()

void OscarSocket::sendStatus(unsigned long status)
{
	kdDebug(14150) << k_funcinfo <<  "SEND (CLI_SETSTATUS)" << endl;

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

//	outbuf.print();

	sendBuf(outbuf, 0x2);

	// convert the weird hex crap from icq to our internal OSCAR_ ints
	// as they are better to handle and not protocol specific

	if (status & ICQ_STATUS_FFC)
	{
		kdDebug(14150) << k_funcinfo <<  "setting to FFC" << endl;
		emit statusChanged(OSCAR_FFC);
	}
	else if (status & ICQ_STATUS_DND)
	{
		kdDebug(14150) << k_funcinfo <<  "setting to DND" << endl;
		emit statusChanged(OSCAR_DND);
	}
	else if (status & ICQ_STATUS_OCC)
	{
		kdDebug(14150) << k_funcinfo <<  "setting to OCC" << endl;
		emit statusChanged(OSCAR_OCC);
	}
	else if (status & ICQ_STATUS_NA)
	{
		kdDebug(14150) << k_funcinfo <<  "setting to NA" << endl;
		emit statusChanged(OSCAR_NA);
	}
	else if (status & ICQ_STATUS_AWAY)
	{
		kdDebug(14150) << k_funcinfo <<  "setting to AWAY" << endl;
		emit statusChanged(OSCAR_AWAY);
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<  "setting to ONLINE" << endl;
		emit statusChanged(OSCAR_ONLINE);
	}
} // END OscarSocket::sendStatus

void OscarSocket::sendReqOfflineMessages()
{
	kdDebug(14150) << k_funcinfo <<  "SEND (CLI_REQOFFLINEMSGS), requesting offline messages" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0015,0x0002,0x0000,0x00010002);

	outbuf.addWord(0x0001); // TLV(1)
	outbuf.addWord(0x000A); // length of TLV, 10
	outbuf.addWord(0x0800); // length of data inside TLV, 8
	unsigned long tmpuin = getSN().toULong(); // own uin
	outbuf.addWord((tmpuin & 0xff00) >> 8);
	outbuf.addWord((tmpuin & 0x00ff));
	outbuf.addWord(0x3c00); // subcommand, request offline messages (60)
	outbuf.addWord(0x0200); // TODO: make this the snac sequence's upper Word minus 1!
   outbuf.print();
	sendBuf(outbuf, 0x2);
}

void OscarSocket::sendAckOfflineMessages()
{
	kdDebug(14150) << k_funcinfo <<  "SEND (CLI_ACKOFFLINEMSGS), requesting offline messages" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0015,0x0002,0x0000,0x00000000);

	int tlvlength = 2 + 4 + 2 + 2;

	outbuf.addWord(0x0001); // TLV(1)
	outbuf.addWord(tlvlength); // length of TLV
	outbuf.addWord(tlvlength-2); // length of data inside TLV
	outbuf.addDWord(getSN().toULong()); // own uin
	outbuf.addWord(0x3e00); // subcommand, acknowledge offline messages (62)
	outbuf.addWord(0x0200); // TODO: make this the snac sequence's upper Word minus 1!
	outbuf.print();
	sendBuf(outbuf, 0x2);
}

void OscarSocket::sendKeepalive()
{
	kdDebug(14150) << k_funcinfo <<  "SEND KEEPALIVE" << endl;
	Buffer outbuf;
//	outbuf.print();
	sendBuf(outbuf, 0x05);
}

void OscarSocket::startKeepalive()
{
//	kdDebug(14150) << k_funcinfo <<  "Called." << endl;
	if (keepaliveTime==0) // nobody wants keepalive, so shut up ;)
		return;

	if (!keepaliveTimer)
	{
		kdDebug(14150) << k_funcinfo <<  "Creating keepaliveTimer" << endl;
		keepaliveTimer=new QTimer(this, "keepaliveTimer");
		QObject::connect(keepaliveTimer,SIGNAL(timeout()),this,SLOT(slotKeepaliveTimer()));
		keepaliveTimer->start(keepaliveTime*1000);
	}
}

void OscarSocket::stopKeepalive()
{
//	kdDebug(14150) << k_funcinfo <<  "Called." << endl;
	if(keepaliveTimer)
	{
		kdDebug(14150) << k_funcinfo <<  "Deleting keepaliveTimer" << endl;
		delete keepaliveTimer;
		keepaliveTimer=0L;
	}
}

void OscarSocket::slotKeepaliveTimer()
{
//	kdDebug(14150) << k_funcinfo <<  "Called." << endl;
	sendKeepalive();
}

void OscarSocket::parseAdvanceMessage(Buffer &buf, UserInfo &user)
{
	kdDebug(14150) << k_funcinfo <<  "called" << endl;

	TLV tlv;
	bool moreTLVs = true; // Flag to indicate if there are more TLV's to parse

	while(moreTLVs)
	{
		tlv = buf.getTLV();

		kdDebug(14150) << k_funcinfo <<  "Found TLV(" << tlv.type << "), length=" << tlv.length << endl;

		switch(tlv.type)
		{
			case 0x0005:
			{
				Buffer type2(tlv.data, tlv.length);
				WORD ackType = type2.getWord();
				kdDebug(14150) << k_funcinfo <<  "acktype=" << ackType << endl;

				if (ackType==0x0000) // normal message
				{
					QPtrList<TLV> lst = buf.getTLVList();
					lst.setAutoDelete(TRUE);

					TLV *messageTLV = findTLV(lst,0x2711); //TLV(10001)
					if(messageTLV)
					{
						Buffer messageBuf(messageTLV->data, messageTLV->length);
						WORD len;
						WORD tcpver;
						char *cap=messageBuf.getBlock(16);

						QString capstring;
						capstring.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
							cap[0], cap[1], cap[2], cap[3],cap[4], cap[5],
							cap[6], cap[7], cap[8], cap[9],
							cap[10], cap[11], cap[12], cap[13],
							cap[14], cap[15]);
						kdDebug(14150) << k_funcinfo <<  "CAPABILITY:" << capstring << endl;

						delete [] cap;
						messageBuf.getByte(); // unknown
						messageBuf.getByte(); // unknown
						messageBuf.getByte(); // unknown
						messageBuf.getDWord(); // unknown, 0=normal message, 1=file ok...
						WORD seq1 = messageBuf.getWord(); // some stupid sequence
						messageBuf.getWord(); // unknown
						WORD seq2 = messageBuf.getWord(); // some stupid sequence
						if (seq1 != seq2)
							kdDebug(14150) << k_funcinfo <<  "TODO: seq1 != seq2, what shall we do now?" << endl;

						char *tmp = messageBuf.getBlock(12); // unknown
						delete[] tmp;
						WORD msgType = messageBuf.getWord(); // message type

						switch(msgType)
						{
							case 0x0001: // plain normal message
							{
								messageBuf.getWord(); // unknown
								messageBuf.getWord(); // unknown, might be priority
								WORD messageLength = messageBuf.getWord();

								char *messagetext = messageBuf.getBlock(messageLength);
								QString message = QString::fromLocal8Bit(messagetext);
								kdDebug(14150) << k_funcinfo <<  "type-2 messagtext=" << messagetext << endl;
								delete [] messagetext;

								DWORD fgColor=messageBuf.getDWord();
								DWORD bgColor=messageBuf.getDWord();
								kdDebug(14150) << k_funcinfo <<  "messageBuf.getLength() after message and colors =" << messageBuf.getLength() << endl;

								DWORD guidlen = messageBuf.getDWord();
								char *guid = messageBuf.getBlock(guidlen);
								kdDebug(14150) << k_funcinfo <<  "type-2 guid=" << guid << endl;
								delete [] guid;

								kdDebug(14150) << k_funcinfo <<  "emit gotIM(), contact='" <<
									user.sn << "', message='" << message << "'" << endl;

								emit gotIM(message, user.sn, false);
								break;
							}

							default:
							{
								kdDebug(14150) << k_funcinfo <<  "Unhandled message-type:" << msgType << endl;
							}
						}
					}
					else
					{
						kdDebug(14150) << k_funcinfo <<  "Could not find TLV(10001) in advanced message!" << endl;
					}

					lst.clear();
				}
				else
				{
					kdDebug(14150) << k_funcinfo <<  "UNHANDLED acktype" << endl;
					delete [] tlv.data;
				}
				break;
			} // END TLV(0x0005)

			default:
			{
				kdDebug(14150) << k_funcinfo <<  "Unhandled TLV(" << tlv.type << ") length=" << tlv.length << endl;
				delete [] tlv.data;
				break;
			}
		}

		if(buf.getLength() > 0)
			moreTLVs=true;
		else
			moreTLVs=false;
	} // END while(moreTLVs)

	kdDebug(14150) << k_funcinfo <<  "END" << endl;
}

// TODO: get on with the coding, need to implement that sequence counting and
// some sort of packet queue so I know what requests I sent out

/*
bool requestAutoReply(unsigned long uin, unsigned long status)
{
	if (status == 0)
		return false;

	responseRequestSeq = --advCounter;
	unsigned char type = 0xE8;
	if (status & ICQ_STATUS_DND)
		type = 0xEB;
	else if (status & ICQ_STATUS_OCCUPIED)
		type = 0xE9;
	else if (status & ICQ_STATUS_NA)
		type = 0xEA;
	else if (status & ICQ_STATUS_FREEFORCHAT)
		type = 0xEC;


	kdDebug(14150) << k_funcinfo <<  "SEND (CLI_SENDMSG), requesting away message" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0006,0x0000,0x00000000);
	outbuf.addDWord(0x00000000); // TIME
	outbuf.addDWord(0x00000000); // ID

	outbuf.addWord(MSGFORMAT_ADVANCED); // type-2 message
	outbuf.addDWord(getSN().toULong()); // own uin


	Buffer tlv10001;
	tlv10001.addWord(0x1B00); // length
	tlv10001.addWord(ICQ_TCP_VERSION);
	tlv10001.addDWord(0x00000000); // CAP
	tlv10001.addDWord(0x00000000); // CAP
	tlv10001.addDWord(0x00000000); // CAP
	tlv10001.addDWord(0x00000000); // CAP
	tlv10001.addDWord(0x00000003); // unknown
	tlv10001.addDWord(0x00000000); // unknown -> 0 = normal message
	<< advCounter << 0xE000 << advCounter
	<< 0x00000000L << 0x00000000L << 0x00000000L
	<< type << (char)3;
	tlv10001.pack((unsigned short)(client->owner->uStatus & 0xFFFF));
	tlv10001 << 0x0100 << 0x0100 << (char)0;

	// TLV(2) =============
	Buffer tlv2;
	tlv2.addWord(0x0000) // acktype, 0 = normal message
	tlv2.addDWord(0x00000000); // time
	tlv2.addDWord(0x00000000); // random message id
	tlv2.addDWord(0x09461349); // CAP SERVERRELAY
	tlv2.addDWord(0x4C7F11D1); // CAP SERVERRELAY
	tlv2.addDWord(0x82224445); // CAP SERVERRELAY
	tlv2.addDWord(0x53540000); // CAP SERVERRELAY

	tlv2.addWord(0x000A); // TLV(10)
	tlv2.addWord(0x0002); // len
	tlv2.addWord(0x0001); // data, acktype2, 1 = normal message

	tlv2.addWord(0x000F); // TLV(15), always empty
	tlv2.addWord(0x0000); // len

	tlv2.addTLV(0x2711, tlv10001.data, tlv10001.length);
	// ========================

	outbuf.addTLV(0x0002, tlv2.data, tlv2.length )
//	outbuf.print();
	sendBuf(outbuf, 0x2);
}
*/
// vim: set noet ts=4 sts=4 sw=4:

