/***************************************************************************
                          libwinpopup.cpp  -  WP Library
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// QT Includes
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>

// KDE Includes
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfileitem.h>

// Kopete Includes
#include "kopeteuiglobal.h"

// Local Includes
#include "libwinpopup.h"

WinPopupLib::WinPopupLib(const QString &smbClient,int groupFreq,int messageCheck)
	: smbClientBin(smbClient), groupCheckFreq(groupFreq), messageCheckFreq(messageCheck)
{
	connect(&updateGroupDataTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGroupData()));
	connect(&messageCheckTimer, SIGNAL(timeout()), this, SLOT(slotCheckForNewMessages()));

	updateGroupDataTimer.start(1, true);
	messageCheckTimer.start(1, true);
}

WinPopupLib::~WinPopupLib()
{
}

/**
 * return the group list
 */
const QStringList WinPopupLib::getGroups()
{
	QStringList ret;
	// Do we need a mutex or semaphore here? GF
	groupMutex.lock();
	QMap<QString, WorkGroup>::ConstIterator end = theGroups.end();
	for(QMap<QString, WorkGroup>::ConstIterator i = theGroups.begin(); i != end; i++)
		ret += i.key();
	groupMutex.unlock();

	return ret;
}

/**
 * return the host list
 */
const QStringList WinPopupLib::getHosts(const QString &Group)
{
	return theGroups[Group].Hosts();
}

/**
 * return if a host is in the host list
 */
bool WinPopupLib::checkHost(const QString &Name)
{
//	kdDebug() << "WP checkHost: " << Name << endl;
	bool ret = false;

	// Do we need a mutex or semaphore here? GF
	groupMutex.lock();
	QMap<QString, WorkGroup>::Iterator end = theGroups.end();
	for(QMap<QString, WorkGroup>::Iterator i = theGroups.begin(); i != end && !ret; i++) {
		if ((*i).Hosts().contains(Name.upper())) {
			ret = true;
			break; //keep the mutex locked as short as possible
		}
	}
	groupMutex.unlock();

	return ret;
}


bool WinPopupLib::checkMessageDir()
{
	QDir dir(WP_POPUP_DIR);
	if (! dir.exists()) {
		int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(), i18n("Working directory /var/lib/winpopup/ does not exist.\n"
								     "If you have not configured anything yet (samba) it may be better to call\n"
								     "Install Into Samba (Configure... -> Account -> Edit)\n"
								     "Should the directory be created? (May need root password)"));
		if (tmpYesNo == KMessageBox::Yes) {
			QString kdesuArgs = "mkdir -p -m 0777 /var/lib/winpopup";
			if (KApplication::kdeinitExecWait("kdesu", kdesuArgs) == 0) return true;
		}
	} else {
		KFileItem tmpFileItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, "/var/lib/winpopup/");
		QString tmpPerms = tmpFileItem.permissionsString();

		if (tmpPerms != "drwxrwxrwx") {

			kdDebug(14170) << "Perms not ok!" << endl;

			int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(), i18n("Permissions of the working directory "
									     "/var/lib/winpopup/ are wrong!\n"
									     "Fix? (May need root password)"));
			if (tmpYesNo == KMessageBox::Yes) {
				QString kdesuArgs = "chmod 0777 /var/lib/winpopup";
				if (KApplication::kdeinitExecWait("kdesu", kdesuArgs) == 0) return true;
			}
		} else {
			return true;
		}
	}

	return false;
}
/**
 * read the groups and their hosts
 */
void WinPopupLib::slotUpdateGroupData()
{
	passedInitialHost = false;
	currentGroupsMap.clear();
	currentHost = "LOCALHOST";
	startReadProcess(currentHost);
}

void WinPopupLib::startReadProcess(const QString &Host)
{
	// for Samba 3
	KProcIO *reader = new KProcIO;
	*reader << smbClientBin << "-N" << "-E" << "-g" << "-L" << Host << "-";

	connect(reader, SIGNAL(readReady(KProcIO *)), this, SLOT(slotReadProcessReady(KProcIO *)));
	connect(reader, SIGNAL(processExited(KProcess *)), this, SLOT(slotReadProcessExited(KProcess *)));

	if (!reader->start(KProcess::NotifyOnExit, true)) {
		// still to come
		kdDebug(14170) << "ReadProcess not started!" << endl;
	}
}

void WinPopupLib::slotReadProcessReady(KProcIO *r)
{
	QString tmpLine = QString::null;
	QRegExp group("^Workgroup\\|(.*)\\|(.*)$"), host("^Server\\|(.*)\\|(.*)$"),
		info("^Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]");

	while (r->readln(tmpLine) > -1) {
		if (info.search(tmpLine) != -1) currentGroup = info.cap(1);
		if (host.search(tmpLine) != -1) currentHosts += host.cap(1);
		if (group.search(tmpLine) != -1) currentGroups[group.cap(1)] = group.cap(2);
	}
}

void WinPopupLib::slotReadProcessExited(KProcess *r)
{
	delete r;

	// Drop the first cycle - it's only the initial search host,
	// the next round are the real masters. GF

	if (passedInitialHost) {

		// move currentHost from todo to done
		todo.remove(currentHost);
		done += currentHost;

		QMap<QString, WorkGroup> newGroups;
		//loop through the read groups and check for new ones
		QMap<QString, QString>::ConstIterator end = currentGroups.end();
		for (QMap<QString, QString>::ConstIterator i = currentGroups.begin(); i != end; i++) {
			QString groupMaster = i.data();
			if (!done.contains(groupMaster)) todo += groupMaster;
		}

		// create a workgroup object and put the hosts in
		WorkGroup nWG;
		nWG.addHosts(currentHosts);

		currentGroupsMap.insert(currentGroup, nWG, true);

	} else {
		passedInitialHost = true;
		QMap<QString, QString>::ConstIterator end = currentGroups.end();
		for (QMap<QString, QString>::ConstIterator i = currentGroups.begin(); i != end; i++) {
			QString groupMaster = i.data();
			todo += groupMaster;
		}
	}

	// maybe restart cycle
	currentHosts.clear();
	currentGroups.clear();
	if (todo.count()) {
		currentHost = todo[0];
		startReadProcess(currentHost);
	} else {
		groupMutex.lock();
		theGroups = currentGroupsMap;
		groupMutex.unlock();
		updateGroupDataTimer.start(groupCheckFreq * 1000, true);
	}
}

/**
 * check for new arrived messages
 */
void WinPopupLib::slotCheckForNewMessages()
{
//	kdDebug(14170) << "check for new Messages: " << this << endl;

	if (!checkMessageDir()) return; // Restart timer if false? GF

	QDir dir(WP_POPUP_DIR);
	const QFileInfoList *messageFiles = dir.entryInfoList(QDir::Files, QDir::Name);
	if (messageFiles) {
		QFileInfoListIterator it(*messageFiles);
		QFileInfo *messageFileInfo;
		while((messageFileInfo = it.current()) != 0) {
			++it;
			if (messageFileInfo->isFile()) {
				QString messageFileName(messageFileInfo->fileName());
				QString messageFilePath(WP_POPUP_DIR);
//				messageFilePath.append("/");
				messageFilePath.append(messageFileName);
				QFile messageFile(messageFilePath);

				if (messageFile.open(IO_ReadOnly)) {
					QTextStream stream(&messageFile);
					QString sender;
					QDateTime time;
					QString text;

					// first line is sender, can this really be empty? GF
					sender = stream.readLine();
					sender = sender.upper();

					// second line is time
					QString tmpTime = stream.readLine();
					time = QDateTime::fromString(tmpTime, Qt::ISODate);

					while (!stream.atEnd()) {
						text.append(stream.readLine());
						text.append('\n');
					}

					// remove trailing CR
					text = text.stripWhiteSpace();

					messageFile.close();

					// delete file
					if (!messageFile.remove()) {
						// QFile::remove() seems to be very persistent, it removes even files with 0444 owned by root
						// if the directory permissions are 0777 - so this is just for safety. GF
						kdDebug(14170) << "Message file not removed - how that?" << endl;
						int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(), i18n("A message file could not be removed; "
											"maybe the permissions are wrong.\n"
											"Fix? (May need root password)"));
						if (tmpYesNo == KMessageBox::Yes) {
							QFileInfo messageFileInfo(messageFile);
							QString kdesuArgs = "chmod 0666 /var/lib/winpopup/" + messageFileInfo.fileName();
							if (KApplication::kdeinitExecWait("kdesu", kdesuArgs) == 0) {
								if (!messageFile.remove())
									KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Still cannot remove it; please fix manually."));
							}
						}
					}
					if (!sender.isEmpty() && time.isValid())
						emit signalNewMessage(text, time, sender);
					else
						kdDebug(14170) << "Received invalid message!" << endl;
				}
			}
		} // while
	} // if messageFiles
	messageCheckTimer.start(messageCheckFreq *1000, true);
}

/**
 * send a message
 */
void WinPopupLib::sendMessage(const QString &Body, const QString &Destination)
{
	KProcess *sender = new KProcess(this);
	*sender << smbClientBin << "-M" << Destination;
	*sender << "-N" << "-";

	connect(sender, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendProcessExited(KProcess *)));

	if (sender->start(KProcess::NotifyOnExit, KProcess::Stdin)) {
		sender->writeStdin(Body.local8Bit(), Body.local8Bit().length());
		if (!sender->closeStdin()) {
			delete sender;
		}
	} else {
		delete sender;
	}
}

void WinPopupLib::slotSendProcessExited(KProcess *p)
{
//	emit sendJobDone(p->pid());
	delete p;
}

void WinPopupLib::settingsChanged(const QString &smbClient, int groupFreq, int messageCheck)
{
	smbClientBin = smbClient;
	groupCheckFreq = groupFreq;
	messageCheckFreq = messageCheck;

	if (updateGroupDataTimer.isActive()) updateGroupDataTimer.changeInterval(groupCheckFreq * 1000);
	if (messageCheckTimer.isActive()) messageCheckTimer.changeInterval(messageCheckFreq * 1000);
}

#include "libwinpopup.moc"

// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
