/*
    oscarconnection.h  -  Implementation of an oscar connection

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

#ifndef OSCARCONNECTION_H
#define OSCARCONNECTION_H

#include <qsocket.h>
#include "buffer.h"

/**Implementation of a base oscar connection.  No login functions, just basic direct Oscar connection functionality.
  *@author Tom Linsky
  */

class OscarDebugDialog;

class OscarConnection : public QSocket  {
  Q_OBJECT
public:
	/** Enum for connection type */
	enum ConnectionType
	{
		DirectIM, Server, SendFile, Redirect
	};

	/** Enum for typing notifications */
	enum TypingNotify
	{
		TypingFinished, TextTyped, TypingBegun
	};

	OscarConnection(const QString &sn, const QString &connName, ConnectionType type,
		const QByteArray &cookie, QObject *parent=0, const char *name=0);
	~OscarConnection();

	/** Sets the pointer to the debug dialog */
	void setDebugDialog(OscarDebugDialog *dialog);
	/** Returns true if we have a debug dialog, false otherwise */
	inline bool hasDebugDialog(void) const { return mHaveDebugDialog; };
	/** Returns pointer to the debug dialog */
	inline OscarDebugDialog *debugDialog(void) const { return mDebugDialog; };
  /** Returns the name of this connection */
  inline QString connectionName(void) const { return mConnName; };
  /** Returns the type of this connection */
  inline int connectionType(void) const { return mConnType; };
  /** Gets the currently logged in user's screen name */
  inline QString getSN(void) const { return mSN; };
  /** Sets the currently logged in user's screen name */
  void setSN(const QString &newSN);
  /** Gets the message cookie */
  inline const QByteArray &cookie(void) const { return mCookie; };
  /** Sets the socket to use socket, state() to connected, and emit connected() */
  virtual void setSocket( int socket );

  //VIRTUAL FUNCTIONS THAT CAN BE OVERLOADED BY CHILD CLASSES
  /** Sends the direct IM message to buddy */
  virtual void sendIM(const QString &message, bool isAuto);
  /** Sends a typing notification to the server
			@param notifyType Type of notify to send
	 */
  virtual void sendTypingNotify(TypingNotify notifyType);
  /** Sends request to the client telling he/she that we want to send a file */
  virtual void sendFileSendRequest(void);



signals: // Signals
	/** Emitted when an IM comes in */
  void gotIM(QString, QString, bool);
  /** called when an AIM protocol error occurs */
  void protocolError(QString, int);
 	/**
	 * Emitted when we get a minityping notifications
	 * First param is the screen name, second is the type
	 * 0: Finished
	 * 1: Typed
	 * 2: Begun (is typing)
	 */
	void gotMiniTypeNotification(QString, int);
	/** Emitted when we are ready to send commands! */
	void connectionReady(QString name);
  /** Emitted when the connection is closed */
  void connectionClosed(QString name);
  /** Emitted when a file transfer is complete */
  void transferComplete(QString name);
  /** Emitted when data is received.. parameter is percentage of transfer complete */
  void percentComplete( unsigned int percent );
  /** Emitted when a file transfer begins */
  void transferBegun(OscarConnection *con, const QString& file, const unsigned long size, const QString &recipient);
	
protected slots: // Protected slots
  /** Called when there is data to be read.
		If you want your connection to be able to receive data, you
		should override this
		No need to connect the signal in derived classes, just override this slot*/
  virtual void slotRead(void);

private slots: // Private slots
	/** Called when we have established a connection */
	void slotConnected(void);
	/** Called when the connection is closed */
	void slotConnectionClosed(void);
	/** Called when a socket error occurs */
	void slotError(int);

private: //private attributes
	/** The ICBM cookie used to authenticate */
	QByteArray mCookie;
	/** The name of the connection */
	QString mConnName;
	/** The connection type */
	ConnectionType mConnType;
	/** The ip of the host we will connect to */
	QString mHost;
	/** The Port we will connect to on the peer machine */
	int mPort;
	/** Pointer to the debug dialog, should not delete */
	OscarDebugDialog *mDebugDialog;
	/** Bool indicating whether or not we have a debug dialog */
	bool mHaveDebugDialog;
  /** The user's screen name */
  QString mSN;

};

#endif
