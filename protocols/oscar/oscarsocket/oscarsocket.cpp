/*
    oscarsocket.cpp  -  Oscar Protocol Implementation

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

extern "C" {
#include "md5.h"
};

#include "oscarsocket.h"
#include "oscarsocket.moc"
#include "oncomingsocket.h"
#include "oscardebugdialog.h"

#include <qdatetime.h>
#include <unistd.h>
#include <stdlib.h>
#include <kdebug.h>
#include <klocale.h>
#define DIRECTCONNECT		0x0f1f
#define DIRECTIM_PORT		4443

#define AIM_MD5_STRING "AOL Instant Messenger (SM)"
#define AIM_CLIENTSTRING "AOL Instant Messenger (SM), version 4.8.2790/WIN32"

#define AIM_CLIENTID 0x0109
#define AIM_MAJOR 0x0004
#define AIM_MINOR 0x0008
#define AIM_POINT 0x0000
#define AIM_BUILD 0x0ae6
static const char AIM_OTHER[] = { 0x00, 0x00, 0x00, 0xbb };
#define AIM_COUNTRY "us"
#define AIM_LANG "en"

#define AIM_CAPS_BUDDYICON      0x00000001
#define AIM_CAPS_VOICE          0x00000002
#define AIM_CAPS_IMIMAGE        0x00000004
#define AIM_CAPS_CHAT           0x00000008
#define AIM_CAPS_GETFILE        0x00000010
#define AIM_CAPS_SENDFILE       0x00000020
#define AIM_CAPS_GAMES          0x00000040
#define AIM_CAPS_SAVESTOCKS     0x00000080
#define AIM_CAPS_SENDBUDDYLIST  0x00000100
#define AIM_CAPS_GAMES2         0x00000200
#define AIM_CAPS_ICQ            0x00000400
#define AIM_CAPS_APINFO			0x00000800
#define AIM_CAPS_ICQRTF			0x00001000
#define AIM_CAPS_EMPTY			0x00002000
#define AIM_CAPS_ICQSERVERRELAY 0x00004000
#define AIM_CAPS_ICQUNKNOWN     0x00008000
#define AIM_CAPS_TRILLIANCRYPT  0x00010000
#define AIM_CAPS_LAST           0x00020000

#define KOPETE_CAPS				AIM_CAPS_IMIMAGE

static const struct {
    DWORD flag;
    char data[16];
} aim_caps[] = {

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
    {AIM_CAPS_ICQ,
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

    {AIM_CAPS_ICQUNKNOWN,
     {0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8,
      0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf}},

    {AIM_CAPS_EMPTY,
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

    {AIM_CAPS_TRILLIANCRYPT,
     {0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb,
      0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00}},

    {AIM_CAPS_APINFO,
     {0xAA, 0x4A, 0x32, 0xB5,
      0xF8, 0x84,
      0x48, 0xc6,
      0xA3, 0xD7,
      0x8C, 0x50, 0x97, 0x19, 0xFD, 0x5B}},

    {AIM_CAPS_LAST,
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
};

static const char *msgerrreason[] = {
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

OscarSocket::OscarSocket(const QString &connName, QObject *parent, const char *name)
    : OscarConnection(connName, CONN_TYPE_SERVER, parent,name)
{
    //connect(this, SIGNAL(readyRead()), this, SLOT(slotRead()));
    connect(this, SIGNAL(connectionClosed()), this, SLOT(OnConnectionClosed()));
    connect(this, SIGNAL(serverReady()), this, SLOT(OnServerReady()));
//    connect(this, SIGNAL(gotBOSRights(WORD,WORD)), this, SLOT(OnGotBOSRights(WORD,WORD)));
//    connect(this, SIGNAL(gotConfig(TAimConfig)), this, SLOT(OnGotConfig(TAimConfig)));
    key = NULL;
    cookie = NULL;
    idle = false;
//    tmpSocket = NULL;
    rateClasses.setAutoDelete(TRUE);
    pendingDirect.setAutoDelete(TRUE);
    sockets.setAutoDelete(TRUE);
    myUserProfile = "Visit the Kopete website at <a href=""http://kopete.kde.org"">http://kopete.kde.org</a>";
    isConnected = false;
}

OscarSocket::~OscarSocket(void)
{
    rateClasses.clear();
}

/** This is called when a connection is established */
void OscarSocket::OnConnect(void)
{
    QString tmp = QString("Connected to " + peerName() + ", port %1").arg(peerPort());
    kdDebug() << "[OSCAR][OnConnect] Connected to " << peerName() << ", port " << peerPort() << endl;
    serverSocket = new OncomingSocket(this,&sockets,address(),DIRECTIM_PORT);
    kdDebug() << "[OSCAR] address() is " << address().toString() << " serverSocket->address() is " << serverSocket->address().toString() << endl;
    emit connectionChanged(1,tmp);
}

/** This function is called when there is data to be read */
void OscarSocket::slotRead(void)
{
	FLAP fl = getFLAP();
	char *buf = new char[fl.length];
	Buffer inbuf;

	if (bytesAvailable() < fl.length)
	{
		while (waitForMore(500) < fl.length)
			kdDebug() << "[OSCAR][OnRead()] not enough data read yet... waiting" << endl;
	}

	int bytesread = readBlock(buf,fl.length);
	if (bytesAvailable())
		emit readyRead(); //there is another packet waiting to be read

	inbuf.setBuf(buf,bytesread);

	//kdDebug() << "[OSCAR] Input: " << endl;
	//inbuf.print();
	if(hasDebugDialog()){
			debugDialog()->addMessageFromServer(inbuf.toString(),connectionName());
	}
  
	switch(fl.channel)
	{
		case 0x01: //new connection negotiation channel
			DWORD flapversion;
			flapversion = inbuf.getDWord();
			if (flapversion == 0x00000001)
			{
				emit connAckReceived();
			}
			else
			{
				kdDebug() << "[OSCAR][OnRead()] could not read flapversion on channel 0x01" << endl;
				return;
			}
			break;

		case 0x02: //snac data channel */
			SNAC s;
			s = inbuf.getSnacHeader();
			//printf("Snac family is %x, subtype is %x, flags are %x, id is %x\n",s.family,s.subtype,s.flags,s.id);
			switch(s.family)
			{
				case 0x0001: //generic service controls
					switch(s.subtype)
					{
					case 0x0001:  //error
#ifdef OSCAR_PACKETLOG
							kdDebug() << "[OSCAR] Generic service error.. remaining data is:" << endl;
							inbuf.print();
#endif
							emit protocolError(i18n("An unknown error occured. Please report this to the Kopete development team by visiting http://kopete.kde.org. The error message was: \"Generic service error: 0x0001/0x0001\""), 0);
							break;
					case 0x0003: //server ready
							parseServerReady(inbuf);
							break;
					case 0x0005: //redirect
							parseRedirect(inbuf);
							break;
					case 0x0007: //rate info request response
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
							kdDebug() << "[OSCAR] Error: unknown family/subtype " << s.family << "/" << s.subtype << endl;
					};
					break;
			case 0x0002: //locate service
					switch(s.subtype) {
					case 0x0003: //locate rights
							parseLocateRights(inbuf);
							break;
					case 0x0006: //user profile
							parseUserProfile(inbuf);
							break;
					default: //invalid subtype
							kdDebug() << "[OSCAR] Error: unknown subtype " << s.family << "/" << s.subtype << endl;
					};
					break;
			case 0x0003: //buddy services
					switch(s.subtype) {
					case 0x0003: //buddy list rights
							parseBuddyRights(inbuf);
							break;
					case 0x000b: //buddy changed status
							parseBuddyChange(inbuf);
							break;
					case 0x000c: //offgoing buddy
							parseOffgoingBuddy(inbuf);
							break;
					default: //invalid subtype
							kdDebug() << "[OSCAR] Error: unknown subtype " << s.family << "/" << s.subtype << endl;
					};
					break;
			case 0x0004: //msg services
					switch(s.subtype) {
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
							kdDebug() << "[OSCAR] Error: unknown subtype " << s.family << "/" << s.subtype << endl;
					};
					break;
			case 0x0009: //bos service
					switch(s.subtype)
					{
					case 0x0003: //bos rights incoming
							parseBOSRights(inbuf);
							break;
					};
					break;
			case 0x0013: //buddy list management
					switch(s.subtype) {
					case 0x0003: //ssi rights
							parseSSIRights(inbuf);
							break;
					case 0x0006: //buddy list
							parseSSIData(inbuf);
							break;
					case 0x000e: //server ack
							parseSSIAck(inbuf);
							break;
					default:
							kdDebug() << "[OSCAR] Error: unknown family/subtype " << s.family << "/" << s.subtype << endl;
					};
					break;        
			case 0x0017: //authorization family
					switch(s.subtype) {
					case 0x0003: //authorization response (and hash) is being sent
							parseAuthResponse(inbuf);
							break;
					case 0x0007: //encryption key is being sent
							parsePasswordKey(inbuf);
							break;
					default: //unknown subtype
							kdDebug() << "[OSCAR] Error: unknown family/subtype " << s.family << "/" << s.subtype << endl;
					};
					break;
			default: //unknown family
					kdDebug() << "[OSCAR] Error: unknown family " << s.family << "/" << s.subtype << endl; 
			};
			break;
			//		case 0x03: //FLAP error channel
			//			break;
			//		case 0x04: //close connection negotiation channel
			//			break;

		default: //oh, crap, something's wrong
		{
			kdDebug() << "[OSCAR] Error: channel " << fl.channel << " does not exist" << endl;
#ifdef OSCAR_PACKETLOG
			kdDebug() << "Input: " << endl;
			inbuf.print();
#endif
		}
	} // END switch(fl.channel)
	delete buf; 
}

/** Sends an authorization request to the server */
void OscarSocket::sendLoginRequest(void)
{
    Buffer outbuf;
    outbuf.addSnac(0x0017,0x0006,0x0000,0x00000000);
    outbuf.addTLV(0x0001,sn.length(),sn.latin1());
    sendBuf(outbuf,0x02);
    emit connectionChanged(2,QString("Requesting login for " + sn + "..."));
}

/** encodes a password, outputs to the 3rd parameter */
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

/** adds the flap version to the buffer */
void OscarSocket::putFlapVer(Buffer &outbuf)
{
    outbuf.addDWord(0x00000001);
}

/** Called when a connection has been closed */
void OscarSocket::OnConnectionClosed(void)
{
    emit statusChanged(OSCAR_OFFLINE);
    kdDebug() << "[OSCAR] Connection closed by server" << endl;
    rateClasses.clear();
    isConnected = false;
}

/** Called when the server aknowledges the connection */
void OscarSocket::OnConnAckReceived(void)
{
    kdDebug() << "[OSCAR] OnConnAckReceived(): Sending flap version to server" << endl;
    Buffer outbuf;
    putFlapVer(outbuf);
    sendBuf(outbuf,0x01);
    sendLoginRequest();
}

/** Sends the output buffer, and clears it */
void OscarSocket::sendBuf(Buffer &outbuf, BYTE chan)
{

		//kdDebug() << "[OSCAR] Output: " << endl;
		//outbuf.print();
		if(hasDebugDialog()){
				debugDialog()->addMessageFromClient(outbuf.toString(), connectionName());
		}
		
		outbuf.addFlap(chan);
		writeBlock(outbuf.getBuf(),outbuf.getLength());
		outbuf.clear();
}

/** Logs in the user! */
void OscarSocket::doLogin(const QString &host, int port, const QString &s, const QString &password)
{
    disconnect(this, SIGNAL(connAckReceived()), this, SLOT(OnBosConnAckReceived()));
    connect(this, SIGNAL(connAckReceived()), this, SLOT(OnConnAckReceived()));
    disconnect(this, SIGNAL(connected()), this, SLOT(OnBosConnect()));
    connect(this, SIGNAL(connected()), this, SLOT(OnConnect()));
    sn = s;
    pass = password;
    kdDebug() << "[OSCAR] Connecting to " << host << ", port " << port << endl;
    connectToHost(host,port);
}

/** The program does this when a key is received */
void OscarSocket::parsePasswordKey(Buffer &inbuf)
{
	kdDebug() << "[OSCAR] Got the key" << endl;;
	WORD keylen;
	keylen = inbuf.getWord();
	if (key)
		delete key;
	key = inbuf.getBlock(keylen);
  sendLogin();
}

/** Sends login information, actually logs onto the server */
void OscarSocket::sendLogin(void)
{
    kdDebug() << "[OSCAR] Sending login info..." << endl;;
    unsigned char digest[16];
    digest[16] = '\0';  //do this so that addTLV sees a NULL-terminator
    Buffer outbuf;
    outbuf.addSnac(0x0017,0x0002,0x0000,0x00000000);
    outbuf.addTLV(0x0001,sn.length(),sn.latin1());
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
    //printf("Outbuf length before flap is: %d\n",outbuf.getLength());
    sendBuf(outbuf,0x02);
    kdDebug() << "[OSCAR] sendLogin emitting connectionChanged" << endl;
    emit connectionChanged(3,"Sending username and password...");
}

/** Called when a cookie is received */
void OscarSocket::connectToBos(void)
{
    kdDebug() << "[OSCAR] Cookie received!... preparing to connect to BOS server" << endl;
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
    kdDebug() << "[OSCAR] Bos server ack'ed us!  Sending auth cookie" << endl;
    sendCookie();
    emit connectionChanged(5,"Connected to server, authorizing...");
}

/** Sends the authorization cookie to the BOS server */
void OscarSocket::sendCookie(void)
{
    Buffer outbuf;
    putFlapVer(outbuf);
    outbuf.addTLV(0x0006,cookielen, cookie);
    sendBuf(outbuf,0x01);
}

/** Called when the server is ready for normal commands */
void OscarSocket::OnServerReady(void)
{
    emit connectionChanged(6,"Authorization successful, getting info from server");
}

/** Gets the rate info from the server */
void OscarSocket::sendRateInfoRequest(void)
{
    kdDebug() << "[OSCAR] Sending rate info request packet!" << endl;
    Buffer outbuf;
    outbuf.addSnac(0x0001,0x0006,0x0000,0x00000006);
    sendBuf(outbuf,0x02);
}

/** Parses the rate info response */
void OscarSocket::parseRateInfoResponse(Buffer &inbuf)
{
	kdDebug() << "[OSCAR] Parsing Rate Info Response" << endl;
	RateClass *rc = NULL;
	WORD numclasses = inbuf.getWord();
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

	kdDebug() << "[OSCAR] The buffer is " << inbuf.getLength() << " bytes long after reading the classes." << endl;
#ifdef OSCAR_PACKETLOG
	kdDebug() << "[OSCAR] It looks like this: " << endl;
	inbuf.print();
#endif

	//now here come the members of each class
	for (unsigned int i=0;i<numclasses;i++)
		{
				WORD classid = inbuf.getWord();
				WORD count = inbuf.getWord();
				kdDebug() << "[OSCAR] Classid: " << classid << ", Count: " << count << endl;
				RateClass *tmp;
				for (tmp=rateClasses.first();tmp;tmp=rateClasses.next())  //find the class we're talking about
						if (tmp->classid == classid)
						{
								rc = tmp;
								break;
						}
				for (WORD j=0;j<count;j++)
				{
						SnacPair *s = new SnacPair;
						s->group = inbuf.getWord();
						s->type = inbuf.getWord();
						if (rc)
								rc->members.append(s);
				}
		}
    kdDebug() << "[OSCAR] The buffer is " << inbuf.getLength() << " bytes long after reading the rate info" << endl;
    sendRateAck();
}

/** Tells the server we accept it's communist rate limits, even though I have no idea what they mean */
void OscarSocket::sendRateAck()
{
    kdDebug() << "[OSCAR] Sending rate ack" << endl;
    emit connectionChanged(7,"Completing login...");
    Buffer outbuf;
    outbuf.addSnac(0x0001,0x0008,0x0000,0x00000008);
    for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
    {
			if (rc->classid != 0x0015) //0x0015 is ICQ
				outbuf.addWord(rc->classid);
		}
    sendBuf(outbuf,0x02);
    requestInfo();
}

/** Called on connection to bos server */
void OscarSocket::OnBosConnect()
{
    kdDebug() << "[OSCAR][OnConnect]: Connected to " << peerName() << ", port " << peerPort() << endl;
}

/** Sends privacy flags to the server  */
void OscarSocket::sendPrivacyFlags(void)
{
    Buffer outbuf;
    outbuf.addSnac(0x0001, 0x0014, 0x0000, 0x00000000);
    //bit 1: let others see idle time
    //bit 2: let other see member since
    outbuf.addDWord(0x00000003);
    sendBuf(outbuf,0x02);
}

/** requests the current user's info */
void OscarSocket::requestMyUserInfo()
{
    Buffer outbuf;
    outbuf.addSnac(0x0001, 0x000e, 0x0000, 0x00000000);
    sendBuf(outbuf,0x02);
}

/** parse my user info */
void OscarSocket::parseMyUserInfo(Buffer &inbuf)
{
    kdDebug() << "[OSCAR] Parsing my user info" << endl;
    UserInfo u = parseUserInfo(inbuf);
    emit gotMyUserInfo(u);
}

/** parse the server's authorization response (which hopefully contains the cookie) */
void OscarSocket::parseAuthResponse(Buffer &inbuf)
{
    QList<TLV> lst = inbuf.getTLVList();
    lst.setAutoDelete(TRUE);
    TLV *sn = findTLV(lst,0x0001);  //screen name
    TLV *url = findTLV(lst,0x0004);  //error url
    TLV *bosip = findTLV(lst,0x0005); //bos server address
    TLV *cook = findTLV(lst,0x0006); //authorization cookie
    TLV *email = findTLV(lst,0x0007); //the e-mail address attached to the account
    TLV *regstatus = findTLV(lst,0x0013); //whether the e-mail address is available to others
    TLV *err = findTLV(lst,0x0008); //whether an error occured
    if (cookie)
	delete cookie;
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
	    kdDebug() << "[OSCAR] server is " << bosServer << ", ip.right(index) is " << ip
		      << ", bosPort is " << bosPort << endl;
	    delete bosip->data;
	}
    if (cook)
	{
	    cookie = cook->data;
	    cookielen = cook->length;
	    connectToBos();
	}
    if (sn)
	delete sn->data;
    if (email)
	delete email->data;
    if (regstatus)
	delete regstatus->data;
    lst.clear();
    if (url)
    	delete url->data;
}

/** finds a tlv of type typ in the list */
TLV * OscarSocket::findTLV(QList<TLV> &l, WORD typ)
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
    kdDebug() << "[OSCAR] Sending client ready! " << endl;
    Buffer outbuf;
    outbuf.addSnac(0x0001,0x0002,0x0000,0x00000002);
    for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
    {
			if (rc->classid != 0x0015) //0x0015 is ICQ
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
					outbuf.addWord(0x059b);
				}
			}
		}
/*  outbuf.addWord(0x0001);
		outbuf.addWord(0x0003);
		outbuf.addWord(0x0004);
    outbuf.addWord(0x0686);
    outbuf.addWord(0x0002);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0003);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0009);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x000a);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x000b);
    outbuf.addWord(0x0001);
    outbuf.addWord(0x0004);
    outbuf.addWord(0x0001); */
    sendBuf(outbuf,0x02);
    emit statusChanged(OSCAR_ONLINE);
    TAimConfig cnf;
    cnf.revision = 0;
    isConnected = true;
    //sendBuddyListRequest(cnf);
}

/** Sends versions so that we get proper rate info */
void OscarSocket::sendVersions(const WORD *families, const int len)
{
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
    QList<TLV> ql = inbuf.getTLVList();
    ql.setAutoDelete(TRUE);
    kdDebug() << "[OSCAR][parseMemRequest] requested offset " << offset
	      << ", length " << len << endl;
    if (len == 0)
	{
	    kdDebug() << "[OSCAR] Length is 0, hashing null!" << endl;
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
    kdDebug() << "[OSCAR] Setting idle time to " << time << endl;
    bool newidle;
    if (time == 0)
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
void OscarSocket::sendBuddyListRequest(const TAimConfig &a)
{
    kdDebug() << "[OSCAR] Requesting SSI data" << endl;
    Buffer outbuf;
    outbuf.addSnac(0x0013,0x0005,0x0000,0x00000000);
    outbuf.addDWord(a.timestamp);
    outbuf.addWord(a.revision);
    sendBuf(outbuf,0x02);
}

/** parses incoming ssi data */
void OscarSocket::parseSSIData(Buffer &inbuf)
{
	int curgroup = -1;
	TAimConfig blist;
	//get fmt version
	inbuf.getByte();
	blist.revision = inbuf.getWord(); //gets the revision

	kdDebug() << "[OSCAR] Receiving buddy list: revision " << blist.revision << endl;

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

		kdDebug() << "[OSCAR] Read server-side list-entry. name: " << ssi->name << ", gid: " << ssi->gid
				<< ", bid: " << ssi->bid << ", type: " << ssi->type << ", tbslen: " << ssi->tlvlength
				<< endl;

		TBuddy *bud;
		switch (ssi->type)
		{
			case 0x0000: //buddy
			{
				bud = new TBuddy;
				bud->name = ssi->name;
				bud->group = curgroup;
				bud->status = OSCAR_OFFLINE;
				kdDebug() << "[OSCAR] Adding Buddy " << ssi->name <<  " to group " << curgroup
						<< " (" << blist.buddyList.getNameGroup(curgroup) << ")" << endl;
				blist.buddyList.add(bud);
				break;
			}

			case 0x0001: //group
			{
				if (namelen) //if it's not the master group
				{
					blist.buddyList.addGroup(ssi->name);
					curgroup++;
				}
				break;
			}

			case 0x0002: // TODO permit buddy
				break;

			case 0x0003: // TODO deny buddy
			{
				bud = new TBuddy;
				bud->name = ssi->name;
				kdDebug() << "[OSCAR] Adding Buddy " << ssi->name << " to deny list." << endl;
				blist.denyList.add(bud);
				emit denyAdded(ssi->name);
				break;
			}

			case 0x0004: // TODO permit-deny setting
				break;
		} // END switch (ssi->type)

		if (name)
			delete name;
	} // END while(inbuf.getLength() > 4)

	blist.timestamp = inbuf.getDWord();
	kdDebug() << "[OSCAR] Finished getting buddy list" << endl;
	sendSSIActivate();
	emit gotConfig(blist);
	sendInfo();
}

/** Requests the user's BOS rights */
void OscarSocket::requestBOSRights(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}

/** Parses SSI rights data */
void OscarSocket::parseBOSRights(Buffer &inbuf)
{
	QList<TLV> ql = inbuf.getTLVList();
	ql.setAutoDelete(TRUE);
	TLV *t;
	WORD maxpermits = 0, maxdenies = 0;
	if ((t = findTLV(ql,0x0001))) //max permits
		maxpermits = (t->data[0] << 8) | t->data[1];
	if ((t = findTLV(ql,0x0002))) //max denies
		maxdenies = (t->data[0] << 8) | t->data[1];
	kdDebug() << "[OSCAR] Maxpermits: " << maxpermits << ", maxdenies: " << maxdenies << endl;
	ql.clear();
	sendGroupPermissionMask();
	sendPrivacyFlags();
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
	delete families;
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

/** Requests location rights */
void OscarSocket::requestLocateRights(void)
{
	Buffer buf;
	buf.addSnac(0x0002,0x0002,0x0000,0x00000002);
	sendBuf(buf,0x02);
}

/** Requests a bunch of information (permissions, rights, my user info, etc) from server */
void OscarSocket::requestInfo(void)
{
	requestMyUserInfo();
	sendSSIRightsRequest();
	sendSSIRequest();
	requestLocateRights();
	requestBuddyRights();
	requestMsgRights();
	requestBOSRights();
}

/** adds a mask of the groups that you want to be able to see you to the buffer */
void OscarSocket::sendGroupPermissionMask(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0009,0x0004,0x0000,0x00000000);
	outbuf.addDWord(0x0000001f);
	sendBuf(outbuf,0x02);
}

/** adds a request for buddy list rights to the buffer */
void OscarSocket::requestBuddyRights(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0003,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);
}

/** adds a request for msg rights to the buffer */
void OscarSocket::requestMsgRights(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0004,0x0000,0x00000004);
	sendBuf(outbuf,0x02);
}

/** Parses the locate rights provided by the server */
void OscarSocket::parseLocateRights(Buffer &/*inbuf*/)
{
	//we don't care what the locate rights are
	//and we don't know what they mean
  //  requestBuddyRights();
}

/** Parses buddy list rights from the server */
void OscarSocket::parseBuddyRights(Buffer &/*inbuf*/)
{
    //NOTE TO TOM: write code to parse buddy rights info
    //requestMsgRights();
}

/** Parses msg rights info from server */
void OscarSocket::parseMsgRights(Buffer &/*inbuf*/)
{
    //NOTE TO TOM: write code to parse this
    //requestBOSRights();
		// After we get this from the server
		// we have to send some messaging paramters
}

/** Parses an incoming IM */
void OscarSocket::parseIM(Buffer &inbuf)
{
		Buffer tmpbuf;
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
    OscarDirectConnection *s;
    WORD msgtype; //used to tell whether it is a direct IM requst, deny, or accept
    switch(channel)
		{
 		case 0x0001: //normal IM
		{
				// Flag to indicate if there are more TLV's to parse
				bool moreTLVs = true;
				// This gets set if we are notified of an auto response
				bool isAutoResponse = false;
				while( moreTLVs ){
						kdDebug() << "[OSCAR] got a normal IM block from " << u.sn << endl;
						type = inbuf.getWord();
						switch(type) {
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
								delete msg;
								kdDebug() << "[OSCAR] IM text: " << message << endl;
								emit gotIM(message,u.sn,isAutoResponse);

								// Check to see if there's anything else
								if(inbuf.getLength() > 0){
										moreTLVs = true;
								} else {
										moreTLVs = false;
								}
								break;
						}
						case 0x0004: // Away message
								// There isn't actually a message in this TLV, it just specifies
								// that the message that was send was an autoresponse
								inbuf.getWord();
								// Set the autoresponse flag
								isAutoResponse = true;

								// Check to see if there's more
								if(inbuf.getLength() > 0){
										moreTLVs = true;
								} else {
										moreTLVs = false;
								}

								break;
						case 0x0008: // User Icon
								// TODO support this
								// The length of the TLV
								length = inbuf.getWord();
								// Get the block
								/*char *msg =*/ inbuf.getBlock(length);

								// Check to see if there are more TLVs
								if(inbuf.getLength() > 0){
										moreTLVs = true;
								} else {
										moreTLVs = false;
								}

						default: //unknown type
								kdDebug() << "[OSCAR][parseIM] unknown msg tlv type " << type;
								if(inbuf.getLength() > 0){
										moreTLVs = true;
								} else {
										moreTLVs = false;
								}

						};
				}
				break;
		};
	case 0x0002: //rendezvous channel
	    kdDebug() << "[OSCAR] IM recieved on channel 2 from " << u.sn << endl;
	    tlv = inbuf.getTLV();
	    kdDebug() << "[OSCAR] The first TLV is of type " << tlv.type;
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
		    char * cook = tmpbuf.getBlock(8);
		    delete cook;
		    //the next 16 bytes are the capability block (what kind of request is this?)
		    char *cap = tmpbuf.getBlock(0x10);
		    int identified = 0;
		    DWORD capflag = 0;
		    for (int i = 0; !(aim_caps[i].flag & AIM_CAPS_LAST); i++)
			{
			    if (memcmp(&aim_caps[i].data, cap, 0x10) == 0)
				{
				    capflag |= aim_caps[i].flag;
				    identified++;
				    break; /* should only match once... */
						}
			}
						if (!identified){
								printf("unknown capability: {%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
												cap[0], cap[1], cap[2], cap[3],
												cap[4], cap[5],
												cap[6], cap[7],
												cap[8], cap[9],
												cap[10], cap[11], cap[12], cap[13],
												cap[14], cap[15]);
						}
						delete cap;
						//Next comes a big TLV chain of stuff that may or may not exist
						QList<TLV> tlvlist = tmpbuf.getTLVList();
						TLV *cur;
						tlvlist.setAutoDelete(true);
						for (cur = tlvlist.first();cur;cur = tlvlist.next())
						{
								//IP address from the perspective of the client
								if (cur->type == 0x0002)
								{
										kdDebug() << "[OSCAR] ClientIP1: " << cur->data[0] << "."
															<< cur->data[1] << "." << cur->data[2] << "."
															<< cur->data[3]  << endl;

								}
								//Secondary IP address from the perspective of the client
								else if (cur->type == 0x0003)
								{
										kdDebug() << "[OSCAR] ClientIP2: " << cur->data[0] << "."
															<< cur->data[1] << "." << cur->data[2] << "."
															<< cur->data[3] << endl;
								}
								//Verified IP address (from perspective of oscar server)
								else if (cur->type == 0x0004)
								{
										DWORD tmpaddr = 0;
										for (int i=0;i<4;i++)
										{
												tmpaddr = (tmpaddr*0x100) + static_cast<unsigned char>(cur->data[i]);
										}
										qh.setAddress(tmpaddr);
										kdDebug() << "[OSCAR] OscarIPRaw: " << cur->data[0] << "." << cur->data[1] << "."
															<< cur->data[2] << "." << cur->data[3] << endl;
										kdDebug() << "[OSCAR] OscarIP: " << qh.toString() << endl;
								}
								//Port number
								else if (cur->type == 0x0005)
								{
										remotePort = (cur->data[1] << 8) | cur->data[0];
										kdDebug() << "[OSCAR] Port# " << remotePort << endl;
								}
								//Error code
								else if (cur->type == 0x000b)
								{
										kdDebug() << "[OSCAR] ICBM ch 2 error code " <<  ((cur->data[1] << 8) | cur->data[0]) << endl;
										emit protocolError(i18n("Rendezvous with buddy failed. Please check your internet connection or try the operation again later. Error code %1.\n").arg((cur->data[1] << 8) | cur->data[0]), 0);
								}
								//Invitation message/ chat description
								else if (cur->type == 0x000c)
								{
										kdDebug() << "[OSCAR] Invited to chat " << cur->data << endl;
								}
								//Character set
								else if (cur->type == 0x000d)
								{
										kdDebug() << "[OSCAR] Using character set " << cur->data << endl;
								}
								//Language
								else if (cur->type == 0x000e)
								{
										kdDebug() << "[OSCAR] Using language " << cur->data << endl;
								}
								else
										kdDebug() << "[OSCAR] ICBM ch2: unknown tlv type " << cur->type << endl;
								delete cur->data;
						}
				}
				else
				{
						kdDebug() << "[OSCAR] Ch 2 IM: unknown TLV type " << type << endl;
				}
				s = new OscarDirectConnection(this, QString(u.sn));
				kdDebug() << "[OSCAR] Connecting to " << qh.toString() << ":" << remotePort << endl;
				s->connectToHost(qh.toString(),DIRECTIM_PORT);
				break;
		default: //unknown channel
				kdDebug() << "[OSCAR] Error: unknown ICBM channel " << channel << endl;
		}
}

/** parses the aim standard user info block */
UserInfo OscarSocket::parseUserInfo(Buffer &inbuf)
{
    UserInfo u;
    u.userclass = 0;
    u.membersince = 0;
    u.onlinesince = 0;
    u.idletime = 0;
    u.sessionlen = 0;
		//Do some sanity checking on the length of the buffer
		if(inbuf.getLength() > 0){
				BYTE len = inbuf.getByte();
				kdDebug() << "[OSCAR] Finished getting user info" << endl;

				// First comes their screen name
				char *cb = inbuf.getBlock(len);
				u.sn = cb;

				// Next comes the warning level
				//for some reason aol multiplies the warning level by 10
				u.evil = inbuf.getWord() / 10;

				//the number of TLV's that follow
				WORD tlvlen = inbuf.getWord();
				kdDebug() << "[OSCAR] ScreenName length: " << len << ", sn: " << u.sn << ", evil: " << u.evil
									<< ", tlvlen: " << tlvlen << endl;
				delete cb;
				for (int i=0;i<tlvlen;i++)
				{
						TLV t = inbuf.getTLV();
						switch(t.type) {
						case 0x0001: //user class
								u.userclass = (((BYTE)t.data[0] << 8)) | ((BYTE)t.data[1]);
								break;
						case 0x0002: //member since
								u.membersince = (DWORD) (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
										| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
								break;
						case 0x0003: //online since
								kdDebug() << "t.data: " << static_cast<uint>((BYTE)t.data[0]) << "\t" << static_cast<uint>((BYTE)t.data[1])  << "\t" << static_cast<uint>((BYTE)t.data[2]) << "\t" << static_cast<uint>((BYTE)t.data[3]) << endl;
								kdDebug() << "u.onlinesince: " << u.onlinesince << endl;
								u.onlinesince = (DWORD) (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
										| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
								break;
						case 0x0004: //idle time
								u.idletime = (WORD) ((((BYTE)t.data[0]) << 8) | ((BYTE)t.data[1]));
								break;
								//case 0x000d: //capability info

								//break;
						case 0x000f: //session length (in seconds)
								u.sessionlen = (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
										| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
								break;
						case 0x0010: //session length (for AOL users)
								u.sessionlen = (((BYTE)t.data[0]) << 24) | (((BYTE)t.data[1]) << 16)
										| (((BYTE)t.data[2]) << 8) | ((BYTE)t.data[3]);
            		break;
						default: //unknown info type
								kdDebug() << "[OSCAR][parseUserInfo] invalid tlv type " << t.type << endl;
						};
						delete t.data;

				}
				// TODO [Sept 27 2002] gives compilation warning on third argument
				// (warning: unsigned int format, long unsigned int arg (arg 3))
				kdDebug() << "[OSCAR], userclass: " << u.userclass << ", membersince: " << u.membersince
									<< ", onlinesince: " << u.onlinesince << ", idletime: " << u.idletime << endl;
				// (unsigned char)u.userclass, u.membersince,(unsigned short)u.onlinesince, u.idletime);
		} else {
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
void OscarSocket::sendIM(const QString &message, const QString &dest, bool isAuto)
{
	//check to see if we have a direct connection to the contact
	OscarConnection *dc = findConnection(dest);
	if (dc)
	{
		dc->sendIM(message,dest,isAuto);
		return;
	}
    kdDebug() << "[OSCAR] Sending " << message << " to " << dest << endl;
    static const char deffeatures[] = {
	0x01, 0x01, 0x01, 0x02
    };
    Buffer outbuf;
    outbuf.addSnac(0x0004,0x0006,0x0000,0x00000000);
    //generate random message cookie
    for (int i=0;i<8;i++)
				outbuf.addByte( (BYTE) rand());
    //add the channel ID
    outbuf.addWord(0x0001);
    //dest sn length
    outbuf.addByte(dest.length());
    //dest sn
    outbuf.addString(dest.latin1(),dest.length());
    //message TLV (type 2)
    outbuf.addWord(0x0002);
    int tlvlen = 0;
    tlvlen += 2; // 0501
    tlvlen += 2 + sizeof(deffeatures);
    tlvlen += 2; // 0101
    tlvlen += 2; // block length
    tlvlen += 4; //charset
    tlvlen += message.length();
    outbuf.addWord(tlvlen);
    outbuf.addWord(0x0501);
    //add deffeatures
    outbuf.addWord(sizeof(deffeatures));
    outbuf.addString(deffeatures, sizeof(deffeatures));
    //add 0x0101
    outbuf.addWord(0x0101);
    // add message length
    outbuf.addWord(message.length() + 0x04);
    // normal char set
    outbuf.addDWord(0x00000000);
    // the actual message
    outbuf.addString(message.local8Bit(),message.length());

    //NOTE TO TOM: there are a lot of other options that can go here
    // IMPLEMENT THEM!
    if ( isAuto )                  
		{
			outbuf.addWord(0x0004);
			outbuf.addWord(0x0000);
		}
		sendBuf(outbuf,0x02);
}

/** Activates the SSI list on the server */
void OscarSocket::sendSSIActivate(void)
{
    Buffer outbuf;
    outbuf.addSnac(0x0013,0x0007,0x0000,0x00000000);
    kdDebug() << "[OSCAR] Sending SSI ACtivate!" << endl;
    sendBuf(outbuf,0x02);
}

/** Parses the oncoming buddy server notification */
void OscarSocket::parseBuddyChange(Buffer &inbuf)
{
    UserInfo u = parseUserInfo(inbuf);
    kdDebug() << "[OSCAR] Got an oncoming buddy, ScreenName: " << u.sn << endl;
    emit gotBuddyChange(u);
}

/** Parses offgoing buddy message from server */
void OscarSocket::parseOffgoingBuddy(Buffer &inbuf)
{
    UserInfo u = parseUserInfo(inbuf);
    kdDebug() << "[OSCAR] A Buddy left :-(" << endl;
    emit gotOffgoingBuddy(u.sn);
}

/** Requests sn's user info */
void OscarSocket::sendUserProfileRequest(const QString &sn)
{
    Buffer outbuf;
    outbuf.addSnac(0x0002, 0x0005, 0x0000, 0x00000000);
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
  QList<TLV> tl = inbuf.getTLVList();
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
  kdDebug() << "ONLINE SINCE TIME IS " << u.onlinesince << endl;
  qdt.setTime_t(static_cast<uint>(u.onlinesince));
  profile += "Online Since: <B>" + qdt.toString() + "</B><br>\n";
  profile += QString("Idle Minutes: <B>%1</B><br>\n<hr><br>").arg(u.idletime);
  QString away, prof;
  for (TLV *cur = tl.first();cur;cur = tl.next())
	{
		switch(cur->type)
		{
				case 0x0001: //profile text encoding
						kdDebug() << "[OSCAR] text encoding is: " << cur->data << endl;
						break;
				case 0x0002: //profile text
						kdDebug() << "[OSCAR] The profile is: " << cur->data << endl;
						prof += cur->data;
						break;
				case 0x0003: //away message encoding
						kdDebug() << "[OSCAR] Away message encoding is: " << cur->data << endl;
						break;
				case 0x0004: //away message
						kdDebug() << "[OSCAR] Away message is: " << cur->data << endl;
						away += cur->data;
						break;
				case 0x0005: //capabilities
						kdDebug() << "[OSCAR] Got capabilities" << endl;
						break;
				default: //unknown
						kdDebug() << "[OSCAR] Unknown user info type " << cur->type << endl;
						break;
		};
		delete cur->data;
	}
  if (away.length())
		profile += "<B>Away Message:</B><br>" + away + "<br><hr>";
  if (prof.length())
		profile += prof;
  else
		profile += "<I>No user information provided</I>";
  tl.clear();
  profile += "<br><hr><I>Legend:</I><br><br><IMG SRC=\"free_icon.png\">: Normal AIM User<br> \
		<IMG SRC=\"aol_icon.png\">: AOL User<br><IMG SRC=\"dt_icon.png\">: Trial AIM User <br> \
		<IMG SRC=\"admin_icon.png\">: Administrator</HTML>";
  emit gotUserProfile(u,profile);
}

/** Sets the away message, makes user away */
void OscarSocket::sendAway(int, const QString &message)
{
    static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
    Buffer outbuf;
    outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);
    if (message.length()) //make sure there actually is an away message there
	{
	    outbuf.addTLV(0x0003,defencoding.length(),defencoding.latin1());
	    outbuf.addTLV(0x0004,message.length(),message.local8Bit());
	    emit statusChanged(OSCAR_AWAY);
	}
    else
	{
	    outbuf.addTLV(0x0004,0,""); //if we send it a tlv with length 0, we become unaway
	    emit statusChanged(OSCAR_ONLINE);
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
    /* This does not work :-( */
    Buffer outbuf;
    kdDebug() << "[OSCAR] Changing password from " << oldpw << " to " << newpw << endl;
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
    kdDebug() << "[OSCAR] Send chat join thingie (That's a technical term)" << endl;
}

/** Handles a redirect */
void OscarSocket::parseRedirect(Buffer &inbuf)
{
    kdDebug() << "[OSCAR] Parsing redirect" << endl;
    OscarConnection *servsock = new OscarConnection("Redirect",CONN_TYPE_REDIRECT);
    QList<TLV> tl = inbuf.getTLVList();
    int n;
    QString host;
    tl.setAutoDelete(true);
    if (!findTLV(tl,0x0006) && !findTLV(tl,0x0005) && !findTLV(tl,0x000e))
    {
    	tl.clear();
     	emit protocolError(i18n("An unknown error occured. Please check your internet connection. The error message was: \"Not enough information found in server redirect\""), 0);
      return;
    }
    for (TLV *tmp = tl.first(); tmp; tmp = tl.next())
    {
			switch (tmp->type)
			{
			case 0x0006: //auth cookie
		    for (int i=0;i<tmp->length;i++)
					//servsock->mAuthCookie[i] = tmp->data[i];
		    	break;
			case 0x000d: //service type
		    //servsock->mConnType = (tmp->data[1] << 8) | tmp->data[0];
		    break;
			case 0x0005: //new ip & port
		    host = tmp->data;
		    n = host.find(':');
		    if (n != -1)
				{
			    //servsock->mHost = host.left(n);
			    //servsock->mPort = host.right(n).toInt();
				}
		    else
				{
			    //servsock->mHost = host;
			    //servsock->mPort = peerPort();
				}
		    //kdDebug() << "[OSCAR] Set host to " << servsock->mHost << ", port to " << servsock->mPort << endl;
		    break;
		default: //unknown
		    kdDebug() << "[OSCAR] Unknown tlv type in parseredirect: " << tmp->type << endl;
		    break;
		}
	  delete tmp->data;
	}
	tl.clear();
	sockets.append(servsock);
	kdDebug() << "[OSCAR] Socket added to connection list!" << endl;
}

/** Request a direct IM session with someone
	type == 0: request
	type == 1: deny
	type == 2: accept */
void OscarSocket::sendDirectIMRequest(const QString &sn)
{
	sendDirectIMInit(sn,0x0000);
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
    delete sn;
    delete ck;
    emit gotAck(nm,typ);
}

// Parses a minityping notification from the server
void OscarSocket::parseMiniTypeNotify(Buffer &inbuf)
{
		//TODO
		// Throw away 8 bytes which are all zeros
		inbuf.getDWord();
		inbuf.getDWord();
		// Throw away two bytes (0x0001) which are always there
		inbuf.getWord();
		// The length of the screen name
		int snlen = inbuf.getByte();
		kdDebug() << "Trying to find username of length: " << snlen << endl;
		// The screen name
		char *sn = inbuf.getBlock(snlen);
		QString screenName = sn;
		delete sn;
		// Get the actual notification
		WORD notification = inbuf.getWord();
		// DEBUG STATEMENT
		kdDebug() << "[OSCAR] Determining Minitype from user "
							<< screenName << endl;
		
		switch(notification){
		case 0x0000:
				emit gotMiniTypeNotification(screenName, 0);
				break;
		case 0x0001:
				// Text Typed
				emit gotMiniTypeNotification(screenName, 1);
				break;
		case 0x0002:
				// Typing Begun
				emit gotMiniTypeNotification(screenName, 2);
				break;
		default:
				kdDebug() << "[OSCAR] MiniType Error: " << notification << endl;
		}
				
}

/** Sends our capabilities to the server */
void OscarSocket::sendCapabilities(unsigned long caps)
{
    Buffer outbuf;
    outbuf.addSnac(0x0002,0x0004,0x0000,0x00000000);
    int sz = 0;
    for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
	if (aim_caps[i].flag & caps)
	    sz += 16;
    kdDebug() << "[OSCAR] Sending capabilities.. size " << sz << endl;
    //TLV (type 5)
    outbuf.addWord(0x0005);
    outbuf.addWord(sz);
    for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
	if (aim_caps[i].flag & caps)
	    outbuf.addString(aim_caps[i].data,16);
    sendBuf(outbuf,0x02);
}

/** Parses a rate change */
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

/** Signs the user off */
void OscarSocket::doLogoff()
{
    Buffer outbuf;
    kdDebug() << "[OSCAR] Sending sign off request" << endl;
    sendBuf(outbuf,0x04);
}

/** Adds a buddy to the server side buddy list */
void OscarSocket::sendAddBuddy(const QString &name, const QString &group)
{
    kdDebug() << "[OSCAR] Sending add buddy" << endl;
    SSI *newitem = ssiData.addBuddy(name,group);
    if (!newitem)
	{
	    sendAddGroup(group);
	    newitem = ssiData.addBuddy(name,group);
	}
    kdDebug() << "[OSCAR] Adding " << newitem->name << ", gid " << newitem->gid
	      << ", bid " << newitem->bid << ", type " << newitem->type
	      << ", datalength " << newitem->tlvlength << endl;
    sendSSIAddModDel(newitem,0x0008);
    //now we need to modify the group our buddy is in
}

/** Adds a group to the server side buddy list */
void OscarSocket::sendAddGroup(const QString &name)
{
    kdDebug() << "[OSCAR] Sending add group" << endl;
    SSI *newitem = ssiData.addGroup(name);
    kdDebug() << "[OSCAR] Adding group gid " << newitem->gid << endl;
    sendSSIAddModDel(newitem,0x0008);
}

/** Sends SSI add, modify, or delete request, to reuse code */
void OscarSocket::sendSSIAddModDel(SSI *item, WORD request_type)
{
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

/** Parses the SSI acknowledgement */
void OscarSocket::parseSSIAck(Buffer &/*inbuf*/)
{
    //there isn't much here...
    //we just need to signal to send the next item in the ssi queue
    emit SSIAck();
}

/** Deletes a buddy from the server side contact list */
void OscarSocket::sendDelBuddy(const QString &budName, const QString &budGroup)
{
	kdDebug() << "[OSCAR] Sending del buddy" << endl;
	SSI *delitem = ssiData.findBuddy(budName,budGroup);
	ssiData.print();
	if (!delitem)
	{
		kdDebug() << "[OSCAR] Item with name " << budName << " and group "
			<< budGroup << "not found" << endl;
		emit protocolError(budName + " in group " + budGroup + " was not found on the server's buddy list and cannot be deleted.",0);
		return;
	}

	kdDebug() << "[OSCAR] Deleting " << delitem->name << ", gid " << delitem->gid
			<< ", bid " << delitem->bid << ", type " << delitem->type
			<< ", datalength " << delitem->tlvlength << endl;

	sendSSIAddModDel(delitem,0x000a);

	if (!ssiData.remove(delitem))
		kdDebug() << "[OSCAR][sendDelBuddy] delitem was not found in the SSI list" << endl;
}

/** Parses a warning notification */
void OscarSocket::parseWarningNotify(Buffer &inbuf)
{
	int newevil = inbuf.getWord() / 10; //aol multiplies warning % by 10, don't know why
	kdDebug() << "[OSCAR} Got a warning: new warning level is " << newevil << endl;
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
	QString msg = "Your message did not get sent: ";
	WORD reason = inbuf.getWord();
	kdDebug() << "[OSCAR] Got an error: " << QTextStream::hex << reason << endl;
	if (reason < msgerrreasonlen)
		msg += msgerrreason[reason];
	else
		msg += "Unknown reason.";
	emit protocolError(msg,reason);
}

/** Request, deny, or accept a direct IM session with someone
type == 0: request
type == 1: deny
type == 2: accept  */
void OscarSocket::sendDirectIMInit(const QString &sn, WORD type)
{
    Buffer outbuf;
    outbuf.addSnac(0x0004,0x0006,0x0000,0x00000000);
    char ck[8];
    //generate a random message cookie
    for (int i=0;i<8;i++)
		{
	    ck[i] = static_cast<BYTE>(rand());
		}
		//add this to the list of pending connections if it is a request
		if ( type == 0 )
		{
			DirectInfo *dinfo = new DirectInfo;
			dinfo->sn = sn;
			for (int i=0;i<8;i++)
				dinfo->cookie[i] = ck[i];
			pendingDirect.append(dinfo);
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
    outbuf.addWord(2+8+16+6+8+6+4);
    outbuf.addWord(type); //2
    outbuf.addString(ck,8); //8
    for (int i=0;aim_caps[i].flag != AIM_CAPS_LAST;i++)
		{
	    if (aim_caps[i].flag & AIM_CAPS_IMIMAGE)
			{
		    outbuf.addString(aim_caps[i].data,0x10);
		    break;
			}
		} //16
    //TLV (type a)
    outbuf.addWord(0x000a);
    outbuf.addWord(0x0002);
    outbuf.addWord(0x0001); //6
    //TLV (type 3)
    outbuf.addWord(0x0003);
    outbuf.addWord(0x0004);
    if (!serverSocket->ok()) //make sure the socket stuff is properly set up
    {
    	kdDebug() << "[Oscar] SERVER SOCKET NOT SET UP... returning from directiminit" << endl;
			return;
		}
    outbuf.addDWord(static_cast<DWORD>(serverSocket->address().ip4Addr())); //8
    //TLV (type 5)
    outbuf.addWord(0x0005);
    outbuf.addWord(0x0002);
    outbuf.addWord(serverSocket->port()); //6
    //TLV (type f)
    outbuf.addTLV(0x000f,0x0000,NULL); //4

    kdDebug() << "[OSCAR] Sending direct IM, type " << type << " from " << serverSocket->address().toString() << ", port " << serverSocket->port() << endl;
    sendBuf(outbuf,0x02);
}

/** Sends a direct IM denial */
void OscarSocket::sendDirectIMDeny(const QString &sn)
{
	sendDirectIMInit(sn,0x0001);
}

/** Sends a direct IM accept */
void OscarSocket::sendDirectIMAccept(const QString &sn)
{
	sendDirectIMInit(sn,0x0002);
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
			nummissed);
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
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0002,0x0000,0x00000002);
	sendBuf(outbuf,0x02);	
}

/** Sends a 0x0013,0x0004 (requests SSI data?) */
void OscarSocket::sendSSIRequest(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0013,0x0004,0x0000,0x00020004);
	sendBuf(outbuf,0x02);
}

/** Parses a 0x0013,0x0003 (SSI rights) from the server */
void OscarSocket::parseSSIRights(Buffer &/*inbuf*/)
{
   //don't really care about this stuff...
   //maybe write code to parse it into something useful later
}

/** Sends the server lots of  information about the currently logged in user */
void OscarSocket::sendInfo(void)
{
	sendMyProfile();
	sendMsgParams();
	sendIdleTime(0);
	sendGroupPermissionMask();
	sendPrivacyFlags();
	sendCapabilities(KOPETE_CAPS);
	sendClientReady();
}

/** Sends the user's profile to the server */
void OscarSocket::sendMyProfile(void)
{
  static const QString defencoding = "text/aolrtf; charset=\"us-ascii\"";
  Buffer outbuf;
  outbuf.addSnac(0x0002,0x0004,0x0000,0x00000004);
  outbuf.addTLV(0x0001,defencoding.length(),defencoding.latin1());
  outbuf.addTLV(0x0002,myUserProfile.length(),myUserProfile.local8Bit());
  sendBuf(outbuf,0x02);
}

/** Sends parameters for ICBM messages */
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

/** Sets the user's profile */
void OscarSocket::setMyProfile(const QString &profile)
{
	myUserProfile = profile;
	if (isConnected)
		sendMyProfile();
}

/** Blocks user sname */
void OscarSocket::sendBlock(const QString &sname)
{
	kdDebug() << "[OSCAR] Sending block buddy" << endl;
  SSI *newitem = ssiData.addBlock(sname);
  if (!newitem)
	{
	   return;
	}
  kdDebug() << "[OSCAR] Adding DENY:" << newitem->name << ", gid " << newitem->gid
	      << ", bid " << newitem->bid << ", type " << newitem->type
	      << ", datalength " << newitem->tlvlength << endl;
  sendSSIAddModDel(newitem,0x0008);
  
  // NOTE TO TOM: use snac headers and SSI acks to do this more correctly
  emit denyAdded(sname);
}

/** Removes the block on user sname */
void OscarSocket::sendRemoveBlock(const QString &sname)
{
	kdDebug() << "[OSCAR] Removing block on " << sname << endl;
  SSI *delitem = ssiData.findDeny(sname);
  if (!delitem)
	{
		kdDebug() << "[OSCAR] Item with name " << sname << "not found" << endl;
	 	emit protocolError(sname + " was not found on the server's deny list and cannot be deleted.",0);
	  return;
	}
  kdDebug() << "[OSCAR] Deleting " << delitem->name << ", gid " << delitem->gid
		<< ", bid " << delitem->bid << ", type " << delitem->type
	  << ", datalength " << delitem->tlvlength << endl;
	sendSSIAddModDel(delitem,0x000a);
  if (!ssiData.remove(delitem))
		kdDebug() << "[OSCAR][sendRemoveBlock] delitem was not found in the SSI list" << endl;
	ssiData.print();

	// NOTE TO TOM: use snac headers and SSI acks to do this more correctly
	emit denyRemoved(sname);
}

/** Reads a FLAP header from the input */
FLAP OscarSocket::getFLAP(void)
{
    FLAP fl;
    int theword, theword2;
    int start;
    int chan;
		//the FLAP start byte
    if ((start = getch()) == 0x2a){
				//get the channel ID
				if ( (chan = getch()) == -1) {
						kdDebug() << "[OSCAR] Error reading channel ID: nothing to be read" << endl;
						fl.channel = 0x00;
				} else {
						fl.channel = chan;
				}

				//get the sequence number
				if((theword = getch()) == -1){
						kdDebug() << "[OSCAR] Error reading sequence number: nothing to be read" << endl;;
						fl.sequence_number = 0x00;
				} else if((theword2 = getch()) == -1){
						kdDebug() << "[OSCAR] Error reading data field length: nothing to be read" << endl;
						fl.sequence_number = 0x00;
				} else {
						// Got both pieces of info we need...
						fl.sequence_number = (theword << 8) | theword2;
				}

				//get the data field length
				if ((theword = getch()) == -1) {
						kdDebug() << "[OSCAR] Error reading sequence number: nothing to be read" << endl;
						fl.length = 0x00;
				} else if((theword2 = getch()) == -1){
						kdDebug() << "[OSCAR] Error reading data field length: nothing to be read" << endl;
						fl.length = 0x00;
				} else {
						fl.length = (theword << 8) | theword2;
				}
		} else {
				kdDebug() << "[OSCAR] Error reading FLAP... start byte is " << start << endl;
		}
    return fl;
}

void OscarSocket::sendMiniTypingNotify(QString screenName,TypingNotify notifyType ){
		//BLARG
		kdDebug() << "[OSCAR] Sending Typing notify " << endl;

		// Build the buffer
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
		switch(notifyType){
		case TypingFinished:
				outbuf.addWord(0x0000);
				break;
		case TextTyped:
				outbuf.addWord(0x0001);
				break;
		case TypingBegun:
				outbuf.addWord(0x0002);
				break;
		default:
				// Error, bad bad, ouchie
				return;
		}

		// Send it
		sendBuf(outbuf, 0x02);
}

/** Called when a direct IM is received */
void OscarSocket::OnDirectIMReceived(QString message, QString sender, bool isAuto)
{
	//for now, let's just emit a gotIM as though it came from the server
	emit gotIM(message,sender,isAuto);
}

/** Called when a direct IM connection suffers an error */
void OscarSocket::OnDirectIMError(QString errmsg, int num)
{
	//let's just emit a protocolError for now
	emit protocolError(errmsg, num);
}

/** looks for a connection named thename.  If such a connection exists, return it, otherwise, return NULL */
OscarConnection * OscarSocket::findConnection(const QString &thename)
{
	OscarConnection *tmp;
	for (tmp = sockets.first(); tmp; tmp = sockets.next())
	{
		if ( !thename.compare(tmp->connectionName()) )
		{
			return tmp;
		}
	}
	return 0L;
}


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
