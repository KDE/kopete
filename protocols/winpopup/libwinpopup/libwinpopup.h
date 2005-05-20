/***************************************************************************
     libwinpopup.h  -  Base class for the WinPopup protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LIBWINPOPUP_H
#define LIBWINPOPUP_H

#define WP_POPUP_DIR "/var/lib/winpopup/"

//QT includes
#include <qobject.h>
#include <qmutex.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qdatetime.h>

// KDE Includes
#include <kprocio.h>

typedef QMap<QString, QString> stringMap;

class WorkGroup
{
	QStringList groupHosts;

public:
	const QStringList &Hosts() { return groupHosts; }
	void addHosts(const QStringList &newHosts) { groupHosts = newHosts; }
};

class WinPopupLib : public QObject
{
	Q_OBJECT

public:
	WinPopupLib(const QString &smbClient,int groupFreq,int messageCheck);
	~WinPopupLib();

	const QStringList getGroups();
	const QStringList getHosts(const QString &Group);
	bool checkHost(const QString &Name);
	bool checkMessageDir();
	void settingsChanged(const QString &smbClient, int groupFreq, int messageCheck);
	void sendMessage(const QString &Body, const QString &Destination);

private:
	bool passedInitialHost;
	QMap<QString, WorkGroup> theGroups, currentGroupsMap;
	QString currentGroup, currentHost;
	QStringList todo, done, currentHosts;
	stringMap currentGroups;
	QMutex groupMutex;
	QTimer updateGroupDataTimer, messageCheckTimer;
	QString smbClientBin;
	int groupCheckFreq, messageCheckFreq;

public slots:
	void slotUpdateGroupData();
	void startReadProcess(const QString &Host);
	void slotReadProcessReady(KProcIO *r);
	void slotReadProcessExited(KProcess *r);
	void slotCheckForNewMessages();
	void slotSendProcessExited(KProcess *p);

signals:
	void signalNewMessage(const QString &, const QDateTime &, const QString &);
};

#endif

// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
