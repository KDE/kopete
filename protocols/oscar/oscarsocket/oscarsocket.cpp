/***************************************************************************
            oscarsocket.cpp  -   communication with AIM oscar server
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@po.cwru.edu
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C" {
#include "md5.h"
};

#include "oscarsocket.h"
#include "oncomingsocket.h"
#include <qdatetime.h>
#include <unistd.h>
#include <stdlib.h>
#include <kdebug.h>
#define DIRECTCONNECT		0x0f1f

#define AIM_MD5_STRING "AOL Instant Messenger (SM)"
#define AIM_CLIENTSTRING "AOL Instant Messenger (SM), version 3.5.1670/WIN32"
#define AIM_CLIENTID 0x0004
#define AIM_MAJOR 0x0003
#define AIM_MINOR 0x0005
#define AIM_POINT 0x0000
#define AIM_BUILD 0x0686
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
#define AIM_CAPS_APINFO         0x00000800
#define AIM_CAPS_ICQRTF					0x00001000
#define AIM_CAPS_EMPTY					0x00002000
#define AIM_CAPS_ICQSERVERRELAY 0x00004000
#define AIM_CAPS_ICQUNKNOWN     0x00008000
#define AIM_CAPS_TRILLIANCRYPT  0x00010000
#define AIM_CAPS_LAST           0x00020000

#define KOPETE_CAPS		AIM_CAPS_IMIMAGE

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

OscarSocket::OscarSocket(QObject *parent, const char *name)
    : ProtocolSocket(parent,name)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(OnRead()));
    connect(this, SIGNAL(connectionClosed()), this, SLOT(OnConnectionClosed()));
    connect(this, SIGNAL(keyReceived()), this, SLOT(OnKeyReceived()));
    connect(this, SIGNAL(cookieReceived()), this, SLOT(OnCookieReceived()));
    connect(this, SIGNAL(serverReady()), this, SLOT(OnServerReady()));
    connect(this, SIGNAL(gotBOSRights(WORD,WORD)), this, SLOT(OnGotBOSRights(WORD,WORD)));
    connect(this, SIGNAL(gotConfig(TAimConfig)), this, SLOT(OnGotConfig(TAimConfig)));
    key = NULL;
    cookie = NULL;
    idle = false;
    tmpSocket = NULL;
    rateClasses.setAutoDelete(TRUE);
    serverSocket = new OncomingSocket(address());
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
    emit connectionChanged(1,tmp);
}

/** This function is called when there is data to be read */
void OscarSocket::OnRead(void)
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
    if (fl.length < bytesAvailable())
	emit readyRead(); //there is another packet waiting to be read
    inbuf.setBuf(buf,bytesread);
    kdDebug() << "[OSCAR] Input: " << endl;
    inbuf.print();
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
	case 0x02: //snac data channel
	    SNAC s;
	    s = inbuf.getSnacHeader();
	    //printf("Snac family is %x, subtype is %x, flags are %x, id is %x\n",s.family,s.subtype,s.flags,s.id);
	    switch(s.family) {
	    case 0x0001: //generic service controls
		switch(s.subtype) {
		case 0x0001:  //error
		    kdDebug() << "[OSCAR] Generic service error.. remaining data is:" << endl;
		    inbuf.print();
		    emit protocolError("Generic service error: 0x0001/0x0001");
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
		case 0x000b: //oncoming buddy
		    parseOncomingBuddy(inbuf);
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
		case 0x0005: //msg rights
		    parseMsgRights(inbuf);
		    break;
		case 0x0007: //incoming IM
		    parseIM(inbuf);
		    break;
		case 0x000c: //message ack
		    parseMsgAck(inbuf);
		    break;
		default: //invalid subtype
		    kdDebug() << "[OSCAR] Error: unknown subtype " << s.family << "/" << s.subtype << endl;
		};
		break;
	    case 0x0009: //bos service
		switch(s.subtype) {
		case 0x0003: //bos rights incoming
		    parseBOSRights(inbuf);
		    break;
		};
		break;
	    case 0x0013: //buddy list management
		switch(s.subtype) {
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
		    WORD keylen;
		    keylen = inbuf.getWord();
		    if (key)
			delete key;
		    key = inbuf.getBlock(keylen);
		    emit keyReceived();
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
	    kdDebug() << "[OSCAR] Error: channel " << fl.channel << " does not exist" << endl;
	    kdDebug() << "Input: " << endl;
	    inbuf.print();
	}
    delete buf;
}

/** Reads a FLAP header from the input */
FLAP OscarSocket::getFLAP(void)
{
    FLAP fl;
    int theword, theword2;
    int start;
    int chan;
    if ((start = getch()) == 0x2a) //the FLAP start byte
	{
	    if ( (chan = getch()) == -1) //get the channel ID
		kdDebug() << "[OSCAR] Error reading channel ID: nothing to be read" << endl;
	    else
		{
		    fl.channel = chan;
		}
	    if ((theword = getch()) == -1) //get the sequence number
		kdDebug() << "[OSCAR] Error reading sequence number: nothing to be read" << endl;;
	    if ((theword2 = getch()) == -1)
		kdDebug() << "[OSCAR] Error reading data field length: nothing to be read" << endl;
	    fl.sequence_number = (theword << 8) | theword2;
	    if ((theword = getch()) == -1) //get the data field length
		kdDebug() << "[OSCAR] Error reading sequence number: nothing to be read" << endl;
	    if ((theword2 = getch()) == -1)
		kdDebug() << "[OSCAR] Error reading data field length: nothing to be read" << endl;
	    fl.length = (theword << 8) | theword2;
	}
    else
	{
	    kdDebug() << "[OSCAR] Error reading FLAP... start byte is " << start << endl;
	}
    return fl;
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
    kdDebug() << "[OSCAR] Connection closed by server" << endl;
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
    kdDebug() << "[OSCAR] Output: " << endl;
    outbuf.print();
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
void OscarSocket::OnKeyReceived(void)
{
    kdDebug() << "[OSCAR] Got the key" << endl;;
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
    outbuf.addTLV(0x000e,0x0002,AIM_COUNTRY);
    outbuf.addTLV(0x000f,0x0002,AIM_LANG);
    //if set, old-style buddy lists will not work... you will need to use SSI
    outbuf.addTLV8(0x004a,0x01);
    //printf("Outbuf length before flap is: %d\n",outbuf.getLength());
    sendBuf(outbuf,0x02);
    kdDebug() << "[OSCAR] sendLogin emitting connectionChanged" << endl;
    emit connectionChanged(3,"Sending username and password...");
}

/** Called when a cookie is received */
void OscarSocket::OnCookieReceived(void)
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
    outbuf.addSnac(0x0001,0x0006,0x0000,0x00000000);
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
    kdDebug() << "[OSCAR] The buffer is " << inbuf.getLength() << " bytes long after reading the classes" << endl;;
    kdDebug() << "[OSCAR] It looks like this: " << endl;
    inbuf.print();
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
    outbuf.addSnac(0x0001,0x0008,0x0000,0x00000000);
    for (RateClass *rc=rateClasses.first();rc;rc=rateClasses.next())
	if (rc->classid<0x0005) outbuf.addWord(rc->classid);
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
    requestLocateRights();
}

/** parse the server's authorization response (which hopefully contains the cookie) */
void OscarSocket::parseAuthResponse(Buffer &inbuf)
{
    QList<TLV> lst = inbuf.getTLVList();
    lst.setAutoDelete(TRUE);
    TLV *sn = findTLV(lst,0x0001);  //screen name
    TLV *bosip = findTLV(lst,0x0005); //bos server address
    TLV *cook = findTLV(lst,0x0006); //authorization cookie
    TLV *email = findTLV(lst,0x0007); //the e-mail address attached to the account
    TLV *regstatus = findTLV(lst,0x0013); //whether the e-mail address is available to others
    TLV *err = findTLV(lst,0x0008); //whether an error occured
    if (cookie)
	delete cookie;
    if (err)
	emit protocolError(QString("Signon error %1 occured").arg((err->data[0] << 8)|err->data[1]));
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
	    emit cookieReceived();
	}
    if (sn)
	delete sn->data;
    if (email)
	delete email->data;
    if (regstatus)
	delete regstatus->data;
    lst.clear();
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
    outbuf.addSnac(0x0001,0x0002,0x0000,0x00000000);
    outbuf.addWord(0x0001);
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
    outbuf.addWord(0x0001);
    sendBuf(outbuf,0x02);
    emit online();
    TAimConfig cnf;
    cnf.revision = 0;
    sendBuddyListRequest(cnf);
}

/** Sends versions so that we get proper rate info */
void OscarSocket::sendVersions(const WORD *families, const int len)
{
    Buffer outbuf;
    outbuf.addSnac(0x0001,0x0017,0x0000,0x00000000);
    for(int i=len-1;i>=0;i--)
	{
	    outbuf.addWord(families[i]);
	    if (families[i] == 0x0001)
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
	    SSI *ssi2 = ssiData.current();
	    if (ssi->name == "CWRU")
		kdDebug() << "[OSCAR] Read CWRU: name: " << ssi2->name << ", gid: " << ssi2->gid 
			  << ", bid: " << ssi2->bid << ", type: " << ssi2->type << ", tbslen: " 
			  << ssi2->tlvlength << endl;
	    kdDebug() << "[OSCAR] Read a buddy: name: " << ssi->name << ", gid: " << ssi->gid 
		      << ", bid: " << ssi->bid << ", type: " << ssi->type << ", tbslen: " << ssi->tlvlength
		      << endl;
	    TBuddy *bud;
	    switch (ssi->type) {
	    case 0x0000: //buddy
		bud = new TBuddy;
		bud->name = ssi->name;
		bud->group = curgroup;
		bud->status = OSCAR_OFFLINE;
		kdDebug() << "[OSCAR] Adding " << ssi->name <<  "to group " << curgroup 
			  << " (" << blist.buddyList.getNameGroup(curgroup) << ")" << endl;
		blist.buddyList.add(bud);
		break;
	    case 0x0001: //group
		if (namelen) //if it's not the master group
		    {
			blist.buddyList.addGroup(ssi->name);
			curgroup++;
		    }
		break;
	    case 0x0002: // TODO permit buddy
		break;
	    case 0x0003: // TODO deny buddy
		break;
	    case 0x0004: // TODO permit-deny setting
		break;
	    };
	    if (name)
		delete name;
	}
    blist.timestamp = inbuf.getDWord();
    kdDebug() << "[OSCAR] Finished getting buddy list" << endl;
    sendSSIActivate();
    emit gotConfig(blist);
}

/** Requests the user's BOS rights */
void OscarSocket::requestBOSRights(void)
{
    Buffer outbuf;
 outbuf.addSnac(0x0009,0x0002,0x0000,0x00000000);
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
	emit gotBOSRights(maxpermits,maxdenies);
	ql.clear();
	sendGroupPermissionMask();
	sendPrivacyFlags();
}

/** Called when bos rights are received */
void OscarSocket::OnGotBOSRights(WORD /*maxperm*/, WORD /*maxdeny*/)
{
	sendClientReady();
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
		emit protocolError(QString("AOL MOTD Error: your connection may be lost. ID: %1").arg(id));
}

/** Requests location rights */
void OscarSocket::requestLocateRights(void)
{
	Buffer buf;
	buf.addSnac(0x0002,0x0002,0x0000,0x00000000);
	sendBuf(buf,0x02);
}

/** Requests a bunch of information (permissions, rights, my user info, etc) from server */
void OscarSocket::requestInfo(void)
{
	requestMyUserInfo();
	sendCapabilities(KOPETE_CAPS);
/*	requestLocateRights(outbuf);
	requestBuddyRights(outbuf);
	requestMsgRights(outbuf);
	requestBOSRights(outbuf);
	sendGroupPermissionMask(outbuf);
	sendPrivacyFlags(outbuf);
	*/
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
	outbuf.addSnac(0x0003,0x0002,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

/** adds a request for msg rights to the buffer */
void OscarSocket::requestMsgRights(void)
{
	Buffer outbuf;
	outbuf.addSnac(0x0004,0x0004,0x0000,0x00000000);
	sendBuf(outbuf,0x02);
}

/** Parses the locate rights provided by the server */
void OscarSocket::parseLocateRights(Buffer &/*inbuf*/)
{
	//we don't care what the locate rights are
	//and we don't know what they mean
    requestBuddyRights();
}

/** Parses buddy list rights from the server */
void OscarSocket::parseBuddyRights(Buffer &/*inbuf*/)
{
    //NOTE TO TOM: write code to parse buddy rights info
    requestMsgRights();
}

/** Parses msg rights info from server */
void OscarSocket::parseMsgRights(Buffer &/*inbuf*/)
{
    //NOTE TO TOM: write code to parse this
    requestBOSRights();
}

/** Parses an incoming IM */
void OscarSocket::parseIM(Buffer &inbuf)
{
    Buffer tmpbuf;
    WORD type = 0;
    WORD length = 0;
    //This is probably the hardest thing to do in oscar
    //first comes an 8 byte ICBM cookie
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
    int remotePort = 0;
    QHostAddress qh;
    QString message;
    QSocket *s = new QSocket;
    switch(channel)
	{
	case 0x0001: //normal IM
	    kdDebug() << "[OSCAR] got a normal IM from " << u.sn << endl;
	    type = inbuf.getWord();
	    length = inbuf.getWord();
	    switch(type) {
	    case 0x0002: //message block
		//first comes 0x0501 (don't know what it is)
		inbuf.getWord();
		//next comes the features length, followed by the features
		int featureslen;
		featureslen = inbuf.getWord();
		inbuf.getBlock(featureslen);
		while (inbuf.getLength() > 0)
		    {
			//then comes 0x0101 (don't know what that is either)
			inbuf.getWord();
			//length of the message
			WORD msglen = inbuf.getWord();
			//unicode encoding of the message
			/*WORD flag1 = */inbuf.getWord();
			/*WORD flag2 = */inbuf.getWord();
			msglen -= 4; //strip off the unicode info
			char *msg = inbuf.getBlock(msglen);
			message = msg;
			delete msg;
			kdDebug() << "[OSCAR] IM text: " << message << endl;
			emit gotIM(message,u.sn,false);
		    }
		break;
	    default: //unknown type
		kdDebug() << "[OSCAR][parseIM] unknown msg tlv type " << type;
	    };
	    break;
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
		    /*WORD status = */tmpbuf.getWord();
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
				    emit protocolError(QString("Rendezvous with buddy failed.  Error code %1.\n").arg((cur->data[1] << 8) | cur->data[0]));
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
	    connect(s,SIGNAL(connected()),this,SLOT(OnConnect()));
	    kdDebug() << "[OSCAR] Connecting to " << qh.toString() << ":" << remotePort << endl;
	    s->connectToHost(qh.toString(),remotePort);
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
    BYTE len = inbuf.getByte();
    char *cb = inbuf.getBlock(len);
    u.sn = cb;
    u.evil = inbuf.getWord();
    WORD tlvlen = inbuf.getWord(); //the number of TLV's that follow
    kdDebug() << "[OSCAR] ScreenName length: " << len << ", sn: " << u.sn << ", evil: " << u.evil
	      << ", tlvlen: " << tlvlen << endl;
    delete cb;
    for (int i=0;i<tlvlen;i++)
	{
	    TLV t = inbuf.getTLV();
	    switch(t.type) {
	    case 0x0001: //user class
		u.userclass = (t.data[0] << 8) | t.data[1];
		break;
	    case 0x0002: //member since
		u.membersince = (t.data[0] << 24) | (t.data[1] << 16)
		    | (t.data[2] << 8) | t.data[3];
		break;
	    case 0x0003: //online since
		u.onlinesince = (t.data[0] << 24) | (t.data[1] << 16)
		    | (t.data[2] << 8) | t.data[3];
		break;
	    case 0x0004: //idle time
		u.idletime = (WORD) ((t.data[0] << 8) | t.data[1]);
		break;
		//case 0x000d: //capability info
				
		//break;
	    case 0x000f: //session length (in seconds)
		u.sessionlen = (t.data[0] << 24) | (t.data[1] << 16)
		    | (t.data[2] << 8) | t.data[3];
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
    return u;
}

/** Sends message to dest */
void OscarSocket::sendIM(const QString &message, const QString &dest, bool isAuto)
{
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
    outbuf.addString(message.latin1(),message.length());

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
void OscarSocket::parseOncomingBuddy(Buffer &inbuf)
{
    UserInfo u = parseUserInfo(inbuf);
    kdDebug() << "[OSCAR] Got an oncoming buddy, ScreenName: " << u.sn << endl;
    emit gotOncomingBuddy(u);
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
    qdt.setTime_t(u.onlinesince);
    profile += "Online Since: <B>" + qdt.toString() + "</B><br>\n";
    profile += QString("Idle Minutes: <B>%1</B><br>\n<hr><br>").arg(u.idletime);  
    QString away, prof;
    for (TLV *cur = tl.first();cur;cur = tl.next())
	{
	    switch(cur->type) {
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
	    outbuf.addTLV(0x0004,message.length(),message.latin1());
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
    ServiceSocket *servsock = new ServiceSocket();
    QList<TLV> tl = inbuf.getTLVList();
    int n;
    QString host;
    tl.setAutoDelete(true);
    if (!findTLV(tl,0x0006) && !findTLV(tl,0x0005) && !findTLV(tl,0x000e))
	{
	    tl.clear();
	    emit protocolError("Not enough information found in server redirect\n");
	    return;
	}
    for (TLV *tmp = tl.first(); tmp; tmp = tl.next())
	{
	    switch (tmp->type)
		{
		case 0x0006: //auth cookie
		    for (int i=0;i<tmp->length;i++)
			servsock->cookie[i] = tmp->data[i];
		    break;
		case 0x000d: //service type
		    servsock->type = (tmp->data[1] << 8) | tmp->data[0];
		    break;
		case 0x0005: //new ip & port
		    host = tmp->data;
		    n = host.find(':');
		    if (n != -1)
			{
			    servsock->host = host.left(n);
			    servsock->conPort = host.right(n).toInt();
			}
		    else
			{
			    servsock->host = host;
			    servsock->conPort = peerPort();
			}
		    kdDebug() << "[OSCAR] Set host to " << servsock->host << ", port to " << servsock->conPort << endl;
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

/** Request a direct IM session with someone */
void OscarSocket::sendDirectIMRequest(const QString &sn)
{
    Buffer outbuf;
    outbuf.addSnac(0x0004,0x0006,0x0000,0x00000000);
    char ck[8];
    //generate a random message cookie
    for (int i=0;i<8;i++)
	{
	    ck[i] = static_cast<BYTE>(rand());
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
    outbuf.addWord(0x0000); //2
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
    while (!serverSocket->ok()) //make sure the socket stuff is properly set up
	usleep(100);
    outbuf.addDWord(static_cast<DWORD>(serverSocket->address().ip4Addr())); //8
    //TLV (type 5)
    outbuf.addWord(0x0005);
    outbuf.addWord(0x0002);
    outbuf.addWord(serverSocket->port()); //6
    //TLV (type f)
    outbuf.addTLV(0x000f,0x0000,NULL); //4

    kdDebug() << "[OSCAR] Sending direct IM request..." << endl;
    sendBuf(outbuf,0x02);
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
	    return;
	}
    kdDebug() << "[OSCAR] Deleting " << delitem->name << ", gid " << delitem->gid
	      << ", bid " << delitem->bid << ", type " << delitem->type
	      << ", datalength " << delitem->tlvlength << endl;
    sendSSIAddModDel(delitem,0x000a);
    if (!ssiData.remove(delitem))
	kdDebug() << "[OSCAR][sendDelBuddy] delitem was not found in the SSI list" << endl;
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
