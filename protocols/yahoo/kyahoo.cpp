/***************************************************************************
                          kyahoo.cpp  -  Yahoo Plugin
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Local Includes
#include "yahoodebug.h"
#include "kyahoo.h"

// Kopete Includes

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>





// Constructor
KYahoo::KYahoo() : QObject(0, "KYahoo")
{
	DEBUG(YDMETHOD, "KYahoo::KYahoo()");

    DEBUG(YDINFO, "Loading Yahoo Protocol Client...");;
    sktSocket = new QSocket;
}


KYahoo::~KYahoo()
{
	DEBUG(YDMETHOD, "KYahoo::~KYahoo()");
}


// Connect to Yahoo
void KYahoo::Connect(QString server, int port, QString username, QString password)
{
	DEBUG(YDMETHOD, "KYahoo::Connect(" << server << ", " << port << ", " << username
			<< ", " << "<password>)");

    m_Server   = server;
    m_Port     = port;
    m_Username = username;
    m_Password = password;

	// First step, read buddy list from msg.edit
	m_BuddyListServer = "msg.edit.yahoo.com";
	m_BuddyListPort   = 80;
    DEBUG(YDINFO, "Attempting to read buddy list from Yahoo server <" << 
			m_BuddyListServer << ">");

    connect(sktSocket, SIGNAL(connected()), this, SLOT(slotBuddyListConnected()));
    connect(sktSocket, SIGNAL(connectionClosed()), this, 
			SLOT(slotBuddyListDisconnected()));
    connect(sktSocket, SIGNAL(readyRead()), this, SLOT(slotBuddyListRead()));
    connect(sktSocket, SIGNAL(error(int)), this, SLOT(slotBuddyListError(int)));
    sktSocket->connectToHost(m_BuddyListServer, m_BuddyListPort);
}


void KYahoo::Disconnect()
{
	DEBUG(YDMETHOD, "KYahoo::Disconnect()");
}


// Connected to Buddy List Server
void KYahoo::slotBuddyListConnected()
{
	DEBUG(YDMETHOD, "KYahoo::slotBuddyListConnected()");

	QString buf = "";
	// XXX URLEncode user and password
	buf += "GET /config/ncclogin?.src=bl&login=" + m_Username + "&passwd=" + 
			m_Password + "&n=1 HTTP/1.0\r\n";
	buf += "Host: " + m_BuddyListServer + (m_BuddyListPort != 80 ? ":" + 
			m_BuddyListPort : "" ) + "\r\n";
	buf += "User-Agent: Mozilla/4.0\r\n";
	buf += "\r\n";
    sktSocket->writeBlock(buf, buf.length());
}


// Read from Buddy List Server
void KYahoo::slotBuddyListRead()
{
	DEBUG(YDMETHOD, "KYahoo::slotBuddyListRead()");

	// Read everything
    QCString data;
    unsigned long available_bytes = sktSocket->bytesAvailable();
    QByteArray read_data(available_bytes + 1);
    read_data.fill('\0');
    sktSocket->readBlock(read_data.data(), available_bytes);
    data = read_data.data();

    DEBUG(YDPROTOCOL, "Buddy List read from server: " << data);

	// XXX Check for invalid password here

	// Grab BUDDYLIST
	QRegExp exp("BEGIN BUDDYLIST(.+)END BUDDYLIST");
	exp.match(data);
	QString buddylist = exp.cap(1);

	DEBUG(YDPROTOCOL, "Buddy List buddies: " << buddylist);

	// Split
	QRegExp groups("\n(.+):(.+)\n");
	groups.setMinimal(TRUE);
	int pos = 0;
	while ( pos >= 0 ) {
		pos = groups.search( buddylist, pos );
		if ( pos > -1 ) {
			DEBUG(YDDEBUG, "Buddy List: group <" << groups.cap(1) << "> has <" << 
					groups.cap(2) << ">");
			pos  += groups.matchedLength()-1;

			QStringList buddies( QStringList::split( ",", groups.cap(2) ) );
			for ( QStringList::Iterator it = buddies.begin(); it != buddies.end(); ++it ) {
				DEBUG(YDINFO, "Adding buddy <" << *it << "> from group <" << 
						groups.cap(1) << ">");
				emit newContact(*it , *it, groups.cap(1));
			}
		}
	}
}


void KYahoo::slotBuddyListError(int error)
{
	DEBUG(YDMETHOD, "KYahoo::slotBuddyListError(" << error << ")");
}


void KYahoo::slotBuddyListDisconnected()
{
	DEBUG(YDMETHOD, "KYahoo::slotBuddyListDisconnected()");

	// XXX Should check if can't connect?

	// Connect to Yahoo Server
    DEBUG(YDINFO, "Attempting to connect to Yahoo server " <<
		m_Server << ":" << m_Port);

    sktSocket = new QSocket;
    connect(sktSocket, SIGNAL(connected()), this, SLOT(slotConnected()));
    connect(sktSocket, SIGNAL(error(int)), this, SLOT(slotError(int)));
    connect(sktSocket, SIGNAL(readyRead()), this, SLOT(slotRead()));
    connect(sktSocket, SIGNAL(connectionClosed()), this, SLOT(slotDisconnected()));

    sktSocket->connectToHost(m_Server, m_Port);
}


void KYahoo::slotConnected()
{
	DEBUG(YDMETHOD, "KYahoo::slotConnected()");
}

void KYahoo::slotDisconnected()
{
	DEBUG(YDMETHOD, "KYahoo::slotDisconnected()");
}

void KYahoo::slotError(int error)
{
	DEBUG(YDMETHOD, "KYahoo::slotError(" << error << ")");
}

void KYahoo::slotRead()
{
	DEBUG(YDMETHOD, "KYahoo::slotRead()");
}


#include "kyahoo.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

