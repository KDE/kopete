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

//QT includes
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QTimer>
#include <QDateTime>
#include <QProcess>

// KDE Includes
#include <KFileItem>

const QString WP_POPUP_DIR = QString::fromLatin1("/var/lib/winpopup");

class KDirLister;

typedef QMap<QString, QString> stringMap;

class WorkGroup
{
	QStringList groupHosts;

public:
	QStringList Hosts() const { return groupHosts; }
	void addHosts(const QStringList &newHosts) { groupHosts = newHosts; }
};

class WinPopupLib : public QObject
{
	Q_OBJECT

public:
	WinPopupLib(const QString &smbClient,int groupFreq);
	~WinPopupLib();

	QStringList getGroups() const;
	QStringList getHosts(const QString &Group) const;
	bool checkHost(const QString &Name) const;
	void settingsChanged(const QString &smbClient, int groupFreq);
	void sendMessage(const QString &Body, const QString &Destination);

private:
	bool passedInitialHost;
	QMap<QString, WorkGroup> theGroups, currentGroupsMap;
	QString currentGroup, currentHost;
	QStringList todo, done, currentHosts;
	stringMap currentGroups;
	QTimer updateGroupDataTimer;
	QString smbClientBin;
	int groupCheckFreq;
	KDirLister *dirLister;
	QProcess *readIpProcess;
	QProcess *readGroupsProcess;

	bool checkMessageDir();

private slots:
	void slotUpdateGroupData();
	void startReadProcess();
	void slotReadIpProcessExited(int i = 1, QProcess::ExitStatus status = QProcess::CrashExit);
	void slotSendIpMessage(int i = 1, QProcess::ExitStatus status = QProcess::CrashExit);
	void slotReadProcessExited(int, QProcess::ExitStatus);
	void slotStartDirLister();
	void slotReadMessages(const KFileItemList &items);

signals:
	void signalNewMessage(const QString &, const QDateTime &, const QString &);
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
