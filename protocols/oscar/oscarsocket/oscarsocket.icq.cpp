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

/**
 * taken from libfaim !!!
 * encodePasswordXOR - Encode a password using old XOR method
 * @password: incoming password
 * @encoded: buffer to put encoded password
 *
 * This takes a const pointer to a (null terminated) string
 * containing the unencoded password. It also gets passed
 * an already allocated buffer to store the encoded password.
 * This buffer should be the exact length of the password without
 * the null. The encoded password buffer /is not %NULL terminated/.
 */
QCString OscarSocket::encodePasswordXOR()
{
	const char *password = pass.latin1();
	QCString encoded;

	kdDebug(14150) << k_funcinfo << endl;
//	kdDebug(14150) << " unencoded pw='" << password << "'" << endl;
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

//	kdDebug(14150) << " encoded pw='" << encoded << "', length=" << encoded.length() << endl;
	return encoded;
}

void OscarSocket::sendLoginICQ()
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
//	emit connectionChanged(3,"Sending username and password...");
}

// Parses all SNAC(15,3) Packets, these are only for ICQ!
void OscarSocket::parseSRV_FROMICQSRV(Buffer &inbuf)
{
	QPtrList<TLV> tl = inbuf.getTLVList();
	tl.setAutoDelete(true);
	TLV *tlv = findTLV(tl,0x0001);
	if (!tlv)
	{
		kdDebug(140150) << k_funcinfo <<  "Bad SNAC(21,3), no TLV(1) found!" << endl;
		return;
	}

	Buffer fromicqsrv(tlv->data, tlv->length);

//	kdDebug(14150) << "==========================================================" << endl;
//	fromicqsrv.print();
//	kdDebug(14150) << "==========================================================" << endl;

	WORD commandlength = fromicqsrv.getLEWord();
	DWORD ourUIN = fromicqsrv.getLEDWord();
	WORD subcmd = fromicqsrv.getLEWord();
	WORD sequence = fromicqsrv.getLEWord();

	kdDebug(14150) << k_funcinfo << "commandlength=" << commandlength <<
		", ourUIN='" << ourUIN << "', subcmd=" << subcmd << ", sequence=" << sequence << endl;

	switch(subcmd)
	{
		case 0x0041: //SRV_OFFLINEMSG
		{
//			kdDebug(14150) << k_funcinfo << "RECV (SRV_OFFLINEMSG), got an offline message" << endl;
			DWORD UIN = fromicqsrv.getLEDWord();
			/*WORD year =*/ fromicqsrv.getLEWord();
			/*BYTE month =*/ fromicqsrv.getLEByte();
			/*BYTE day =*/ fromicqsrv.getLEByte();
			/*BYTE hour =*/ fromicqsrv.getLEByte();
			/*BYTE minute =*/ fromicqsrv.getLEByte();
			WORD type = fromicqsrv.getLEWord();
			WORD msglen = fromicqsrv.getLEWord();
			char *msg = fromicqsrv.getLEBlock(msglen); // Get the message
			QString message = msg;
			delete [] msg;

			kdDebug(14150) << k_funcinfo << "Offline message from '" << UIN <<
				"' type=" << (type & 0xFF) << ", message='" << message << "'" << endl;

			emit gotIM(message, QString::number(UIN), false);
			break;
		}

		case 0x0042: //SRV_DONEOFFLINEMSGS
		{
//			kdDebug(14150) << k_funcinfo << "RECV (SRV_DONEOFFLINEMSG), last offline message" << endl;
			sendAckOfflineMessages();
			break;
		}

		case 0x07da: //SRV_META
		{
			WORD type = fromicqsrv.getLEWord();
			BYTE result = fromicqsrv.getLEByte();

			switch(type)
			{
				case 0x01a4: // SRV_METAFOUND
				case 0x01ae: // SRV_METALAST
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAFOUND or SRV_METALAST)" << endl;

					char *tmptxt;
					ICQSearchResult searchResult;
					// codes taken from libicq, some kind of failure,
					// have to find out what they are for
					if ((result == 0x32) || (result == 0x14) || (result == 0x1E))
					{
						searchResult.uin=1;
						emit gotSearchResult(searchResult,0);
						break;
					}

					// DATALEN; The length of the following data.
					/*WORD datalen = */fromicqsrv.getLEWord();
					searchResult.uin = fromicqsrv.getLEDWord();

					tmptxt=fromicqsrv.getLELNTS();
					searchResult.nickName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt=fromicqsrv.getLELNTS();
					searchResult.firstName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt=fromicqsrv.getLELNTS();
					searchResult.lastName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt=fromicqsrv.getLELNTS();
					searchResult.eMail = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					// FLAGS; 1 = anyone can add me to list, 0 = auth required.
					BYTE flags = fromicqsrv.getLEByte();
					searchResult.needAuth = (flags==0x00);

					// STATUS; 0 = Offline, 1 = Online, 2 = not webaware.
					searchResult.status = fromicqsrv.getLEWord();

					// FIXME: said to be the last searchresult
					// unfortunately I still get some results afterwards
					if (type==0x01ae)
					{
						// MISSED
						// The number of users not returned that matched this search.
						DWORD missed = fromicqsrv.getLEDWord();
//						kdDebug(14150) << "!LAST search result, missed=" << missed << endl;
						emit gotSearchResult(searchResult,missed);
					}
					else
					{
						emit gotSearchResult(searchResult,-1);
					}
					break;
				} // END SRV_METAFOUND/SRV_METALAST

				case 0x019a: // SRV_METAINFO410 seems to be unused
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAINFO410) !!!" << endl;
					/*
					char *tmptxt;
					ICQInfoResult res;

					fromicqsrv.getLEWord(); // datalen
					res.uin = fromicqsrv.getLEDWord();

					tmptxt=fromicqsrv.getLELNTS();
					res.nickName = QString::fromLatin1(tmptxt);
					delete [] tmptxt;

					tmptxt=fromicqsrv.getLELNTS();
					res.firstName = QString::fromLatin1(tmptxt);
					delete [] tmptxt;

					tmptxt=fromicqsrv.getLELNTS();
					res.lastName = QString::fromLatin1(tmptxt);
					delete [] tmptxt;

					kdDebug(14150) << k_funcinfo << "emitting gotICQUserInfo()" << endl;
					emit gotICQUserInfo(sequence, res);
					*/
					break;
				} // END SRV_METAINFO410


				case 200: // SRV_METAGENERAL
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAGENERAL)" << endl;
					char *tmptxt;
					ICQGeneralUserInfo res;

					tmptxt = fromicqsrv.getLELNTS();
					res.nickName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.firstName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.lastName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.eMail = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.city = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.state = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.phoneNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.faxNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.street = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.cellularNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.zip = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					res.countryCode = fromicqsrv.getLEWord();
					res.timezoneCode = fromicqsrv.getLEByte(); // UTC+(tzcode * 30min)
// 					kdDebug(14150) << k_funcinfo << "timezoneCode=" <<
// 						res.timezoneCode << endl;
					res.publishEmail = (fromicqsrv.getLEByte()==0x01);
					res.showOnWeb = (fromicqsrv.getLEWord()==0x0001);

					emit gotICQGeneralUserInfo(sequence, res);
					break;
				} // END SRV_METAGENERAL (200)

				case 210: // SRV_METAWORK
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAWORK)" << endl;
					char *tmptxt;
					ICQWorkUserInfo res;

					tmptxt = fromicqsrv.getLELNTS();
					res.city = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.state = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.phone = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.fax = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.address = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.zip = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					res.countryCode = fromicqsrv.getLEWord();

					tmptxt = fromicqsrv.getLELNTS();
					res.company = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.department = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.position = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					res.occupation = fromicqsrv.getLEWord();

					tmptxt = fromicqsrv.getLELNTS();
					res.homepage = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					emit gotICQWorkUserInfo(sequence, res);
					break;
				} // END SRV_METAWORK (210)

				case 220: // SRV_METAMORE
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAMORE)" << endl;
					char *tmptxt;
					ICQMoreUserInfo res;

					res.age = fromicqsrv.getLEWord();
					res.gender = fromicqsrv.getLEByte();

					tmptxt = fromicqsrv.getLELNTS();
					res.homepage = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					WORD y = fromicqsrv.getLEWord();
					BYTE m = fromicqsrv.getLEByte();
					BYTE d = fromicqsrv.getLEByte();
					if (y==0 && m==0 && d==0) // stops QDate from spewing out errors
						res.birthday = QDate();
					else
						res.birthday = QDate(y,m,d);

					res.lang1 = fromicqsrv.getLEByte();
					res.lang2 = fromicqsrv.getLEByte();
					res.lang3 = fromicqsrv.getLEByte();

					WORD unknown = fromicqsrv.getLEWord();
					kdDebug(14150) << k_funcinfo <<
						"unknown last word=" << unknown << endl;

					emit gotICQMoreUserInfo(sequence, res);
					break;
				} // END SRV_METAMORE

				case 230: // SRV_METAABOUT
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAABOUT)" << endl;
					char *tmptxt;
					QString res;

					tmptxt = fromicqsrv.getLELNTS();
					res = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					emit gotICQAboutUserInfo(sequence, res);
					break;
				} // END SRV_METAABOUT

				case 235:
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: SRV_METAMOREEMAIL subtype=" << type << endl;
					break;
				}

				case 240:
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: SRV_METAINTEREST subtype=" << type << endl;
					break;
				}

				case 250:
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: SRV_METABACKGROUND subtype=" << type << endl;
					break;
				}

				case 270:
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: SRV_META270 subtype=" << type << endl;
					break;
				}

				default:
				{
		 			kdDebug(14150) << k_funcinfo << "RECV (SRV_META), UNHANDLED, subtype=" <<
						type << ", result=" << (int)result << endl;
					break;
				}
			} // END switch(type)
			break;
		} // END SRV_META

		default:
		{
			kdDebug(14150) << k_funcinfo << "Unknown SNAC(21,03) subcommand is" << subcmd << endl;
			break;
		}
	} // END switch(subcmd)

//	kdDebug(14150) << k_funcinfo << "deleting tlv data" << endl;
	delete [] tlv->data;

//	kdDebug(14150) << k_funcinfo << "END" << endl;
} // END OscarSocket::parseSRV_FROMICQSRV()

void OscarSocket::sendICQStatus(unsigned long status)
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
	outbuf.addWord(0x0008); // TLV(8)
	outbuf.addWord(0x0002); // length 2
	outbuf.addWord(0x0000); // error code - 0
//	outbuf.print();

	sendBuf(outbuf, 0x2);

	// convert the weird hex crap from icq to our internal OSCAR_ ints
	// as they are better to handle and not protocol specific

	if (status & ICQ_STATUS_FFC)
	{
		kdDebug(14150) << k_funcinfo << "setting to FFC" << endl;
		emit statusChanged(OSCAR_FFC);
	}
	else if (status & ICQ_STATUS_DND)
	{
		kdDebug(14150) << k_funcinfo << "setting to DND" << endl;
		emit statusChanged(OSCAR_DND);
	}
	else if (status & ICQ_STATUS_OCC)
	{
		kdDebug(14150) << k_funcinfo << "setting to OCC" << endl;
		emit statusChanged(OSCAR_OCC);
	}
	else if (status & ICQ_STATUS_NA)
	{
		kdDebug(14150) << k_funcinfo << "setting to NA" << endl;
		emit statusChanged(OSCAR_NA);
	}
	else if (status & ICQ_STATUS_AWAY)
	{
		kdDebug(14150) << k_funcinfo << "setting to AWAY" << endl;
		emit statusChanged(OSCAR_AWAY);
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "setting to ONLINE" << endl;
		emit statusChanged(OSCAR_ONLINE);
	}
} // END OscarSocket::sendStatus
/*
void OscarSocket::fillDirectInfo(Buffer &directInfo)
{
	kdDebug(14150) << k_funcinfo << "IP=" << mDirectIMMgr->address().toString() <<
		", Port=" << mDirectIMMgr->port() << endl;

	directInfo.addDWord(htonl(mDirectIMMgr->address().ip4Addr())); // IP
	directInfo.addWord(0x0000);
	directInfo.addWord(mDirectIMMgr->port()); // Port

	directInfo.addByte(0x01) // Mode
	directInfo.addWord(ICQ_TCP_VERSION); // icq tcp protocol version, v8 currently

	directInfo.addDWord(0) ; // TODO: DC cookie!
	directInfo.addWord(0x0000);
	directInfo.addWord(0x0050);
	directInfo.addWord(0x0000);
	directInfo.addWord(0x0003);
	directInfo.addDWord(0x00000000); //InfoUpdateTime
	directInfo.addDWord(0x00000000); //PhoneStatusTime
	directInfo.addDWord(0x00000000); //PhoneBookTime
	directInfo.addWord(0x0000);
}*/


void OscarSocket::sendKeepalive()
{
	kdDebug(14150) << k_funcinfo << "SEND KEEPALIVE" << endl;
	Buffer outbuf;
//	outbuf.print();
	sendBuf(outbuf, 0x05);
}

void OscarSocket::startKeepalive()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
	if (keepaliveTime==0) // nobody wants keepalive, so shut up ;)
		return;

	if (!keepaliveTimer)
	{
		kdDebug(14150) << k_funcinfo << "Creating keepaliveTimer" << endl;
		keepaliveTimer=new QTimer(this, "keepaliveTimer");
		QObject::connect(keepaliveTimer,SIGNAL(timeout()),this,SLOT(slotKeepaliveTimer()));
		keepaliveTimer->start(keepaliveTime*1000);
	}
}

void OscarSocket::stopKeepalive()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
	if(keepaliveTimer)
	{
		kdDebug(14150) << k_funcinfo << "Deleting keepaliveTimer" << endl;
		delete keepaliveTimer;
		keepaliveTimer=0L;
	}
}

void OscarSocket::slotKeepaliveTimer()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
	sendKeepalive();
}

void OscarSocket::parseAdvanceMessage(Buffer &buf, UserInfo &user)
{
	kdDebug(14150) << k_funcinfo << "called" << endl;

	TLV tlv;
	bool moreTLVs = true; // Flag to indicate if there are more TLV's to parse

	while(moreTLVs)
	{
		tlv = buf.getTLV();

		kdDebug(14150) << k_funcinfo << "Found TLV(" << tlv.type << "), length=" << tlv.length << endl;

		switch(tlv.type)
		{
			case 0x0005:
			{
				Buffer type2(tlv.data, tlv.length);
				WORD ackType = type2.getWord();
				kdDebug(14150) << k_funcinfo << "acktype=" << ackType << endl;

				if (ackType==0x0000) // normal message
				{
					// ERROR
					QPtrList<TLV> lst = buf.getTLVList(); //type2.getTLVList();
					lst.setAutoDelete(TRUE);

					TLV *messageTLV = findTLV(lst,0x2711); //TLV(10001)
					if(messageTLV)
					{
						Buffer messageBuf(messageTLV->data, messageTLV->length);
						//WORD len;
						//WORD tcpver;
						char *cap=messageBuf.getBlock(16);

						QString capstring;
						capstring.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
							cap[0], cap[1], cap[2], cap[3],cap[4], cap[5],
							cap[6], cap[7], cap[8], cap[9],
							cap[10], cap[11], cap[12], cap[13],
							cap[14], cap[15]);
						kdDebug(14150) << k_funcinfo << "CAPABILITY:" << capstring << endl;

						delete [] cap;
						messageBuf.getByte(); // unknown
						messageBuf.getByte(); // unknown
						messageBuf.getByte(); // unknown
						messageBuf.getDWord(); // unknown, 0=normal message, 1=file ok...
						WORD seq1 = messageBuf.getWord(); // some stupid sequence
						messageBuf.getWord(); // unknown
						WORD seq2 = messageBuf.getWord(); // some stupid sequence
						if (seq1 != seq2)
							kdDebug(14150) << k_funcinfo << "TODO: seq1 != seq2, what shall we do now?" << endl;

						char *tmp = messageBuf.getBlock(12); // unknown
						delete[] tmp;
						WORD msgType = messageBuf.getWord(); // message type

						switch(msgType)
						{
							case 0x0001: // plain normal message
							{
								messageBuf.getWord(); // unknown
								messageBuf.getWord(); // unknown, might be priority
								char *messagetext = messageBuf.getLNTS();
								QString message = QString::fromLocal8Bit(messagetext);
								delete [] messagetext;
								kdDebug(14150) << k_funcinfo <<
									"type-2 messagtext=" << message << endl;
								/*DWORD fgColor=*/messageBuf.getDWord();
								/*DWORD bgColor=*/messageBuf.getDWord();
								kdDebug(14150) << k_funcinfo <<
									"messageBuf.getLength() after message and colors =" <<
									messageBuf.getLength() << endl;

								DWORD guidlen = messageBuf.getDWord();
								char *guid = messageBuf.getBlock(guidlen);
								kdDebug(14150) << k_funcinfo << "type-2 guid=" << guid << endl;
								delete [] guid;

								kdDebug(14150) << k_funcinfo << "emit gotIM(), contact='" <<
									user.sn << "', message='" << message << "'" << endl;

								emit gotIM(message, user.sn, false);
								break;
							}

							default:
							{
								kdDebug(14150) << k_funcinfo <<
									"Unhandled message-type:" << msgType << endl;
							}
						} // END switch(msgType)
					} // END found TLV(10001)
					else
					{
						kdDebug(14150) << k_funcinfo << "Could not find TLV(10001) in advanced message!" << endl;
						kdDebug(14150) << k_funcinfo << "contained TLVs:" << endl;
						TLV *t;
						for(t=lst.first(); t; t=lst.next())
						{
							kdDebug(14150) << k_funcinfo << "TLV(" << t->type << ") length=" << t->length << endl;
						}
					}

					lst.clear();
				}
				else
				{
					kdDebug(14150) << k_funcinfo << "UNHANDLED acktype" << endl;
					delete [] tlv.data;
				}
				break;
			} // END TLV(0x0005)

			default:
			{
				kdDebug(14150) << k_funcinfo << "Unhandled TLV(" << tlv.type << ") length=" << tlv.length << endl;
				delete [] tlv.data;
				break;
			}
		}

		if(buf.getLength() > 0)
			moreTLVs=true;
		else
			moreTLVs=false;
	} // END while(moreTLVs)

	kdDebug(14150) << k_funcinfo << "END" << endl;
}

// TODO: get on with the coding, need to implement that sequence counting and
// some sort of packet queue so I know what requests I sent out

/*
bool requestAutoReply(unsigned long uin, unsigned long status)
{
	if (status == 0)
		return false;

	int advCounter = 0;
	unsigned char type = 0xE8;
	if (status & ICQ_STATUS_DND)
		type = 0xEB;
	else if (status & ICQ_STATUS_OCCUPIED)
		type = 0xE9;
	else if (status & ICQ_STATUS_NA)
		type = 0xEA;
	else if (status & ICQ_STATUS_FREEFORCHAT)
		type = 0xEC;


	kdDebug(14150) << k_funcinfo << "SEND (CLI_SENDMSG), requesting away message" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0006,0x0000,0x00000000);
	outbuf.addDWord(0x00000000); // TIME
	int id1 = rand() & 0xFFFF;
	int id2 = rand() & 0xFFFF;
	outbuf.addWord(id1); // ID
	outbuf.addWord(id2); // ID

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
	tlv10001.addWord(advCounter);
	tlv10001.addWord(0xE000);
	tlv10001.addWord(advCounter);

	tlv10001.addDWord(0x00000000); // 12 unknown bytes, always zero
	tlv10001.addDWord(0x00000000);
	tlv10001.addDWord(0x00000000);
	tlv10001.addWord(0x0001) // message type - normal message

	addByte << (char)3;
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



WORD OscarSocket::sendCLI_TOICQSRV(const WORD subcommand, Buffer &data)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_TOICQSRV), subcommand=" << subcommand << endl;

	Buffer outbuf;
	DWORD word1 = 0x0000; // TODO: is this really right?
	DWORD word2 = (toicqsrv_seq);
	DWORD snacid = (word1 << 16) | word2;

	// 2nd try
/*	toicqsrv_seq++;
	DWORD snacid = toicqsrv_seq;*/
	// ---

	outbuf.addSnac(OSCAR_FAM_21,0x0002,0x0000,snacid);
	// yum yum, one up sequence, starts at 1 and has to be 2 for the first
	// usage on the LEWord added some lines under this comment ;)
	toicqsrv_seq++;

	int tlvLen = 10 + data.getLength();

	kdDebug(14150) << k_funcinfo <<
		"snacid=" << snacid <<
		", toicqsrv_seq=" << toicqsrv_seq <<
		", tlvLen=" << tlvLen << endl;

	outbuf.addWord(0x0001); // TLV(1)
	outbuf.addWord(tlvLen); // length of TLV, 10 if no data
	// NOTE: EVERYTHING IN THIS TLV IS LITTLE-ENDIAN
	outbuf.addLEWord(tlvLen-2); // length of data inside TLV, 8 if no data
	outbuf.addLEDWord(getSN().toULong()); // own uin
	outbuf.addLEWord(subcommand); // subcommand
	outbuf.addLEWord(toicqsrv_seq); // TODO: make this the snac sequence's upper Word minus 1!

	if (data.getLength() > 0)
		outbuf.addString(data.getBuf(), data.getLength());

// 	kdDebug(14150) << "==========================================" << endl;
// 	outbuf.print();
// 	kdDebug(14150) << "==========================================" << endl;

	sendBuf(outbuf, 0x2);
	return (toicqsrv_seq);
}

void OscarSocket::sendCLI_SEARCHBYUIN(const unsigned long uin)
{
	kdDebug(14150) << k_funcinfo << "SEND CLI_SEARCHBYUIN (CLI_META), uin=" << uin << endl;
	Buffer search;
	// NOTE: EVERYTHING IN THIS BUFFER IS LITTLE-ENDIAN
	search.addLEWord(0x0569); // subtype: 1385
	search.addLEWord(0x0136); // key = search for uin
	search.addLEWord(0x0004); // length of following data
	search.addLEDWord(uin); // the uin to search for

	sendCLI_TOICQSRV(0x07d0, search); // command = 2000, CLI_META
}

void OscarSocket::sendCLI_SEARCHWP(
	const QString &first,
	const QString &last,
	const QString &nick,
	const QString &mail,
	int minage,
	int maxage,
	int sex,
	int lang, // TODO: unused
	const QString &city,
	const QString state,
	int country,
	const QString &company,
	const QString &department,
	const QString &position,
	int occupation,
	bool onlineOnly) /*TODO: add all fields or make this somehow clever*/
{
	kdDebug(14150) << k_funcinfo << "SEND CLI_SEARCHWP (CLI_META)" << endl;

	Buffer search;
	// NOTE: EVERYTHING IN THIS BUFFER IS LITTLE-ENDIAN
	search.addLEWord(0x0533); // subtype: 1331

	//LNTS FIRST
	search.addLEWord(first.length());
	if(first.length()>0)
		search.addLEString(first.local8Bit(), first.length());

	// LNTS LAST
	search.addLEWord(last.length());
	if(last.length()>0)
		search.addLEString(last.local8Bit(), last.length());

	// LNTS NICK
	search.addLEWord(nick.length());
	if(nick.length()>0)
		search.addLEString(nick.local8Bit(), nick.length());

	// LNTS EMAIL
	search.addLEWord(mail.length());
	if(mail.length()>0)
		search.addLEString(mail.local8Bit(), mail.length());

	// WORD.L MINAGE
	search.addLEWord(minage);

	// WORD.L MAXAGE
	search.addLEWord(maxage);

	// BYTE xx SEX 1=fem, 2=mal, 0=dontcare
	if (sex==1)
		search.addLEByte(0x01);
	else if(sex==2)
		search.addLEByte(0x02);
	else
		search.addLEByte(0x00);

	// BYTE xx LANGUAGE
	search.addLEByte(lang);

	// LNTS CITY
	search.addLEWord(city.length());
	if(city.length()>0)
		search.addLEString(city.local8Bit(), city.length());

	// LNTS STATE
	search.addLEWord(state.length());
	if(state.length()>0)
		search.addLEString(state.local8Bit(), state.length());

	// WORD.L xx xx COUNTRY
	search.addLEWord(country);

	// LNTS COMPANY
	search.addLEWord(company.length());
	if(company.length()>0)
		search.addLEString(company.local8Bit(), company.length());

	// LNTS DEPARTMENT
	search.addLEWord(department.length());
	if(department.length()>0)
		search.addLEString(department.local8Bit(), department.length());

	// LNTS POSITION
	search.addLEWord(position.length());
	if(position.length()>0)
		search.addLEString(position.local8Bit(), position.length());

	// BYTE xx OCCUPATION
	search.addLEByte(occupation);

	//WORD.L xx xx PAST
	search.addLEWord(0x0000);

	//LNTS PASTDESC - The past description to search for.
	search.addLEWord(0x0000);

	// WORD.L xx xx INTERESTS - The interests category to search for.
	search.addLEWord(0x0000);

	// LNTS INTERDESC - The interests description to search for.
	search.addLEWord(0x0000);

	// WORD.L xx xx AFFILIATION - The affiliation to search for.
	search.addLEWord(0x0000);

	// LNTS AFFIDESC - The affiliation description to search for.
	search.addLEWord(0x0000);

	// WORD.L xx xx HOMEPAGE - The home page category to search for.
	search.addLEWord(0x0000);

	// LNTS HOMEDESC - The home page description to search for.
	search.addLEWord(0x0000);

	// BYTE xx ONLINE 1=online onliners, 0=dontcare
	if(onlineOnly)
		search.addLEByte(0x01);
	else
		search.addLEByte(0x00);

	sendCLI_TOICQSRV(0x07d0, search); // command = 2000, CLI_META
}

void OscarSocket::sendReqOfflineMessages()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_REQOFFLINEMSGS), requesting offline messages" << endl;

	Buffer empty;
	sendCLI_TOICQSRV(0x003c, empty);
}

void OscarSocket::sendAckOfflineMessages()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_ACKOFFLINEMSGS), acknowledging offline messages" << endl;

	Buffer empty;
	sendCLI_TOICQSRV(0x003e, empty);
}

WORD OscarSocket::sendReqInfo(const unsigned long uin)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_METAREQINFO), requesting user information" << endl;

	Buffer req; // ! LITTLE-ENDIAN
	req.addLEWord(0x04d0); // subtype: 1232
	req.addLEDWord(uin);
	WORD ret = sendCLI_TOICQSRV(0x07d0, req);
	return ret;
}

// vim: set noet ts=4 sts=4 sw=4:
