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

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

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

	static KYahoo *engine();

	void slotBuddyListConnected(); // Buddy List Server Connection
	void slotBuddyListDisconnected();
	void slotBuddyListRead();
	void slotBuddyListError(int error);

	void slotConnected();
	void slotDisconnected();
	void slotRead();
	void slotError(int error);

	/* Callback wrappers */
	void yahooLoginResponseReceiver(int id, int succ, char *url);
	void yahooGotBuddiesReceiver(int id, YList * buds);
    /*
	TODO RECEIVERS:
	void yahoo_status_changed(int id, char *who, int stat, char *msg, int away);
	void yahoo_got_im(int id, char *who, char *msg, long tm, int stat);
	void yahoo_got_conf_invite(int id, char *who, char *room, char *msg, char **members);
	void yahoo_conf_userdecline(int id, char *who, char *room, char *msg);
	void yahoo_conf_userjoin(int id, char *who, char *room);
	void yahoo_conf_userleave(int id, char *who, char *room);
	void yahoo_conf_message(int id, char *who, char *room, char *msg);
	void yahoo_got_file(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize);
	void yahoo_contact_added(int id, char *myid, char *who, char *msg);
	void yahoo_rejected(int id, char *who, char *msg);
	void yahoo_typing_notify(int id, char *who, int stat);
	void yahoo_game_notify(int id, char *who, int stat);
	void yahoo_mail_notify(int id, char *from, char *subj, int cnt);
	void yahoo_system_message(int id, char *msg);
	void yahoo_error(int id, char *err, int fatal);
	void yahoo_add_handler(int id, int fd, yahoo_input_condition cond);
	void yahoo_remove_handler(int id, int fd);
	int yahoo_connect(char *host, int port);
    */
signals:
	void loginResponse(int id, int succ, char *url);
	void gotBuddies(int id, YList * buds);

	/*
	TODO SIGNALS:
	void yahoo_status_changed(int id, char *who, int stat, char *msg, int away);
	void yahoo_got_im(int id, char *who, char *msg, long tm, int stat);
	void yahoo_got_conf_invite(int id, char *who, char *room, char *msg, char **members);
	void yahoo_conf_userdecline(int id, char *who, char *room, char *msg);
	void yahoo_conf_userjoin(int id, char *who, char *room);
	void yahoo_conf_userleave(int id, char *who, char *room);
	void yahoo_conf_message(int id, char *who, char *room, char *msg);
	void yahoo_got_file(int id, char *who, char *url, long expires, char *msg, char *fname, unsigned long fesize);
	void yahoo_contact_added(int id, char *myid, char *who, char *msg);
	void yahoo_rejected(int id, char *who, char *msg);
	void yahoo_typing_notify(int id, char *who, int stat);
	void yahoo_game_notify(int id, char *who, int stat);
	void yahoo_mail_notify(int id, char *from, char *subj, int cnt);
	void yahoo_system_message(int id, char *msg);
	void yahoo_error(int id, char *err, int fatal);
	void yahoo_add_handler(int id, int fd, yahoo_input_condition cond);
	void yahoo_remove_handler(int id, int fd);
	int yahoo_connect(char *host, int port);
    */


	 
	void newContact(QString, QString, QString); // Create new contact in parent

private:
	/* Callback wrappers */
	
	QSocket *sktSocket; // Socket
	QString m_Username, m_Password, m_Server; // User data
	int m_Port;

	QString m_BuddyListServer; // Buddy List server
	int m_BuddyListPort;

	static KYahoo* engineStatic_;
};

#endif


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

