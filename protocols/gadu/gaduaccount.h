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

#include "gaducommands.h"
#include "gadusession.h"
#include "libgadu.h"

#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>
#include <kaction.h>

class KopeteAccount;
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

	// those two functions are connected straight to gadusession ones
	// with the same names/params. This was the easiest way to
	// make this interface public
	bool pubDirSearch(QString &name, QString &surname, QString &nick, 
			    int UIN, QString &city, int gender, 
			    int ageFrom, int ageTo, bool onlyAlive);
                            
	void pubDirSearchClose();
	
	// tls 
	bool isConnectionEncrypted();
	void useTls( bool ut );
	
signals:
	void pubDirSearchResult( const searchResult & );

protected slots:
	virtual void loaded();

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
	void statusChanged( struct gg_event* e );
	void slotSessionDisconnect();
	void userlist( const gaduContactsList& );
	void connectionFailed( struct gg_event* /*e*/ );
	void connectionSucceed( struct gg_event* /*e*/ );

	void slotChangePassword();
	void slotSearch();
	
	void slotCommandDone( const QString&, const QString& );
	void slotCommandError( const QString&, const QString& );
	void slotSearchResult( const searchResult &result );
	
private:
	void initConnections();
	void initActions();

	GaduSession* session_;
	QPtrList<GaduCommand> commandList_;
	
	KActionMenu *actionMenu_;

	QTimer	*pingTimer_;

	GaduContact* myself_;
	KopeteOnlineStatus status_;
	QString	nick_;
	
	QTextCodec *textcodec_;

	KAction* onlineAction;
	KAction* offlineAction;
	KAction* busyAction;
	KAction* invisibleAction;
	KAction* descrAction;
	KAction* searchAction;
	
	bool isUsingTls;
};

#endif
