/*
  oscarsocket.icq.cpp  -  ICQ specific part of Oscarsocket

  Copyright (c) 2004 by Stefan Gehn <metz AT gehn.net>
  Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>


  Much of this code was learned from code of the following apps:
    gaim - http://gaim.sourceforge.net
    micq - http://www.micq.org
    sim-icq - http://sim-icq.sourceforge.net

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
#include "oscarcontact.h"
#include <kopeteonlinestatus.h>

#include <qtextcodec.h>
#include <stdlib.h>
#include <qtimer.h>
#include <kdebug.h>

/**
 * taken from libfaim (gaim)
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
void OscarSocket::encodePasswordXOR(const QString &originalPassword, QString &encodedPassword)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	// TODO: check if latin1 is the right conversion
	const char *password = originalPassword.latin1();

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

	encodedPassword=QString::null;

// ========================================================================================


	for (i = 0; i < 8; i++)
	{
		if(password[i] == 0)
		{
			//kdDebug(14150) << k_funcinfo << "found \\0 in password @ " << i << endl;
			break;
		}
		encodedPassword.append(password[i] ^ encoding_table[i]);
	}


// ========================================================================================

/*
	// old way
	for (i = 0; i < strlen(password); i++)
	{
		encodedPassword.append(password[i] ^ encoding_table[i]);
	}
*/

// ========================================================================================
/*
	for (i = 0; i < 16; i++)
	{
		if(password[i] == 0)
		{
			kdDebug(14150) << k_funcinfo << "found \\0 in password @ " << i << endl;
			break;
		}

		char enc = (password[i] ^ encoding_table[i]);
		if (enc == 0)
		{
			kdDebug(14150) << k_funcinfo << "encoded char is NULL, escaping @ " << i << endl;
			encodedPassword.append("\\");
			enc = '0';
		}
		else if (enc == '\\')
		{
			kdDebug(14150) << k_funcinfo << "encoded char is \\, escaping @ " << i << endl;
			encodedPassword.append("\\");
        }
		encodedPassword.append(enc);
	}
*/
// ========================================================================================

#ifdef OSCAR_PWDEBUG
	kdDebug(14150) << " plaintext pw='" << originalPassword << "', length=" <<
		originalPassword.length() << endl;

	kdDebug(14150) << " encoded   pw='" << encodedPassword  << "', length=" <<
		encodedPassword.length() << endl;
#endif
}

void OscarSocket::sendLoginICQ()
{
	kdDebug(14150) << k_funcinfo << "Sending ICQ login info... (CLI_COOKIE)" << endl;

	Buffer outbuf;

	putFlapVer(outbuf); // FLAP Version
	outbuf.addTLV(0x0001, getSN().length(), getSN().latin1()); // login name

	QString encodedPassword;
	encodePasswordXOR(loginPassword, encodedPassword);

	outbuf.addTLV(0x0002, encodedPassword.length(), encodedPassword.latin1() );

	outbuf.addTLV(0x0003, strlen(ICQ_CLIENTSTRING), ICQ_CLIENTSTRING);
	outbuf.addTLV16(0x0016, ICQ_CLIENTID);
	outbuf.addTLV16(0x0017, ICQ_MAJOR);
	outbuf.addTLV16(0x0018, ICQ_MINOR);
	outbuf.addTLV16(0x0019, ICQ_POINT);
	outbuf.addTLV16(0x001a, ICQ_BUILD);
	outbuf.addTLV(0x0014, 0x0004, ICQ_OTHER); // distribution chan
	outbuf.addTLV(0x000f, 0x0002, ICQ_LANG);
	outbuf.addTLV(0x000e, 0x0002, ICQ_COUNTRY);

#ifdef OSCAR_PWDEBUG
	kdDebug(14150) << "CLI_COOKIE packet:" << endl << outbuf.toString() << endl;
#endif

	sendBuf(outbuf,0x01);
//	emit connectionChanged(3,"Sending username and password...");
}


// Parses all SNAC(0x15,3) Packets, these are only for ICQ!
void OscarSocket::parseSRV_FROMICQSRV(Buffer &inbuf)
{
	TLV tlv = inbuf.getTLV();
	if (tlv.type != 1)
	{
		kdDebug(14150) << k_funcinfo <<  "Bad SNAC(21,3), no TLV(1) found!" << endl;
		return;
	}

	Buffer fromicqsrv(tlv.data, tlv.length);
	WORD commandlength = fromicqsrv.getLEWord();
	DWORD ourUIN = fromicqsrv.getLEDWord();
	WORD subcmd = fromicqsrv.getLEWord();  // AKA 'data type' in the docs at iserverd1.khstu.ru
	WORD sequence = fromicqsrv.getLEWord();

	kdDebug(14150) << k_funcinfo <<
		"commandlength=" << commandlength <<
		", ourUIN='" << ourUIN << "', subcmd=" << subcmd <<
		", sequence=" << sequence << endl;

	switch(subcmd)
	{
		case 0x0042: //SRV_DONEOFFLINEMSGS
		{
			sendAckOfflineMessages();
			break;
		}

		case 0x0041: //SRV_OFFLINEMSG
		{
			OscarMessage oMsg;
			UserInfo u;

			u.sn = QString::number(fromicqsrv.getLEDWord());
			// !Can be NULL if contact is not in contactlist
			OscarContact *contact = static_cast<OscarContact*>(mAccount->contacts()[u.sn]);


			// BEGIN packet parsing
			WORD year = fromicqsrv.getLEWord();
			BYTE month = fromicqsrv.getLEByte();
			BYTE day = fromicqsrv.getLEByte();
			BYTE hour = fromicqsrv.getLEByte();
			BYTE minute = fromicqsrv.getLEByte();
			BYTE msgtype = fromicqsrv.getByte();
			BYTE msgflags = fromicqsrv.getByte();
			const char *msgString = fromicqsrv.getLELNTS(); // Get the message
			// END packet parsing

			QDate date;
			date.setYMD((int)year,(int)month,(int)day);
			QTime time;
			time.setHMS((int)hour, (int)minute, 0);
			oMsg.timestamp.setDate(date);
			oMsg.timestamp.setTime(time);
			oMsg.setText(ServerToQString(msgString, contact, false), OscarMessage::Plain);

			delete [] msgString;

			parseMessage(u, oMsg, msgtype, msgflags);
			break;
		}

		case 0x07da: //SRV_META
		{
			WORD type = fromicqsrv.getLEWord();
			BYTE result = fromicqsrv.getLEByte();

			switch(type)
			{
				case 1:
				{
		 			kdDebug(14150) << k_funcinfo << "RECV SRV_METAERROR, result=" << (int)result << endl;

					char *errorstring = fromicqsrv.getBlock(fromicqsrv.length());
					QString error = QString::fromLatin1(errorstring);
					delete [] errorstring;

					emit protocolError(i18n("An error occurred. Message was: %1").arg(error), (int)result);
					break;
				}
				case 0x01a4: // SRV_METAFOUND
				case 0x01ae: // SRV_METALAST
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAFOUND or SRV_METALAST)" << endl;

					char *tmptxt;
					ICQSearchResult searchResult;
					// codes taken from libicq, some kind of failure,
					// TODO: have to find out what they are for
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

					if (type==0x01ae) // Last search result
					{
						// MISSED
						// The number of users not returned that matched this search.
						DWORD missed = fromicqsrv.getLEDWord();
						emit gotSearchResult(searchResult,missed);
					}
					else
					{
						emit gotSearchResult(searchResult,-1);
					}
					break;
				} // END SRV_METAFOUND/SRV_METALAST

				case 160: // SRV_METASECURITYDONE
				{
					kdDebug(14150) << k_funcinfo <<
						"RECV (SRV_METASECURITYDONE), IGNORING, length=" <<
						fromicqsrv.length() << endl;
					break;
				}


				case 200: // SRV_METAGENERAL
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAGENERAL)" << endl;
					char *tmptxt;
					ICQGeneralUserInfo res;

					// FIXME: we need to know the oscarcontact this packet was received for!
					OscarContact *contact = 0L;

					tmptxt = fromicqsrv.getLELNTS();
					//kdDebug(14150) << k_funcinfo << "converting nickname string" << endl;
					res.nickName = ServerToQString(tmptxt, contact);
					//res.nickName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					//kdDebug(14150) << k_funcinfo << "converting firstname string" << endl;
					res.firstName = ServerToQString(tmptxt, contact);
					//res.firstName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					//kdDebug(14150) << k_funcinfo << "converting lastname string" << endl;
					res.lastName = ServerToQString(tmptxt, contact);
					//res.lastName = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					//kdDebug(14150) << k_funcinfo << "converting email string" << endl;
					res.eMail = ServerToQString(tmptxt, contact);
					//res.eMail = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.city = ServerToQString(tmptxt, contact);
					//res.city = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.state = ServerToQString(tmptxt, contact);
					//res.state = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.phoneNumber = ServerToQString(tmptxt, contact);
					//res.phoneNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.faxNumber = ServerToQString(tmptxt, contact);
					//res.faxNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.street = ServerToQString(tmptxt, contact);
					//res.street = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.cellularNumber = ServerToQString(tmptxt, contact);
					//res.cellularNumber = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					tmptxt = fromicqsrv.getLELNTS();
					res.zip = ServerToQString(tmptxt, contact);
					//res.zip = QString::fromLocal8Bit(tmptxt);
					delete [] tmptxt;

					res.countryCode = fromicqsrv.getLEWord();
					res.timezoneCode = fromicqsrv.getLEByte(); // UTC+(tzcode * 30min)
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
					if (unknown != 0)
					{
						kdDebug(14150) << k_funcinfo <<
							"unknown last word=" << unknown << endl;
					}

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
					ICQMailList mailList;
					kdDebug(14150) << k_funcinfo << "RECV (SRV_METAEMAILS)" << endl;
					BYTE numMails = fromicqsrv.getLEByte();
//					kdDebug(14150) << k_funcinfo << "numMails=" << numMails << endl;
					if (numMails > 0)
					{
						BYTE publishMail;
						char *tmptxt;
						QString mailAddress;

						for(unsigned int mailx=0; mailx <= numMails; mailx++)
						{
							publishMail = fromicqsrv.getLEByte();
							tmptxt = fromicqsrv.getLELNTS();
							mailAddress = QString::fromLocal8Bit(tmptxt); // TODO: encoding
							mailList.insert(mailAddress, (publishMail==0x01));
							delete [] tmptxt;

							kdDebug(14150) << k_funcinfo <<
								"mail address:" << mailAddress <<
								", publish=" << publishMail << endl;
						}
					}
					else
					{
						kdDebug(14150) << k_funcinfo <<
							"no mail addresses in SRV_METAEMAILS" << endl;
					}
					emit gotICQEmailUserInfo(sequence, mailList);
					break;
				}

				case 240:
				{
					kdDebug(14150) << k_funcinfo <<
						"RECV (SRV_METAINTEREST)" << endl;
					ICQInfoItemList interests = extractICQItemList( fromicqsrv );
					emit gotICQInfoItemList( sequence, interests );
					break;
				}

				case 250:
				{
					kdDebug(14150) << k_funcinfo <<
						"RECV (SRV_METABACKGROUND)" << endl;
					// Get past affiliations
					ICQInfoItemList past = extractICQItemList( fromicqsrv );
					// Now get current organization memberships
					ICQInfoItemList current = extractICQItemList( fromicqsrv );
					// Tell anything that's interested
					emit gotICQInfoItemList(sequence, current, past);
					break;
				}

				case 260:
				{
					kdDebug(14150) << k_funcinfo << "RECV (SRV_META_SHORT_USERINFO)"
						<< endl;
					char *tmptxt;
					ICQSearchResult res;
					kdDebug(14150) << k_funcinfo << "Success byte is " << result << endl;
					if ( result == 0x0A )
					{
						tmptxt = fromicqsrv.getLELNTS();
						kdDebug(14150) << k_funcinfo << "converting nickname string" << endl;
						res.nickName = QString::fromLocal8Bit(tmptxt);
						delete [] tmptxt;

						tmptxt = fromicqsrv.getLELNTS();
						kdDebug(14150) << k_funcinfo << "converting firstname string" << endl;
						res.firstName = QString::fromLocal8Bit(tmptxt);
						delete [] tmptxt;

						tmptxt = fromicqsrv.getLELNTS();
						kdDebug(14150) << k_funcinfo << "converting lastname string" << endl;
						res.lastName = QString::fromLocal8Bit(tmptxt);
						delete [] tmptxt;

						tmptxt = fromicqsrv.getLELNTS();
						kdDebug(14150) << k_funcinfo << "converting email string" << endl;
						res.eMail = QString::fromLocal8Bit(tmptxt);
						delete [] tmptxt;
					}

					emit gotICQShortInfo(sequence, res);
					break;
				}

				case 270:
				{
					kdDebug(14150) << k_funcinfo <<
						"TODO: SRV_META270 subtype=" << type << endl;
					break;
				}

				case 410: // SRV_METAINFO410 seems to be unused
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

	fromicqsrv.clear();
	delete [] tlv.data;
} // END OscarSocket::parseSRV_FROMICQSRV()


ICQInfoItemList OscarSocket::extractICQItemList(Buffer& theBuffer)
{
	ICQInfoItemList theList;

	if(theBuffer.length() == 0)
	{
		kdDebug(14150) << k_funcinfo << "Nothing in buffer!" << endl;
		return theList;
	}

	BYTE numItems = theBuffer.getLEByte(); //get the number of items to read
	//kdDebug(14150) << k_funcinfo << numItems << " items received." << endl;

	if(numItems > 0)
	{
		WORD topic;  // Identifies the topic of the interest
		char* tmptxt; // Description of the interest (raw)
		for(unsigned int i = 0; i < numItems; i++)
		{
			topic = theBuffer.getLEWord();
			tmptxt = theBuffer.getLELNTS();

			ICQInfoItem thisItem;
			thisItem.category = topic;
			thisItem.description = QString::fromLocal8Bit(tmptxt); // TODO: encoding
			theList.append(thisItem);
			delete [] tmptxt;
		}
	}
	return theList;
}

void OscarSocket::sendICQStatus(unsigned long status)
{
	if (!mIsICQ)
		return;

	kdDebug(14150) << k_funcinfo << "SEND (CLI_SETSTATUS)" << endl;

	if (status & ICQ_STATUS_SET_INVIS)
		sendChangeVisibility(0x03);
	else
		sendChangeVisibility(0x04);

	Buffer outbuf;
	outbuf.addSnac(0x0001,0x001e,0x0000,0x00000000);

	outbuf.addWord(0x0006);
	outbuf.addWord(0x0004);
	outbuf.addDWord(status);
	fillDirectInfo(outbuf);
	outbuf.addWord(0x0008); // TLV(8)
	outbuf.addWord(0x0002); // length 2
	outbuf.addWord(0x0000); // error code - 0
//	outbuf.print();

	sendBuf(outbuf, 0x02);
} // END OscarSocket::sendICQStatus


void OscarSocket::fillDirectInfo(Buffer &directInfo)
{
	directInfo.addWord(0x000C); // TLV(12)
	directInfo.addWord(0x0025); // length 25

#if 0
	if(mDirectIMMgr)
	{
		kdDebug(14150) << k_funcinfo <<
			"bindhost=" << mDirectIMMgr->socket()->bindHost() <<
			", bindPort=" << mDirectIMMgr->socket()->bindPort() << endl;

		kdDebug(14150) << k_funcinfo <<
			"host=" << mDirectIMMgr->socket()->host() <<
			", Port=" << mDirectIMMgr->socket()->port() << endl;

		directInfo.addDWord(setIPv4Address(mDirectIMMgr->socket()->host())); // IP
		directInfo.addWord(0x0000);
		directInfo.addWord(mDirectIMMgr->socket()->port().toUShort()); // Port
	}
	else
#endif
	{
		directInfo.addDWord(0x00000000);
		directInfo.addWord(0x0000);
		directInfo.addWord(0x0000);
	}

	directInfo.addByte(0x00); // Mode, TODO: currently fixed to "Direct Connection disabled"
	directInfo.addWord(ICQ_TCP_VERSION); // icq tcp protocol version, v8 currently

	//directInfo.addDWord(mDirectConnnectionCookie);
	directInfo.addDWord(0x00000000);
	directInfo.addDWord(0x00000050); // web front port
	directInfo.addDWord(0x00000003); // number of following client features
	directInfo.addDWord(0x00000000); // TODO: InfoUpdateTime
	directInfo.addDWord(0x00000000); // TODO: PhoneStatusTime
	directInfo.addDWord(0x00000000); // TODO: PhoneBookTime
	directInfo.addWord(0x0000);
}


void OscarSocket::sendKeepalive()
{
	//kdDebug(14150) << k_funcinfo << "SEND KEEPALIVE" << endl;
	Buffer outbuf;
	sendBuf(outbuf, 0x05);
}

void OscarSocket::startKeepalive()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
	if (keepaliveTime==0 || !mIsICQ) // nobody wants keepalive, so shut up ;)
		return;

	if (!keepaliveTimer)
	{
		//kdDebug(14150) << k_funcinfo << "Creating keepaliveTimer" << endl;
		keepaliveTimer = new QTimer(this, "keepaliveTimer");
		QObject::connect(keepaliveTimer, SIGNAL(timeout()),
			this, SLOT(slotKeepaliveTimer()));
		keepaliveTimer->start(keepaliveTime*1000);
	}
}

void OscarSocket::stopKeepalive()
{
//	kdDebug(14150) << k_funcinfo << "Called." << endl;
	if(keepaliveTimer && mIsICQ)
	{
		//kdDebug(14150) << k_funcinfo << "Deleting keepaliveTimer" << endl;
		delete keepaliveTimer;
		keepaliveTimer = 0L;
	}
}

void OscarSocket::slotKeepaliveTimer()
{
	sendKeepalive();
}

void OscarSocket::parseAdvanceMessage(Buffer &messageBuf, UserInfo &user, Buffer &ack)
{
	WORD ackStatus = P2P_ONLINE;
	WORD ackFlags = 0x0000;
	QString ackMessage("");
	bool sendAck = true;

	kdDebug(14150) << k_funcinfo << "RECV TYPE-2 message" << endl;

	//kdDebug(14150) << "packet:" << endl << messageBuf.toString() << endl;

	if(mAccount->myself()->onlineStatus().internalStatus() == OSCAR_NA)
		ackStatus = P2P_NA;
	else if(mAccount->myself()->onlineStatus().internalStatus() == OSCAR_AWAY)
		ackStatus  = P2P_AWAY;
	else
		ackStatus = P2P_ONLINE;

	WORD unk = messageBuf.getLEWord(); // unknown
	ack.addLEWord(unk);

	WORD seq2 = messageBuf.getLEWord(); // some stupid sequence
	ack.addLEWord(seq2);

	char *unkblock = messageBuf.getBlock(12); // unknown, always zero
	ack.addString(unkblock,12);
	delete [] unkblock;

	BYTE msgType = messageBuf.getByte(); // message type
	ack.addByte(msgType); // add to CLI_ACKMSG
	BYTE msgFlags = messageBuf.getByte(); // message flags
	ack.addByte(msgFlags); // add to CLI_ACKMSG

	WORD status = messageBuf.getLEWord();
	WORD priority = messageBuf.getLEWord();

	kdDebug(14150) << k_funcinfo <<
		"msgType=" << (int)msgType <<
		", msgFlags=" << (int)msgFlags <<
		", status=" << (int)status <<
		", priority=" << (int)priority << endl;

	const char *messagetext = messageBuf.getLNTS();

	switch(msgType)
	{
		case MSG_GET_AWAY:
		case MSG_GET_OCC:
		case MSG_GET_NA:
		case MSG_GET_DND:
		case MSG_GET_FFC:
		{
			kdDebug(14150) << k_funcinfo << "RECV TYPE-2 IM, away message request" << endl;
			ackMessage = mAccount->awayMessage();
			if (ackMessage.isNull())
				ackMessage="";
			break;
		}

		case MSG_AUTO:
		case MSG_NORM:
		case MSG_URL:
		{
			kdDebug(14150) << k_funcinfo << "RECV TYPE-2 IM, normal/auto message" << endl;
			OscarMessage oMsg;

			BYTE r = messageBuf.getLEByte();
			BYTE g = messageBuf.getLEByte();
			BYTE b = messageBuf.getLEByte();
			/*BYTE n =*/ messageBuf.getLEByte();
			/*kdDebug(14150) << k_funcinfo << "fg color=(" << (int)r << ", " << (int)g << ", " << (int)b <<
				", " << (int)n << ")" << endl;*/
			oMsg.fgColor.setRgb((int)r, (int)g, (int)b);

			r = messageBuf.getLEByte();
			g = messageBuf.getLEByte();
			b = messageBuf.getLEByte();
			/*n =*/ messageBuf.getLEByte();
			/*kdDebug(14150) << k_funcinfo << "bg color=(" << (int)r << ", " << (int)g << ", " << (int)b <<
				", " << (int)n << ")" << endl;*/
			oMsg.bgColor.setRgb((int)r, (int)g, (int)b);

			bool utf = false;
			bool rtf = false;
			if(messageBuf.length() > 0)
			{
				DWORD guidlen = messageBuf.getLEDWord();
				char *guid = messageBuf.getBlock(guidlen);
				if(QString::fromLatin1(guid) == QString::fromLatin1("{0946134E-4C7F-11D1-8222-444553540000}"))
				{
					//kdDebug(14150) << k_funcinfo << "Peer announces message is UTF!" << endl;
					utf = true;
				}
				else if (QString::fromLatin1(guid) == QString::fromLatin1("{97B12751-243C-4334-AD22-D6ABF73F1492}"))
				{
					rtf = true;
				}
				else
				{
					kdDebug(14150) << k_funcinfo <<
					"TYPE-2 guid (neither UTF nor RTF GUID!) = '" << guid << "'" << endl;
				}
				delete [] guid;
			}

			//kdDebug(14150) << k_funcinfo << "messagetext='" << messagetext << "'" << endl;

			OscarContact *contact = static_cast<OscarContact*>(mAccount->contacts()[tocNormalize(user.sn)]);

			QString msgText = ServerToQString(messagetext, contact, utf, rtf);
			if (rtf /*msgText.startsWith("{\\rtf")*/)
				oMsg.setText(msgText, OscarMessage::Rtf);
			else
				oMsg.setText(msgText, OscarMessage::Plain);

			if(!oMsg.text().isEmpty())
				parseMessage(user, oMsg, msgType, msgFlags);

			kdDebug(14150) << k_funcinfo <<
				"SEND ACKMSG, status=" << ackStatus <<
				", flags=" << ackFlags <<
				", message='" << ackMessage << "'" << endl;
			ack.addLEWord(ackStatus);
			ack.addLEWord(ackFlags);
			ack.addLNTS(ackMessage.latin1()); // TODO: encoding

			if(msgType==0x0001)
			{
				ack.addLEDWord(0x00000000);
				ack.addLEDWord(0xFFFFFF00);
			}
			sendBuf(ack, 0x02); // send back ack
			sendAck = false; // already sent
			break;
		}

		default: // something else we don't support yet
		{
			ackStatus = P2P_REFUSE;
			ackFlags = 0x0001;
		}
	} // END switch(msgType)

	delete [] messagetext;

	if(sendAck)
	{
		kdDebug(14150) << k_funcinfo <<
			"SEND ACKMSG, status=" << ackStatus <<
			",flags=" << ackFlags <<
			", message='" << ackMessage << "'" << endl;

		ack.addLEWord(ackStatus);
		ack.addLEWord(ackFlags);
		ack.addLNTS(ackMessage.latin1()); // TODO: encoding

		/*kdDebug(14150) << k_funcinfo <<
		"ACK dump:" << endl <<
		"------------------------------------------------------" << endl <<
		ack.toString() << endl <<
		"------------------------------------------------------" << endl;*/
		sendBuf(ack, 0x02); // send back ack
	}

	kdDebug(14150) << k_funcinfo << "END" << endl;
}

void OscarSocket::requestAwayMessage(OscarContact *c)
{
	if(!c)
		return;

	kdDebug(14150) << k_funcinfo << "Called for '" << c->displayName() << "'" << endl;

	DWORD receiverStatus = c->userInfo().icqextstatus;
	WORD type = (MSG_FLAG_GETAUTO << 8);

	if (receiverStatus == ICQ_STATUS_OFFLINE)		return;
	else if (receiverStatus & ICQ_STATUS_IS_DND)	type |= MSG_GET_DND;
	else if (receiverStatus & ICQ_STATUS_IS_OCC)	type |= MSG_GET_OCC;
	else if (receiverStatus & ICQ_STATUS_IS_NA)		type |= MSG_GET_NA;
	else if (receiverStatus & ICQ_STATUS_IS_AWAY)	type |= MSG_GET_AWAY;
	else if (receiverStatus & ICQ_STATUS_IS_FFC)	type |= MSG_GET_FFC;

	if(!sendType2IM(c, "", type))
		emit receivedAwayMessage(c->contactName(), i18n("Client does not support away messages"));
}

bool OscarSocket::sendType2IM(OscarContact *c, const QString &text, WORD type)
{
	if(!c)
		return false;

	if(!c->hasCap(CAP_ICQSERVERRELAY))
	{
		kdDebug(14150) << k_funcinfo <<
			"Contact '" << c->displayName() <<
			"' does not support type-2 messages" << endl <<
			"caps are: " << c->userInfo().capabilities << endl;
		return false;
	}

	kdDebug(14150) << k_funcinfo << "Called for '" << c->displayName() <<
		"', text='" << text << "' type=" << type << endl;

	WORD status = static_cast<OscarContact *>(mAccount->myself())->userInfo().icqextstatus;
	WORD flags = 0;

	if (status == (WORD)ICQ_STATUS_OFFLINE)		/* no change */;
	else if (status & ICQ_STATUS_IS_DND)	status = ICQ_STATUS_IS_DND  | (status & ICQ_STATUS_IS_INVIS);
	else if (status & ICQ_STATUS_IS_OCC)	status = ICQ_STATUS_IS_OCC  | (status & ICQ_STATUS_IS_INVIS);
	else if (status & ICQ_STATUS_IS_NA)		status = ICQ_STATUS_IS_NA   | (status & ICQ_STATUS_IS_INVIS);
	else if (status & ICQ_STATUS_IS_AWAY)	status = ICQ_STATUS_IS_AWAY | (status & ICQ_STATUS_IS_INVIS);
	else if (status & ICQ_STATUS_IS_FFC)	status = ICQ_STATUS_IS_FFC  | (status & ICQ_STATUS_IS_INVIS);
	else
		status &= ICQ_STATUS_IS_INVIS;

	DWORD receiverStatus = c->userInfo().icqextstatus;
	if (receiverStatus == ICQ_STATUS_OFFLINE)		/* no change */;
	else if (receiverStatus & ICQ_STATUS_IS_DND)	flags = MSG_FLAG_CONTACTLIST;
	else if (receiverStatus & ICQ_STATUS_IS_OCC)	flags = MSG_FLAG_CONTACTLIST;
	else if (receiverStatus & ICQ_STATUS_IS_NA)		flags = MSG_FLAG_UNKNOWN;
	else if (receiverStatus & ICQ_STATUS_IS_AWAY)	flags = MSG_FLAG_UNKNOWN;
	else if (receiverStatus & ICQ_STATUS_IS_FFC)	flags = MSG_FLAG_LIST | MSG_FLAG_UNKNOWN;
	else											flags = MSG_FLAG_LIST | MSG_FLAG_UNKNOWN;

	DWORD time = rand() % 0xFFFF;
	DWORD id = rand() % 0xFFFF;

	type2SequenceNum--;

	kdDebug(14150) << k_funcinfo << "SEND (CLI_SENDMSG) type-2, text='" << text <<
		"' to " << c->displayName() << "(" << c->contactName() << ")" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_4,0x0006,0x0000,toicqsrv_seq);
	toicqsrv_seq++;

	outbuf.addDWord(time); // TIME
	outbuf.addDWord(id); // ID

	outbuf.addWord(MSGFORMAT_ADVANCED); // type-2 message

	outbuf.addBUIN(c->contactName().latin1()); //dest sn
	//outbuf.addString(c->contactName().latin1(), c->contactName().length()); //dest sn

	Buffer tlv5;
	{ // TLV(5)
		tlv5.addWord(0x0000); // acktype, 0 = normal message
		tlv5.addDWord(time); // time
		tlv5.addDWord(id); // random message id
		tlv5.addDWord(0x09461349); // CAP SERVERRELAY
		tlv5.addDWord(0x4C7F11D1); // CAP SERVERRELAY
		tlv5.addDWord(0x82224445); // CAP SERVERRELAY
		tlv5.addDWord(0x53540000); // CAP SERVERRELAY

		tlv5.addWord(0x000A); // TLV(10)
		tlv5.addWord(0x0002); // len
		tlv5.addWord(0x0001); // data, acktype2, 1 = normal message

		tlv5.addWord(0x000F); // TLV(15), always empty
		tlv5.addWord(0x0000); // len

		Buffer tlv10001;
		{ // TLV(10001)
			tlv10001.addLEWord(0x001B); // length
			tlv10001.addLEWord(ICQ_TCP_VERSION);
			tlv10001.addDWord(0x00000000); // CAP
			tlv10001.addDWord(0x00000000); // CAP
			tlv10001.addDWord(0x00000000); // CAP
			tlv10001.addDWord(0x00000000); // CAP
			tlv10001.addWord(0x0000);
			tlv10001.addByte(0x03); // unknown
			tlv10001.addDWord(0x00000000); // unknown -> 0 = normal message
			tlv10001.addWord(type2SequenceNum);
			tlv10001.addLEWord(0x00E);
			tlv10001.addWord(type2SequenceNum);

			tlv10001.addDWord(0x00000000); // 12 unknown bytes, always zero
			tlv10001.addDWord(0x00000000);
			tlv10001.addDWord(0x00000000);

			tlv10001.addLEWord(type); // message type
			tlv10001.addWord(status); // own status + a bit of bit-fscking
			tlv10001.addWord(flags); // message flags/priority

			const char *str = text.latin1(); // TODO: encoding

			int len = strlen(str);
			tlv10001.addLEWord(len+1);
			tlv10001.addString(str, len);
			tlv10001.addByte(0x00);

			if(type == MSG_NORM)
			{
				tlv10001.addDWord(0x00000000); // fg color
				tlv10001.addDWord(0xFFFFFF00); // bg color
			}
			//kdDebug(14150) << "tlv10001 :" << endl << tlv10001.toString() << endl;
		} // END TLV(10001)

		tlv5.addTLV(0x2711, tlv10001.length(), tlv10001.buffer());
	} // END TLV(5)

	outbuf.addTLV(0x0005, tlv5.length(), tlv5.buffer());
	outbuf.addDWord(0x00030000); // empty TLV(3)
	//kdDebug(14150) << "CLI_SENDMSG packet:" << endl << outbuf.toString() << endl;
	sendBuf(outbuf, 0x02);

	return true;
}



WORD OscarSocket::sendCLI_TOICQSRV(const WORD subcommand, Buffer &data)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_TOICQSRV), subcommand=" << subcommand << endl;

	Buffer outbuf;
	DWORD word1 = 0x0000; // TODO: is this really right?
	DWORD word2 = (toicqsrv_seq);
	DWORD snacid = (word1 << 16) | word2;

	outbuf.addSnac(OSCAR_FAM_21,0x0002,0x0000,snacid);
	// yum yum, one up sequence, starts at 1 and has to be 2 for the first
	// usage on the LEWord added some lines under this comment ;)
	toicqsrv_seq++;

	int tlvLen = 10 + data.length();

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

	// TODO: make this the snac sequence's upper Word minus 1!
	outbuf.addLEWord(toicqsrv_seq);

	if (data.length() > 0)
		outbuf.addString(data.buffer(), data.length());

	sendBuf(outbuf, 0x02);
	return (toicqsrv_seq);
}

void OscarSocket::sendCLI_SEARCHBYUIN(const unsigned long uin)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND CLI_SEARCHBYUIN (CLI_META), uin=" << uin << endl;
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
	int lang,
	const QString &city,
	const QString state,
	int country,
	const QString &company,
	const QString &department,
	const QString &position,
	int occupation,
	bool onlineOnly) /* TODO: add all fields or make this somehow clever */
{
	kdDebug(14150) << k_funcinfo << "SEND CLI_SEARCHWP (CLI_META)" << endl;

	Buffer search;
	// NOTE: EVERYTHING IN THIS BUFFER IS LITTLE-ENDIAN
	search.addLEWord(0x0533); // subtype: 1331

	//LNTS FIRST
	search.addLEWord(first.length());
	if(first.length()>0)
		search.addLEString(first.latin1(), first.length());

	// LNTS LAST
	search.addLEWord(last.length());
	if(last.length()>0)
		search.addLEString(last.latin1(), last.length());

	// LNTS NICK
	search.addLEWord(nick.length());
	if(nick.length()>0)
		search.addLEString(nick.latin1(), nick.length());

	// LNTS EMAIL
	search.addLEWord(mail.length());
	if(mail.length()>0)
		search.addLEString(mail.latin1(), mail.length());

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
		search.addLEString(city.latin1(), city.length());

	// LNTS STATE
	search.addLEWord(state.length());
	if(state.length()>0)
		search.addLEString(state.latin1(), state.length());

	// WORD.L xx xx COUNTRY
	search.addLEWord(country);

	// LNTS COMPANY
	search.addLEWord(company.length());
	if(company.length()>0)
		search.addLEString(company.latin1(), company.length());

	// LNTS DEPARTMENT
	search.addLEWord(department.length());
	if(department.length()>0)
		search.addLEString(department.latin1(), department.length());

	// LNTS POSITION
	search.addLEWord(position.length());
	if(position.length()>0)
		search.addLEString(position.latin1(), position.length());

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
	if (!mIsICQ) // NOT ON AIM
		return;

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

WORD OscarSocket::sendShortInfoReq(const unsigned long uin)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_META_SHORT_USERINFO), requesting short user info" << endl;
	Buffer req;
	req.addLEWord(0x04ba); //subtype: 1210
	req.addLEDWord(uin);
	WORD ret = sendCLI_TOICQSRV(0x07d0, req);
	return ret;
}

void OscarSocket::sendCLI_METASETGENERAL(const ICQGeneralUserInfo &i)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_METASETGENERAL), sending general user information" << endl;

	Buffer req; // ! LITTLE-ENDIAN
	req.addLEWord(0x03ea); // subtype: 1002
	req.addLELNTS(i.nickName.latin1()); // TODO: check encoding
	req.addLELNTS(i.firstName.latin1());
	req.addLELNTS(i.lastName.latin1());
	req.addLELNTS(i.eMail.latin1());
	req.addLELNTS(i.city.latin1());
	req.addLELNTS(i.state.latin1());
	req.addLELNTS(i.phoneNumber.latin1());
	req.addLELNTS(i.faxNumber.latin1());
	req.addLELNTS(i.street.latin1());
	req.addLELNTS(i.cellularNumber.latin1());
	req.addLELNTS(i.zip.latin1());
	req.addLEWord(i.countryCode);
	req.addLEByte(i.timezoneCode);
	req.addLEByte(i.publishEmail ? 0x00 : 0x01);

	sendCLI_TOICQSRV(0x07d0, req);
}


void OscarSocket::sendCLI_METASETWORK(const ICQWorkUserInfo &i)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_METASETWORK), sending work user information" << endl;

	Buffer req; // Little Endian
	req.addLEWord(0x03f3); //1011
	req.addLELNTS(i.city.latin1());
	req.addLELNTS(i.state.latin1());
	req.addLELNTS(i.phone.latin1());
	req.addLELNTS(i.fax.latin1());
	req.addLELNTS(i.address.latin1());
	req.addLELNTS(i.zip.latin1());
	req.addLEWord(i.countryCode);
	req.addLELNTS(i.company.latin1());
	req.addLELNTS(i.department.latin1());
	req.addLELNTS(i.position.latin1());
	// The work ZIP code (for some reason duplicated)
	req.addLELNTS(i.zip.latin1());
	req.addLEWord(i.occupation);
	req.addLELNTS(i.homepage.latin1());

	sendCLI_TOICQSRV(0x07d0, req);
}


void OscarSocket::sendCLI_METASETMORE(const ICQMoreUserInfo &i)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_METASETMORE)" <<
		", sending more user information (age, bday, lang)" << endl;

	Buffer req; // Little Endian
	req.addLEWord(0x03fd); //1021
	req.addLEWord(i.age); // WORD
	req.addLEByte(i.gender); // BYTE
	req.addLELNTS(i.homepage.latin1()); // LNTS
	req.addLEWord(i.birthday.year()); // WORD
	req.addLEByte(i.birthday.month()); // BYTE
	req.addLEByte(i.birthday.day()); // BYTE
	req.addLEByte(i.lang1); // BYTE
	req.addLEByte(i.lang2); // BYTE
	req.addLEByte(i.lang3); // BYTE

	sendCLI_TOICQSRV(0x07d0, req);
}


void OscarSocket::sendCLI_METASETSECURITY(bool requireauth, bool webaware,
	BYTE direct)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_METASETSECURITY), sending security user information" << endl;

	Buffer req; // ! LITTLE-ENDIAN
	req.addLEWord(0x0424); // subtype: 1060
	req.addLEByte(requireauth ? 0x01 : 0x00);
	req.addLEByte(webaware ? 0x00 : 0x01);
	req.addLEByte(direct);
	req.addLEByte(0x00); // KIND: user kind, unknown

	sendCLI_TOICQSRV(0x07d0, req);
}

void OscarSocket::sendCLI_SENDSMS(const QString &phonenumber,
	const QString &message, const QString &senderUIN, const QString &senderNick)
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_SENDSMS), sending SMS to '" << phonenumber <<
		"', message=" << message << endl;

	QTextCodec *codec = QTextCodec::codecForMib(2252);
	if(!codec)
		return;

	QString time = QDateTime::currentDateTime(Qt::UTC).
		toString("dddd, dd MMM yyyy hh::mm:ss GMT");

	QCString xml = "<icq_sms_message><destination>";
		xml += phonenumber.latin1();
		xml += "</destination>";
		xml += "<text>" + message.utf8() + "</text>";
		xml += "<codepage>1252</codepage><encoding>utf8</encoding>";
		xml += "<senders_UIN>" + senderUIN.utf8() + "</senders_UIN><senders_name>";
		xml += senderNick.utf8();
		xml += "</senders_name>";
		xml += "<delivery_receipt>Yes</delivery_receipt><time>" + time.utf8() + "</time>";
		xml += "</icq_sms_message>";

	Buffer req; // ! LITTLE-ENDIAN
	req.addLEWord(5250); // subtype: 5250
	req.addWord(0x0001);
	req.addWord(0x0016);
	req.addDWord(0x00000000);
	req.addDWord(0x00000000);
	req.addDWord(0x00000000);
	req.addDWord(0x00000000);
	req.addWord(0x0000); // TLV(0)
	req.addLEWord(xml.length()+1);
	req.addLEString(xml, xml.length()+1);

	sendCLI_TOICQSRV(2000, req);
}


// vim: set noet ts=4 sts=4 sw=4:
