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

#define CONN_TYPE_DIRECTIM			1
#define CONN_TYPE_SERVER				2
#define CONN_TYPE_FILETRANSFER	4
#define CONN_TYPE_REDIRECT			8

class OscarDebugDialog;

class OscarConnection : public QSocket  {
  Q_OBJECT
public:
	OscarConnection(const QString &connName, int type, QObject *parent=0, const char *name=0);
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
  /** Sends an IM */
  virtual void sendIM(const QString &message, const QString &dest, bool isAuto);

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

	
protected slots: // Private slots
  /** Called when there is data to be read.
		If you want your connection to be able to receive data, you
		should override this
		No need to connect the signal in derived classes, just override this slot*/
  virtual void slotRead(void);

private: //private attributes
	/** The cookie used to authenticate */
	//char mAuthCookie[256];
	/** The name of the connection */
	QString mConnName;
	/** The connection type */
	int mConnType;
	/** The ip of the host we will connect to */
	QString mHost;
	/** The Port we will connect to on the peer machine */
	int mPort;
	/** Pointer to the debug dialog, should not delete */
	OscarDebugDialog *mDebugDialog;
	/** Bool indicating whether or not we have a debug dialog */
	bool mHaveDebugDialog;
};

#endif
