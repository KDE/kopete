/*
  oscarsocket.cpp  -  Oscar Protocol Implementation

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
#include "rateclass.h"
#include "oscardebug.h"

#include <stdlib.h>
#include <netinet/in.h> // for htonl()

#include "oscaraccount.h"

#include <qobject.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kdebug.h>
//#include <kextsock.h>

// ---------------------------------------------------------------------------------------

OscarSocket::OscarSocket(const QString &connName, const QByteArray &cookie,
	OscarAccount *account, QObject *parent, const char *name, bool isicq)
	: OscarConnection(connName, Server, cookie, parent, name)
{
//	kdDebug(14150) << k_funcinfo << "connName=" << connName <<
//		QString::fromLatin1( isicq?" ICQ":" AIM" ) << endl;

	mIsICQ=isicq;
	toicqsrv_seq=1;
	type2SequenceNum=0xFFFF;
	flapSequenceNum=rand() & 0x7FFF; // value taken from libicq
	mPwEncryptionKey=0L;
	mCookie=0L;
	loginStatus=0;
	gotAllRights=0;
	keepaliveTime=30;
	keepaliveTimer=0L;
	rateClasses.setAutoDelete(TRUE);
	isLoggedIn=false;
	idle=false;
	//mDirectConnnectionCookie=rand();
	mAccount=account;
	//mDirectIMMgr=0L;
	//mFileTransferMgr=0L;
	//bSomethingOutgoing=false;

	connect(this, SIGNAL(socketClosed(const QString &, bool)),
		this, SLOT(slotConnectionClosed(const QString &, bool)));
	connect(this, SIGNAL(moreToRead()),
		this, SLOT(slotRead()));
	/*
	connect(this, SIGNAL(serverReady()), this, SLOT(OnServerReady()));
	*/
}

OscarSocket::~OscarSocket()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;

	if(socketStatus() == OscarConnection::Connecting ||
		socketStatus() == OscarConnection::Connected )
	{
		kdDebug(14150) << k_funcinfo <<
			"ERROR: WE ARE NOT DISCONNECTED YET" << endl;

		stopKeepalive();

		disconnect(mSocket, 0, 0, 0);
		mSocket->reset();
	}

	//delete mDirectIMMgr;
	//delete mFileTransferMgr;
	delete[] mCookie;
	delete[] mPwEncryptionKey;

	for (RateClass *tmp=rateClasses.first(); tmp; tmp=rateClasses.next())
		disconnect(tmp, SIGNAL(dataReady(Buffer &)), this, SLOT(writeData(Buffer &)));
	rateClasses.clear();
}

DWORD OscarSocket::setIPv4Address(const QString &address)
{
	kdDebug(14150) << "Setting IP address to: " << address << endl;
	QString a=address.simplifyWhiteSpace();

	QStringList ipv4Addr=QStringList::split(".", a, FALSE);
	if (ipv4Addr.count() == 4)
	{
		unsigned long newAddr=0;
		int i=0;
		bool ok=true;
		while (ok && i < 4)
		{
			unsigned long value=ipv4Addr[i].toUInt(&ok);
			if (value > 255)
				ok=false;
			if (ok)
				newAddr=newAddr * 256 + value;
			i++;
		}
		if (ok)
			return newAddr;
	}
	return 0;
}

SSIData& OscarSocket::ssiData()
{
	return mSSIData;
}

void OscarSocket::slotConnected()
{
	kdDebug(14150) << k_funcinfo <<
		"Connected to '" << peerHost() <<
		"', port '" << peerPort() << "'" << endl;

#if 0
	QString h=mSocket->localAddress()->nodeName();
	mDirectIMMgr=new OncomingSocket(this, h, DirectIM);
	mFileTransferMgr=new OncomingSocket(this, h, SendFile, SENDFILE_PORT);
#endif
	//emit connectionChanged(1, QString("Connected to %2, port %1").arg(peerPort()).arg(peerName()));
}

void OscarSocket::slotConnectionClosed(const QString &connName, bool expected)
{
	kdDebug(14150) << k_funcinfo << "Connection for account '" <<
		mAccount->accountId() << "' closed, expected = " << expected << endl;

	if(mSocket->bytesAvailable() > 0)
	{
		kdDebug(14150) << k_funcinfo <<
			mSocket->bytesAvailable() <<
			" bytes were left to read, maybe connection broke down?" << endl;
	}

	stopKeepalive();
	rateClasses.clear();
	loginStatus=0;
	idle=false;
	gotAllRights=0;
	isLoggedIn=false;

	disconnect(this, SIGNAL(connAckReceived()), 0, 0);
	disconnect(this, SIGNAL(socketConnected(const QString &)), 0, 0);

	mSocket->reset();

	/*if(mDirectIMMgr)
	{
		delete mDirectIMMgr;
		mDirectIMMgr=0L;
	}

	if(mFileTransferMgr)
	{
		delete mFileTransferMgr;
		mFileTransferMgr=0L;
	}*/

	//kdDebug(14150) << k_funcinfo << "emitting statusChanged(OSCAR_OFFLINE)" << endl;
	emit statusChanged(OSCAR_OFFLINE);

	if (!expected)
		mAccount->disconnect(KopeteAccount::ConnectionReset);
}


void OscarSocket::slotRead()
{
	//kdDebug(14150) << k_funcinfo << "-----------------------" << endl;

	//int waitCount=0;
	char *buf=0L;
	Buffer inbuf;
	FLAP fl;
	int bytesread=0;

	/*kdDebug(14150) << k_funcinfo <<
		mSocket->bytesAvailable() << " bytes to read" << endl;*/

	// a FLAP is min 6 bytes, we cannot read more out of
	// a buffer without these 6 initial bytes

	/*
	if(mSocket->bytesAvailable() < 6)
	{
		kdDebug(14150) << k_funcinfo <<
		"less than 6 bytes available, waiting for m data" << endl;
		mSocket->waitForMore(200);
		return;
	}
	*/

	fl = getFLAP();

	if (fl.error || fl.length == 0)
	{
		kdDebug(14150) << k_funcinfo << "Could not read full FLAP, aborting" << endl;
		return;
	}

	buf = new char[fl.length];

	bytesread = mSocket->readBlock(buf, fl.length);
	if(bytesread != fl.length)
	{
		kdDebug(14150) << k_funcinfo << "WARNING, couldn't read all of" \
			" that packet, this will probably break things!!!" << endl;
	}

	inbuf.setBuf(buf, bytesread);

#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << "=== INPUT ===" << inbuf.toString();
#endif

	//kdDebug(14150) << "FLAP channel is " << fl.channel << endl;

	switch(fl.channel)
	{
		case 0x01: //new connection negotiation channel
		{
			DWORD flapversion=inbuf.getDWord();
			if (flapversion == 0x00000001)
			{
				emit connAckReceived();
			}
			else
			{
				kdDebug(14150) << k_funcinfo <<
					"!Could not read FLAP version on channel 0x01" << endl;
				break;
			}
			break;
		} // END 0x01

		case 0x02: //snac data channel
		{
			SNAC s;
			s=inbuf.getSnacHeader();

			if (s.family != 3 && s.subtype != 11) // avoid the spam of all the online notifies
			{
				kdDebug(14150) << k_funcinfo << "SNAC(" << s.family << "," <<
					s.subtype << "), id=" << s.id << endl;
			}

			switch(s.family)
			{
				case OSCAR_FAM_1: // Service Controls
				{
					switch(s.subtype)
					{
						case 0x0001:
							kdDebug(14150) << k_funcinfo << "SNAC Family 1 error" << endl;
							parseError(s.family, s.id, inbuf);
							break;
						case 0x0003: //server ready
							parseServerReady(inbuf);
							break;
						case 0x0005: //redirect
							kdDebug(14150) << k_funcinfo <<
								"TODO: Parse redirect! ================" << endl;
							//parseRedirect(inbuf);
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
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" <<
								s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END OSCAR_FAM_1

				case OSCAR_FAM_2: // Location service
				{
					switch(s.subtype)
					{
						case 0x0001:
							kdDebug(14150) << k_funcinfo << "SNAC Family 2 error" << endl;
							parseError(s.family, s.id, inbuf);
							break;
						case 0x0003: //locate rights
							parseLocateRights(inbuf);
							break;
						case 0x0006: //user info (AIM)
							parseUserLocationInfo(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" <<
								s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0002

				case OSCAR_FAM_3: // Contact services
				{
					switch(s.subtype)
					{
						case 0x0001: // contact list error (SRV_CONTACTERR)
							kdDebug(14150) << k_funcinfo << "SNAC Family 3 error" << endl;
							kdDebug(14150) << k_funcinfo <<
								"RECV SRV_CONTACTERR, UNHANDLED!!!" << endl;
							break;
						case 0x0003: //buddy list rights
							parseBuddyRights(inbuf);
							break;
						case 0x000a: // server refused adding contact to list (SRV_REFUSED)
							kdDebug(14150) << k_funcinfo <<
								"RECV SRV_REFUSED, UNHANDLED!!!" << endl;
							break;
						case 0x000b: //contact changed status, (SRV_USERONLINE)
							parseUserOnline(inbuf);
							break;
						case 0x000c: //contact went offline
							parseUserOffline(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC(" <<
								s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END 0x0003

				case OSCAR_FAM_4: //msg services
				{
					switch(s.subtype)
					{
						case 0x0001: //msg error
							kdDebug(14150) << k_funcinfo << "SNAC Family 4 (ICBM) error" << endl;
							parseError(s.family, s.id, inbuf);
							break;
						case 0x0005: //msg rights SNAC(4,5)
							parseMsgRights(inbuf);
							break;
						case 0x0007: //incoming IM SNAC(4,7)
							parseIM(inbuf);
							break;
						case 0x000a: //missed messages SNAC(4,10)
							parseMissedMessage(inbuf);
							break;
						case 0x000b: // message ack SNAC(4,11)
							parseMsgAck(inbuf);
							break;
						case 0x000c: // server ack for type-2 message SNAC(4,12)
							parseSrvMsgAck(inbuf);
							break;
						case 0x0014: // Mini-Typing notification
							parseMiniTypeNotify(inbuf);
							break;
						default: //invalid subtype
							kdDebug(14150) << k_funcinfo << "Unknown SNAC("
								<< s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END OSCAR_FAM_4

				case OSCAR_FAM_9: // BOS
				{
					switch(s.subtype)
					{
						case 0x0001: //BOS error
							kdDebug(14150) << k_funcinfo << "SNAC Family 9 (BOS) error" << endl;
							parseError(s.family, s.id, inbuf);
							break;
						case 0x0003: //bos rights incoming
							parseBOSRights(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC("
								<< s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END OSCAR_FAM_9

				case OSCAR_FAM_19: // Contact list management (SSI)
				{
					switch(s.subtype)
					{
						case 0x0003: //ssi rights
							parseSSIRights(inbuf);
							break;
						case 0x0006: //buddy list
							parseSSIData(inbuf);
							break;
						case 0x000e: //server ack for adding/changing roster items
							parseSSIAck(inbuf, s.id);
							break;
						case 0x000f: //ack when contactlist timestamp/length matches those values sent
							parseSSIOk(inbuf);
							break;
						case 0x001b: // auth reply
							parseAuthReply(inbuf);
							break;
						case 0x001c: // "added by" message
							kdDebug(14150) << k_funcinfo << "IGNORE 'added by' message" << endl;
							break;
						default: //invalid subtype
							kdDebug(14150) << k_funcinfo << "Unknown SNAC("
								<< s.family << ",|" << s.subtype << "|)" << endl;
					};
					break;
				} // END OSCAR_FAM_19

				case OSCAR_FAM_21: // ICQ 0x15 packets
				{
					switch(s.subtype)
					{
						case 0x0001:
							parseError(s.family, s.id, inbuf);
							break;
						case 0x0003:
							parseSRV_FROMICQSRV(inbuf);
							break;
						default:
							kdDebug(14150) << k_funcinfo << "Unknown SNAC("
								<< s.family << ",|" << s.subtype << "|)" << endl;
					}
					break;
				} // END OSCAR_FAM_21

				case OSCAR_FAM_23: //authorization family, TODO: also for icq registration
				{
					switch(s.subtype)
					{
						case 0x0001: //registration refused!
							emit protocolError(i18n("Registration refused. Check your user name and password and try again"), 0);
							break;
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
				} // END OSCAR_FAM_23

				default:
					kdDebug(14150) << k_funcinfo << "Unknown SNAC(|" << s.family << "|," << s.subtype << ")" << endl;

			}; // END switch (s.family)
			break;
		} // END channel 0x02

		case 0x03: //FLAP error channel
		{
			kdDebug(14150) << "FLAP error channel, UNHANDLED" << endl;
			break;
		} // END channel 0x03

		case 0x04: //close connection negotiation channel
		{
			parseConnectionClosed(inbuf);
			break;
		}

		case 0x05:
		{
			kdDebug(14150) << k_funcinfo << "RECV KEEPALIVE" << endl;
			break;
		}

		default: //oh, crap, something is really wrong
		{
			kdDebug(14150) << k_funcinfo << "Unknown channel " << fl.channel << endl;
		}

	} // END switch(fl.channel)

	inbuf.clear(); // separate buf from inbuf again
	delete [] buf;

	// another flap is waiting in read buffer
	if (mSocket->bytesAvailable() > 0)
		QTimer::singleShot(0, this, SLOT(slotRead()));
}


void OscarSocket::sendLoginRequest()
{
	kdDebug(14150) << k_funcinfo << "Called" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_23,0x0006,0x0000,0x00000000);
	outbuf.addTLV(0x0001, getSN().length(), getSN().latin1());
	sendBuf(outbuf,0x02);
//	emit connectionChanged(2,QString("Requesting login for " + getSN() + "..."));
}

void OscarSocket::putFlapVer(Buffer &outbuf)
{
	outbuf.addDWord(0x00000001);
}

void OscarSocket::OnConnAckReceived()
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	if(mIsICQ)
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

void OscarSocket::sendBuf(Buffer &outbuf, BYTE chan)
{

	//For now, we use 0 as the sequence number because rate
	//limiting can cause packets from different classes to be
	//sent out in different order
	outbuf.addFlap(chan, 0);

	//Read SNAC family/type from buffer if able
	SNAC s = outbuf.readSnacHeader();

	//if the snac was read without a problem, find its rate class
	if ( !s.error )
	{
		//Pointer to proper rate class
		RateClass *rc = 0L;

		//Determine rate class
		for ( RateClass *tmp=rateClasses.first(); tmp; tmp = rateClasses.next() )
		{
			if ( tmp->isMember(s) )
			{
				kdDebug(14150) << k_funcinfo << "Rate class (id=" << tmp->id() <<
					") found: SNAC(" << s.family << "," << s.subtype << ")" << endl;
				rc = tmp;
				break;
			}
		}

		if ( rc )
			rc->enqueue(outbuf);
		else
			writeData(outbuf);
	}
	else
	{
		writeData(outbuf);
	}
}

/* Writes the next packet in the queue onto the wire */
void OscarSocket::writeData(Buffer &outbuf)
{
	//Update packet sequence number
	outbuf.changeSeqNum(flapSequenceNum);
	flapSequenceNum++;

	if(socketStatus() != OscarConnection::Connected)
	{
		kdDebug(14150) << k_funcinfo <<
			"Socket is NOT open, can't write to it right now" << endl;
		return;
	}

	//kdDebug(14150) << k_funcinfo << "Writing data" << endl;
#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << "--- OUTPUT ---" << outbuf.toString() << endl;
#endif

	//actually put the data onto the wire
	if(mSocket->writeBlock(outbuf.buffer(), outbuf.length()) == -1)
	{
		kdDebug(14150) << k_funcinfo << "writeBlock() call failed!" << endl;
		/*
		kdDebug(14150) << k_funcinfo <<
		mSocket->strError(mSocket->socketStatus(), mSocket->systemError()) << endl;
		*/
	}

	if ( sender() && sender()->isA("RateClass") )
		((RateClass *)sender())->dequeue();
}

// Logs in the user!
void OscarSocket::doLogin(
	const QString &host, int port,
	const QString &name, const QString &password,
	const QString &userProfile, const unsigned long initialStatus,
	const QString &/*awayMessage*/)
{
	QString realHost = host;

	if (isLoggedIn)
	{
		kdDebug(14150) << k_funcinfo << "We're already connected, aborting!" << endl;
		return;
	}
	if(realHost.isEmpty())
	{
		kdDebug(14150) << k_funcinfo << " Tried to connect without a hostname!" << endl;
		kdDebug(14150) << k_funcinfo << "Using login.oscar.aol.com" << endl;
		realHost = QString::fromLatin1("login.oscar.aol.com");
	}
	if(port < 1)
	{
		kdDebug(14150) << k_funcinfo << " Tried to connect to port < 1! Using port 5190" << endl;
		port = 5190;
	}
	if(password.isEmpty())
	{
		kdDebug(14150) << k_funcinfo << " Tried to connect without a password!" << endl;
		return;
	}

	kdDebug(14150) << k_funcinfo << "Connecting to '" << host << "', port=" << port << endl;

	disconnect(this, SIGNAL(socketConnected(const QString &)),
		this, SLOT(OnBosConnect()));
	disconnect(this, SIGNAL(connAckReceived()),
		this, SLOT(OnBosConnAckReceived()));

	connect(this, SIGNAL(socketConnected(const QString &)),
		this, SLOT(slotConnected()));
	connect(this, SIGNAL(connAckReceived()),
		this, SLOT(OnConnAckReceived()));

	setSN(name);
	loginPassword=password;
	loginProfile=userProfile;
	loginStatus=initialStatus;

	/*kdDebug(14150) << k_funcinfo <<
		"emitting statusChanged(OSCAR_CONNECTING)" << endl;*/
	emit statusChanged(OSCAR_CONNECTING);

	connectTo(realHost, QString::number(port));
}

void OscarSocket::parsePasswordKey(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "Got the key" << endl;

	WORD keylen = inbuf.getWord();
	delete[] mPwEncryptionKey;
	mPwEncryptionKey = inbuf.getBlock(keylen);
	sendLoginAIM();
}

void OscarSocket::connectToBos()
{
	kdDebug(14150) << k_funcinfo <<
		"Cookie received! Connecting to BOS server..." << endl;

//	emit connectionChanged(4,"Connecting to server...");
	disconnect(this, SIGNAL(connAckReceived()),
		this, SLOT(OnConnAckReceived()));
	disconnect(this, SIGNAL(socketConnected(const QString &)),
		this, SLOT(slotConnected()));

	// NOTE: Disconnect from socketClosed signal so we do not set our
	// account status (and thus our icon) from connecting to offline
	disconnect(this, SIGNAL(socketClosed(const QString &, bool)),
		this, SLOT(slotConnectionClosed(const QString &, bool)));

	mSocket->reset();

	connect(this, SIGNAL(connAckReceived()),
		this, SLOT(OnBosConnAckReceived()));
	connect(this, SIGNAL(socketConnected(const QString &)),
		this, SLOT(OnBosConnect()));

	// NOTE: Reconnect socketClosed status to track if we (were) disconnected
	// from the main BOS server
	connect(this, SIGNAL(socketClosed(const QString &, bool)),
		this, SLOT(slotConnectionClosed(const QString &, bool)));

	connectTo(bosServer, QString::number(bosPort));
}

void OscarSocket::OnBosConnAckReceived()
{
	kdDebug(14150) << k_funcinfo <<
		"BOS server ack'ed us! Sending auth cookie" << endl;

	sendCookie();
//	emit connectionChanged(5,"Connected to server, authorizing...");
}

void OscarSocket::sendCookie()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_COOKIE)" << endl;
	Buffer outbuf;
	putFlapVer(outbuf);
	outbuf.addTLV(0x0006, mCookieLength, mCookie);
	sendBuf(outbuf, 0x01);
}


void OscarSocket::OnBosConnect()
{
	kdDebug(14150) << k_funcinfo << "Connected to " << peerHost() <<
		", port " << peerPort() << endl;
}


void OscarSocket::parseAuthResponse(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	QPtrList<TLV> lst = inbuf.getTLVList();
	lst.setAutoDelete(TRUE);
	TLV *sn = findTLV(lst,0x0001);  //screen name
	TLV *url = findTLV(lst,0x0004);  //error url
	TLV *bosip = findTLV(lst,0x0005); //bos server address
	TLV *cook = findTLV(lst,0x0006); //authorization cookie
	//TLV *email = findTLV(lst,0x0007); //the email address attached to the account
	//TLV *regstatus = findTLV(lst,0x0013); //whether the email address is available to others
	TLV *err = findTLV(lst,0x0008); //whether an error occurred
	TLV *passChangeUrl = findTLV(lst,0x0054); //Password change URL, TODO: use for AIM accounts

	if (passChangeUrl)
	{
		kdDebug(14150) << k_funcinfo << "password change url='"  << passChangeUrl->data << "'" << endl;
		delete [] passChangeUrl->data;
	}

	delete[] mCookie;

	if (err)
	{
		QString errorString;
		int errorCode = (err->data[0] << 8)|err->data[1];

		switch(errorCode)
		{
			case 1: { errorString = i18n("Sign on failed because the screen name you provided is not registered on the AIM network. Please visit http://aim.aol.com to create a screen name for use on the AIM network."); break;  }
			case 5: { errorString = i18n("Sign on failed because the password supplied for this screen name is invalid. Please check your password and try again."); break; }
			case 0x11: { errorString = i18n("Sign on failed because your account is currently suspended."); break; }
			case 0x14: { errorString = i18n("The AOL Instant Messenger service is temporarily unavailable. Please try again later."); break; }
			case 0x18: { errorString = i18n("You have been connecting and disconnecting too frequently. Wait ten minutes and try again. If you continue to try, you will need to wait even longer."); break; }
			case 0x1c: { errorString = i18n("The client you are using is too old. Please upgrade."); break; }
			default: { errorString = i18n("Authentication failed."); break; }
		}

		emit protocolError(errorString, errorCode);
		delete[] err->data;
	}

	if (bosip)
	{
		QString ip = bosip->data;
		int index;
		index = ip.find(':');
		bosServer = ip.left(index);
		ip.remove(0,index+1); //get rid of the colon and everything before it
		bosPort = ip.toInt();

		kdDebug(14150) << "server is " << bosServer <<
			", ip.right(index) is " << ip <<
			", bosPort is " << bosPort << endl;

		delete[] bosip->data;
	}

	if (cook)
	{
		mCookie=cook->data;
		mCookieLength=cook->length;
		connectToBos();
	}

	if (sn)
		delete [] sn->data;

	if (url)
		delete [] url->data;

	lst.clear();
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



void OscarSocket::requestBOSRights(void)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQBOS) Requesting BOS rights" << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}

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
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}




// Requests location rights (CLI_REQLOCATION)
void OscarSocket::requestLocateRights()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQLOCATION), Requesting rights for location service" << endl;
	Buffer buf;
//	buf.addSnac(0x0002,0x0002,0x0000,0x00000002);
	buf.addSnac(0x0002,0x0002,0x0000,0x00000000);
	sendBuf(buf,0x02);
}

/** adds a mask of the groups that you want to be able to see you to the buffer */
void OscarSocket::sendGroupPermissionMask()
{
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0004,0x0000,0x00000000);
	outbuf.addDWord(0x0000001f);
	sendBuf(outbuf,0x02);
}

// adds a request for buddy list rights to the buffer
void OscarSocket::requestBuddyRights()
{
	kdDebug(14150) << k_funcinfo <<
		"SEND (CLI_REQBUDDY), Requesting rights for BLM (local buddy list management)" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_3,0x0002,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

// Parses the locate rights provided by the server (SRV_REPLYLOCATION)
void OscarSocket::parseLocateRights(Buffer &/*inbuf*/)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYLOCATION), TODO: Ignoring location rights" << endl;
	//we don't care what the locate rights are
	//and we don't know what they mean
	//  requestBuddyRights();

	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}

void OscarSocket::parseBuddyRights(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_REPLYBUDDY), TODO: Ignoring Buddy Rights" << endl;
	// TODO: use these values if possible

	while(1)
	{
		TLV t = inbuf.getTLV();
		if(t.data == 0L)
			break;

		Buffer tlvBuf(t.data, t.length);
		switch(t.type)
		{
			case 0x0001:
				kdDebug(14150) << k_funcinfo <<
					"max contactlist size     = " << tlvBuf.getWord() << endl;
				break;
			case 0x0002:
				kdDebug(14150) << k_funcinfo <<
					"max no. of watchers      = " << tlvBuf.getWord() << endl;
				break;
			case 0x0003:
				kdDebug(14150) << k_funcinfo <<
					"max online notifications = " << tlvBuf.getWord() << endl;
				break;
			default:
				break;
		}
		tlvBuf.clear();
	}

	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}




// parses the standard user info block
bool OscarSocket::parseUserInfo(Buffer &inbuf, UserInfo &u)
{
	u.userclass=0;
	u.evil=0;
	u.idletime = 0;
	u.sessionlen = 0;
	u.localip = 0;
	u.realip = 0;
	u.port = 0;
	u.fwType = 0;
	u.version = 0;
	u.icqextstatus=0;
	u.capabilities=0;

	if(inbuf.length() == 0)
	{
		kdDebug(14150) << k_funcinfo << "ZERO sized userinfo!" << endl;
		return false;
	}

	char *cb = inbuf.getBUIN(); // screenname/uin
	u.sn = tocNormalize(QString::fromLatin1(cb)); // screennames and uin are always us-ascii
	delete [] cb;

	// Next comes the warning level
	//for some reason aol multiplies the warning level by 10
	u.evil = (int)(inbuf.getWord() / 10);

	WORD tlvlen = inbuf.getWord(); //the number of TLV's that follow

	/*kdDebug(14150) << k_funcinfo
		<< "Contact: '" << u.sn <<
		"', number of TLVs following " << tlvlen << endl;*/

	for (unsigned int i=0; i<tlvlen; i++)
	{
		TLV t = inbuf.getTLV();
		Buffer tlvBuf(t.data,t.length);

		switch(t.type)
		{
			case 0x0001: //user class
			{
				u.userclass = tlvBuf.getWord();
				break;
			}
			case 0x0002: //member since time
			case 0x0005: // member since time (again)
			{
				u.membersince.setTime_t(tlvBuf.getDWord());
				break;
			}
			case 0x0003: //online since time
			{
				u.onlinesince.setTime_t(tlvBuf.getDWord());
				break;
			}
			case 0x0004: //idle time
			{
				u.idletime = tlvBuf.getWord();
				break;
			}
			case 0x0006:
			{
				u.icqextstatus = tlvBuf.getDWord();
				break;
			}
			case 0x000a: // IP in a DWORD [ICQ]
			{
				u.realip = htonl(tlvBuf.getDWord());
				break;
			}
			case 0x000c: // CLI2CLI
			{
				u.localip = htonl(tlvBuf.getDWord());
				u.port = tlvBuf.getDWord();
				u.fwType = static_cast<int>(tlvBuf.getWord());
				u.version = tlvBuf.getWord();
				// ignore the rest of the packet for now
				break;
			}
			case 0x000d: //capability info
			{
				u.capabilities = parseCapabilities(tlvBuf);
				break;
			}
			case 0x0010: //session length (for AOL users, in seconds)
			case 0x000f: //session length (for AIM users, in seconds)
			{
				u.sessionlen = tlvBuf.getDWord();
				break;
			}
			case 0x001e: // unknown, empty
				break;
			default: // unknown info type
			{
				/*kdDebug(14150) << k_funcinfo << "Unknown TLV(" << t.type <<
					") length=" << t.length << " in userinfo for user '" <<
					u.sn << "'" << tlvBuf.toString() << endl;*/
			}
		}; // END switch()
		tlvBuf.clear(); // unlink tmpBuf from tlv data
		delete [] t.data; // get rid of tlv data.
	} // END for (unsigned int i=0; i<tlvlen; i++)
	return true;
}



const DWORD OscarSocket::parseCapabilities(Buffer &inbuf)
{
//
// FIXME: port capabilities array to some qt based list class, makes usage of memcmp obsolete
//
	DWORD capflags = 0;

	#ifdef OSCAR_CAP_DEBUG
	QString dbgCaps = "CAPS: ";
	#endif

	while(inbuf.length() >= 16)
	{
		char *cap;
		cap = inbuf.getBlock(16);

		for (unsigned int i=0; oscar_caps[i].flag != AIM_CAPS_LAST; i++)
		{
			if (oscar_caps[i].flag == AIM_CAPS_KOPETE)
			{
				if (memcmp(&oscar_caps[i].data, cap, 12) == 0)
				{
					capflags |= oscar_caps[i].flag;
					kdDebug(14150) << k_funcinfo <<
						"Kopete Ver " << cap[12] << "." << cap[13] << "." << cap[14] << cap[15] << endl;
				}
			}
			else if (oscar_caps[i].flag == AIM_CAPS_MICQ)
			{
				if (memcmp(&oscar_caps[i].data, cap, 12) == 0)
				{
					capflags |= oscar_caps[i].flag;
					kdDebug(14150) << k_funcinfo <<
						"MICQ Ver " << cap[12] << "." << cap[13] << "." << cap[14] << cap[15] << endl;
				}
			}
			else if (memcmp(&oscar_caps[i].data, cap, 0x10) == 0)
			{
				capflags |= oscar_caps[i].flag;

#ifdef OSCAR_CAP_DEBUG
				switch(oscar_caps[i].flag)
				{
					case AIM_CAPS_BUDDYICON:
						dbgCaps += "AIM_CAPS_BUDDYICON ";
						break;
					case AIM_CAPS_VOICE:
						dbgCaps += "AIM_CAPS_VOICE ";
						break;
					case AIM_CAPS_IMIMAGE:
						dbgCaps += "AIM_CAPS_IMIMAGE ";
						break;
					case AIM_CAPS_CHAT:
						dbgCaps += "AIM_CAPS_CHAT ";
						break;
					case AIM_CAPS_GETFILE:
						dbgCaps += "AIM_CAPS_GETFILE ";
						break;
					case AIM_CAPS_SENDFILE:
						dbgCaps += "AIM_CAPS_SENDFILE ";
						break;
					case AIM_CAPS_GAMES2:
					case AIM_CAPS_GAMES:
						dbgCaps += "AIM_CAPS_GAMES ";
						break;
					case AIM_CAPS_SAVESTOCKS:
						dbgCaps += "AIM_CAPS_SAVESTOCKS ";
						break;
					case AIM_CAPS_SENDBUDDYLIST:
						dbgCaps += "AIM_CAPS_SENDBUDDYLIST ";
						break;
					case AIM_CAPS_ISICQ:
						dbgCaps += "AIM_CAPS_ISICQ ";
						break;
					case AIM_CAPS_APINFO:
						dbgCaps += "AIM_CAPS_APINFO ";
						break;
					case AIM_CAPS_RTFMSGS:
						dbgCaps += "AIM_CAPS_RTFMSGS ";
						break;
					case AIM_CAPS_EMPTY:
						dbgCaps += "AIM_CAPS_EMPTY ";
						break;
					case AIM_CAPS_ICQSERVERRELAY:
						dbgCaps += "AIM_CAPS_ICQSERVERRELAY ";
						break;
					case AIM_CAPS_IS_2001:
						dbgCaps += "AIM_CAPS_IS_2001 ";
						break;
					case AIM_CAPS_TRILLIANCRYPT:
						dbgCaps += "AIM_CAPS_TRILLIANCRYPT ";
						break;
					case AIM_CAPS_UTF8:
						dbgCaps += "AIM_CAPS_UTF8 ";
						break;
					case AIM_CAPS_IS_WEB:
						dbgCaps += "AIM_CAPS_IS_WEB ";
						break;
					case AIM_CAPS_INTEROPERATE:
						dbgCaps += "AIM_CAPS_INTEROPERATE ";
						break;
					case AIM_CAPS_MACICQ:
						dbgCaps += "AIM_CAPS_MACICQ";
 						break;

					default:
						QString capstring;
						capstring.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
							cap[0], cap[1], cap[2], cap[3],cap[4], cap[5],
							cap[6], cap[7], cap[8], cap[9],
							cap[10], cap[11], cap[12], cap[13],
							cap[14], cap[15]);
						kdDebug(14150) << k_funcinfo << "Unknown Capability: " << capstring << endl;
				} // END switch
#endif // OSCAR_CAP_DEBUG

				break;
			} // END if(memcmp...
		} // END for...
		delete [] cap;
	}
	#ifdef OSCAR_CAP_DEBUG
	kdDebug(14150) << k_funcinfo << dbgCaps << endl;
	#endif
	return capflags;
}


void OscarSocket::parseUserOnline(Buffer &inbuf)
{
	UserInfo u;
	if (parseUserInfo(inbuf, u))
	{
		//kdDebug(14150) << k_funcinfo << "RECV SRV_USERONLINE, name=" << u.sn << endl;
		emit gotContactChange(u);
	}
}

void OscarSocket::parseUserOffline(Buffer &inbuf)
{
	UserInfo u;
	if (parseUserInfo(inbuf, u))
	{
		//kdDebug(14150) << k_funcinfo << "RECV SRV_USEROFFLINE, name=" << u.sn << endl;
		emit gotOffgoingBuddy(u.sn);
	}
}


void OscarSocket::sendChangePassword(const QString &newpw, const QString &oldpw)
{
	// FIXME: This does not work :-(

	kdDebug(14150) << k_funcinfo << "Changing password from " << oldpw << " to " << newpw << endl;
	Buffer outbuf;
	outbuf.addSnac(0x0007,0x0004,0x0000,0x00000000);
	outbuf.addTLV(0x0002,newpw.length(),newpw.latin1());
	outbuf.addTLV(0x0012,oldpw.length(),oldpw.latin1());
	sendBuf(outbuf,0x02);
}


#if 0
void OscarSocket::parseRedirect(Buffer &inbuf)
{
	// FIXME: this function does not work, but it is never
	// called unless you want to connect to the advertisements server
	kdDebug(14150) << "Parsing redirect" << endl;

	OscarConnection *servsock = new OscarConnection(getSN(),"Redirect",Redirect,QByteArray(8));

	QPtrList<TLV> tl = inbuf.getTLVList();
//	int n;
	QString host;
	tl.setAutoDelete(true);

	if (!findTLV(tl,0x0006) && !findTLV(tl,0x0005) && !findTLV(tl,0x000e))
	{
		tl.clear();
		emit protocolError(
			i18n("An unknown error occurred. Please check " \
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
				kdDebug(14150) << "Set host to " << servsock->mHost <<
					", port to " << servsock->mPort << endl;*/
				break;
			}

			default: //unknown
				kdDebug(14150) << "Unknown tlv type in parseredirect: " << tmp->type << endl;
				break;
		}
		delete [] tmp->data;
	}
	tl.clear();
	//sockets.append(servsock);
	delete servsock;
	kdDebug(14150) << "Socket added to connection list!" << endl;
}
#endif




void OscarSocket::sendLocationInfo(const QString &profile, const unsigned long caps)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_SETUSERINFO/CLI_SET_LOCATION_INFO)" << endl;
	Buffer outbuf, capBuf;

	unsigned long sendCaps;
	if(caps==0)
		sendCaps = mIsICQ ? KOPETE_ICQ_CAPS : KOPETE_AIM_CAPS;
	else
		sendCaps = caps;

	outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);
	if(!profile.isNull() && !mIsICQ)
	{
		static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
		outbuf.addTLV(0x0001, defencoding.length(), defencoding.latin1());
		outbuf.addTLV(0x0002, profile.length(), profile.local8Bit());
		//kdDebug(14150) << k_funcinfo << "adding profile=" << profile << endl;
	}

	for (int i=0; oscar_caps[i].flag != AIM_CAPS_LAST; i++)
	{
		if (oscar_caps[i].flag & sendCaps)
			capBuf.addString(oscar_caps[i].data, 16);
	}
	//kdDebug(14150) << k_funcinfo << "adding capabilities, size=" << capBuf.length() << endl;
	outbuf.addTLV(0x0005, capBuf.length(), capBuf.buffer());

	sendBuf(outbuf,0x02);
}


void OscarSocket::doLogoff()
{
	kdDebug(14150) << k_funcinfo << endl;

	mSSIData.clear();
	if(isLoggedIn && (socketStatus() == OscarConnection::Connected))
	{
		kdDebug(14150) << k_funcinfo << "Sending sign off request" << endl;
		Buffer outbuf;
		sendBuf(outbuf, 0x04);
		close();
	}
	else
	{
		if(socketStatus() == OscarConnection::Connecting ||
			socketStatus() == OscarConnection::Connected )
		{
			kdDebug(14150) << k_funcinfo << endl <<
				" === ERROR ===" << endl <<
				"We are either not logged in correctly or something" <<
				"wicked happened, closing down socket..." << endl <<
				" === ERROR ===" << endl;

			//mSocket->close();
			mSocket->reset();

			// FIXME ?
			// The above socket calls _should_ trigger a connectionClosed,
			// I hope it really does [mETz, 05-05-2004]
			//emit connectionClosed(QString::null);
		}
	}
}


// Gets this user status through BLM service
void OscarSocket::sendAddBuddylist(const QString &contactName)
{
	kdDebug(14150) << k_funcinfo << "Sending BLM add buddy" << endl;
	QStringList Buddy = contactName;
	sendBuddylistAdd(Buddy);
}

void OscarSocket::sendDelBuddylist(const QString &contactName)
{
	kdDebug(14150) << k_funcinfo << "Sending BLM del buddy" << endl;
	QStringList Buddy = contactName;
	sendBuddylistDel(Buddy);
}



void OscarSocket::parseError(WORD family, WORD snacID, Buffer &inbuf)
{
	QString msg;
	WORD reason = inbuf.getWord();
	kdDebug(14150) << k_funcinfo <<
		"Got an error, SNAC family=" << family << ", reason=" << reason << endl;


	if (reason < msgerrreasonlen)
	{
		switch (family)
		{
			case OSCAR_FAM_2:
				// Ignores recipient is not logged in errors, usually caused by querying aim userinfo
				if (reason == 4)
				{
					kdDebug(14150) << k_funcinfo <<
						"IGNORED Family 2 error, recipient not logged in" << endl;
					return;
				}
				msg = i18n("Sending userprofile failed: %1").arg(msgerrreason[reason]);
				break;
			case OSCAR_FAM_4:
				// Ignores rate to client errors, usually caused by querying away messages
				if (reason == 3)
				{
					kdDebug(14150) << k_funcinfo <<
						"IGNORED Family 4 error, rate to client" << endl;
					return;
				}
				msg = i18n("Your message did not get sent because the following" \
					" error occurred: %1").arg(msgerrreason[reason]);
				break;
			case OSCAR_FAM_21:
			{
				if (reason == 2)
				{
					msg = i18n("Your ICQ information request was denied by the " \
						"ICQ-Server, please try again later.");
				}
				else
				{
					msg = i18n("Your ICQ information request failed.\n" \
						"%1").arg(msgerrreason[reason]);
				}
				break;
			}
			default:
				msg = i18n("Generic Packet error: %1").arg(msgerrreason[reason]);
				break;
		}
	}
	else
	{
		if (family == OSCAR_FAM_2)
		{
			msg = i18n("Sending userprofile failed: Unknown Error.\n" \
				"Please report a bug at http://bugs.kde.org");
		}
		else if (family == OSCAR_FAM_4)
		{
			msg = i18n("Your message did not get sent: Unknown Error.\n" \
				"Please report a bug at http://bugs.kde.org");
		}
		else
		{
			msg = i18n("Generic Packet error: Unknown Error.\n" \
				"Please report a bug at http://bugs.kde.org");
		}
	}

	emit protocolError(msg, reason);
	// in case a special request failed
	emit snacFailed(snacID);
}



void OscarSocket::sendInfo()
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	// greater 7 and thus sendInfo() is not getting called again
	// except on reconnnect
	gotAllRights=99;

	sendLocationInfo(loginProfile); // CLI_SETUSERINFO
	loginProfile = QString::null;

	sendMsgParams(); // CLI_SETICBM

	sendIdleTime(0); // CLI_SNAC1_11, sent before CLI_SETSTATUS

	sendICQStatus(loginStatus); // send initial login status in case we are ICQ

	if (!mIsICQ)
	{
		sendGroupPermissionMask(); // unknown to icq docs
		sendPrivacyFlags(); // unknown to icq docs
	}

	sendClientReady(); // CLI_READY
	sendReqOfflineMessages(); // CLI_REQOFFLINEMSGS
	startKeepalive();

	requestMyUserInfo();
}


// Reads a FLAP header from the input
FLAP OscarSocket::getFLAP()
{
	BYTE start = 0x00;
	char peek[6];
	FLAP fl;
	fl.error = false;

	if (mSocket->peekBlock(&peek[0], 6) != 6 )
	{
		kdDebug(14150) << k_funcinfo <<
			"Error reading 6 bytes for FLAP" << endl;
		fl.error = true;
		return fl;
	}

	Buffer buf(&peek[0], 6);
	start = buf.getByte();

	if (start == 0x2a)
	{
		//get the channel ID
		fl.channel = buf.getByte();
		//kdDebug(14150) << k_funcinfo << "FLAP channel=" << fl.channel << endl;

		//get the sequence number
		fl.sequence_number = buf.getWord();
		//kdDebug(14150) << k_funcinfo << "FLAP sequence_number=" << fl.sequence_number << endl;

		// get the packet length
		fl.length = buf.getWord();
		//kdDebug(14150) << k_funcinfo << "FLAP length=" << fl.length << endl;
		//kdDebug(14150) << k_funcinfo << "bytes available=" << mSocket->bytesAvailable() << endl;

		if(mSocket->bytesAvailable() < fl.length+6)
		{
			kdDebug(14150) << k_funcinfo <<
				"Not enough data in recv buffer to read the full FLAP (want " <<
				fl.length+6 << " bytes, got " << mSocket->bytesAvailable() <<
				"bytes), aborting" << endl;
			fl.error = true;
		}
	}
	else
	{
		kdDebug(14150) << k_funcinfo <<
			"Error reading FLAP, start byte is '" << start << "'" << endl;
		fl.error = true;
	}

	// get rid of FLAP header because we successfully retrieved it
	if (!fl.error)
	{
		/*kdDebug(14150) << k_funcinfo <<
			"Returning FLAP, channel=" << fl.channel << ", length=" << fl.length << endl;*/
		mSocket->readBlock(0L, 6);
	}
	return fl;
}



#if 0
// Called when a direct IM is received
void OscarSocket::OnDirectIMReceived(QString message, QString sender, bool isAuto)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	OscarMessage oMsg;
	oMsg.setText(message, OscarMessage::Plain);
	oMsg.setType(isAuto ? OscarMessage::Away : OscarMessage::Normal);

	emit receivedMessage(sender, oMsg);
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

void OscarSocket::sendFileSendRequest(const QString &sn, const KFileItem &finfo)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	sendRendezvous(sn, 0x0000, AIM_CAPS_SENDFILE, &finfo);
}

OscarConnection * OscarSocket::sendFileSendAccept(const QString &sn,
	const QString &fileName)
{
	sendRendezvous(sn, 0x0001, AIM_CAPS_SENDFILE);
	mFileTransferMgr->addFileInfo(sn,
		new KFileItem(KFileItem::Unknown, KFileItem::Unknown, fileName));

	return mFileTransferMgr->establishOutgoingConnection(sn);
}

void OscarSocket::sendFileSendDeny(const QString &sn)
{
	sendRendezvous(sn, 0x0002, AIM_CAPS_SENDFILE);
}

void OscarSocket::OnFileTransferBegun(OscarConnection *con, const QString& file,
	const unsigned long size, const QString &recipient)
{
	kdDebug(14150) << k_funcinfo << "emitting transferBegun()" << endl;
	emit transferBegun(con, file, size, recipient);
}

OncomingSocket *OscarSocket::serverSocket(DWORD capflag)
{
	if ( capflag & AIM_CAPS_IMIMAGE ) //direct im
		return mDirectIMMgr;
	else  //must be a file transfer?
		return mFileTransferMgr;
}
#endif



void OscarSocket::parseConnectionClosed(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (DISCONNECT)" << endl;

	// This is a part of icq login procedure,
	// Move this into its own function!
	QPtrList<TLV> lst=inbuf.getTLVList();
	lst.setAutoDelete(TRUE);
/*
	kdDebug(14150) << "contained TLVs:" << endl;
	TLV *t;
	for(t=lst.first(); t; t=lst.next())
	{
		kdDebug(14150) << "TLV(" << t->type << ") with length " << t->length << endl;
	}
*/
	TLV *uin=findTLV(lst,0x0001);
	if(uin)
	{
		kdDebug(14150) << k_funcinfo << "found TLV(1) [UIN], uin=" << uin->data << endl;
		delete [] uin->data;
	}

	TLV *descr=findTLV(lst,0x0004);
	if(!descr)
		descr=findTLV(lst,0x000b);
	if(descr)
	{
		kdDebug(14150) << k_funcinfo << "found TLV(4) [DESCRIPTION] reason=" << descr->data << endl;
		delete [] descr->data;
	}

	TLV *err=findTLV(lst,0x0008);
	if (!err)
		err=findTLV(lst,0x0009);
	if (err)
	{
		kdDebug(14150) << k_funcinfo << k_funcinfo << "found TLV(8) [ERROR] error= " <<
			((err->data[0] << 8)|err->data[1]) << endl;

		WORD errorNum = ((err->data[0] << 8)|err->data[1]);

		switch(errorNum)
		{
			case 0x0001: // multiple logins (on same UIN)
			{
// 				kdDebug(14150) << k_funcinfo <<
// 					"multiple logins (on same UIN)!!!" << endl;
				emit protocolError(
					i18n("You have logged in more than once with the same %1," \
						" this login is now disconnected.").arg((mIsICQ ? "UIN" : "buddy name")), 0);
				break;
			}

			case 0x0004:
			case 0x0005: // bad password
			{
// 				kdDebug(14150) << k_funcinfo << "bad password!!!" << endl;
				emit protocolError(
					i18n("Could not log on to %1 with account %2 as the password was" \
						" incorrect.").arg((mIsICQ ? "ICQ" : "AIM")).arg(getSN()), 5);
				break;
			}

			case 0x0007: // non-existant ICQ#
			case 0x0008: // non-existant ICQ#
			{
// 				kdDebug(14150) << k_funcinfo << "non-existant ICQ#" << endl;
				emit protocolError(
					i18n("Could not log on to %1 with nonexistent account %2.").arg(
						(mIsICQ ? "ICQ" : "AIM")).arg(getSN()), 0);
				break;
			}

			case 0x0015: // too many clients from same IP
			case 0x0016: // too many clients from same IP
			{
/*				kdDebug(14150) << k_funcinfo <<
					"too many clients from same IP" << endl;*/
				emit protocolError(
					i18n("Could not log on to %1 as there are too many clients" \
						" from the same computer.").arg(
							(mIsICQ ? "ICQ" : "AIM")), 0);
				break;
			}

			case 0x0018: // rate exceeded (turboing)
			{
				/*kdDebug(14150) << k_funcinfo <<
					"rate exceeded (maybe reconnecting too fast), " \
					"server-ban for at least 10 mins!!!" << endl;*/
				emit protocolError(
					i18n("Server has blocked %1 account %2 for sending messages too quickly." \
						" Wait ten minutes and try again. If you continue to try, you will" \
						" need to wait even longer.").arg(
							(mIsICQ ? "ICQ" : "AIM")).arg(getSN()), 0);
				break;
			}
		}

		if (errorNum != 0x0000)
			mAccount->disconnect(KopeteAccount::Manual); //doLogoff();

		delete [] err->data;
	}

	TLV *server = findTLV(lst,0x0005);
	if (server)
	{
		kdDebug(14150) << k_funcinfo << "found TLV(5) [SERVER]" << endl;
		QString ip=server->data;
		int index=ip.find(':');
		bosServer=ip.left(index);
		ip.remove(0,index+1); //get rid of the colon and everything before it
		bosPort=ip.toInt();
		kdDebug(14150) << k_funcinfo << "We should reconnect to server '" << bosServer <<
			"' on port " << bosPort << endl;
		delete[] server->data;
	}

	TLV *cook=findTLV(lst,0x0006);
	if (cook)
	{
		kdDebug(14150) << "found TLV(6) [COOKIE]" << endl;
		mCookie=cook->data;
		mCookieLength=cook->length;
//		delete [] cook->data;
		connectToBos();
	}
	lst.clear();
}

void OscarSocket::sendAuthRequest(const QString &contact, const QString &reason)
{
	kdDebug(14150) << k_funcinfo << "contact='" << contact << "', reason='" << reason <<
		"'" << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0018,0x0000,0x00000000);

	outbuf.addBUIN(contact.ascii()); //dest sn
	outbuf.addBSTR(reason.local8Bit());
	outbuf.addWord(0x0000);
	sendBuf(outbuf,0x02);
}

void OscarSocket::sendAuthReply(const QString &contact, const QString &reason, bool grant)
{
	kdDebug(14150) << k_funcinfo << "contact='" << contact << "', reason='" << reason <<
		"', grant=" << grant << endl;

	Buffer outbuf;
	outbuf.addSnac(0x0013,0x001a,0x0000,0x00000000);
	outbuf.addBUIN(contact.ascii()); //dest sn
	outbuf.addByte(grant ? 0x01 : 0x00);
	outbuf.addBSTR(reason.local8Bit());
	sendBuf(outbuf,0x02);
}

void OscarSocket::parseAuthReply(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	char *tmp = 0L;
	BYTE grant = 0;

	tmp = inbuf.getBUIN();
	QString contact = ServerToQString(tmp, 0L, false);
	delete [] tmp;

	grant = inbuf.getByte();

	tmp = inbuf.getBSTR();
	QString reason = ServerToQString(tmp, 0L, false);
	delete [] tmp;

	emit gotAuthReply(contact, reason, (grant==0x01));
}


void OscarSocket::sendBuddylistAdd(QStringList &contacts)
{
	kdDebug(14150) << k_funcinfo << "SEND CLI_ADDCONTACT (add to local userlist)" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_3,0x0004,0x0000,0x00000000);
	for(QStringList::Iterator it = contacts.begin(); it != contacts.end(); ++it)
	{
		QCString contact = (*it).latin1();
		outbuf.addByte(contact.length());
		outbuf.addString(contact, contact.length());
	}
	sendBuf(outbuf,0x02);
}

void OscarSocket::sendBuddylistDel(QStringList &contacts)
{
	kdDebug(14150) << k_funcinfo << "SEND CLI_DELCONTACT (delete from local userlist)" << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_3,0x0005,0x0000,0x00000000);
	for(QStringList::Iterator it = contacts.begin(); it != contacts.end(); ++it)
	{
		QCString contact = (*it).latin1();
		outbuf.addByte(contact.length());
		outbuf.addString(contact, contact.length());
	}
	sendBuf(outbuf,0x02);
}


const QString OscarSocket::ServerToQString(const char* string, OscarContact *contact, bool isUtf8)
{
	//kdDebug(14150) << k_funcinfo << "called" << endl;

	int length = strlen(string);
	int cresult = -1;
	QTextCodec *codec = 0L;

	if(contact != 0L)
	{
		if(contact->encoding() != 0)
			codec = QTextCodec::codecForMib(contact->encoding());
		else
			codec = 0L;
#ifdef CHARSET_DEBUG
		if(codec)
		{
			kdDebug(14150) << k_funcinfo <<
				"using per-contact encoding, MIB=" << contact->encoding() << endl;
		}
#endif
	}

	if(!codec && isUtf8) // in case the per-contact codec didn't work we check if the caller thinks this message is utf8
	{
		codec = QTextCodec::codecForMib(106); //UTF-8
		if(codec)
		{
			cresult = codec->heuristicContentMatch(string, length);
#ifdef CHARSET_DEBUG
			kdDebug(14150) << k_funcinfo <<
				"result for FORCED UTF-8 = " << cresult <<
				", message length = " << length << endl;
#endif
			/*if(cresult < (length/2)-1)
				codec = 0L;*/
		}
	}

	return KopeteMessage::decodeString( string, codec );
}

#include "oscarsocket.moc"
// vim: set noet ts=4 sts=4 sw=4:
