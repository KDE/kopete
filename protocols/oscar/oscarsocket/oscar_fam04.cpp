/*
    oscar_fam4.cpp  -  Oscar Protocol, SNAC Family 4 (ICBM)

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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
#include "oscardebug.h"
#include "oscaraccount.h"

#include <stdlib.h>

#include <qtextcodec.h>

#include <kdebug.h>


// SNAC(04,04) CLI_ICBM_PARAM_REQ
// http://iserverd.khstu.ru/oscar/snac_04_04.html
// Sends a request for msg rights
void OscarSocket::requestMsgRights()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_REQICBM), Requesting rights for ICBM (instant messages)" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0004,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}


// SNAC(04,02) CLI_SET_ICBM_PARAMS
// http://iserverd.khstu.ru/oscar/snac_04_02.html
// Sends parameters for ICBM messages (CLI_SETICBM)
void OscarSocket::sendMsgParams()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_SETICBM)" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0002,0x0000,0x00000002);

	// max channels
	//this is read-only, and must be set to 0 here
	outbuf.addWord(0x0000);

	//these are all read-write
	// channel-flags
	// outbuf.addDWord(mIsICQ ? 0x00000003 : 0x0000000b);
	// bit 1 : messages allowed (always 1 or you cannot send IMs)
	// bit 2 : missed call notifications enabled
	// bit 4 : typing notifications enabled
	outbuf.addDWord(0xb);

	//max message length (8000 bytes)
	outbuf.addWord(0x1f40);
	//max sender warning level (999)
	outbuf.addWord(0x03e7);
	//max receiver warning level  (999)
	outbuf.addWord(0x03e7);
	//min message interval limit  (0 msec)
	outbuf.addWord(0x0000);
	// unknown parameter
	outbuf.addWord(0x0000);

	sendBuf(outbuf,0x02);
}


// SNAC(04,05) SRV_ICBM_PARAMS
// http://iserverd.khstu.ru/oscar/snac_04_05.html
void OscarSocket::parseMsgRights(Buffer &inbuf)
{

	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYICBM) Parsing ICBM rights" << endl;

	WORD channel = inbuf.getWord();
	kdDebug(14150) << k_funcinfo << "channel=" << channel << endl;

	/**
	 * bit1: messages allowed for specified channel
	 * bit2: missed calls notifications enabled for specified channel
	 * bit4: client supports typing notifications
	 */
	DWORD messageFlags = inbuf.getDWord();
	WORD maxMessageSnacSize = inbuf.getWord();
	WORD maxSendWarnLvl = inbuf.getWord(); // max sender Warning Level
	WORD maxRecvWarnLvl = inbuf.getWord(); // max Receiver Warning Level
	WORD minMsgInterval = inbuf.getWord(); // minimum message interval (msec)


	kdDebug(14150) << k_funcinfo << "messageFlags       = " << messageFlags << endl;
	kdDebug(14150) << k_funcinfo << "maxMessageSnacSize = " << maxMessageSnacSize << endl;
	kdDebug(14150) << k_funcinfo << "maxSendWarnLvl     = " << maxSendWarnLvl << endl;
	kdDebug(14150) << k_funcinfo << "maxRecvWarnLvl     = " << maxRecvWarnLvl << endl;
	kdDebug(14150) << k_funcinfo << "minMsgInterval     = " << minMsgInterval << endl;

	/*WORD unknown = */inbuf.getWord();

	// After we get this from the server
	// we have to send some messaging parameters
	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}


// SNAC(04,0A) SRV_MISSED_MESSAGE
// http://iserverd.khstu.ru/oscar/snac_04_0a.html
void OscarSocket::parseMissedMessage(Buffer &inbuf)
{
	while (inbuf.length() > 0)
	{
		// get the channel (this isn't used anywhere)
		/*WORD channel =*/ inbuf.getWord();

		// get user info
		UserInfo u;
		parseUserInfo(inbuf, u);

		// get number of missed messages
		WORD nummissed = inbuf.getWord();

		//the number the aol servers report seems to be one too many
		nummissed--;

		// get reason for missed messages
		WORD reason = inbuf.getWord();

		// FIXME: concatenating i18n string is wrong!
		QString errstring = i18n(
			"You missed one message from %1. Reason given:",
			"You missed %n messages from %1. Reason given:",
			nummissed).arg(u.sn) + "\n";
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
				errstring += i18n("Unknown reason.");
				break;
		};
		emit protocolError(errstring, 0);
	}
}


// SNAC(04,0B) CLI_ICBM_SENDxACK
// http://iserverd.khstu.ru/oscar/snac_04_0b.html
void OscarSocket::parseMsgAck(Buffer &inbuf)
{

	WORD sublen, seq2;
	BYTE msgFlags, msgType;
	WORD msgStatus, msgPrio;

	inbuf.getBlock(8);

	inbuf.getWord(); // message-type, only type-2 is acknowledged so this is always 2

	char *tmp = inbuf.getBUIN();
	QString uin = QString::fromLatin1(tmp);
	delete [] tmp;

	inbuf.getWord(); // unk

	sublen = inbuf.getLEWord(); // len of following subchunk
	//kdDebug(14150) << k_funcinfo << "sublen=" << sublen << endl;
	inbuf.getBlock(sublen); // ignore subchunk
	//kdDebug(14150) << k_funcinfo << "len after subchunk=" << inbuf.length() << endl;

	inbuf.getLEWord();
	seq2 = inbuf.getLEWord();
	inbuf.getBlock(12); // ignore 12 zero bytes
	//kdDebug(14150) << k_funcinfo << "len after 12 zero bytes=" << inbuf.length() << endl;

	msgType = inbuf.getByte(); //
	msgFlags = inbuf.getByte(); // type and flags have wrong order because it's a little-endian word
	msgStatus = inbuf.getLEWord();
	msgPrio = inbuf.getLEWord();

	WORD txtLen = inbuf.getLEWord();
	char *txtStr = inbuf.getBlock(txtLen);
	QString text = QString::fromLatin1(txtStr); // TODO: encoding
	delete [] txtStr;

	kdDebug(14150) << k_funcinfo << "RECV (ACKMSG) uin=" << uin <<
		" msgType=" << (int)msgType << " msgFlags=" << (int)msgFlags <<
		" msgStatus=" << (int)msgStatus << " msgPrio=" << (int)msgPrio <<
		" text='"  << text << "'" << endl;

	if(msgFlags == MSG_FLAG_GETAUTO)
	{
		emit receivedAwayMessage(uin, text);
	}

	//TODO: there can be more data after text
}

// SNAC(04,0C) SRV_MSG_ACK
// http://iserverd.khstu.ru/oscar/snac_04_0c.html
void OscarSocket::parseSrvMsgAck(Buffer &inbuf)
{
	//8 byte cookie is first
	/*char *ck =*/ inbuf.getBlock(8);
	//delete [] ck;
	WORD type = inbuf.getWord();

	char *sn = inbuf.getBUIN();
	QString nm = QString::fromLatin1(sn);
	delete [] sn;

	kdDebug(14150) << k_funcinfo <<
		"RECV (SRV_SRVACKMSG) sn=" << nm << ", type=" << type << endl;

	emit gotAck(nm,type);
}


// SNAC(04,14) TYPING_NOTIFICATION
// http://iserverd.khstu.ru/oscar/snac_04_14.html
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
	QString screenName = QString::fromLatin1(sn); // TODO: encoding
	delete [] sn;

	// Get the actual notification
	WORD notification = inbuf.getWord();

//	kdDebug(14150) << k_funcinfo <<
//		"Determining Minitype from user '" << screenName << "'" << endl;

	switch(notification)
	{
		case 0x0000: // Typing finished
			emit gotMiniTypeNotification(screenName, OscarConnection::TypingFinished);
			break;
		case 0x0001: // Text Typed
			emit gotMiniTypeNotification(screenName, OscarConnection::TextTyped);
			break;
		case 0x0002: // Typing Started
			emit gotMiniTypeNotification(screenName, OscarConnection::TypingBegun);
			break;
		default:
			kdDebug(14150) << k_funcinfo << "MiniType Error: " << notification << endl;
	}
}

// SNAC(04,14) TYPING_NOTIFICATION
// http://iserverd.khstu.ru/oscar/snac_04_14.html
// Sends a minityping notification
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
	outbuf.addSnac(OSCAR_FAM_4,0x0014,0x0000,0x00000001);
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


void OscarSocket::parseIM(Buffer &inbuf)
{
	QByteArray cook(8);
	//This is probably the hardest thing to do in oscar
	//first comes an 8 byte ICBM cookie (random)
	// from icq docs:
	// this value is
	// ((time(NULL) - (8*60*60)) + DayOfWeek*60*60*24)) * 1500
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
	// information, the rest are not even though there is no separation.
	//
	// That also means that TLV types can be duplicated between the
	// userinfo block and the rest of the message, however there should
	// never be two TLVs of the same type in one block.
	UserInfo u;
	parseUserInfo(inbuf, u);

	switch(channel)
	{
		case MSGFORMAT_SIMPLE: //normal IM
		{
			parseSimpleIM(inbuf, u);
			break;
		}; // END MSGFORMAT_SIMPLE


		case MSGFORMAT_ADVANCED: //AIM rendezvous, ICQ advanced messages
		{
#if 0
			if (mIsICQ) // TODO: unify AIM and ICQ in this place
#endif
			{
				kdDebug(14150) << k_funcinfo << "IM received on channel 2 from '" << u.sn << "'" << endl;
				TLV tlv5tlv = inbuf.getTLV();
				kdDebug(14150) << k_funcinfo << "The first TLV is of type " << tlv5tlv.type << endl;
				if (tlv5tlv.type != 0x0005)
				{
					kdDebug(14150) << k_funcinfo << "Aborting because first TLV != TLV(5)" << endl;
					break;
				}

				Buffer tlv5(tlv5tlv.data, tlv5tlv.length);

				/*WORD ackType =*/ tlv5.getWord();
				DWORD msgTime = tlv5.getDWord();
				DWORD msgRandomId = tlv5.getDWord();
				char *capData = tlv5.getBlock(16);
				DWORD capFlag = parseCap(capData);
				delete [] capData;

				QPtrList<TLV> lst = tlv5.getTLVList();
				lst.setAutoDelete(TRUE);

				TLV *msgTLV = findTLV(lst,0x2711);  //message tlv
				if(!msgTLV)
				{
					kdDebug(14150) << k_funcinfo << "Aborting because TLV(10001) wasn't found (no message?)" << endl;
					break;
				}

				switch(capFlag)
				{
					case AIM_CAPS_ISICQ:
						// found in direct-connection stuff?
						break;

					case AIM_CAPS_ICQSERVERRELAY:
					{
						Buffer messageBuf(msgTLV->data, msgTLV->length);
						WORD len = messageBuf.getLEWord();
						if (len != 0x001b)
							kdDebug(14150) << k_funcinfo << "wrong len till SEQ1!" << endl;
						WORD tcpVer = messageBuf.getLEWord();
						//kdDebug(14150) << k_funcinfo << "len=" << len << ", tcpver=" << tcpVer << endl;
						char *cap=messageBuf.getBlock(16);
						WORD unk1 = messageBuf.getLEWord();
						DWORD unk2 = messageBuf.getLEDWord();
						BYTE unk3 = messageBuf.getLEByte();
						WORD seq1 = messageBuf.getLEWord();


						Buffer ack; // packet sent back as acknowledgment
						ack.addSnac(4, 11, 0, 0);
						ack.addDWord(msgTime);
						ack.addDWord(msgRandomId);
						ack.addWord(0x0002); // type-2 ack
						ack.addBUIN(u.sn.latin1());
						ack.addWord(0x0003); // unknown
						ack.addLEWord(len);
						ack.addLEWord(tcpVer);
						ack.addString(cap, 16);
						ack.addLEWord(unk1);
						ack.addLEDWord(unk2);
						ack.addLEByte(unk3);
						ack.addLEWord(seq1);

						parseAdvanceMessage(messageBuf, u, ack);
						break;
					}
					default: // TODO
						break;
				} // END switch(capFlag)

				break;
			}
#if 0
			else
			{

				// ===========================================
				// TODO: unify AIM and ICQ parts
				// ===========================================

				unsigned int remotePort = 0;
				QString qh;
				QString message;
				WORD msgtype = 0x0000; //used to tell whether it is a direct IM requst, deny, or accept
				DWORD capflag = 0x00000000; //used to tell what kind of rendezvous this is
				/*OncomingSocket *sockToUse;*/ //used to tell which listening socket to use
				QString fileName; //the name of the file to be transferred (if any)
				long unsigned int fileSize = 0; //the size of the file(s) to be transferred

				kdDebug(14150) << k_funcinfo << "IM received on channel 2 from " << u.sn << endl;
				TLV tlv = inbuf.getTLV();
				kdDebug(14150) << k_funcinfo << "The first TLV is of type " << tlv.type;
				if (tlv.type == 0x0005) //connection info
				{
					Buffer tmpbuf(tlv.data, tlv.length);
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
					capflag = parseCap(cap);
					if (capflag == 0x00000000)
						kdDebug(14150) << k_funcinfo << "unknown CAP: " << CapToString(cap) << endl;
					delete [] cap;

					//Next comes a big TLV chain of stuff that may or may not exist
					QPtrList<TLV> tlvlist = tmpbuf.getTLVList();
					TLV *cur;
					tlvlist.setAutoDelete(true);

					for(cur = tlvlist.first();cur;cur = tlvlist.next())
					{
						if (cur->type == 0x0002)
						{
							//IP address from the perspective of the client
							kdDebug(14150) << "ClientIP1: " << cur->data[0] << "."
								<< cur->data[1] << "." << cur->data[2] << "."
								<< cur->data[3]  << endl;
						}
						else if (cur->type == 0x0003)
						{
							//Secondary IP address from the perspective of the client
								kdDebug(14150) << "ClientIP2: " << cur->data[0] << "."
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
							qh = cur->data[0] + '.' + cur->data[1] + '.' + cur->data[2] + '.' + cur->data[3];
							kdDebug(14150) << "OscarIPRaw: " <<
								cur->data[0] << "." << cur->data[1] << "." <<
								cur->data[2] << "." << cur->data[3] << endl;
							kdDebug(14150) << "OscarIP: " << qh << endl;
						}
						else if (cur->type == 0x0005) //Port number
						{
							remotePort = (cur->data[0] << 8) | cur->data[1];
							kdDebug(14150) << k_funcinfo << "remotePort=" << remotePort << endl;
						}
						//else if (cur->type == 0x000a)
						//{
						//}
						//Error code
						else if (cur->type == 0x000b)
						{
							kdDebug(14150) << k_funcinfo << "ICBM ch 2 error code " <<
								((cur->data[1] << 8) | cur->data[0]) << endl;

							emit protocolError(
								i18n("Rendezvous with buddy failed. Please check your " \
									"internet connection or try the operation again later. " \
									"Error code %1.\n").arg((cur->data[1] << 8) | cur->data[0]), 0);
						}
						//Invitation message/ chat description
						else if (cur->type == 0x000c)
						{
							message = cur->data;
							kdDebug(14150) << k_funcinfo << "Invited to chat " << cur->data << endl;
						}
						//Character set
						else if (cur->type == 0x000d)
						{
							kdDebug(14150) << k_funcinfo << "Using character set " << cur->data << endl;
						}
						//Language
						else if (cur->type == 0x000e)
						{
							kdDebug(14150) << k_funcinfo << "Using language " << cur->data << endl;
						}
						//File transfer
						else if (cur->type == 0x2711)
						{
							Buffer thebuf(cur->data,cur->length);

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
					kdDebug(14150) << k_funcinfo << "IM: unknown TLV type " << tlv.type << endl;
				}

				// Set the appropriate server socket
				sockToUse = serverSocket(capflag);

				if (msgtype == 0x0000) // initiate
				{
					kdDebug(14150) << k_funcinfo << "adding " << u.sn << " to pending list." << endl;
					if(capflag & AIM_CAPS_IMIMAGE) //if it is a direct IM rendezvous
					{
						sockToUse->addPendingConnection(u.sn, cook, 0L, qh, 4443, DirectInfo::Outgoing);
						emit gotDirectIMRequest(u.sn);
					}
					else // file send
					{
						sockToUse->addPendingConnection(u.sn, cook, 0L, qh, remotePort, DirectInfo::Outgoing);
						emit gotFileSendRequest(u.sn, message, fileName, fileSize);
					}
				}
				else if (msgtype == 0x0001) //deny
				{
					if(capflag & AIM_CAPS_IMIMAGE)
						emit protocolError(i18n("Direct IM request denied by %1").arg(u.sn),0);
					else
						emit protocolError(i18n("Send file request denied by %1").arg(QString(u.sn)),0);
					sockToUse->removeConnection(u.sn);
				}
			} // END if (mIsICQ)
#endif
			break;
		} // END MSGFORMAT_ADVANCED


		case MSGFORMAT_SERVER: // non-acknowledged, server messages (ICQ ONLY I THINK)
		{
			parseServerIM(inbuf, u);
			break;
		}; // END MSGFORMAT_SERVER

		default: // unknown channel
			kdDebug(14150) << "Error: unknown ICBM channel " << channel << endl;
	} // END switch(channel)
}



void OscarSocket::parseSimpleIM(Buffer &inbuf, const UserInfo &u)
{
	OscarContact *contact = static_cast<OscarContact*>(mAccount->contacts()[tocNormalize(u.sn)]);

	bool moreTLVs = true; // Flag to indicate if there are more TLV's to parse
	bool isAutoResponse = false; // This gets set if we are notified of an auto response
	WORD length = 0;

	kdDebug(14150) << k_funcinfo << "RECV TYPE-1 IM from '" << u.sn << "'" << endl;

	while(moreTLVs)
	{
		WORD type = inbuf.getWord();
		kdDebug(14150) << k_funcinfo << "TLV(" << type << ")" << endl;
		switch(type)
		{
			case 0x0002: //TLV(2), message block
			{
				// This is the total length of the rest of this message TLV
				length = inbuf.getWord();
				TLV caps = inbuf.getTLV(); // TLV(1281), CAPABILITIES
				if (caps.type == 1281)
				{
					//kdDebug(14150) << k_funcinfo << "TLV(1281), CAPABILITIES" << endl;
					Buffer capBuf(caps.data, caps.length);
					/*
					while(capBuf.length() > 0)
					{
						BYTE capPart = capBuf.getByte();
						kdDebug(14150) << k_funcinfo <<
							"capPart = '" << capPart << "'" << endl;

						if (capPart==0x06)
						{
							kdDebug(14150) << k_funcinfo <<
								"TLV(1281) says sender does UTF-8 :)" << endl;
						}
					}
					*/
					capBuf.clear();
				}
				delete [] caps.data;

				TLV tlvMessage = inbuf.getTLV(); // TLV(257), MESSAGE
				if(tlvMessage.type == 0x0101)
				{
					//kdDebug(14150) << k_funcinfo << "TLV(257), MESSAGE" << endl;
					Buffer msgBuf(tlvMessage.data, tlvMessage.length);

					WORD charsetNumber = msgBuf.getWord();
					/*WORD charsetSubset =*/ msgBuf.getWord();
					int messageLength = msgBuf.length();
					if (messageLength < 1)
						break;

					OscarMessage oMsg;

					if (charsetNumber == 0x0002) // UCS-2BE (or UTF-16)
					{
						//kdDebug(14150) << k_funcinfo << "UTF-16BE message" << endl;
						const unsigned short *txt = msgBuf.getWordBlock((int)messageLength/2);
						oMsg.setText(QString::fromUcs2(txt),
							mIsICQ ? OscarMessage::Plain : OscarMessage::AimHtml);
						delete [] txt;
					}
					else if (charsetNumber == 0x0003) // local encoding, usually iso8859-1
					{
						//kdDebug(14150) << k_funcinfo << "ISO8859-1 message" << endl;
						const char *messagetext = msgBuf.getBlock(messageLength);
						oMsg.setText(QString::fromLatin1(messagetext),
							mIsICQ ? OscarMessage::Plain : OscarMessage::AimHtml);
						delete [] messagetext;
					}
					else
					{ // BEGIN unknown or us-ascii
						/*kdDebug(14150) << k_funcinfo <<
							"Unknown encoding or US-ASCII, guessing encoding" << endl;*/
						const char *messagetext = msgBuf.getBlock(messageLength);
						oMsg.setText(ServerToQString(messagetext, contact, false),
							mIsICQ ? OscarMessage::Plain : OscarMessage::AimHtml);
						delete [] messagetext;
					} // END unknown or us-ascii


					parseMessage(u, oMsg, isAutoResponse ? MSG_AUTO : MSG_NORM, 0);

					msgBuf.clear();
				}
				else
				{
					kdDebug(14150) << k_funcinfo <<
						"Cannot find TLV(257), no message inside packet???" << endl;
				}

				//kdDebug(14150) << k_funcinfo << "deleting data from TLV(257)" << endl;
				delete [] tlvMessage.data; // getTLV uses getBlock() internally! same as aboves delete applies

				moreTLVs = (inbuf.length() > 0);
				break;
			} // END TLV(0x0002), normal message block

			case 0x0004: // AIM Away message
			{
				kdDebug(14150) << k_funcinfo << "AIM autoresponse." << endl;
				// There isn't actually a message in this TLV, it just specifies
				// that the message that was send was an autoresponse
				inbuf.getWord();
				// Set the autoresponse flag
				isAutoResponse = true;

				moreTLVs = (inbuf.length() > 0);
				break;
			} // END TLV(0x0004), AIM away message

			case 0x0008: // User Icon
			{
				kdDebug(14150) << k_funcinfo << "AIM USER ICON." << endl;
				// TODO support this
				// The length of the TLV
				length = inbuf.getWord();
				/*char *msg =*/ inbuf.getBlock(length);
				moreTLVs = (inbuf.length() > 0);
				break;
			}

			case 0x000b: //unknown
			{
				/*length = */inbuf.getWord();
				moreTLVs = (inbuf.length() > 0);
				break;
			}

			default: //unknown type
			{
				kdDebug(14150) << k_funcinfo <<
					"Unknown message type, type=" << type << endl;
				moreTLVs = (inbuf.length() > 0);
				break;
			}
		};
	}
}



void OscarSocket::parseServerIM(Buffer &inbuf, const UserInfo &u)
{
	kdDebug(14150) << k_funcinfo << "IM received on channel 4 from " << u.sn << endl;
	TLV tlv5tlv = inbuf.getTLV();
	kdDebug(14150) << k_funcinfo << "The first TLV is of type " << tlv5tlv.type << endl;
	if (tlv5tlv.type != 0x0005)
	{
		kdDebug(14150) << k_funcinfo << "Aborting because first TLV != TLV(5)" << endl;
		return;
	}

	Buffer tlv5(tlv5tlv.data, tlv5tlv.length);

	DWORD uin = tlv5.getLEDWord(); // little endian for no sane reason!
	if(QString::number(uin) != u.sn)
	{
		kdWarning(14150) << k_funcinfo <<
		"type-4 message uin does not match uin found in packet header!" << endl;
	}
	BYTE msgtype = tlv5.getByte();
	BYTE msgflags = tlv5.getByte();

	kdDebug(14150) << k_funcinfo <<
		"MSGFORMAT_SERVER; server message, TLV(5) length= " << tlv5tlv.length <<
		", uin=" << uin <<
		", type=" << msgtype <<
		", flags=" << msgflags << endl;

	// can be NULL if contact is not in contactlist!
	OscarContact *contact = static_cast<OscarContact*>(mAccount->contacts()[u.sn]);

	const char *msgText = tlv5.getLNTS();

	OscarMessage oMsg;
	oMsg.setText(ServerToQString(msgText, contact, false), OscarMessage::Plain);

	delete [] msgText; // getBlock allocates memory, we HAVE to free it again!

	if(!oMsg.text().isEmpty())
		parseMessage(u, oMsg, msgtype, msgflags);

	//kdDebug(14150) << k_funcinfo << "END" << endl;
} // END parseServerIM()


void OscarSocket::parseMessage(const UserInfo &u, OscarMessage &message, const BYTE type, const BYTE /*flags*/)
{
	switch(type)
	{
		case MSG_AUTO:
			kdDebug(14150) << k_funcinfo <<
				"Got an automatic message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::Away);
			emit receivedAwayMessage(u.sn, message.text()); // only sets contacts away message var
			emit receivedMessage(u.sn, message); // also displays message in chatwin
			break;
		case MSG_NORM:
			kdDebug(14150) << k_funcinfo <<
				"Got a normal message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::Normal);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_URL:
			kdDebug(14150) << k_funcinfo <<
				"Got an URL message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::URL);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_AUTHREJ:
			kdDebug(14150) << k_funcinfo <<
				"Got an 'auth rejected' message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::DeclinedAuth);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_AUTHACC:
			kdDebug(14150) << k_funcinfo <<
				"Got an 'auth granted' message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::GrantedAuth);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_WEB:
			kdDebug(14150) << k_funcinfo <<
				"Got a web panel message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::WebPanel);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_EMAIL:
			kdDebug(14150) << k_funcinfo <<
				"Got an email message: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::EMail);
			emit receivedMessage(u.sn, message);
			break;
		case MSG_CHAT:
		case MSG_FILE:
		case MSG_CONTACT:
		case MSG_EXTENDED:
			kdDebug(14150) << k_funcinfo <<
				"Got an unsupported message, dropping: '" << message.text() << "'" << endl;
			break; // TODO: unsupported and for now dropped messages
		default:
			kdDebug(14150) << k_funcinfo <<
				"Got unknown message type, treating as normal: '" << message.text() << "'" << endl;
			message.setType(OscarMessage::Normal);
			emit receivedMessage(u.sn, message);
			break;
	} // END switch
}



// TODO: Split up into type-1, type-2 and type-4 messaging
// currently we only do simple type-1
void OscarSocket::sendIM(const QString &message, OscarContact *contact, bool isAuto)
{
	//check to see if we have a direct connection to the contact
	#if 0
	OscarConnection *dc = mDirectIMMgr->findConnection(contact->contactName());
	if (dc)
	{
		kdDebug(14150) << k_funcinfo <<
			"Sending direct IM " << message <<
			" to " << contact->contactName() << endl;

		dc->sendIM(message, isAuto);
		return;
	}
	#endif

	kdDebug(14150) << k_funcinfo << "SEND (CLI_SENDMSG), msg='" << message <<
		"' to '" << contact->contactName() << "'" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0006,0x0000, toicqsrv_seq);
	toicqsrv_seq++;

	for (int i=0;i<8;i++) //generate random message cookie (MID, message ID)
		outbuf.addByte( (BYTE) rand());

	outbuf.addWord(0x0001); // message type, this is only type-1
	// TODO: support more types
	// 2 -> special messages (also known as advanced messages)
	// 4 -> url etc.

	outbuf.addByte((contact->contactName()).length()); //dest sn length
	outbuf.addString(contact->contactName().latin1(), contact->contactName().length()); //dest sn

	// ====================================================================================
	Buffer tlv2;
	tlv2.addWord(0x0501); // add TLV(0x0501) also known as TLV(1281)
	if(mIsICQ)
	{
		static const char icqfeatures[] = {0x01, 0x06}; // 0x06 == UTF support
		tlv2.addWord(sizeof(icqfeatures)); // add TLV length
		tlv2.addString(icqfeatures, sizeof(icqfeatures));
	}
	else
	{
		static const char aimfeatures[] = {0x01, 0x01, 0x01, 0x02};
		tlv2.addWord(sizeof(aimfeatures)); // add TLV length
		tlv2.addString(aimfeatures, sizeof(aimfeatures));
	}

	QTextCodec *codec = 0L;
	WORD charset = 0x0000; // default to ascii
	WORD charsubset = 0x0000;
	int length = message.length();
	unsigned char *utfMessage = 0L;

	codec=QTextCodec::codecForMib(3); // US-ASCII

	if(codec)
	{
		if(codec->canEncode(message))
		{
			//kdDebug(14150) << k_funcinfo << "Going to encode as US-ASCII" << endl;
			charset=0x0000; // send US-ASCII
		}
		else
		{
			codec=0L; // we failed encoding it as US-ASCII
			//kdDebug(14150) << k_funcinfo << "Cannot encode as US-ASCII" << endl;
		}
	}

	if(!codec && contact->hasCap(AIM_CAPS_UTF8))
	{
		// use UTF is peer supports it and encoding as US-ASCII failed
		length=message.length()*2;
		utfMessage=new unsigned char[length];
		for(unsigned int l=0; l<message.length(); l++)
		{
			utfMessage[l*2]=message.unicode()[l].row();
			utfMessage[(l*2)+1]=message.unicode()[l].cell();
		}
		charset=0x0002; // send UTF-16BE
	}
	else
	{
		//kdDebug(14150) << k_funcinfo << "Won't send as UTF-16BE, codec value=" << (void *)codec << endl;
	}

	// no codec and no charset and per-contact encoding set
	if(!codec && charset != 0x0002 && contact->encoding() != 0)
	{
		codec=QTextCodec::codecForMib(contact->encoding());
		if(codec)
		{
			charset=0x0003;
			/*kdDebug(14150) << k_funcinfo <<
				"Using per-contact encoding, encoding name:" << codec->name() << endl;*/
		}
		else
		{
			//kdDebug(14150) << k_funcinfo << "Could not find QTextCodec for per-contact encoding!" << endl;
		}
	}
	else
	{
		//kdDebug(14150) << k_funcinfo << "Won't use per-contact encoding, codec value=" << (void *)codec << endl;
	}

	if(!codec && charset != 0x0002) // it's neither unicode nor did we find a codec so far!
	{
		kdDebug(14150) << k_funcinfo <<
			"Couldn't find suitable encoding for outgoing message, " <<
			"encoding using ISO-8859-1, prepare for receiver getting unreadable text :)" << endl;
		charset=0x0003;
		codec=QTextCodec::codecForMib(4); // ISO-8859-1
	}

	tlv2.addWord(0x0101); //add TLV(0x0101) also known as TLV(257)
	tlv2.addWord(length + 0x04); // add TLV length
	tlv2.addWord(charset); // normal char set
	tlv2.addWord(charsubset); // normal char set

	if(utfMessage)
	{
		kdDebug(14150) << k_funcinfo << "Outgoing message encoded as 'UTF-16BE'" << endl;
		tlv2.addString(utfMessage, length); // the actual message
		delete [] utfMessage;
	}
	else
	{
		kdDebug(14150) << k_funcinfo << "Outgoing message encoded as '" << codec->name() << "'" << endl;
		QCString outgoingMessage=codec->fromUnicode(message);
		tlv2.addString(outgoingMessage, length); // the actual message
	}
	// ====================================================================================

	outbuf.addTLV(0x0002, tlv2.length(), tlv2.buffer());

	if(isAuto) // No clue about this stuff, probably AIM-specific [mETz]
	{
		outbuf.addWord(0x0004);
		outbuf.addWord(0x0000);
	}

	if(mIsICQ)
	{
		//outbuf.addWord(0x0003); // TLV.Type(0x03) - request an ack from server
		//outbuf.addWord(0x0000);

		outbuf.addWord(0x0006); // TLV.Type(0x06) - store message if recipient offline
		outbuf.addWord(0x0000);
	}

	sendBuf(outbuf,0x02);
}


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



/* Request, deny, or accept a rendezvous session with someone
type == 0: request
type == 1: deny
type == 2: accept
*/
void OscarSocket::sendRendezvous(const QString &/*sn*/, WORD /*type*/, DWORD /*rendezvousType*/, const KFileItem */*finfo*/)
{
	kdDebug(14150) << "DISABLED!" << endl;
#if 0
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
	if(type == 0)
	{
		sockToUse->addPendingConnection(sn, ck, finfo, QString::null, 0,
			DirectInfo::Incoming);
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
	if(!finfo) //this is a simple direct IM
	{
		if (type == 0x0000)
			outbuf.addWord(2+8+16+6+8+6+4);
		else
			outbuf.addWord(2+8+16);
	}
	else //this is a file transfer request
	{
		if (type == 0x0000)
		{
			outbuf.addWord(2+8+16+6+8+6+4+2+2+2+2+4+
				finfo->url().fileName().length()+4);
		}
		else
		{
			outbuf.addWord(2+8+16);
		}
	}

	outbuf.addWord(type); //2
	outbuf.addString(ck,8); //8
	for (int i=0;oscar_caps[i].flag != AIM_CAPS_LAST;i++)
	{
		if (oscar_caps[i].flag & rendezvousType)
		{
			outbuf.addString(oscar_caps[i].data,0x10);
			break;
		}
	} //16

	if(type == 0x0000 ) //if this is an initiate rendezvous command
	{
		//TLV (type a)
		outbuf.addWord(0x000a);
		outbuf.addWord(0x0002);
		outbuf.addWord(0x0001); //6
		//TLV (type 3)
		outbuf.addWord(0x0003);
		outbuf.addWord(0x0004);

		if (sockToUse->mSocket->socketStatus() < KExtendedSocket::created)
		{  //make sure the socket stuff is properly set up
			kdDebug(14150) << k_funcinfo << "SERVER SOCKET NOT SET UP... " <<
			"returning from sendRendezvous" << endl;

			emit protocolError(i18n("Error setting up listening socket." \
					" The request will not be send."), 0);
			return;
		}

		outbuf.addDWord(static_cast<DWORD>(setIPv4Address(sockToUse->mSocket->host()))); //8
		//TLV (type 5)
		outbuf.addWord(0x0005);
		outbuf.addWord(0x0002); //8
		outbuf.addWord(sockToUse->mSocket->port().toUShort()); //6
		//TLV (type f)
		outbuf.addTLV(0x000f,0x0000,NULL); //4

		if ( finfo )
		{
			outbuf.addWord(0x2711); //2
			outbuf.addWord(2+2+4+finfo->url().fileName().length()+4); //2
			outbuf.addWord(0x0001); //more than 1 file? (0x0002 for multiple -- implement later)
			outbuf.addWord(0x0001); //number of files
			outbuf.addDWord(finfo->size());
			outbuf.addString(finfo->url().fileName().latin1(),
				finfo->url().fileName().length());
			outbuf.addDWord(0x00000000);
		}
	}

	kdDebug(14150) << "Sending direct IM, type " << type << " from " <<
		sockToUse->mSocket->host() << ", port " << sockToUse->mSocket->port() << endl;

	sendBuf(outbuf,0x02);
#endif
}


/*
  Request a direct IM session with someone
	type == 0: request
	type == 1: deny
	type == 2: accept
*/
void OscarSocket::sendDirectIMRequest(const QString &sn)
{
	sendRendezvous(sn,0x0000,AIM_CAPS_IMIMAGE);
}


void OscarSocket::sendDirectIMDeny(const QString &sn)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	sendRendezvous(sn,0x0001,AIM_CAPS_IMIMAGE);
}


void OscarSocket::sendDirectIMAccept(const QString &sn)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	sendRendezvous(sn,0x0002,AIM_CAPS_IMIMAGE);
	/*
	if(!mDirectIMMgr->establishOutgoingConnection(sn))
	{
		kdDebug(14150) << k_funcinfo << sn <<
			" not found in pending connection list" << endl;
	}
	*/
}
