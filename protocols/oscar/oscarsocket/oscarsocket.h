 /*
    oscarsocket.h  -  Oscar Protocol Implementation

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

#ifndef OSCARSOCKET_H
#define OSCARSOCKET_H

#include "protocolsocket.h"
#include <qlist.h>
#include "buffer.h"
#include "oncomingsocket.h"
#include "ssidata.h"

struct FLAP { //flap header
	BYTE channel;
	WORD sequence_number;
	WORD length;
};

struct SnacPair { //just a group+type pair
	WORD group;
	WORD type;
};

struct RateClass { //rate info
	WORD classid;
	DWORD windowsize;
	DWORD clear;
	DWORD alert;
	DWORD limit;
	DWORD disconnect;
	DWORD current;
	DWORD max;
	BYTE unknown[5];
	QList<SnacPair> members;
};

class ServiceSocket;

#define OSCAR_SERVER 	"login.oscar.aol.com"
#define OSCAR_PORT 		5190
#define OSCAR_OFFLINE	0
#define OSCAR_ONLINE	1
#define OSCAR_AWAY		2
      
#define USERCLASS_TRIAL			0x0001
#define USERCLASS_UNKNOWN2 	0x0002
#define USERCLASS_AOL				0x0004
#define USERCLASS_UNKNOWN4	0x0008
#define USERCLASS_AIM				0x0010
#define USERCLASS_AWAY			0x0020
#define	USERCLASS_ACTIVEBUDDY	0x0400

/**Implements the actual communication with the oscar server
  *@author Tom Linsky
  */

class OscarSocket : public ProtocolSocket  {
	Q_OBJECT
public:
	OscarSocket(QObject *parent=0, const char *name=0);
	~OscarSocket();
  /** Sends an authorization request to the server */
  void sendLoginRequest(void);
  /** encodes a password, outputs to the 3rd parameter */
  int encodePassword(unsigned char *digest);
  /** Logs in the user! */
  void doLogin(const QString &host, int port, const QString &s, const QString &password);
  /** Gets the rate info from the server */
  void sendRateInfoRequest(void);
  /** requests the current user's info */
  void requestMyUserInfo(void);
  /** Sets idle time */
  void sendIdleTime(DWORD time);
  /** requests ssi data from the server */
  void sendBuddyListRequest(const TAimConfig &);
  /** Sends message to dest */
  void sendIM(const QString &message, const QString &dest, bool isAuto);
  /** Requests sn's user info */
  void sendUserProfileRequest(const QString &sn);
  /** Sets the away message, makes user away */
  void sendAway(int, const QString &message);
  /** Sends someone a warning */
  void sendWarning(const QString &target, bool isAnonymous);
  /** Changes a user's password!!!!!! */
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
  /** Adds a group to the server side buddy list */
  virtual void sendAddGroup(const QString &name);
  /** Deletes a buddy from the server side contact list */
  virtual void sendDelBuddy(const QString &budName, const QString &budGroup);
  /** Sends the server lots of  information about the currently logged in user */
  void sendInfo(void);
  /** Sends the user's profile to the server */
  void sendMyProfile();
  /** Sets the user's profile */
  void setMyProfile(const QString &profile);
  /** Returns the user's profile */
  inline QString getMyProfile(void) { return myUserProfile; };
  /** Blocks user sname */
  void sendBlock(const QString &sname);
  /** Removes the block on user sname */
  void sendRemoveBlock(const QString &sname);
public slots: // Public slots
  /** This is called when a connection is established */
  void OnConnect(void);
  /** This function is called when there is data to be read */
  void OnRead(void);
private: // Private methods
  /** Reads a FLAP header from the input */
  FLAP getFLAP(void);
  /** adds the flap version to the buffer */
  void putFlapVer(Buffer &buf);
  /** Sends the output buffer, and clears it */
  void sendBuf(Buffer &buf, BYTE chan);
  /** Sends login information, actually logs onto the server */
  void sendLogin(void);
  /** Sends the authorization cookie to the BOS server */
  void sendCookie(void);
  /** Parses the rate info response */
  void parseRateInfoResponse(Buffer &inbuf);
  /** Tells the server we accept it's communist rate limits, even though I have no idea what they mean */
  void sendRateAck(void);
  /** Sends privacy flags to the server  */
  void sendPrivacyFlags(void);
  /** parse my user info */
  void parseMyUserInfo(Buffer &inbuf);
  /** finds a tlv of type typ in the list */
  TLV * findTLV(QList<TLV> &l, WORD typ);
  /** parse the server's authorization response (which hopefully contains the cookie) */
  void parseAuthResponse(Buffer &inbuf);
  /** tells the server that the client is ready to receive commands & stuff */
  void sendClientReady(void);
  /** Sends versions so that we get proper rate info */
  void sendVersions(const WORD *families, const int len);
  /** Handles AOL's evil attempt to thwart 3rd party apps using Oscar.  It requests a segment and offset of aim.exe.  We can thwart it with help from the good people at Gaim */
  void parseMemRequest(Buffer &inbuf);
  /** parses incoming ssi data */
  void parseSSIData(Buffer &inbuf);
  /** Requests the user's SSI rights */
  void requestBOSRights(void);
  /** Parses SSI rights data */
  void parseBOSRights(Buffer &inbuf);
  /** Parses the server ready response */
  void parseServerReady(Buffer &inbuf);
  /** parses server version info */
  void parseServerVersions(Buffer &inbuf);
  /** Parses Message of the day */
  void parseMessageOfTheDay(Buffer &inbuf);
  /** Requests location rights */
  void requestLocateRights(void);
  /** Requests a bunch of information (permissions, rights, my user info, etc) from server */
  void requestInfo(void);
  /** adds a mask of the groups that you want to be able to see you to the buffer */
  void sendGroupPermissionMask(void);
  /** adds a request for buddy list rights to the buffer */
  void requestBuddyRights(void);
  /** adds a request for msg rights to the buffer */
  void requestMsgRights(void);
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
  void sendSSIActivate(void);
  /** Parses the oncoming buddy server notification */
  void parseOncomingBuddy(Buffer &inbuf);
  /** Parses offgoing buddy message from server */
  void parseOffgoingBuddy(Buffer &inbuf);
  /** Parses someone's user info */
  void parseUserProfile(Buffer &inbuf);
  /** Handles a redirect */
  void parseRedirect(Buffer &inbuf);
  /** Parses a message ack from the server */
  void parseMsgAck(Buffer &inbuf);
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
  /** Request, deny, or accept a direct IM session with someone
		type == 0: request
		type == 1: deny
		type == 2: accept  */
  void sendDirectIMInit(const QString &sn, WORD type);
  /** Sends a 0x0013,0x0002 (requests SSI rights information) */
  void sendSSIRightsRequest(void);
  /** Sends a 0x0013,0x0004 (requests SSI data?) */
  void sendSSIRequest(void);
  /** Parses a 0x0013,0x0003 (SSI rights) from the server */
  void parseSSIRights(Buffer &inbuf);
  /** Sends parameters for ICBM messages */
  void sendMsgParams(void);
private slots: // Private slots
  /** Called when a connection has been closed */
  void OnConnectionClosed(void);
  /** Called when the server aknowledges the connection */
  void OnConnAckReceived(void);
  /** The program does this when a key is received */
  void OnKeyReceived(void);
  /** Called when a cookie is received */
  void OnCookieReceived(void);
  /** called when a conn ack is recieved for the BOS connection */
  void OnBosConnAckReceived(void);
  /** Called when the server is ready for normal commands */
  void OnServerReady(void);
  /** Called on connection to bos server */
  void OnBosConnect();
  /** Called when bos rights are received */
  void OnGotBOSRights(WORD maxperm, WORD maxdeny);
signals: // Signals
  /** Tells when the connection ack has been recieved on channel 1 */
  void connAckReceived(void);
  /** The server has sent the key with which to encrypt the password */
  void keyReceived(void);
  /** authorization successful... the authorization cookie has been recieved */
  void cookieReceived(void);
  /** The bos server is ready to be sent commands */
  void serverReady(void);
private: // Private attributes
  /** The key used to encrypt the password */
  char * key;
  /** The user's screen name */
  QString sn;
  /** The user's password */
  QString pass;
  /** The authorization cookie */
  char * cookie;
  /** ip address of the bos server */
  QString bosServer;
  /** The length of the cookie */
  WORD cookielen;
  /** The port of the bos server */
  int bosPort;
  /** Stores rate class information */
  QList<RateClass> rateClasses;
  /** tells whether we are idle */
  bool idle;
  /** A collections of the sockets we are connected with */
  QList<ServiceSocket> sockets;
  /** A temp socket, used for making temporary connectionz */
//  QSocket * tmpSocket;
  /** Socket for direct connections */
  OncomingSocket *serverSocket;
  /** SSI server stored data */
  SSIData ssiData;
  /** Socket for direct connections */
  QSocket * connsock;
  /** The currently logged in user's profile */
  QString myUserProfile;
  /** Tells if we are connected to the server and ready to operate */
  bool isConnected;
signals: // Signals
  /** Called when an SSI acknowledgement is recieved */
  void SSIAck();
  /** emitted when BOS rights are received */
  void gotBOSRights(WORD,WORD);
  /** emitted when a buddy gets blocked */
  void denyAdded(QString);
  /** emitted when a block is removed on a buddy */
  void denyRemoved(QString);
};

#endif
