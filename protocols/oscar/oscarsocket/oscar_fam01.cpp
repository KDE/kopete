/*
    oscar_fam01.cpp  -  Oscar Protocol, SNAC Family 1 (Generic service controls)

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

extern "C"
{
#include "md5.h"
}

#include "oscarsocket.h"
#include "oscardebug.h"
#include "oscaraccount.h"

#include "rateclass.h"

#include <kdebug.h>

const WORD CLI_READY = 0x0002;
const WORD CLI_SERVICExREQ = 0x0004;
const WORD CLI_RATES_REQUEST = 0x0006;
const WORD CLI_RATES_ACK = 0x0008;
const WORD CLI_REQ_SELFINFO = 0x000e;
const WORD CLI_SETxIDLExTIME = 0x0011;
const WORD CLI_SET_PRIVACY = 0x0014;
const WORD CLI_FAMILIES_VERSIONS = 0x0017;
const WORD CLI_VERIFICATION_REPLY = 0x0020;


// SNAC(01,03) SRV_FAMILIES
// http://iserverd.khstu.ru/oscar/snac_01_03.html
void OscarSocket::parseServerReady(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_FAMILIES), got list of families" << endl;

	int famcount; //the number of families received
	//WORD *families = new WORD[inbuf.length()];
	WORD *families = new WORD[(int)inbuf.length()/2];
	for (famcount=0; inbuf.length() > 1; famcount++)
	{
		families[famcount] = inbuf.getWord();
	}

	sendVersions(families, famcount); // send back a CLI_FAMILIES packet
	//emit serverReady(); // What is this exactly used for? [mETz]
	delete [] families;
}


// SNAC(01,04)  CLI_SERVICExREQ
// http://iserverd.khstu.ru/oscar/snac_01_04.html
void OscarSocket::sendRequestService(const WORD serviceFamily)
{
	kdDebug(14150) << k_funcinfo <<
		"Requesting new service with SNAC family = " << serviceFamily << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_SERVICExREQ,0x0000,0x00000000);
	outbuf.addWord(serviceFamily);
	sendBuf(outbuf,0x02);
}


// SNAC(01,06)  CLI_RATES_REQUEST
// http://iserverd.khstu.ru/oscar/snac_01_06.html
void OscarSocket::sendRateInfoRequest()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_RATESREQUEST)" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_RATES_REQUEST,0x0000,0x00000006);
	sendBuf(outbuf,0x02);
}


// SNAC(01,0F)  SRV_ONLINExINFO
// http://iserverd.khstu.ru/oscar/snac_01_0f.html
void OscarSocket::parseMyUserInfo(Buffer &inbuf)
{
	if (gotAllRights > 7)
	{
		kdDebug(14150) << k_funcinfo "RECV (SRV_REPLYINFO) Parsing OWN user info" << endl;
		UserInfo u;
		parseUserInfo(inbuf, u);
		emit gotContactChange(u); // gotMyUserInfo(u);
		return;
	}

	kdDebug(14150) << k_funcinfo <<
		"RECV (SRV_REPLYINFO) Ignoring OWN user info, gotAllRights=" <<
		gotAllRights << endl;

	gotAllRights++;
	if (gotAllRights==7)
	{
		kdDebug(14150) << k_funcinfo "gotAllRights==7" << endl;
		sendInfo();
	}
}


// SNAC(01,07)  SRV_RATE_LIMIT_INFO
// http://iserverd.khstu.ru/oscar/snac_01_07.html
void OscarSocket::parseRateInfoResponse(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo <<
		"RECV (SRV_RATES), Parsing Rate Info Response" << endl;

	RateClass *rc = 0;
	//SnacPair *snacPair = 0;
	unsigned int i = 0;
	WORD numclasses = inbuf.getWord();

	/*kdDebug(14150) << k_funcinfo <<
	"Number of Rate Classes=" << numclasses << endl;*/

	for (i=0; i<numclasses; i++)
	{
		WORD classid;
		DWORD windowsize, clear, alert, limit, disc,
			current, max, lastTime;
		BYTE currentState;

		rc = new RateClass;

		classid = inbuf.getWord();
		//kdDebug(14150) << k_funcinfo << "Rate classId=" << rc->classid << endl;
		windowsize = inbuf.getDWord();
		clear = inbuf.getDWord();
		alert = inbuf.getDWord();
		limit = inbuf.getDWord();
		disc = inbuf.getDWord();
		current = inbuf.getDWord();
		max = inbuf.getDWord();
		// TODO: From http://iserverd.khstu.ru/oscar/snac_01_07.html
		// Last time  -  not present in protocol version 2
		// Current state  -  not present in protocol version 2
		// i.e. find out when protocol version 2 is used (if it
		// isn't obsolete already)
		lastTime = inbuf.getDWord();
		currentState = inbuf.getByte();

		rc->setRateInfo(classid, windowsize, clear, alert, limit,
			disc, current, max, lastTime, currentState);

		connect(rc, SIGNAL(dataReady(Buffer &)),
			this, SLOT(writeData(Buffer &)));

		rateClasses.append(rc);
	}

#ifdef OSCAR_PACKETLOG
	kdDebug(14150) << "REMAINING BUFFER:" << inbuf.toString() << endl;
#endif

	//now here come the members of each class
	for (i=0; i<numclasses; i++)
	{
		WORD classid = inbuf.getWord();
		WORD count = inbuf.getWord();

//		kdDebug(14150) << k_funcinfo << "Classid: " << classid <<
//			", Count: " << count << endl;

		RateClass *tmp;
		rc = 0;
		// find the class we're talking about
		for (tmp=rateClasses.first(); tmp; tmp=rateClasses.next())
		{
			if (tmp->id() == classid)
			{
				rc = tmp;
				break;
			}
		}

		// do not go into next for() loop if we didnt find a RateClass anyway
		if (rc == 0)
		{
			kdDebug(14150) << k_funcinfo << "WARNING: Could not find RateClass" <<
				"to store SNAC group/type pairs into. Missing class id = " <<
				classid << endl;
			continue;
		}

		for (WORD j=0; j<count; j++)
		{
			/*
			snacPair = new SnacPair;
			snacPair->group = inbuf.getWord();
			snacPair->type = inbuf.getWord();
			if (rc)
				rc->addMember(s);
			*/

			WORD group = inbuf.getWord();
			WORD type = inbuf.getWord();
			rc->addMember(group, type);
		}
	}

	if(inbuf.length() != 0)
	{
		kdDebug(14150) << k_funcinfo <<
			"ERROR: Did not parse all Rates successfully!" << endl;
	}

	sendRateAck();
}


// SNAC(01,08)  CLI_RATES_ACK
// http://iserverd.khstu.ru/oscar/snac_01_08.html
void OscarSocket::sendRateAck()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_ACKRATES)" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_RATES_ACK,0x0000,0x00000000);
	for (RateClass *rc = rateClasses.first(); rc; rc = rateClasses.next())
	{
		/*kdDebug(14150) << k_funcinfo << "adding classid " << rc->id() <<
			 " to the rate acknowledgement" << endl;*/
		outbuf.addWord(rc->id());
	}
	sendBuf(outbuf,0x02);

	//request our user info after sending the rate ack
	requestInfo();
}


void OscarSocket::requestInfo()
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;

	requestMyUserInfo(); // CLI_REQINFO
	sendSSIRightsRequest();  // CLI_REQLISTS
	sendRosterRequest(); // CLI_CHECKROSTER
	requestLocateRights(); // CLI_REQLOCATION
	requestBuddyRights(); // CLI_REQBUDDY
	requestMsgRights(); // CLI_REQICBML
	requestBOSRights(); // CLI_REQBOS

	kdDebug(14150) << k_funcinfo << "resetting gotAllRights to 0!" << endl;
	gotAllRights=0;
	// next received packet should be a SRV_REPLYINFO
}


// SNAC(01,14)  CLI_SET_PRIVACY
// http://iserverd.khstu.ru/oscar/snac_01_14.html
void OscarSocket::sendPrivacyFlags()
{
	if (mIsICQ)
		return;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_SET_PRIVACY,0x0000,0x00000000);
	//bit 1: let others see idle time
	//bit 2: let other see member since
	//TODO: Make this configurable
	outbuf.addDWord(0x00000003);
	sendBuf(outbuf,0x02);
}


// SNAC(01,0E)  CLI_REQ_SELFINFO
// http://iserverd.khstu.ru/oscar/snac_01_0e.html
void OscarSocket::requestMyUserInfo()
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_REQINFO)" << endl;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_REQ_SELFINFO,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}


void OscarSocket::parseRateChange(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_RATE_LIMIT_WARN)" << endl;

	WORD code = inbuf.getWord();

	switch(code)
	{
		case 0x0001:
			kdDebug(14150) << k_funcinfo <<
				"Rate limits parameters changed" << endl;
			break;
		case 0x0002:
			kdDebug(14150) << k_funcinfo <<
				"Rate limits warning (current level < alert level)" << endl;
			break;
		case 0x0003:
			kdDebug(14150) << k_funcinfo <<
				"Rate limit hit (current level < limit level)" << endl;
			break;
		case 0x0004:
			kdDebug(14150) << k_funcinfo <<
				"Rate limit clear (current level > clear level)" << endl;
			break;
		default:
			kdDebug(14150) << k_funcinfo <<
				"unknown code for rate limit warning" << endl;
	}

	WORD rateclass = inbuf.getWord();

	DWORD windowSize = inbuf.getDWord();
	DWORD clearLevel = inbuf.getDWord();
	DWORD alertLevel = inbuf.getDWord();
	DWORD limitLevel = inbuf.getDWord();
	DWORD disconnectLevel = inbuf.getDWord();
	DWORD currentLevel = inbuf.getDWord();
	DWORD maxLevel = inbuf.getDWord();
	DWORD lastTime = inbuf.getDWord();
	BYTE currentState = inbuf.getByte();

	if (code != 0x0002) // do not spam me just with warnings, only critical limits please
	{
		kdDebug(14150) << k_funcinfo <<
		"RATE LIMIT DATA (class id " << rateclass << ") -----" << endl <<
		"  windowSize= " << windowSize << endl <<
		"  clearLevel= " << clearLevel << endl <<
		"  alertLevel= " << alertLevel << endl <<
		"  limitLevel= " << limitLevel << endl <<
		"  disconnectLevel= " << disconnectLevel << endl <<
		"  currentLevel= " << currentLevel << endl <<
		"  maxLevel= " << maxLevel << endl <<
		"  lastTime= " << lastTime << endl <<
		"  currentState= " << (WORD)currentState << endl;
	}

	if (currentLevel <= disconnectLevel)
	{
		emit protocolError(i18n("The account %1 will be disconnected for " \
			"exceeding the rate limit. Please wait approximately 10 minutes" \
			" before reconnecting.")
			.arg(mAccount->accountId()), 0);
		//let the account properly clean itself up
		mAccount->disconnect(KopeteAccount::Manual);
	}
	else
	{
		if (code == 0x0002 || code == 0x0003)
		{
			kdDebug(14150) << k_funcinfo "WARNING about rate-limit in class "
				<< rateclass << " received. Current level: "
				<< currentLevel << endl;
		}
	}

/*
 Docs from http://iserverd.khstu.ru/oscar/snac_01_0a.html

0x0001 Rate limits parameters changed
0x0002 Rate limits warning (current level < alert level)
0x0003 Rate limit hit (current level < limit level)
0x0004 Rate limit clear (current level become > clear level)

xx xx		word	Message code (see above)
xx xx		word	Rate class ID
xx xx xx xx	dword	Window size
xx xx xx xx	dword	Clear level
xx xx xx xx	dword	Alert level
xx xx xx xx	dword	Limit level
xx xx xx xx	dword	Disconnect level
xx xx xx xx	dword	Current level
xx xx xx xx	dword	Max level
xx xx xx xx	dword	Last time
xx			byte	Current state
*/
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


void OscarSocket::parseMessageOfTheDay(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_MOTD)" << endl;
	WORD id = inbuf.getWord();
	if (id < 4)
	{
		emit protocolError(i18n(
			"An unknown error occurred. Your connection may be lost. " \
			"The error was: \"AOL MOTD Error: your connection may be lost. ID: %1\"").arg(id), 0);
	}
}



void OscarSocket::parseServerVersions(Buffer &inbuf)
{
	kdDebug(14150) << k_funcinfo << "RECV (SRV_FAMILIES2), got list of families this server understands" << endl;

	int srvFamCount;
	for (srvFamCount=0; inbuf.length(); srvFamCount++)
	{
		kdDebug(14150) << k_funcinfo << "server family=" << inbuf.getWord() <<
			", server version=" << inbuf.getWord() << endl;
	}

	//The versions are not important to us at all
	//now we can request rates
	sendRateInfoRequest(); // CLI_RATESREQUEST
}

/**
 *  Handles AOL's evil attempt to thwart 3rd party apps using Oscar.
 *  It requests a length and offset of aim.exe.  We can thwart it with
 *  help from the good people at Gaim
 */
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
		outbuf.addSnac(OSCAR_FAM_1,CLI_VERIFICATION_REPLY,0x0000,0x00000000);
		outbuf.addWord(0x0010); //hash is always 0x10 bytes
		outbuf.addString((char *)digest, 0x10);
		sendBuf(outbuf,0x02);
	}
	ql.clear();
}




void OscarSocket::sendClientReady()
{
	kdDebug(14150) << "SEND (CLI_READY) sending client ready, end of login procedure." << endl;

	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_READY,0x0000,0x00000000);

	for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
	{
		/*kdDebug(14150) << "adding family '" << rc->classid <<
			"' to CLI_READY packet" << endl;*/

		outbuf.addWord(rc->id());

		if (rc->id() == 0x0001)
		{
			outbuf.addWord(0x0003);
		}
		else if (rc->id() == 0x0013)
		{
			outbuf.addWord(mIsICQ ? 0x0002 : 0x0003);
		}
		else
		{
			outbuf.addWord(0x0001);
		}

		if(mIsICQ)
		{
			if(rc->id() == 0x0002)
				outbuf.addWord(0x0101);
			else
				outbuf.addWord(0x0110);
			outbuf.addWord(0x047b);
		}
		else // AIM
		{
			if (rc->id() == 0x0008 ||
				rc->id() == 0x000b ||
				rc->id() == 0x000c)
			{
				outbuf.addWord(0x0104);
				outbuf.addWord(0x0001);
			}
			else
			{
				outbuf.addWord(0x0110);
				outbuf.addWord(0x059b);
			}
		}
	}
	sendBuf(outbuf,0x02);

	kdDebug(14150) << "============================================================================" << endl;
	kdDebug(14150) << "============================================================================" << endl;

	isLoggedIn = true;
	emit loggedIn();
}

void OscarSocket::sendVersions(const WORD *families, const int len)
{
	kdDebug(14150) << k_funcinfo << "SEND (CLI_FAMILIES)" << endl;
	WORD val;
	Buffer outbuf;
	outbuf.addSnac(OSCAR_FAM_1,CLI_FAMILIES_VERSIONS,0x0000,0x00000000);

	for(int i=0;i<len;i++)
	{
		outbuf.addWord(families[i]);
		if(families[i]==0x0001)
		{
			val=0x0003;
		}
		else if (families[i]==0x0013)
		{
			if(mIsICQ)
				val=0x0004; // for ICQ2002
			else
				val=0x0003;
		}
		else
			val=0x0001;

//		kdDebug(14150) << k_funcinfo << "family=" << families[i] << ", val=" << val << endl;
		outbuf.addWord(val);
	}
	sendBuf(outbuf,0x02);

}

// SNAC(01,11)  CLI_SETxIDLExTIME
// http://iserverd.khstu.ru/oscar/snac_01_11.html
void OscarSocket::sendIdleTime(DWORD time)
{
	if (!isLoggedIn)
		return;

	bool newidle = (time!=0);
	if (newidle != idle) //only do stuff if idle status changed
	{
		kdDebug(14150) << k_funcinfo <<
			"SEND (CLI_SETxIDLExTIME), sending idle time, time=" << time << endl;
		idle = newidle;
		Buffer outbuf;
		outbuf.addSnac(OSCAR_FAM_1,CLI_SETxIDLExTIME,0x0000,0x00000000);
		outbuf.addDWord(time);
		sendBuf(outbuf,0x02);
	}
}


