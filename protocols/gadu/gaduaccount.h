// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
// gaduaccount.h
//
// Copyright (C)	2003	Zack Rusin <zack@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
#ifndef GADUACCOUNT_H
#define GADUACCOUNT_H

#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopeteonlinestatus.h"

#include "libgadu.h"

#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>

class GaduContact;
class GaduProtocol;
class GaduSession;
class GaduCommand;
class QTimer;
class KActionMenu;

class GaduAccount : public KopeteAccount
{
	Q_OBJECT
public:
	typedef QMap< uin_t, GaduContact* > ContactsMap;
	GaduAccount( KopeteProtocol* parent, const QString& accountID,
							 const char* name=0L );
	//{
	void setAway( bool isAway, const QString& awayMessage = QString::null );
	KopeteContact* myself() const;
	KActionMenu* actionMenu();
	//}
public slots:
	//{
	void connect();
	void disconnect();
	//}

	void changeStatus( const KopeteOnlineStatus& status, const QString& descr=QString::null );
	void slotLogin( int status = GG_STATUS_AVAIL, const QString& dscr = QString::null );
	void slotLogoff();
	void slotGoOnline();
	void slotGoOffline();
	void slotGoInvisible();
	void slotGoBusy();
	void slotDescription();

	void removeContact( const GaduContact* c );

	void addNotify( uin_t uin );
	void notify( uin_t* userlist, int count );
	void sendMessage( uin_t recipient, const QString& msg, int msgClass=GG_CLASS_CHAT );
	void error( const QString& title, const QString& message );
	void pong();
	void pingServer();

protected:
	//{
	bool addContactToMetaContact( const QString &contactId, const QString &displayName,
																KopeteMetaContact *parentContact );
	//}
private slots:
	void startNotify();
	void messageReceived( struct gg_event* e );
	void ackReceived( struct gg_event* /* e */ );
	void notify( struct gg_event* e );
	void notifyDescription( struct gg_event* e );
	void statusChanged( struct gg_event* e );
	void slotSessionDisconnect();
	void userlist( const QString& );
	void connectionFailed( struct gg_event* /*e*/ );
	void connectionSucceed( struct gg_event* /*e*/ );

	void slotChangePassword();

	void slotCommandDone( const QString&, const QString& );
	void slotCommandError( const QString&, const QString& );
private:
	void initConnections();
	void initActions();

	GaduSession* session_;
	QPtrList<GaduCommand> commandList_;
	ContactsMap contactsMap_;

	KActionMenu *actionMenu_;

	QTimer	*pingTimer_;

	GaduContact* myself_;
	KopeteOnlineStatus status_;
	QString	nick_;
};

#endif
