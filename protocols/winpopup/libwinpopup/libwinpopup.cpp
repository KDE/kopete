/***************************************************************************
                          libwinpopup.cpp  -  WP Library
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// Local Includes
#include "libwinpopup.h"

// QT Includes
#include <qprocess.h>
#include <qdatetime.h>
#include <qthread.h>
#include <qsemaphore.h>
#include <qregexp.h>

// KDE Includes
#include <kapplication.h>

void UpdateThread::run()
{
	theOwner->update(false);
}

KWinPopup::KWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName): theUpdateThread(this), updating(1), reading(1)
{
	checkForMessages.connect(&checkForMessages, SIGNAL(timeout()), this, SLOT(doCheck()));
	checkForMessages.start(1000, false);
	connect(&updateData, SIGNAL(timeout()), this, SLOT(updateNoWait()));
	updateData.start(10000, false);
	mySMBClientPath = SMBClientPath;
	myInitialSearchHost = InitialSearchHost;
	myHostName = HostName;
}

KWinPopup::~KWinPopup()
{
}

bool KWinPopup::sendMessage(const QString &Body, const QString &Destination)
{
	QProcess sender;
	sender.addArgument(mySMBClientPath);
	sender.addArgument("-M");
	sender.addArgument(Destination);
	sender.addArgument("-N");
	sender.addArgument("-U");
	sender.addArgument(myHostName);
	if(!sender.launch(Body + "\n")) return 1;

/*	sender.writeToStdin(Body);
	sender.writeToStdin("\n");
	sender.closeStdin();
*/
	int i;
	for(i = 0; i < 150 && sender.isRunning(); i++)
	{	KApplication::kApplication()->processEvents();
		usleep(100000);
	}

	return i < 150;
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
		Date = QDateTime::fromString(fin.readLine());

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
	if(updating.tryAccess(1))
	{	doUpdate();
		updating--;
	}
	else
		if(Wait)
		{	updating++;
			updating--;
		}
}

void KWinPopup::doUpdate()
{	
	QMap<QString, WorkGroup> newGroups;

	// get our domain master
	QString theGroup;
	QString theMaster = grabData(myInitialSearchHost, &theGroup).second[theGroup];

	QStringList done, todo;	// contain servers, _not_ groups
	todo += theMaster;

	// get groups & our hosts
	while(todo.count())
	{
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
}

QPair<stringMap, stringMap> KWinPopup::grabData(const QString &Host, QString *theGroup, QString *theOS, QString *theSoftware)	// populates theGroup!
{
	QProcess sender;
	sender.addArgument(mySMBClientPath);
	sender.addArgument("-L");
	sender.addArgument(Host);
	sender.addArgument("-N");
	if(!sender.launch(""))
	{	qDebug("Couldn't launch smbclient!");
		return QPair<stringMap, stringMap>();
	}

	int Phase = 0;
	QRegExp pair("^\\t([^\\s]+)\\s+([^\\s].*)$"), base("^\\t([^\\s]+)\\s*$"), sep("^\\t-{9}"), info("^Domain=\\[([^\\]]+)\\] OS=\\[([^\\]]+)\\] Server=\\[([^\\]]+)\\]");
	
	stringMap hosts, groups;

	while(sender.isRunning() || sender.canReadLineStdout())
	{	QString Line = sender.readLineStdout();
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
		if(sep.search(Line) != -1 || Line == "") Phase++;
	}

	return QPair<stringMap, stringMap>(hosts, groups);
}

const Host KWinPopup::getHostInfo(const QString &Group, const QString &aHost)
{
	reading++;
	Host ret = theGroups[Group].Hosts[aHost];
	reading--;
	return ret;
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

