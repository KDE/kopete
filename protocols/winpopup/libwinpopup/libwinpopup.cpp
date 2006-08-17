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
#include <kdirlister.h>

// Kopete Includes
#include "kopeteuiglobal.h"

// Local Includes
#include "libwinpopup.h"

WinPopupLib::WinPopupLib(const QString &smbClient,int groupFreq)
	: smbClientBin(smbClient), groupCheckFreq(groupFreq)
{
	connect(&updateGroupDataTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGroupData()));

	updateGroupDataTimer.start(1, true);
	QTimer::singleShot(1, this, SLOT(slotStartDirLister()));
}

WinPopupLib::~WinPopupLib()
{
}

void WinPopupLib::slotStartDirLister()
{
	if (checkMessageDir()) {
		dirLister = new KDirLister();
		dirLister->setAutoUpdate(true);
		connect(dirLister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(slotNewMessages(const KFileItemList &)));
		connect(dirLister, SIGNAL(completed()), this, SLOT(slotListCompleted()));
		dirLister->openURL(KURL::fromPathOrURL(WP_POPUP_DIR));
	}
}

/**
 * return the group list
 */
const QStringList WinPopupLib::getGroups()
{
	QStringList ret;
	QMap<QString, WorkGroup>::ConstIterator end = theGroups.end();
	for(QMap<QString, WorkGroup>::ConstIterator i = theGroups.begin(); i != end; i++)
		ret += i.key();

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

	QMap<QString, WorkGroup>::Iterator end = theGroups.end();
	for(QMap<QString, WorkGroup>::Iterator i = theGroups.begin(); i != end && !ret; i++) {
		if ((*i).Hosts().contains(Name.upper())) {
			ret = true;
			break;
		}
	}

	return ret;
}


bool WinPopupLib::checkMessageDir()
{
	QDir dir(WP_POPUP_DIR);
	if (! dir.exists()) {
		int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
												  i18n("Working directory %1 does not exist.\n"
													   "If you have not configured anything yet (samba) please see\n"
													   "Install Into Samba (Configure... -> Account -> Edit) information\n"
													   "on how to do this.\n"
													   "Should the directory be created? (May need root password)").arg(WP_POPUP_DIR),
												  QString::fromLatin1("Winpopup"), i18n("Create Directory"), i18n("Do Not Create"));
		if (tmpYesNo == KMessageBox::Yes) {
			QStringList kdesuArgs = QStringList(QString("-c mkdir -p -m 0777 " + WP_POPUP_DIR));
			if (KApplication::kdeinitExecWait("kdesu", kdesuArgs) == 0) return true;
		}
	} else {
		KFileItem tmpFileItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, KURL::fromPathOrURL(WP_POPUP_DIR));
		mode_t tmpPerms = tmpFileItem.permissions();

		if (tmpPerms != 0777) {

			kdDebug(14170) << "Perms not ok!" << endl;

			int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
													  i18n("Permissions of the working directory "
														   "%1 are wrong!\n"
														   "You will not receive messages if you say no.\n"
														   "You can also correct it manually (chmod 0777 %1) and restart kopete.\n"
														   "Fix? (May need root password)").arg(WP_POPUP_DIR),
													  QString::fromLatin1("Winpopup"), i18n("Fix"), i18n("Do Not Fix"));
			if (tmpYesNo == KMessageBox::Yes) {
				QStringList kdesuArgs = QStringList(QString("-c chmod 0777 " + WP_POPUP_DIR));
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
	todo.clear();
	currentGroupsMap.clear();
	currentHost = QString::fromLatin1("LOCALHOST");
	startReadProcess(currentHost);
}

void WinPopupLib::startReadProcess(const QString &Host)
{
	currentHosts.clear();
	currentGroups.clear();
	currentGroup = QString();

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
			info("^Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]"),
			error("Connection.*failed");

	while (r->readln(tmpLine) > -1) {
		if (info.search(tmpLine) != -1) currentGroup = info.cap(1);
		if (host.search(tmpLine) != -1) currentHosts += host.cap(1);
		if (group.search(tmpLine) != -1) currentGroups[group.cap(1)] = group.cap(2);
		if (error.search(tmpLine) != -1) {
			kdDebug(14170) << "Connection to " << currentHost << " failed!" << endl;
			if (currentHost == QString::fromLatin1("LOCALHOST")) currentHost = QString::fromLatin1("failed"); // to be sure
		}
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

		if (!currentGroups.isEmpty()) {
			QMap<QString, WorkGroup> newGroups;
			//loop through the read groups and check for new ones
			QMap<QString, QString>::ConstIterator end = currentGroups.end();
			for (QMap<QString, QString>::ConstIterator i = currentGroups.begin(); i != end; i++) {
				QString groupMaster = i.data();
				if (!done.contains(groupMaster)) todo += groupMaster;
			}
		}

		if (!currentGroup.isEmpty() && !currentHosts.isEmpty()) {
			// create a workgroup object and put the hosts in
			WorkGroup nWG;
			nWG.addHosts(currentHosts);

			currentGroupsMap.insert(currentGroup, nWG, true);
		}

	} else {
		passedInitialHost = true;
		if (!currentGroups.isEmpty()) {
			QMap<QString, QString>::ConstIterator end = currentGroups.end();
			for (QMap<QString, QString>::ConstIterator i = currentGroups.begin(); i != end; i++) {
				QString groupMaster = i.data();
				todo += groupMaster;
			}
		} else {
			if (currentHost == QString::fromLatin1("failed"))
				KMessageBox::error(Kopete::UI::Global::mainWidget(),
								   i18n("Connection to localhost failed!\n"
									    "Is your samba server running?"),
								   QString::fromLatin1("Winpopup"));
		}
	}

	// maybe restart cycle
	if (todo.count()) {
		currentHost = todo[0];
		startReadProcess(currentHost);
	} else {
		theGroups = currentGroupsMap;
		updateGroupDataTimer.start(groupCheckFreq * 1000, true);
	}
}

void WinPopupLib::slotListCompleted()
{
	/// only to check received messages during start up, then we use newItems. GF
	disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListCompleted()));
	readMessages(dirLister->items());
}

void WinPopupLib::slotNewMessages(const KFileItemList &items)
{
	readMessages(items);
}

/**
 * read new arrived messages
 */
void WinPopupLib::readMessages(const KFileItemList &items)
{
	QPtrListIterator<KFileItem> it(items);
	KFileItem *tmpItem;
	while ((tmpItem = it.current()) != 0) {
		if (tmpItem->isFile()) {
			QFile messageFile(tmpItem->url().path());

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
					int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
															  i18n("A message file could not be removed; "
																   "maybe the permissions are wrong.\n"
																   "Fix? (May need root password)"),
															  QString::fromLatin1("Winpopup"), i18n("Fix"), i18n("Do Not Fix"));
					if (tmpYesNo == KMessageBox::Yes) {
						QStringList kdesuArgs = QStringList(QString("-c chmod 0666 " + tmpItem->url().path()));
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
		} // isFile
		++it;
	} // while
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

void WinPopupLib::settingsChanged(const QString &smbClient, int groupFreq)
{
	smbClientBin = smbClient;
	groupCheckFreq = groupFreq;

	if (updateGroupDataTimer.isActive()) updateGroupDataTimer.changeInterval(groupCheckFreq * 1000);
}

#include "libwinpopup.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
