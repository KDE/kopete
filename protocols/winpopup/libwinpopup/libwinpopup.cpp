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


// Compatibility problems?
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// QT Includes
#include <qfile.h>
#include <qprocess.h>
#include <qregexp.h>

// KDE Includes
#include <kapplication.h>
#include <kprocio.h>
#include <kprocess.h>
#include <kdebug.h>

// Local Includes
#include "libwinpopup.h"
#include "libwinpopup.moc"

void UpdateThread::run()
{
	theOwner->update(false);
}

KWinPopup::KWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName, int HostCheckFrequency, int MessageCheckFrequency): theUpdateThread(this), updating(1), reading(1)
{
	checkForMessages.connect(&checkForMessages, SIGNAL(timeout()), this, SLOT(doCheck()));
	checkForMessages.start(MessageCheckFrequency * 1000, false);
	connect(&updateData, SIGNAL(timeout()), this, SLOT(updateInBackground()));
	updateData.start(HostCheckFrequency * 1000, false);
	mySMBClientPath = SMBClientPath;
	myInitialSearchHost = InitialSearchHost;
	myHostName = HostName;
	myMessageCheckFrequency = MessageCheckFrequency;
	myHostCheckFrequency = HostCheckFrequency;
	updateNoWait();
	online = true;
}

KWinPopup::~KWinPopup()
{
}

void KWinPopup::goOffline()
{
	if(!online) return;
	updateData.stop();
	checkForMessages.stop();
	theGroups.clear();
	online = false;
}

void KWinPopup::goOnline()
{
	if(online) return;
	checkForMessages.start(myMessageCheckFrequency * 1000, false);
	updateData.start(myHostCheckFrequency * 1000, false);
	updateInBackground();
	online = true;
}

pid_t KWinPopup::sendMessage(const QString &Body, const QString &Destination)
{
	/* we are the parent of the process */
	KProcess *sender = new KProcess(this);
	/* set the executable */
	*sender <<  mySMBClientPath;
	/* set command line argumets */
	*sender << "-M" << Destination << "-N" << "-U" << myHostName;
	*sender << Body;
	QObject::connect(sender, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendProcessExited(KProcess *)));
	/* default is NotifyOnExit */
	sender->start();
	return sender->pid();
}

void KWinPopup::slotSendProcessExited(KProcess *p)
{
	emit sendJobDone(p->pid());
	delete p;
}

void KWinPopup::messageHandler()
{
	QFile infile(WP_MESSAGE_FILE);
	if(infile.open(IO_ReadOnly))
	{
		QString From = "", Text = "";
		QDateTime Date;
		QTextStream fin(&infile);

		From = fin.readLine();
		if(From == "" || fin.eof()) { infile.close(); return; }

		QString DateString = fin.readLine();
		if(DateString == "" || fin.eof()) { infile.close(); return; }
		Date = QDateTime::fromString(DateString, Qt::ISODate);

		for(; !fin.eof(); )
			Text += fin.readLine() + "\n";
		infile.close();

		receivedMessage(Text, Date, From);
	}
}

time_t getLastMod()
{
	struct stat StatBuf;
	if(stat(WP_MESSAGE_FILE, &StatBuf))
 		return 0;
	return StatBuf.st_mtime;
}

void KWinPopup::doCheck()
{
	static time_t theLastMod = 0;
	time_t newLastMod = getLastMod();
	if(!theLastMod) theLastMod = newLastMod;
	if(newLastMod != theLastMod)
	{
		theLastMod = newLastMod;
		messageHandler();
	}
}

void KWinPopup::update(bool Wait)
{
//	qDebug("Starting update(%d)", Wait);
	if(updating.tryAccess(1))
	{	doUpdate();
		updating--;
	}
	else
		if(Wait)
		{	updating++;
			updating--;
		}
//	qDebug("Leaving update");
}

void KWinPopup::doUpdate()
{
//	qDebug("Got initial search host: %s", myInitialSearchHost.latin1());

	// get our domain master
	QString theGroup;
	QString theMaster = grabData(myInitialSearchHost, &theGroup).second[theGroup];

//	qDebug("Got master: %s", theMaster.latin1());

	QStringList done, todo;	// contain servers, _not_ groups
	todo += theMaster;

	// get groups & our hosts
	QMap<QString, WorkGroup> newGroups;
	while(todo.count())
	{
//		qDebug("Searching server: %s", todo[0].latin1());

		// move one item from the todo to done.
		QString thisServer = todo[0], thisGroup;
		todo.remove(thisServer);
		done += thisServer;

		// grab data
		QPair<stringMap, stringMap> thisPair = grabData(thisServer, &thisGroup);

		// put all new workgroups on todo list
		for(stringMap::Iterator i = thisPair.second.begin(); i != thisPair.second.end(); i++)
			if( (!todo.contains(i.data())) && (!done.contains(i.data())) )
				todo += i.data();

		// create & insert the workgroup object
		WorkGroup nWG(thisServer);
		for(stringMap::Iterator i = thisPair.first.begin(); i != thisPair.first.end(); i++)
			nWG.Hosts[i.key()] = Host(i.data());
		newGroups[thisGroup] = nWG;
	}

	// actually write the groups now
	reading++;
	theGroups = newGroups;
	reading--;
//	qDebug("Leaving doUpdate.");
}

QPair<stringMap, stringMap> KWinPopup::grabData(const QString &Host, QString *theGroup, QString *theOS, QString *theSoftware)	// populates theGroup!
{
	static int times = 0;
	times++;
	QProcess *sender = new QProcess();
	sender->addArgument(mySMBClientPath);
	sender->addArgument("-L");
	sender->addArgument(Host);
	sender->addArgument("-N");
	connect(sender, SIGNAL(destroyed()), sender, SLOT(kill()));
	if(!sender->launch(""))
	{
		kdDebug( 14170 ) << k_funcinfo << "Couldn't launch smbclient (" << times << ")" << endl;
		return QPair<stringMap, stringMap>();
	}

	int Phase = 0;
	QRegExp pair("^\\t([^\\s]+)(\\s+([^\\s].*))?$"), base("^\\t([^\\s]+)\\s*$"), sep("^\\t-{9}"), info("^Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]");

	stringMap hosts, groups;

	while(sender->isRunning() || sender->canReadLineStdout())
	{
		while(!sender->canReadLineStdout() && sender->isRunning());
		QString Line = sender->readLineStdout();
		if(Phase == 0 && info.search(Line) != -1)
		{	if(theGroup) *theGroup = info.cap(1);
			if(theOS) *theOS = info.cap(2);
			if(theSoftware) *theSoftware = info.cap(3);
		}
		if(Phase == 4 && pair.search(Line) != -1)
			hosts[pair.cap(1)] = pair.cap(3);
		if(Phase == 4 && base.search(Line) != -1)
			hosts[base.cap(1)] = "";
		if(Phase == 6 && pair.search(Line) != -1)
			groups[pair.cap(1)] = pair.cap(3);
		if(sep.search(Line) != -1 || Line.isEmpty()) Phase++;
	}

	delete sender;

	return QPair<stringMap, stringMap>(hosts, groups);
}

QPair<stringMap, stringMap> KWinPopup::newGrabData(const QString &Host, QString *theGroup, QString *theOS, QString *theSoftware)	// populates theGroup!
{
	KProcIO *sender = new KProcIO();
	*sender << mySMBClientPath << "-L" << Host << "-N";

	if(!sender->start())
	{
		kdDebug( 14170 ) << k_funcinfo << "Couldn't launch smbclient" << endl;
		return QPair<stringMap, stringMap>();
	}
	sender->closeStdin();

	int Phase = 0;
	QRegExp pair("^\\t([^\\s]+)\\s+([^\\s].*)$"), base("^\\t([^\\s]+)\\s*$"), sep("^\\t-{9}"), info("^Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]");
	stringMap hosts, groups;


	for(QString Line; sender->isRunning() || sender->readln(Line) > -1; Line = "")
	{
		if(Line.isEmpty()) while(sender->readln(Line) == -1 && sender->isRunning());
		if(Phase == 0 && info.search(Line) != -1)
		{	if(theGroup) *theGroup = info.cap(1);
			if(theOS) *theOS = info.cap(2);
			if(theSoftware) *theSoftware = info.cap(3);
		}
		if(Phase == 4 && pair.search(Line) != -1)
			hosts[pair.cap(1)] = pair.cap(2);
		if(Phase == 4 && base.search(Line) != -1)
			hosts[base.cap(1)] = "";
		if(Phase == 6 && pair.search(Line) != -1)
			groups[pair.cap(1)] = pair.cap(2);
		if(sep.search(Line) != -1 || Line.isEmpty()) Phase++;
	}

	sender->closeStdout();
	sender->closeStderr();
	delete sender;

	return QPair<stringMap, stringMap>(hosts, groups);
}

const Host KWinPopup::getHostInfo(const QString &Group, const QString &aHost)
{
	reading++;
	Host ret = theGroups[Group].Hosts[aHost];
	reading--;
	return ret;
}

const QStringList KWinPopup::getHostDetails( const QString &Host )
{
	QStringList theHostDetails;
	QString theGroup, theOS, theSoftware;

	//QPair<stringMap, stringMap> thisPair = grabData( Host, &theGroup, &theOS, &theSoftware );
	grabData( Host, &theGroup, &theOS, &theSoftware );

	theHostDetails << theGroup << theOS << theSoftware;
	return( theHostDetails );
}

const QStringList KWinPopup::getGroups()
{
	QStringList ret;
	update();

	reading++;
	for(QMap<QString, WorkGroup>::Iterator i = theGroups.begin(); i != theGroups.end(); i++)
		ret += i.key();
	reading--;

	return ret;
}

const QStringList KWinPopup::getHosts(const QString &Group)
{
	QStringList ret;

	reading++;
	for(QMap<QString, Host>::Iterator i = theGroups[Group].Hosts.begin(); i != theGroups[Group].Hosts.end(); i++)
		ret += i.key();
	reading--;

	return ret;
}

bool KWinPopup::checkHost(const QString &Name)
{
	bool ret = false;

	reading++;
	for(QMap<QString, WorkGroup>::Iterator i = theGroups.begin(); i != theGroups.end() && !ret; i++)
		if((*i).Hosts.contains(Name)) ret = true;
	reading--;

	return ret;
}

//#include "libwinpopup.moc.cpp"

// vim: set noet ts=4 sts=4 sw=4:

