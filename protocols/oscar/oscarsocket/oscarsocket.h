/*
    oscarsocket.h  -  Oscar Protocol Implementation

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

#ifndef OSCARSOCKET_H
#define OSCARSOCKET_H

#include "oscardirectconnection.h"
#include "ssidata.h"
#include "aimbuddylist.h"
#include "oncomingsocket.h"

#include <qptrlist.h>

class KFileItem;
class OscarAccount;
class QTimer;

struct FLAP
{ //flap header
	 BYTE channel;
	 WORD sequence_number;
	 WORD length;
	 bool error;
};

struct SnacPair
{ //just a group+type pair
	 WORD group;
	 WORD type;
};

struct RateClass
{ //rate info
	 WORD classid;
	 DWORD windowsize;
	 DWORD clear;
	 DWORD alert;
	 DWORD limit;
	 DWORD disconnect;
	 DWORD current;
	 DWORD max;
	 BYTE unknown[5];
	 QPtrList<SnacPair> members;
};

struct UserInfo
{ //user info
	 QString sn;
	 int evil;
	 int userclass;
	 unsigned long membersince;
	 unsigned long onlinesince;
	 long capabilities;
	 long sessionlen;
	 int idletime;

	 unsigned long icqextstatus;
};

#define OSCAR_FAM_1				0x0001 // Services
#define OSCAR_FAM_2				0x0002 // Location
#define OSCAR_FAM_3				0x0003 // Contacts, adding, removal, statuschanges
#define OSCAR_FAM_4				0x0004 // ICBM, messaging
#define OSCAR_FAM_9				0x0009 // BOS, visible/invisible lists
#define OSCAR_FAM_11				0x000b // Interval
#define OSCAR_FAM_19				0x0013 // Roster, Contactlist
#define OSCAR_FAM_21				0x0015 // icq metasearch, sms, offline messages
#define OSCAR_FAM_23				0x0017 // new user, registration


// used for SRV_RECVMSG, SNAC(4,7)
#define MSGFORMAT_SIMPLE		0x0001
#define MSGFORMAT_ADVANCED		0x0002
#define MSGFORMAT_SERVER		0x0004

#define OSCAR_SERVER 			"login.oscar.aol.com"
#define OSCAR_PORT 				5190
/*
#define OSCAR_OFFLINE		0
#define OSCAR_ONLINE			1
#define OSCAR_AWAY			2
*/
const unsigned short ICQ_STATUS_ONLINE		= 0x0000;
const unsigned short ICQ_STATUS_OFFLINE	= 0xFFFF;
const unsigned short ICQ_STATUS_AWAY		= 0x0001;
const unsigned short ICQ_STATUS_DND			= 0x0002;
const unsigned short ICQ_STATUS_NA			= 0x0004;
const unsigned short ICQ_STATUS_OCC			= 0x0010;
const unsigned short ICQ_STATUS_FFC			= 0x0020;

#define USERCLASS_TRIAL					0x0001
#define USERCLASS_UNKNOWN2 			0x0002
#define USERCLASS_AOL					0x0004
#define USERCLASS_UNKNOWN4				0x0008
#define USERCLASS_AIM					0x0010
#define USERCLASS_AWAY					0x0020
#define USERCLASS_ACTIVEBUDDY			0x0400


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

static const struct
{
    DWORD flag;
    char data[16];
} oscar_caps[] =
{
	// Chat is oddball.
	{AIM_CAPS_CHAT,
	{0x74, 0x8f, 0x24, 0x20, 0x62, 0x87, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	// These are mostly in order.
	{AIM_CAPS_VOICE,
	{0x09, 0x46, 0x13, 0x41, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	{AIM_CAPS_SENDFILE,
	{0x09, 0x46, 0x13, 0x43, 0x4c, 0x7f, 0x11, 0xd1,
		0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},

	// Advertised by the EveryBuddy client.
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

#define KOPETE_AIM_CAPS				AIM_CAPS_IMIMAGE | AIM_CAPS_SENDFILE | AIM_CAPS_GETFILE
// AIM_CAPS_ICQSERVERRELAY commented out until we support
// type-2 icq-messages (SRV_RECVMSG with type=0x0004)
#define KOPETE_ICQ_CAPS				AIM_CAPS_ISICQ | AIM_CAPS_IS_2001 | AIM_CAPS_ICQSERVERRELAY

/**Implements the actual communication with the oscar server
*@author Tom Linsky
*/

class OscarSocket : public OscarConnection
{
	Q_OBJECT
	public:
	OscarSocket(const QString &connName,
	const QByteArray &cookie, OscarAccount *account, QObject *parent=0,
	const char *name=0);
	~OscarSocket();

	/** Sends an authorization request to the server */
	void sendLoginRequest();

	/** encodes a password using md5, outputs to digest */
	int encodePassword(unsigned char *digest);
	/** same as above but for icq which needs a XOR method to encode the password
	 *  returns the encoded password
	 */
	QCString encodePasswordXOR();

	/** Logs in the user! */
	void doLogin(const QString &host, int port, const QString &s, const QString &password);
	/** Gets the rate info from the server */
	void sendRateInfoRequest();
	/** requests the current user's info */
	void requestMyUserInfo();
	/** Sets idle time */
	void sendIdleTime(DWORD time);
	/** requests ssi data from the server */
	void sendBuddyListRequest();
	/** Sends message to dest */
	void sendIM(const QString &message, const QString &dest, bool isAuto);
	/** Requests sn's user info */
	void sendUserProfileRequest(const QString &sn);
	/** Sets the away message, makes user away */
	void sendAway(bool away, const QString &message=0L);
	/** Sends someone a warning */
	void sendWarning(const QString &target, bool isAnonymous);
	/** Changes a user's password (AIM Method) */
	void sendChangePassword(const QString &newpw, const QString &oldpw);
	/** Joins the given chat room */
	void sendChatJoin(const QString &name, const int exchange);
	/** Sends a request for direct IM */
	void sendDirectIMRequest(const QString &sn);
	/** Sends a direct IM denial */
	void sendDirectIMDeny(const QString &sn);
	/** Sends a direct IM accept */
	void sendDirectIMAccept(const QString &sn);
	/** Sends our capabilities to the server */
	void sendCapabilities(unsigned long caps);
	/** Signs the user off */
	virtual void doLogoff();
	/** Adds a buddy to the server side buddy list */
	virtual void sendAddBuddy(const QString &name, const QString &group);
	/** Changes a buddy's group on the server */
	virtual void sendChangeBuddyGroup(const QString &buddyName,
									  const QString &oldGroup,
									  const QString &newGroup);
	/** Adds a group to the server side buddy list */
	virtual void sendAddGroup(const QString &name);
	/** Changes a group's name on the server side buddy list */
	virtual void sendChangeGroupName(const QString &currentName,
									 const QString &newName);
	/** Removes a group from the server side information */
	virtual void sendDelGroup(const QString &groupName);
	/** Deletes a buddy from the server side buddy list */
	virtual void sendDelBuddy(const QString &budName, const QString &budGroup);
	/** Sends the server lots of  information about the currently logged in user */
	void sendInfo();
	/** Sends the user's profile to the server */
	void sendMyProfile();
	/** Sets the user's profile */
	void setMyProfile(const QString &profile);
	/** Returns the user's profile */
	inline QString getMyProfile() const { return myUserProfile; };
	/** Blocks user sname */
	void sendBlock(const QString &sname);
	/** Removes the block on user sname */
	void sendRemoveBlock(const QString &sname);
	/**
	* Sends a typing notification to the server
	* @param screenName The name of the person to send to
	* @param notifyType Type of notify to send
	*/
	void sendMiniTypingNotify(QString screenName, TypingNotify notifyType);
	/** Initiate a transfer of the given file to the given sn */
	void sendFileSendRequest(const QString &sn, const KFileItem &finfo);
	/** Sends a file transfer deny to @sn */
	void sendFileSendDeny(const QString &sn);
	/** Accepts a file transfer from sn, returns OscarConnection created */
	OscarConnection *sendFileSendAccept(const QString &sn, const QString &fileName);

	/** send status (ICQ method) */
	void sendStatus(unsigned long status);
	/** send request for offline messages (ICQ method) */
	void sendReqOfflineMessages();
	/** send acknowledgment for offline messages (ICQ method) */
	void sendAckOfflineMessages();
	/** sends a KEEPALIVE packet, empty FLAP channel 5 */
	void sendKeepalive();

	public slots:
	/** This is called when a connection is established */
	void OnConnect();
	/** This function is called when there is data to be read */
	virtual void slotRead();

	private:
	/** adds the flap version to the buffer */
	void putFlapVer(Buffer &buf);
	/** Reads a FLAP header from the input */
	FLAP getFLAP();
	/** Sends the output buffer, and clears it */
	void sendBuf(Buffer &buf, BYTE chan);

	/**
	* Sends login information, actually logs
	* onto the server
	*/
	void sendLoginAIM();
	void sendLoginICQ();

	/** Called when a cookie is received */
	void connectToBos();
	/** Sends the authorization cookie to the BOS server */
	void sendCookie();
	/** Parses the rate info response */
	void parseRateInfoResponse(Buffer &inbuf);
	/**
	* Tells the server we accept it's communist rate
	* limits, even though I have no idea what they mean
	*/
	void sendRateAck();
	/** Sends privacy flags to the server  */
	void sendPrivacyFlags();
	/** parse my user info */
	void parseMyUserInfo(Buffer &inbuf);
	/** finds a tlv of type typ in the list */
	TLV * findTLV(QPtrList<TLV> &l, WORD typ);
	/**
	* Parse the server's authorization response
	* (which hopefully contains the cookie)
	*/
	void parseAuthResponse(Buffer &inbuf);
	/** The program does this when a key is received */
	void parsePasswordKey(Buffer &inbuf);
	/**
	* tells the server that the client is
	* ready to receive commands & stuff */
	void sendClientReady();
	/** Sends versions so that we get proper rate info */
	void sendVersions(const WORD *families, const int len);
	/**
	* Handles AOL's evil attempt to thwart 3rd
	* party apps using Oscar.  It requests a
	* segment and offset of aim.exe.  We can
	* thwart it with help from the good people
	* at Gaim
	*/
	void parseMemRequest(Buffer &inbuf);
	/** parses incoming ssi data */
	void parseSSIData(Buffer &inbuf);
	/** Requests the user's SSI rights */
	void requestBOSRights();
	/** Parses SSI rights data */
	void parseBOSRights(Buffer &inbuf);
	/** Parses the server ready response */
	void parseServerReady(Buffer &inbuf);
	/** parses server version info */
	void parseServerVersions(Buffer &inbuf);
	/** Parses Message of the day */
	void parseMessageOfTheDay(Buffer &inbuf);
	/** Requests location rights */
	void requestLocateRights();
	/** Requests a bunch of information (permissions, rights, my user info, etc) from server */
	void requestInfo();
	/** adds a mask of the groups that you want to be able to see you to the buffer */
	void sendGroupPermissionMask();
	/** adds a request for buddy list rights to the buffer */
	void requestBuddyRights();
	/** adds a request for msg rights to the buffer */
	void requestMsgRights();
	/** Parses the locate rights provided by the server */
	void parseLocateRights(Buffer &inbuf);
	/** Parses buddy list rights from the server */
	void parseBuddyRights(Buffer &inbuf);
	/** Parses msg rights info from server */
	void parseMsgRights(Buffer &inbuf);
	/** Parses an incoming IM */
	void parseIM(Buffer &inbuf);
	/** parses the aim standard user info block */
	UserInfo parseUserInfo(Buffer &inbuf);
	/** Activates the SSI list on the server */
	void sendSSIActivate();
	/** Parses the oncoming buddy server notification */
	void parseBuddyChange(Buffer &);
	/** Parses offgoing buddy message from server */
	void parseOffgoingBuddy(Buffer &);
	/** Parses someone's user info */
	void parseUserProfile(Buffer &);
	/** Handles a redirect */
	void parseRedirect(Buffer &);
	/** Parses a message ack from the server */
	void parseMsgAck(Buffer &);
	/** Parses a minityping notification from server */
	void parseMiniTypeNotify(Buffer &);


	void parseICQ_CLI_META(Buffer &);
	void parseAdvanceMessage(Buffer &, UserInfo &);

	/** Parses a rate change */
	void parseRateChange(Buffer &inbuf);
	/** Sends SSI add, modify, or delete request to reuse code */
	void sendSSIAddModDel(SSI *item, WORD request_type);
	/** Parses the SSI acknowledgement */
	void parseSSIAck(Buffer &inbuf);
	/** Parses a warning notification */
	void parseWarningNotify(Buffer &inbuf);
	/** Parses a message sending error */
	void parseError(Buffer &inbuf);
	/** Parses a missed message notification */
	void parseMissedMessage(Buffer &inbuf);
	/** Request, deny, or accept a rendezvous session with someone
	type == 0: request
	type == 1: deny
	type == 2: accept  */
	void sendRendezvous(const QString &sn, WORD type, DWORD rendezvousType, const KFileItem *finfo=0L);
	/** Sends a 0x0013,0x0002 (requests SSI rights information) */
	void sendSSIRightsRequest();
	/** Sends a 0x0013,0x0004 (requests SSI data?) */
	void sendSSIRequest();
	/** Parses a 0x0013,0x0003 (SSI rights) from the server */
	void parseSSIRights(Buffer &inbuf);
	/** Sends parameters for ICBM messages */
	void sendMsgParams();
	/** Returns the appropriate server socket, based on the capability flag it is passed. */
	OncomingSocket * serverSocket(DWORD capflag);

	void startKeepalive();
	void stopKeepalive();

	private slots:
	/** Called when a connection has been closed */
	void OnConnectionClosed();
	/** Called when the server aknowledges the connection */
	void OnConnAckReceived();
	/** called when a conn ack is recieved for the BOS connection */
	void OnBosConnAckReceived();
	/** Called when the server is ready for normal commands */
	void OnServerReady();
	/** Called on connection to bos server */
	void OnBosConnect();
	/** Called when a direct IM is received */
	void OnDirectIMReceived(QString, QString, bool);
	/** Called when a direct IM connection suffers an error */
	void OnDirectIMError(QString, int);
	/** Called when a direct IM connection bites the dust */
	void OnDirectIMConnectionClosed(QString);
	/** Called whenever a direct IM connection gets a typing notification */
	void OnDirectMiniTypeNotification(QString screenName, int notify);
	/** Called when a direct connection is set up and ready for use */
	void OnDirectIMReady(QString name);
	/** Called when a file transfer begins */
	void OnFileTransferBegun(OscarConnection *con, const QString& file,
		const unsigned long size, const QString &recipient);

	void slotKeepaliveTimer();

	signals:
	/** The server has sent the key with which to encrypt the password */
	void keyReceived();
	/** The bos server is ready to be sent commands */
	void serverReady();
	/** A buddy has left */
	void gotOffgoingBuddy(QString);
	/** A buddy has arrived! */
	void gotBuddyChange(UserInfo);
	/** A user profile has arrived */
	void gotUserProfile(UserInfo, QString);
	/** Emitted when the status of the connection changes during login */
//	void connectionChanged(int, QString);
	/** Emitted when my user info is received */
	void gotMyUserInfo(UserInfo);
	/** A buddy list has been received */
	void gotConfig(AIMBuddyList &);
	/** emitted when we have recieved an ack from the server */
	void gotAck(QString, int);

	/**
	 * Emitted (with new status as parameter) when our status has changed
	 */
	void statusChanged( const KopeteOnlineStatus &newStatus );

	/** Emitted when the logged in user has been warned
	 * The int is the new warning level.
	 * The QString is the name of the user which warned us (QString::null if anonymous)
	 * WARNING: this is emitted every time the server notifies us about our warning level,
	 * so natural decreases in level will be signalled.
	 */
	void gotWarning(int, QString);
	/** Emitted when someone has requested a direct IM session with us */
	void gotDirectIMRequest(QString);
	/** Emitted when someone has requested to send a file to us */
	void gotFileSendRequest(QString, QString, QString, unsigned long);

	private:
	/** The OscarAccount we're assocated with */
	OscarAccount *mAccount;
	/** The key used to encrypt the password */
	char * key;
	/** The user's password */
	QString pass;
	/** The authorization cookie */
	char * mCookie;
	/** ip address of the bos server */
	QString bosServer;
	/** The length of the cookie */
	WORD cookielen;
	/** The port of the bos server */
	int bosPort;
	/** Stores rate class information */
	QPtrList<RateClass> rateClasses;
	/** tells whether we are idle */
	bool idle;
	/** Socket for direct connections */
	OncomingSocket *mDirectIMMgr;
	/** Socket for file transfers */
	OncomingSocket *mFileTransferMgr;
	/** SSI server stored data */
	SSIData ssiData;
	/** Socket for direct connections */
	QSocket * connsock;
	/** The currently logged in user's profile */
	QString myUserProfile;
	/** Tells if we are connected to the server and ready to operate */
	bool isConnected;

	/** counter to find out if we got all packets needed before sending
	  * out more info and the final CLI_READY command which is the end of a login procedure
	  */
	int gotAllRights;

	int keepaliveTime;
	QTimer *keepaliveTimer;

	signals:
	/** Called when an SSI acknowledgement is recieved */
	void SSIAck();
	/** emitted when BOS rights are received */
	// void gotBOSRights(WORD,WORD);
	/** emitted when a buddy gets blocked */
	void denyAdded(QString);
	/** emitted when a block is removed on a buddy */
	void denyRemoved(QString);
	/** Tells when the connection ack has been recieved on channel 1 */
	void connAckReceived();
	/** emitted when a direct connection has been terminated */
	void directIMConnectionClosed(QString name);
};
#endif
