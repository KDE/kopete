/***************************************************************************
     libwinpopup.h  -  Base class for the WinPopup protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LIBWINPOPUP_H
#define __LIBWINPOPUP_H

// The winpopup message file (really should go into config.h)
#define WP_MESSAGE_FILE "/tmp/.winpopup"

// Local Includes

// QT Includes
#include <qobject.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qthread.h>
#include <qsemaphore.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qmap.h>

// KDE Includes

class Host
{
	bool Available;
	QString myDescription;

public:
	const QString &description() { return myDescription; }
	
	Host() { Available = true; }
	Host(const QString &nDesc) { myDescription = nDesc; Available = true; }
};

class WorkGroup
{
	bool Available;
	QString myMaster;

public:
	const QString &master() { return myMaster; }

	QMap<QString, Host> Hosts;

	WorkGroup() { Available = true; }
	WorkGroup(const QString &nMaster) { myMaster = nMaster; Available = true; }
};

typedef QMap<QString, QString> stringMap;

class KWinPopup;

class UpdateThread: public QThread
{
	KWinPopup *theOwner;

protected:
	void run();

public:
	UpdateThread(KWinPopup *owner) { theOwner = owner; }
};

class KWinPopup: public QObject
{
	Q_OBJECT

private:
	QString myHostName, myInitialSearchHost, mySMBClientPath;
	QTimer checkForMessages, updateData;
	UpdateThread theUpdateThread;
	QSemaphore updating, reading;
	QMap<QString, WorkGroup> theGroups;

	void messageHandler();
	QPair<stringMap, stringMap> grabData(const QString &Host, QString *theGroup = 0, QString *theOS = 0, QString *theSoftware = 0);	// theGroup gets populated

	void doUpdate();

public slots:
	void doCheck();
	void updateNoWait() { update(false); }
	void update(bool Wait = true);

// API section:

protected:
	virtual void receivedMessage(const QString &Body, const QDateTime &Arrival, const QString &From) = 0;
	// overload this to be called when a new message arrives

public:
	bool sendMessage(const QString &Body, const QString &Destination);
	// use this to send a message

	void setSMBClientPath(const QString &SMBClientPath) { mySMBClientPath = SMBClientPath; }
	void setInitialSearchHost(const QString &InitialSearchHost) { myInitialSearchHost = InitialSearchHost; }
	void setHostName(const QString &HostName) { myHostName = HostName; }

	const QStringList getGroups();
	const QStringList getHosts(const QString &Group);
	const Host getHostInfo(const QString &Group, const QString &Host);
	bool checkHost(const QString &Name);

	KWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName);
	~KWinPopup();
};

#endif
