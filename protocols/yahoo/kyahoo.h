/***************************************************************************
            kyahoo.h  -  Implementation for the Yahoo protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KYAHOO_H
#define KYAHOO_H


// Local Includes

// Kopete Includes

// QT Includes
#include <qobject.h>
#include <qsocket.h>

// KDE Includes


// Yahoo Protocol Connection
class KYahoo : public QObject {
	Q_OBJECT public:
		KYahoo();
		~KYahoo();

    public slots: 
		void Connect(QString server, int port, QString username, 
				QString password); // Connect
		void Disconnect();

		void slotBuddyListConnected(); // Buddy List Server Connection
		void slotBuddyListDisconnected();
		void slotBuddyListRead();
		void slotBuddyListError(int error);

		void slotConnected();
		void slotDisconnected();
		void slotRead();
		void slotError(int error);

	signals: 
		void newContact(QString, QString, QString); // Create new contact in parent

	private:
		QSocket *sktSocket; // Socket
		QString m_Username, m_Password, m_Server; // User data
		int m_Port;

		QString m_BuddyListServer; // Buddy List server
		int m_BuddyListPort;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:
