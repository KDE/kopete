// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2003 Zack Rusin 		<zack@kde.org>
//
// gaduaccount.h
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
#include "gaducontactlist.h"
#include "libgadu/libgadu.h"

#include <qhostaddress.h>
#include <qmap.h>
#include <qstring.h>
#include <qptrlist.h>
#include <kaction.h>
#include <kfiledialog.h>

class KopeteAccount;
class GaduContact;
class GaduProtocol;
class GaduSession;
class GaduCommand;
class QTimer;
class KActionMenu;
class KopeteMessage;

class GaduAccount : public KopeteAccount
{
	Q_OBJECT

public:
	GaduAccount( KopeteProtocol*, const QString& accountID,  const char* name = 0L );
	//{
	void setAway( bool isAway, const QString& awayMessage = QString::null );
	KActionMenu* actionMenu();
	//}
	enum tlsConnection{ TLS_ifAvaliable=0, TLS_only, TLS_no };

public slots:
	//{
	void connect();
	void disconnect();
	//}

	void changeStatus( const KopeteOnlineStatus& status, const QString& descr = QString::null );
	void slotLogin( int status = GG_STATUS_AVAIL, const QString& dscr = QString::null );
	void slotLogoff();
	void slotGoOnline();
	void slotGoOffline();
	void slotGoInvisible();
	void slotGoBusy();
	void slotDescription();
	void slotSearch( int uin = 0);

	void removeContact( const GaduContact* c );

	void addNotify( uin_t uin );
	void notify( uin_t* userlist, int count );

	void sendMessage( uin_t recipient, const KopeteMessage& msg,
			int msgClass = GG_CLASS_CHAT );

	void error( const QString& title, const QString& message );

	void pong();
	void pingServer();

	// those two functions are connected straight to gadusession ones
	// with the same names/params. This was the easiest way to
	// make this interface public
	bool pubDirSearch( QString& name, QString& surname, QString& nick,
			    int UIN, QString& city, int gender,
			    int ageFrom, int ageTo, bool onlyAlive );
	void pubDirSearchClose();

	// tls
	tlsConnection useTls();
	void setUseTls( tlsConnection  ut );

signals:
	void pubDirSearchResult( const SearchResult& );

protected slots:
	virtual void loaded();

protected:
	//{
	bool addContactToMetaContact( const QString& contactId,
			const QString& displayName,
			KopeteMetaContact* parentContact );
	//}

private slots:
	void startNotify();
	void notify( KGaduNotifyList* );

	void messageReceived( KGaduMessage* );
	void ackReceived( unsigned int );
	void contactStatusChanged( KGaduNotify* );
	void slotSessionDisconnect();

	void slotExportContactsList();
	void slotExportContactsListToFile();
	void slotImportContactsFromFile();
	void slotFriendsMode();

	void userlist( const QString& contacts );
	GaduContactsList* userlist();

	void connectionFailed( gg_failure_t failure );
	void connectionSucceed( );

	void slotChangePassword();

	void slotCommandDone( const QString&, const QString& );
	void slotCommandError( const QString&, const QString& );

	void slotSearchResult( const SearchResult& result );
	void userListExportDone();

private:
	void initConnections();
	void initActions();

	QPtrList<GaduCommand> commandList_;

	GaduSession*		session_;

	QTimer*			pingTimer_;

	KopeteOnlineStatus	status_;
	QString			nick_;

	QTextCodec*		textcodec_;
	KFileDialog*		saveListDialog;
	KFileDialog*		loadListDialog;

	KActionMenu*		actionMenu_;
	KAction*		searchAction;
	KAction*		listputAction;
	KAction*		listToFileAction;
	KAction*		listFromFileAction;
	KAction*		friendsModeAction;
	bool			connectWithSSL;

	int			currentServer;
	QValueList<QHostAddress> servers_;
	unsigned int		serverIP;

	QString			lastDescription;
	bool			forFriends;
};

#endif
