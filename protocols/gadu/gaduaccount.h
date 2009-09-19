// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef GADUACCOUNT_H
#define GADUACCOUNT_H

#include "kopetepasswordedaccount.h"
#include "kopeteonlinestatus.h"
#include "kopetecontact.h"

#include "gaducontactlist.h"
#include "gadusession.h"

#include <libgadu.h>

#include <qhostaddress.h>
#include <qmap.h>
#include <qstring.h>
#include <kaction.h>
#include <kfiledialog.h>

class GaduAccountPrivate;

class GaduContact;
namespace Kopete { class Protocol; }
namespace Kopete { class Message; }
namespace Kopete { class StatusMessage; }
class KActionMenu;
class GaduDCC;

class GaduAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT

public:
	GaduAccount( Kopete::Protocol*, const QString& accountID );
	~GaduAccount();
	//{
	void setAway( bool isAway, const QString& awayMessage = QString() );
	virtual void fillActionMenu( KActionMenu *actionMenu );
	virtual bool hasCustomStatusMenu() const;
	void dccRequest( GaduContact* );
	void sendFile( GaduContact* , QString& );
	//}
	enum tlsConnection{ TLS_ifAvaliable = 0, TLS_only, TLS_no };
	unsigned int getPersonalInformation();
	bool publishPersonalInformation( ResLine& d );

public slots:
	//{
	void connectWithPassword(const QString &password);
	void disconnect( DisconnectReason );
	void disconnect();
	void setOnlineStatus( const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                      const OnlineStatusOptions& options = None );
	void setStatusMessage( const Kopete::StatusMessage& statusMessage );
	//}

	void changeStatus( const Kopete::OnlineStatus& status, const QString& descr = QString() );
	void slotLogin( int status = GG_STATUS_AVAIL, const QString& dscr = QString() );
	void slotLogoff();
	void slotGoOnline();
	void slotGoOffline();
	void slotGoInvisible();
	void slotGoBusy();
	void slotDescription();
	void slotSearch( int uin = 0);

	void removeContact( const GaduContact* );

	void addNotify( uin_t );
	void notify( uin_t*, int );

	void sendMessage( uin_t recipient, const Kopete::Message& msg,
			int msgClass = GG_CLASS_CHAT );

	void error( const QString& title, const QString& message );

	void pong();
	void pingServer();

	// those two functions are connected straight to gadusession ones
	// with the same names/params. This was the easiest way to
	// make this interface public
	unsigned int pubDirSearch(  ResLine& query, int ageFrom, int ageTo, bool onlyAlive );
	void pubDirSearchClose();

	// tls
	tlsConnection useTls();
	void setUseTls( tlsConnection );

	// dcc
	bool dccEnabled();
	bool setDcc( bool );

	// anons
	bool ignoreAnons();
	void setIgnoreAnons( bool );

	// export list on change
	bool exportListOnChange();
	void setExportListOnChange( bool );

	// import list on startup
	bool importListOnLogin();
	void setImportListOnLogin( bool );

	// forFriends
	bool loadFriendsMode();
	void saveFriendsMode( bool );

signals:
	void pubDirSearchResult( const SearchResult&, unsigned int );

protected:
	//{
	bool createContact( const QString& contactId,
			Kopete::MetaContact* parentContact );
	bool loadImportListOnLogin();
	bool loadExportListOnChange();
	void userListNotification(QString what);
	//}

private slots:
	void startNotify();

	void messageReceived( KGaduMessage* );
	void ackReceived( unsigned int );
	void contactStatusChanged( KGaduNotify* );
	void slotSessionDisconnect( Kopete::Account::DisconnectReason );

	void slotExportContactsList();
	void slotExportContactsListToFile();
	void slotImportContactsFromFile();
	void slotImportContactsList();
	void slotDeleteContactsList();
	void slotFriendsMode();

	void userlist( const QString& contacts );
	GaduContactsList* userlist();
	void slotUserlistSynch();
	
	void connectionFailed( gg_failure_t failure );
	void connectionSucceed( );

	void slotChangePassword();

	void slotCommandDone( const QString&, const QString& );
	void slotCommandError( const QString&, const QString& );

	void slotSearchResult( const SearchResult& result, unsigned int seq );
	void userListExportDone();
	void userListDeleteDone();

	void slotIncomingDcc( unsigned int );

private:
	void initConnections();
	void initActions();
	void dccOn();
	void dccOff();
	void userlistChanged();

	GaduAccountPrivate* p;
};

#endif
