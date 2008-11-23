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
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>

// KDE Includes
#include <KDebug>
#include <KMessageBox>
#include <KLocale>
#include <KDirLister>
#include <KToolInvocation>

// Kopete Includes
#include "kopeteuiglobal.h"

// Local Includes
#include "libwinpopup.h"

WinPopupLib::WinPopupLib(const QString &smbClient, int groupFreq)
	: smbClientBin(smbClient), groupCheckFreq(groupFreq)
{
	connect(&updateGroupDataTimer, SIGNAL(timeout()), this, SLOT(slotUpdateGroupData()));

	updateGroupDataTimer.setSingleShot(true);
	updateGroupDataTimer.start(1);
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
		connect(dirLister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(slotReadMessages(const KFileItemList &)));
		connect(dirLister, SIGNAL(completed()), this, SLOT(slotListCompleted()));
		dirLister->openUrl(KUrl(WP_POPUP_DIR));
	}
}

/**
 * return the group list
 */
QStringList WinPopupLib::getGroups() const
{
	QStringList ret;
	QMap<QString, WorkGroup>::const_iterator i;
	for (i = theGroups.constBegin(); i != theGroups.constEnd(); ++i)
		ret += i.key();

	return ret;
}

/**
 * return the host list
 */
QStringList WinPopupLib::getHosts(const QString &Group) const
{
	return theGroups[Group].Hosts();
}

/**
 * return if a host is in the host list
 */
bool WinPopupLib::checkHost(const QString &Name) const
{
	bool ret = false;

	foreach (WorkGroup tmpGroup, theGroups) {
		if (tmpGroup.Hosts().contains(Name.toUpper())) {
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
												  i18n("The working directory %1 does not exist.\n"
													   "If you have not yet configured anything for Samba please see\n"
													   "Install Into Samba (Configure... -> Account -> Edit) information\n"
													   "on how to do this.\n"
													   "Should the directory be created? (May require the root password)", WP_POPUP_DIR),
		                                          QString::fromLatin1("Winpopup"), KGuiItem(i18n("Create Directory")), KGuiItem(i18n("Do Not Create")));
		if (tmpYesNo == KMessageBox::Yes) {
			QStringList kdesuArgs = QStringList(QString("-c mkdir -p -m 0777 " + WP_POPUP_DIR));
			if (KToolInvocation::kdeinitExecWait("kdesu", kdesuArgs) == 0) return true;
		}
	} else {
		KFileItem tmpFileItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, KUrl(WP_POPUP_DIR));
		mode_t tmpPerms = tmpFileItem.permissions();

		if (tmpPerms != 0777) {

			kDebug(14170) << "Perms not ok!";

			int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
													  i18n("Permissions of the working directory "
														   "%1 are incorrect.\n"
														   "You will not receive messages if choose No.\n"
														   "You can also correct it manually (chmod 0777 %1), restart Kopete.\n"
														   "Fix this (may require the root password)?", WP_POPUP_DIR),
			                                          QString::fromLatin1("Winpopup"), KGuiItem(i18n("Fix")), KGuiItem(i18n("Do Not Fix")));
			if (tmpYesNo == KMessageBox::Yes) {
				QStringList kdesuArgs = QStringList(QString("-c chmod 0777 " + WP_POPUP_DIR));
				if (KToolInvocation::kdeinitExecWait("kdesu", kdesuArgs) == 0) return true;
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
	currentGroup.clear();

	// for Samba 3
	readGroupsProcess = new QProcess;
	QStringList args;
	args << "-N" << "-g" << "-L" << Host << "-";

	connect(readGroupsProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotReadProcessExited(int, QProcess::ExitStatus)));

	readGroupsProcess->setProcessChannelMode(QProcess::MergedChannels);
	readGroupsProcess->start(smbClientBin, args);
}

void WinPopupLib::slotReadProcessExited(int i, QProcess::ExitStatus status)
{
	if (i > 0 || status == QProcess::CrashExit) {
		todo.removeAll(currentHost);
		done += currentHost;
	} else {
		QByteArray outputData = readGroupsProcess->readAll();
		if (!outputData.isEmpty()) {
			QString outputString = QString::fromUtf8(outputData.data());
			QStringList outputList = outputString.split('\n');
			QRegExp group("Workgroup\\|(.[^\\|]+)\\|(.+)"), host("Server\\|(.[^\\|]+)\\|(.+)"),
					info("Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]"),
					error("Connection.*failed");
			foreach (QString line, outputList) {
				if (info.indexIn(line) != -1) currentGroup = info.cap(1);
				if (host.indexIn(line) != -1) currentHosts += host.cap(1);
				if (group.indexIn(line) != -1) currentGroups[group.cap(1)] = group.cap(2);
			}
		}

		delete readGroupsProcess;
		readGroupsProcess = 0;

		// Drop the first cycle - it's only the initial search host,
		// the next round are the real masters. GF

		if (passedInitialHost) {

			// move currentHost from todo to done
			todo.removeAll(currentHost);
			done += currentHost;

			if (!currentGroups.isEmpty()) {
				QMap<QString, WorkGroup> newGroups;
				//loop through the read groups and check for new ones
				QMap<QString, QString>::ConstIterator end = currentGroups.constEnd();
				for (QMap<QString, QString>::ConstIterator i = currentGroups.constBegin(); i != end; i++) {
					QString groupMaster = i.value();
					if (!done.contains(groupMaster)) todo += groupMaster;
				}
			}

			if (!currentGroup.isEmpty() && !currentHosts.isEmpty()) {
				// create a workgroup object and put the hosts in
				WorkGroup nWG;
				nWG.addHosts(currentHosts);

				currentGroupsMap.remove(currentGroup);
				currentGroupsMap.insert(currentGroup, nWG);
			}

		} else {
			passedInitialHost = true;
			if (!currentGroups.isEmpty()) {
				foreach (QString groupMaster, currentGroups) {
					todo += groupMaster;
				}
			} else {
				if (currentHost == QString::fromLatin1("failed"))
					KMessageBox::error(Kopete::UI::Global::mainWidget(),
									i18n("Connection to localhost failed.\n"
											"Is your samba server running?"),
									QString::fromLatin1("Winpopup"));
			}
		}
	}

	// maybe restart cycle
	if (todo.count()) {
		currentHost = todo.at(0);
		startReadProcess(currentHost);
	} else {
		theGroups = currentGroupsMap;
		updateGroupDataTimer.setSingleShot(true);
		updateGroupDataTimer.start(groupCheckFreq * 1000);
	}
}

void WinPopupLib::slotListCompleted()
{
	/// only to check received messages during start up, then we use newItems. GF
	disconnect(dirLister, SIGNAL(completed()), this, SLOT(slotListCompleted()));
	slotReadMessages(dirLister->items());
}

/**
 * read new arrived messages
 */
void WinPopupLib::slotReadMessages(const KFileItemList &items)
{
	KFileItem tmpItem;
	foreach (tmpItem, items) {
		if (tmpItem.isFile()) {
			QFile messageFile(tmpItem.url().path());

			if (messageFile.open(QIODevice::ReadOnly)) {
				QTextStream stream(&messageFile);
				QString sender;
				QDateTime time;
				QString text;

				// first line is sender, can this really be empty? GF
				sender = stream.readLine();
				sender = sender.toUpper();

				// second line is time
				QString tmpTime = stream.readLine();
				time = QDateTime::fromString(tmpTime, Qt::ISODate);

				while (!stream.atEnd()) {
					text.append(stream.readLine());
					text.append('\n');
				}

				// remove trailing CR
				text = text.trimmed();

				messageFile.close();

				// delete file
				if (!messageFile.remove()) {
					// QFile::remove() seems to be very persistent, it removes even files with 0444 owned by root
					// if the directory permissions are 0777 - so this is just for safety. GF
					kDebug(14170) << "Message file not removed - how that?";
					int tmpYesNo =  KMessageBox::warningYesNo(Kopete::UI::Global::mainWidget(),
															  i18n("A message file could not be removed; "
																   "maybe the permissions are incorrect.\n"
																   "Fix this (may require the root password)?"),
					                                          QString::fromLatin1("Winpopup"), KGuiItem(i18n("Fix")), KGuiItem(i18n("Do Not Fix")));
					if (tmpYesNo == KMessageBox::Yes) {
						QStringList kdesuArgs = QStringList(QString("-c chmod 0666 " + tmpItem.url().path()));
						if (KToolInvocation::kdeinitExecWait("kdesu", kdesuArgs) == 0) {
							if (!messageFile.remove())
								KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Still cannot remove it; please fix it manually."));
						}
					}
				}
				if (!sender.isEmpty() && time.isValid())
					emit signalNewMessage(text, time, sender);
				else
					kDebug(14170) << "Received invalid message!";
			}
		} // isFile
	} // foreach
}

/**
 * send a message
 */
void WinPopupLib::sendMessage(const QString &Body, const QString &Destination)
{
	QProcess *sender = new QProcess(this);
	QStringList args;
	args << "-M" << Destination << "-N" << "-";
	sender->start(smbClientBin, args);
	sender->write(Body.trimmed().toLocal8Bit());
	sender->closeWriteChannel();
}

void WinPopupLib::settingsChanged(const QString &smbClient, int groupFreq)
{
	smbClientBin = smbClient;
	groupCheckFreq = groupFreq;

	if (updateGroupDataTimer.isActive()) updateGroupDataTimer.setInterval(groupCheckFreq * 1000);
}

#include "libwinpopup.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
