// vim: set noet ts=4 sts=4 sw=4 :
// -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2003 Zack Rusin 		<zack@kde.org>
//
// gaduaccount.cpp
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
// 02110-1301, USA

#include "gaduaccount.h"
#include "gaducontact.h"
#include "gaduprotocol.h"
#include "gaduaway.h"
#include "gadupubdir.h"
#include "gadudcc.h"
#include "gadudcctransaction.h"

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopetepassword.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include <kpassworddialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <ktemporaryfile.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kio/netaccess.h>
#include <kicon.h>

#include <qapplication.h>
#include <qdialog.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qhostaddress.h>
#include <QByteArray>
#include <QList>
#include <QPointer>

#include <netinet/in.h>
#include <kconfiggroup.h>

class GaduAccountPrivate {

public:
	GaduAccountPrivate() {}

	GaduSession*	session_;
	GaduDCC*	gaduDcc_;

	QTimer*		pingTimer_;

	QTextCodec*	textcodec_;
	KFileDialog*	saveListDialog;
	KFileDialog*	loadListDialog;

	KAction*	searchAction;
	KAction*	listPutAction;
	KAction*	listGetAction;
	KAction*	listDeleteAction;
	KAction*	listToFileAction;
	KAction*	listFromFileAction;
	KAction*	friendsModeAction;


	bool		connectWithSSL;

	int		currentServer;
	unsigned int	serverIP;

	QString		lastDescription;
	bool		forFriends;
	bool		ignoreAnons;

	QTimer*         exportTimer_;
	bool		exportUserlist;
	bool		exportListMode;
	bool		importListMode;
	KConfigGroup*			config;
	Kopete::OnlineStatus		status;
	QList<unsigned int>		servers;
	KGaduLoginParams		loginInfo;
};

// 10s is enough ;p
#define USERLISTEXPORT_TIMEOUT (10*1000)

// FIXME: use dynamic cache please, i consider this as broken resolution of this problem
static const char* const servers_ip[] = {
	"217.17.41.88",
	"217.17.41.85",
	"217.17.45.143",
	"217.17.45.144",
	"217.17.45.145",
	"217.17.45.146",
	"217.17.45.147",
	"217.17.41.82",
	"217.17.41.83",
	"217.17.41.84",
	"217.17.41.86",
	"217.17.41.87",
	"217.17.41.92",
	"217.17.41.93",
	"217.17.45.133"
};

#define NUM_SERVERS (sizeof(servers_ip)/sizeof(char*))

 GaduAccount::GaduAccount( Kopete::Protocol* parent, const QString& accountID )
: Kopete::PasswordedAccount( parent, accountID, false )
{
	QHostAddress ip;
	p = new GaduAccountPrivate;

	p->pingTimer_ = NULL;
	p->saveListDialog = NULL;
	p->loadListDialog = NULL;
	p->forFriends = false;

	p->textcodec_ = QTextCodec::codecForName( "CP1250" );
	p->session_ = new GaduSession( this );
	p->session_->setObjectName( QLatin1String("GaduSession") );

	setMyself( new GaduContact( accountId().toInt(), this, Kopete::ContactList::self()->myself() ) );

	p->status = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
	p->lastDescription.clear();

	for ( unsigned int i = 0; i < NUM_SERVERS ; i++ ) {
		ip.setAddress( QString( servers_ip[i] ) );
		p->servers.append( htonl( ip.toIPv4Address() ) );
		kDebug( 14100 ) << "adding IP: " <<  p->servers[ i ] << " to cache";
	}
	p->currentServer = -1;
	p->serverIP = 0;

	// initialize KGaduLogin structure to default values
	p->loginInfo.uin		= accountId().toInt();
	p->loginInfo.useTls		= false;
	p->loginInfo.status		= GG_STATUS_AVAIL;
	p->loginInfo.server		= 0;
	p->loginInfo.client_port	= 0;
	p->loginInfo.client_addr	= 0;

	p->pingTimer_ = new QTimer( this );
	p->exportTimer_ = new QTimer( this );

	p->gaduDcc_ = NULL;

	p->config = configGroup();

	p->exportUserlist=false;
	p->exportListMode = loadExportListOnChange();
	p->importListMode = loadImportListOnLogin();
	p->ignoreAnons = ignoreAnons();
	p->forFriends = loadFriendsMode();

	initConnections();
	initActions();

	QString nick = p->config->readEntry( QString::fromAscii( "nickName" ), QString() );
	if ( !nick.isNull() ) {
		myself()->setNickName( nick );
	}
}

GaduAccount::~GaduAccount()
{
	delete p;
}

void
GaduAccount::initActions()
{

	p->searchAction = new KAction( i18n( "&Search for Friends" ), this );
	QObject::connect( p->searchAction, SIGNAL(triggered(bool)), this, SLOT(search()) );

	p->listPutAction = new KAction( i18n( "Export Contacts to Server" ), this );
	p->listPutAction->setIcon ( KIcon ( "document-export" ) );
	QObject::connect( p->listPutAction, SIGNAL(triggered(bool)), this, SLOT(slotExportContactsList()) );

	p->listGetAction = new KAction( i18n( "Import Contacts from Server" ), this );
	p->listGetAction->setIcon ( KIcon ( "document-import" ) );
	QObject::connect( p->listGetAction, SIGNAL(triggered(bool)), this, SLOT(slotImportContactsList()) );

	p->listDeleteAction = new KAction( i18n( "Delete Contacts from Server" ), this );
	p->listDeleteAction->setIcon ( KIcon ( "document-close" ) );
	QObject::connect( p->listDeleteAction, SIGNAL(triggered(bool)), this, SLOT(slotDeleteContactsList()) );

	p->listToFileAction = new KAction( i18n( "Export Contacts to File..." ), this );
	p->listToFileAction->setIcon ( KIcon ( "document-save" ) );
	QObject::connect( p->listToFileAction, SIGNAL(triggered(bool)), this, SLOT(slotExportContactsListToFile()) );

	p->listFromFileAction = new KAction( i18n( "Import Contacts from File..." ), this );
	p->listFromFileAction->setIcon ( KIcon ( "document-open" ) );
	QObject::connect( p->listFromFileAction, SIGNAL(triggered(bool)), this, SLOT(slotImportContactsFromFile()) );

	p->friendsModeAction = new KToggleAction( i18n( "Only for Friends" ), this );
	QObject::connect( p->friendsModeAction, SIGNAL(triggered(bool)), this, SLOT(slotFriendsMode()) );

	static_cast<KToggleAction*>(p->friendsModeAction)->setChecked( p->forFriends );
}

void
GaduAccount::initConnections()
{
	QObject::connect( p->session_, SIGNAL(error(QString,QString)),
				SLOT(error(QString,QString)) );
	QObject::connect( p->session_, SIGNAL(messageReceived(KGaduMessage*)),
				SLOT(messageReceived(KGaduMessage*))  );
	QObject::connect( p->session_, SIGNAL(contactStatusChanged(KGaduNotify*)),
				SLOT(contactStatusChanged(KGaduNotify*)) );
	QObject::connect( p->session_, SIGNAL(connectionFailed(gg_failure_t)),
				SLOT(connectionFailed(gg_failure_t)) );
	QObject::connect( p->session_, SIGNAL(connectionSucceed()),
				SLOT(connectionSucceed()) );
	QObject::connect( p->session_, SIGNAL(disconnect(Kopete::Account::DisconnectReason)),
				SLOT(slotSessionDisconnect(Kopete::Account::DisconnectReason)) );
	QObject::connect( p->session_, SIGNAL(ackReceived(uint)),
				SLOT(ackReceived(uint)) );
	QObject::connect( p->session_, SIGNAL(pubDirSearchResult(SearchResult,uint)),
				SLOT(slotSearchResult(SearchResult,uint)) );
	QObject::connect( p->session_, SIGNAL(userListExported()),
				SLOT(userListExportDone()) );
	QObject::connect( p->session_, SIGNAL(userListDeleted()),
				SLOT(userListDeleteDone()) );
	QObject::connect( p->session_, SIGNAL(userListRecieved(QString)),
				SLOT(userlist(QString)) );
	QObject::connect( p->session_, SIGNAL(incomingCtcp(uint)),
				SLOT(slotIncomingDcc(uint)) );

	QObject::connect( p->pingTimer_, SIGNAL(timeout()),
				SLOT(pingServer()) );

	QObject::connect( p->exportTimer_, SIGNAL(timeout()),
				SLOT(slotUserlistSynch()) );
}

void
GaduAccount::setAway( bool isAway, const QString& awayMessage )
{
	unsigned int currentStatus;

	if ( isAway ) {
		currentStatus = ( awayMessage.isEmpty() ) ? GG_STATUS_BUSY : GG_STATUS_BUSY_DESCR;
	}
	else{
		currentStatus = ( awayMessage.isEmpty() ) ? GG_STATUS_AVAIL : GG_STATUS_AVAIL_DESCR;
	}
	changeStatus( GaduProtocol::protocol()->convertStatus( currentStatus ), awayMessage );
}


void
GaduAccount::fillActionMenu( KActionMenu *actionMenu )
{
	kDebug(14100);

	actionMenu->setIcon( myself()->onlineStatus().iconFor( this ) );
	actionMenu->menu()->addTitle( myself()->onlineStatus().iconFor( myself() ), i18n( "%1 <%2> ",
	    myself()->displayName(), accountId() ) );

	if ( p->session_->isConnected() ) {
		p->searchAction->setEnabled( true );
		p->listPutAction->setEnabled( true );
		p->listGetAction->setEnabled( true );
		p->listDeleteAction->setEnabled( true );
		p->friendsModeAction->setEnabled( true );
	}
	else {
		p->searchAction->setEnabled( false );
		p->listPutAction->setEnabled( false );
		p->listGetAction->setEnabled( false );
		p->listDeleteAction->setEnabled( false );
		p->friendsModeAction->setEnabled( false );
	}

	if ( !contacts().isEmpty() ) {
		if ( p->saveListDialog ) {
			p->listToFileAction->setEnabled( false );
		}
		else {
			p->listToFileAction->setEnabled( true );
		}

		p->listToFileAction->setEnabled( true );
	} else {
		p->listPutAction->setEnabled( false );
		p->listToFileAction->setEnabled( false );
	}

	if ( p->loadListDialog ) {
		p->listFromFileAction->setEnabled( false );
	}
	else {
		p->listFromFileAction->setEnabled( true );
	}

	KAction* action = new KAction(
		KIcon(QIcon(GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL ).iconFor( this ))),
		i18n("Go O&nline"), this );
        //, "actionGaduConnect" );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotGoOnline()));
	actionMenu->addAction( action );

	action = new KAction(
		KIcon(QIcon(GaduProtocol::protocol()->convertStatus( GG_STATUS_BUSY ).iconFor( this ))),
		i18n( "Set &Busy" ), this );
        //, "actionGaduConnect" );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotGoBusy()) );
	actionMenu->addAction( action );

	action = new KAction(
		KIcon(QIcon(GaduProtocol::protocol()->convertStatus( GG_STATUS_INVISIBLE ).iconFor( this ))),
		i18n( "Set &Invisible" ), this );
        //, "actionGaduConnect" );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotGoInvisible()) );
	actionMenu->addAction( action );

	action = new KAction(
		KIcon(QIcon(GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ).iconFor( this ))),
		i18n( "Go &Offline" ), this );
        //, "actionGaduConnect" );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotGoOffline()) );
	actionMenu->addAction( action );

	action = new KAction( KIcon("edit-rename"), i18n( "Set &Description..." ), this );
        //, "actionGaduDescription" );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotDescription()) );
	actionMenu->addAction( action );
	actionMenu->addAction( p->friendsModeAction );

	actionMenu->addSeparator();

	actionMenu->addAction( p->searchAction );

	actionMenu->addSeparator();

	KActionMenu *listMenuAction = new KActionMenu(
		KIcon ( "user-identity" ),
		i18n("Contacts"), this);

	listMenuAction->addAction( p->listPutAction );
	listMenuAction->addAction( p->listGetAction );
	listMenuAction->addAction( p->listDeleteAction );
	listMenuAction->addSeparator();
	listMenuAction->addAction( p->listToFileAction );
	listMenuAction->addAction( p->listFromFileAction );
	listMenuAction->addSeparator();

	action = new KToggleAction( i18n( "Export Contacts on change" ), this );
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(setExportListOnChange(bool)));
	static_cast<KToggleAction*>(action)->setChecked(p->exportListMode);

	listMenuAction->addAction( action );

	actionMenu->addAction( listMenuAction );

	KAction *propertiesAction = new KAction( KIcon("configure"), i18n("Properties"), actionMenu );
	QObject::connect( propertiesAction, SIGNAL(triggered(bool)), this, SLOT(editAccount()) );
	actionMenu->addAction( propertiesAction );

}

bool
GaduAccount::hasCustomStatusMenu() const
{
	return true;
}

void
GaduAccount::connectWithPassword(const QString& password)
{
	if (password.isEmpty()) {
		return;
	}
	if (isConnected ())
		return;
	// FIXME: add status description to this mechainsm, this is a hack now. libkopete design issue.
	changeStatus( initialStatus(), p->lastDescription );
}

void
GaduAccount::disconnect()
{
	disconnect( Manual );
}

void
GaduAccount::disconnect( DisconnectReason reason )
{
	slotGoOffline();
	p->connectWithSSL = true;
	Kopete::Account::disconnected( reason );
}

void
GaduAccount::setOnlineStatus( const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason, const OnlineStatusOptions& /*options*/ )
{
	kDebug(14100) << "Called";
	changeStatus( status, reason.message() );
}

void
GaduAccount::setStatusMessage( const Kopete::StatusMessage& statusMessage )
{
	changeStatus( myself()->onlineStatus(), statusMessage.message() );
}

void
GaduAccount::slotUserlistSynch()
{
	if ( !p->exportUserlist || p->exportListMode ) {
		return;
	}
	p->exportUserlist = false;
	kDebug(14100) << "userlist changed, exporting";
	slotExportContactsList();
}

void
GaduAccount::userlistChanged()
{
	p->exportUserlist = true;
	p->exportTimer_->start( USERLISTEXPORT_TIMEOUT );
}

bool
GaduAccount::createContact( const QString& contactId, Kopete::MetaContact* parentContact )
{
	kDebug(14100) << "createContact " << contactId;

	bool ok=false;
	uin_t uinNumber = contactId.toUInt(&ok);
	if(!ok || uinNumber == 0) {
		kDebug( 14100 ) << "Invalid GaduGadu number:" << contactId;
		return false;
	}

	GaduContact* newContact = new GaduContact( uinNumber, this, parentContact );
	newContact->setParentIdentity( accountId() );
	addNotify( uinNumber );

	userlistChanged();

	return true;
}

void
GaduAccount::changeStatus( const Kopete::OnlineStatus& status, const QString& descr )
{
	unsigned int ns;

	kDebug(14100) << "##### change status #####";
	kDebug(14100) << "### Status = " << p->session_->isConnected();
	kDebug(14100) << "### Status description = \"" << descr << "\"";

	// if change to not available, log off
	if ( GG_S_NA( status.internalStatus() ) ) {
		if ( !p->session_->isConnected() ) {
			return;//already logged off
		}
		else {
			 if ( status.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
				if ( p->session_->changeStatusDescription( status.internalStatus(), descr, p->forFriends ) != 0 ) {
					return;
				}
			}
		}
		p->session_->logoff();
		dccOff();
	}
	else {
		// if status is for no desc, but we get some desc, than convert it to status with desc
		if (!descr.isEmpty() && !GaduProtocol::protocol()->statusWithDescription( status.internalStatus() ) ) {
			// and rerun us again. This won't cause any recursive call, as both conversions are static
			ns = GaduProtocol::protocol()->statusToWithDescription( status );
			changeStatus( GaduProtocol::protocol()->convertStatus( ns ), descr );
			return;
		}

		// well, if it's empty but we want to set status with desc, change it too
                if (descr.isEmpty() && GaduProtocol::protocol()->statusWithDescription( status.internalStatus() ) ) {
			ns = GaduProtocol::protocol()->statusToWithoutDescription( status );
			changeStatus( GaduProtocol::protocol()->convertStatus( ns ), descr );
			return;
		}

		if ( !p->session_->isConnected() ) {
			if ( password().cachedValue().isEmpty() ) {
				// FIXME: when status string added to connect(), use it here
				p->lastDescription = descr;
				connect( status/*, descr*/ );
				return;
			}

			if ( useTls() != TLS_no ) {
				p->connectWithSSL = true;
			}
			else {
				p->connectWithSSL = false;
			}
			dccOn();
			p->serverIP = 0;
			p->currentServer = -1;
			p->status = status;
			kDebug(14100) << "#### Connecting..., tls option "<< (int)useTls() << " ";
			p->lastDescription = descr;
			slotLogin( status.internalStatus(), descr );
			return;
		}
		else {
			p->status = status;
			if ( descr.isEmpty() ) {
				if ( p->session_->changeStatus( status.internalStatus(), p->forFriends ) != 0 )
					return;
			}
			else {
				if ( p->session_->changeStatusDescription( status.internalStatus(), descr, p->forFriends ) != 0 )
					return;
			}
		}
	}

	myself()->setOnlineStatus( status );
	myself()->setStatusMessage( Kopete::StatusMessage(descr) );

	if ( status.internalStatus() == GG_STATUS_NOT_AVAIL || status.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
		if ( p->pingTimer_ ){
			p->pingTimer_->stop();
		}
	}
	p->lastDescription = descr;
}

void
GaduAccount::slotLogin( int status, const QString& dscr )
{
	p->lastDescription	= dscr;

	myself()->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING ));
	myself()->setStatusMessage( Kopete::StatusMessage(dscr) );

	if ( !p->session_->isConnected() ) {
		if ( password().cachedValue().isEmpty() ) {
			connectionFailed( GG_FAILURE_PASSWORD );
		}
		else {
			p->loginInfo.password		= p->textcodec_->fromUnicode(password().cachedValue());
			p->loginInfo.useTls		= p->connectWithSSL;
			p->loginInfo.status		= status;
			p->loginInfo.statusDescr	= dscr;
			p->loginInfo.forFriends		= p->forFriends;
			p->loginInfo.server		= p->serverIP;
			if ( dccEnabled() ) {
				p->loginInfo.client_addr	= gg_dcc_ip;
				p->loginInfo.client_port	= gg_dcc_port;
			}
			else {
				p->loginInfo.client_addr	= 0;
				p->loginInfo.client_port	= 0;
			}
			p->session_->login( &p->loginInfo );
		}
	}
	else {
		p->session_->changeStatus( status );
	}
}

void
GaduAccount::slotLogoff()
{
	if ( p->session_->isConnected() || p->status == GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING )) {
		p->status = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		changeStatus( p->status );
		p->session_->logoff();
		dccOff();
	}
}

void
GaduAccount::slotGoOnline()
{
	changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL ) );
}
void
GaduAccount::slotGoOffline()
{
	slotLogoff();
	dccOff();
}

void
GaduAccount::slotGoInvisible()
{
	changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_INVISIBLE ) );
}

void
GaduAccount::slotGoBusy()
{
	changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_BUSY ) );
}

void
GaduAccount::removeContact( const GaduContact* c )
{
	if ( isConnected() ) {
		const uin_t u = c->uin();
		p->session_->removeNotify( u );
		userlistChanged();
	}
}

void
GaduAccount::addNotify( uin_t uin )
{
	if ( p->session_->isConnected() ) {
		p->session_->addNotify( uin );
	}
}

void
GaduAccount::notify( uin_t* userlist, int count )
{
	if ( p->session_->isConnected() ) {
		p->session_->notify( userlist, count );
	}
}

void
GaduAccount::sendMessage( uin_t recipient, const Kopete::Message& msg, int msgClass )
{
	if ( p->session_->isConnected() ) {
		p->session_->sendMessage( recipient, msg, msgClass );
	}
}

void
GaduAccount::error( const QString& title, const QString& message )
{
	KMessageBox::error( Kopete::UI::Global::mainWidget(), title, message );
}

void
GaduAccount::messageReceived( KGaduMessage* gaduMessage )
{
	GaduContact* contact = 0;
	QList<Kopete::Contact*> contactsListTmp;

	// FIXME:check for ignored users list

	if ( gaduMessage->sender_id == 0 ) {
		//system message, display them or not?
		kDebug(14100) << "####" << " System Message " << gaduMessage->message;
		return;
	}

	contact = static_cast<GaduContact*> ( contacts().value( QString::number( gaduMessage->sender_id ) ) );

	if ( !contact ) {
		if ( p->ignoreAnons == true ) {
			return;
		}

		Kopete::MetaContact* metaContact = new Kopete::MetaContact ();
		metaContact->setTemporary ( true );
		contact = new GaduContact( gaduMessage->sender_id, this, metaContact );
		Kopete::ContactList::self ()->addMetaContact( metaContact );
		addNotify( gaduMessage->sender_id );
	}

	contactsListTmp.append( myself() );

	Kopete::Message msg( contact, contactsListTmp );
	msg.setTimestamp( gaduMessage->sendTime );
	msg.setHtmlBody( gaduMessage->message );
	msg.setDirection( Kopete::Message::Inbound );

	contact->messageReceived( msg );
}

void
GaduAccount::ackReceived( unsigned int recipient  )
{
	GaduContact* contact;

	contact = static_cast<GaduContact*> ( contacts().value( QString::number( recipient ) ) );
	if ( contact ) {
		kDebug(14100) << "####" << "Received an ACK from " << contact->uin();
		contact->messageAck();
	}
	else {
		kDebug(14100) << "####" << "Received an ACK from an unknown user : " << recipient;
	}
}

void
GaduAccount::contactStatusChanged( KGaduNotify* gaduNotify )
{
	kDebug(14100) << "####" << " contact's status changed, uin:" << gaduNotify->contact_id;

	GaduContact* contact;

	contact = static_cast<GaduContact*>( contacts().value( QString::number( gaduNotify->contact_id ) ) );
	if( !contact ) {
		kDebug(14100) << "Notify not in the list " << gaduNotify->contact_id;
		return;
	}

	contact->changedStatus( gaduNotify );
}

void
GaduAccount::pong()
{
	kDebug(14100) << "####" << " Pong...";
}

void
GaduAccount::pingServer()
{
	kDebug(14100) << "####" << " Ping...";
	p->session_->ping();
}

void
GaduAccount::connectionFailed( gg_failure_t failure )
{
	bool tryReconnect = false;
	QString pass;


	switch (failure) {
		case GG_FAILURE_PASSWORD:
			password().setWrong();
			// user pressed CANCEL
			p->status = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
			myself()->setOnlineStatus( p->status );
			disconnected( BadPassword );
			return;
		default:
			if ( p->connectWithSSL ) {
				if ( useTls() != TLS_only ) {
					slotCommandDone( QString(), i18n( "connection using SSL was not possible, retrying without." ) );
					kDebug( 14100 ) << "try without tls now";
					p->connectWithSSL = false;
					tryReconnect = true;
					p->currentServer = -1;
					p->serverIP = 0;
					break;
				}
			}
			else {
				if ( p->currentServer == NUM_SERVERS - 1 ) {
					p->serverIP = 0;
					p->currentServer = -1;
					kDebug(14100) << "trying : " << "IP from hub ";
				}
				else {
					p->serverIP = p->servers[ ++p->currentServer ];
					kDebug(14100) << "trying : " << p->currentServer << " IP " << p->serverIP;
					tryReconnect = true;
				}
			}
		break;
	}

	if ( tryReconnect ) {
			slotLogin( p->status.internalStatus() , p->lastDescription );
	}
	else {
		error( i18n( "unable to connect to the Gadu-Gadu server(\"%1\").", GaduSession::failureDescription( failure ) ),
				i18n( "Connection Error" ) );
		p->status = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		myself()->setOnlineStatus( p->status );
		disconnected( InvalidHost );
	}
}

void
GaduAccount::dccOn()
{
	if ( dccEnabled() ) {
		if ( !p->gaduDcc_ ) {
			p->gaduDcc_ = new GaduDCC( this );
		}
		kDebug( 14100 ) << " turn DCC on for " << accountId();
		p->gaduDcc_->registerAccount( this );
		p->loginInfo.client_port	= p->gaduDcc_->listeingPort();
	}
}

void
GaduAccount::dccOff()
{
	if ( p->gaduDcc_ ) {
		kDebug( 14100 ) << "destroying dcc in gaduaccount ";
		delete p->gaduDcc_;
		p->gaduDcc_ = NULL;
		p->loginInfo.client_port	= 0;
		p->loginInfo.client_addr	= 0;
	}
}

void
GaduAccount::slotIncomingDcc( unsigned int uin )
{
	GaduContact* contact;
	GaduDCCTransaction* trans;

	if ( !uin ) {
		return;
	}

	contact = static_cast<GaduContact*>( contacts().value( QString::number( uin ) ) );

	if ( !contact ) {
	  kDebug(14100) << "attempt to make dcc connection from unknown uin " << uin;
		return;
	}

	// if incapabile to transfer files, forget about it.
	if ( contact->contactPort() < 10 ) {
	  kDebug(14100) << "can't respond to " << uin << " request, his listeing port is too low";
	  return;
	}

	trans = new GaduDCCTransaction( p->gaduDcc_ );
	if ( trans->setupIncoming( p->loginInfo.uin, contact ) == false ) {
		delete trans;
	}

}

void
GaduAccount::connectionSucceed( )
{
	kDebug(14100) << "#### Gadu-Gadu connected! ";
	p->status =  GaduProtocol::protocol()->convertStatus( p->session_->status() );
	myself()->setOnlineStatus( p->status );
	myself()->setStatusMessage( Kopete::StatusMessage(p->lastDescription) );
	startNotify();


	if ( p->importListMode ) {
		p->session_->requestContacts();
	}
	p->pingTimer_->start( 3*60*1000 );//3 minute timeout
	pingServer();

	// check if we need to export userlist every USERLISTEXPORT_TIMEOUT ms
	p->exportTimer_->start( USERLISTEXPORT_TIMEOUT );
}

void
GaduAccount::startNotify()
{
	int i = 0;
	if ( contacts().isEmpty() ) {
		// we MUST send at least empty notify request to receive messages
		p->session_->notify(NULL, 0);
		return;
	}

	uin_t* userlist = 0;
	userlist = new uin_t[ contacts().count() ];

	QHashIterator<QString, Kopete::Contact*> it(contacts());
	for(  i=0 ; it.hasNext() ; ) {
		it.next();
		userlist[i++] = static_cast<GaduContact*> (it.value())->uin();
	}

	p->session_->notify( userlist, contacts().count() );
	delete [] userlist;
}

void
GaduAccount::slotSessionDisconnect( Kopete::Account::DisconnectReason reason )
{
	uin_t status;

	kDebug(14100) << "Disconnecting";

	if (p->pingTimer_) {
		p->pingTimer_->stop();
	}

	setAllContactsStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );

	status = myself()->onlineStatus().internalStatus();
	if ( status != GG_STATUS_NOT_AVAIL || status != GG_STATUS_NOT_AVAIL_DESCR ) {
		myself()->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
	}
	GaduAccount::disconnect( reason );
}

void
GaduAccount::userlist( const QString& contactsListString )
{
	kDebug(14100)<<"### Got userlist - gadu account\n" <<  contactsListString;

	GaduContactsList contactsList( contactsListString );
	QString contactName;
	QStringList groups;
	GaduContact* contact;
	Kopete::MetaContact* metaContact;
	unsigned int i;

	if ( contactsList.size() == 0 ) {
		userListNotification(i18n("Imported contacts list is empty."));
		return;
	}

	// don't export any new changes that were just imported :-)
	p->exportTimer_->stop();
	for ( i = 0; i != contactsList.size() ; i++ ) {
		kDebug(14100) << "uin " << contactsList[i].uin;

		if ( contactsList[i].uin.isEmpty() ) {
			kDebug(14100) << "no Uin, strange.. ";
			continue;
		}

		if ( contacts().value( contactsList[i].uin ) ) {
			kDebug(14100) << "UIN already exists in contacts "<< contactsList[i].uin;
		}
		else {
			kDebug(14100) << "Adding UIN";
			contactName = GaduContact::findBestContactName( &contactsList[i] );
			bool s = addContact( contactsList[i].uin, contactName, 0L, Kopete::Account::DontChangeKABC);
			if ( s == false ) {
				kDebug(14100) << "There was a problem adding UIN "<< contactsList[i].uin << "to users list";
				continue;
			}
		}
		contact = static_cast<GaduContact*>( contacts().value( contactsList[i].uin ) );
		if ( contact == NULL ) {
			kDebug(14100) << "oops, no Kopete::Contact in contacts()[] for some reason, for \"" << contactsList[i].uin << "\"";
			continue;
		}

		// update/add infor for contact
		contact->setContactDetails( &contactsList[i] );

		if ( !( contactsList[i].group.isEmpty() ) ) {
			// FIXME: libkopete bug i guess, by default contact goes to top level group
			// if user desrired to see contact somewhere else, remove it from top level one
			metaContact = contact->metaContact();
			metaContact->removeFromGroup( Kopete::Group::topLevel() );
			// put him in all desired groups:
			groups = contactsList[i].group.split( ',', QString::SkipEmptyParts );
			for ( QStringList::Iterator groupsIterator = groups.begin(); groupsIterator != groups.end(); ++groupsIterator ) {
				metaContact->addToGroup( Kopete::ContactList::self ()->findGroup ( *groupsIterator) );
			}
		}
	}
	userListNotification(i18n("Contacts imported."));
	// start to check if we need to export userlist
	p->exportUserlist = false;
	p->exportTimer_->start( USERLISTEXPORT_TIMEOUT );
}

void
GaduAccount::userListExportDone()
{
	userListNotification(i18n( "Contacts exported.") );
}

void
GaduAccount::userListDeleteDone()
{
	userListNotification(i18n( "Contacts deleted from the server.") );
}

void
GaduAccount::userListNotification( QString what )
{
	if ( !isBusy() )
		KNotification::event( QString::fromLatin1("kopete_gadu_contactslist"), what, accountIcon());
}

void
GaduAccount::slotFriendsMode()
{
	p->forFriends = !p->forFriends;
	kDebug( 14100 ) << "for friends mode: " << p->forFriends << " desc" << p->lastDescription;
	// now change status, it will changing it with p->forFriends flag
	changeStatus( p->status, p->lastDescription );

	saveFriendsMode( p->forFriends );

}

// FIXME: make loading and saving nonblocking (at the moment KFileDialog stops plugin/kopete)

void
GaduAccount::slotExportContactsListToFile()
{
	KTemporaryFile tempFile;

	if ( p->saveListDialog ) {
		kDebug( 14100 ) << " save contacts to file: alread waiting for input ";
		return;
	}

	p->saveListDialog = new KFileDialog( QString( "::kopete-gadu" + accountId() ), QString(),
					Kopete::UI::Global::mainWidget() );
	p->saveListDialog->setCaption(
	    i18n("Save Contacts List for Account %1 As",
	    myself()->displayName() ) );

	if ( p->saveListDialog->exec() == QDialog::Accepted ) {
		QByteArray list = p->textcodec_->fromUnicode( userlist()->asString() );

		if ( !tempFile.open() ) {
			// say cheese, can't create file.....
			error( i18n( "Unable to create temporary file." ), i18n( "Save Contacts List Failed" ) );
		}
		else {
			QTextStream tempStream ( &tempFile );
			tempStream << list.data();
			tempStream.flush();

			bool res = KIO::NetAccess::upload(
								tempFile.fileName() ,
								p->saveListDialog->selectedUrl() ,
								Kopete::UI::Global::mainWidget()
								);
			if ( !res ) {
				// say it failed
				error( KIO::NetAccess::lastErrorString(), i18n( "Save Contacts List Failed" ) );
			}
		}

	}
	delete p->saveListDialog;
	p->saveListDialog = NULL;
}

void
GaduAccount::slotImportContactsFromFile()
{
	KUrl url;
	QByteArray list;
	QString oname;

	if ( p->loadListDialog ) {
		kDebug( 14100 ) << "load contacts from file: alread waiting for input ";
		return;
	}

	p->loadListDialog = new KFileDialog( QString( "::kopete-gadu" + accountId() ), QString(),
					Kopete::UI::Global::mainWidget() );
	p->loadListDialog->setCaption(
	    i18n("Load Contacts List for Account %1 As",
	    myself()->displayName() ) );

	if ( p->loadListDialog->exec() == QDialog::Accepted ) {
		url = p->loadListDialog->selectedUrl();
		kDebug(14100) << "a:" << url << "\nb:" << oname;
		if ( KIO::NetAccess::download( url, oname, Kopete::UI::Global::mainWidget() ) ) {
			QFile tempFile( oname );
			if ( tempFile.open( QIODevice::ReadOnly ) ) {
				list = tempFile.readAll();
				tempFile.close();
				KIO::NetAccess::removeTempFile( oname );
				// and store it
				kDebug( 14100 ) << "loaded list:";
				kDebug( 14100 ) << list;
				kDebug( 14100 ) << " --------------- ";
				userlist( p->textcodec_->toUnicode( list ) );
			}
			else {
				error( tempFile.errorString(),
					i18n( "Contacts List Load Has Failed" ) );
			}
		}
		else {
			// say, it failed misourably
			error( KIO::NetAccess::lastErrorString(),
				i18n( "Contacts List Load Has Failed" ) );
		}

	}
	delete p->loadListDialog;
	p->loadListDialog = NULL;
}

unsigned int
GaduAccount::getPersonalInformation()
{
	return p->session_->getPersonalInformation();
}

bool
GaduAccount::publishPersonalInformation( ResLine& d )
{
	return p->session_->publishPersonalInformation( d );
}

void
GaduAccount::slotExportContactsList()
{
	p->session_->exportContactsOnServer( userlist() );
}

void
GaduAccount::slotDeleteContactsList()
{
	p->session_->deleteContactsOnServer( );
}

void
GaduAccount::slotImportContactsList()
{
	p->session_->requestContacts();
}


GaduContactsList*
GaduAccount::userlist()
{
	GaduContact* contact;
	GaduContactsList* contactsList = new GaduContactsList();

	if ( contacts().isEmpty() ) {
		return contactsList;
	}

	QHashIterator<QString, Kopete::Contact*> contactsIterator( contacts() );

	for( ; contactsIterator.hasNext() ; ) {
		contactsIterator.next();
		contact = static_cast<GaduContact*>( contactsIterator.value() );
		contactsList->addContact( *contact->contactDetails() );
	}

	return contactsList;
}

void
GaduAccount::slotSearch( int uin )
{
	GaduPublicDir* dir = new GaduPublicDir( this, uin );
	dir->setObjectName( QLatin1String("GaduPublicDir") );
}

void
GaduAccount::slotChangePassword()
{
}

void
GaduAccount::slotCommandDone( const QString& /*title*/, const QString& what )
{
	if ( !isBusy() )
		KNotification::event( KNotification::Notification, what );
}

void
GaduAccount::slotCommandError(const QString& title, const QString& what )
{
	error( title, what );
}

void
GaduAccount::slotDescription()
{
	QPointer <GaduAway> away = new GaduAway( this );

	if( away->exec() == QDialog::Accepted && away ) {
		changeStatus( GaduProtocol::protocol()->convertStatus( away->status() ),
					away->awayText() );
	}
	delete away;
}

unsigned int
GaduAccount::pubDirSearch( ResLine& query, int ageFrom, int ageTo, bool onlyAlive )
{
	return p->session_->pubDirSearch( query, ageFrom, ageTo, onlyAlive );
}

void
GaduAccount::pubDirSearchClose()
{
	p->session_->pubDirSearchClose();
}

void
GaduAccount::slotSearchResult( const SearchResult& result, unsigned int seq )
{
	emit pubDirSearchResult( result, seq );
}

void
GaduAccount::sendFile( GaduContact* peer, QString& filePath )
{
	GaduDCCTransaction* gtran = new GaduDCCTransaction( p->gaduDcc_ );
	gtran->setupOutgoing( peer, filePath );
}

void
GaduAccount::dccRequest( GaduContact* peer )
{
	if ( peer && p->session_ ) {
		p->session_->dccRequest( peer->uin() );
	}
}

// dcc settings
bool
GaduAccount::dccEnabled()
{
	QString s = p->config->readEntry( QString::fromAscii( "useDcc" ), QString() );
	kDebug( 14100 ) << "dccEnabled: " << s;
	if ( s == QString::fromAscii( "enabled" ) ) {
		return true;
	}
	return false;
}

bool
GaduAccount::setDcc( bool d )
{
	QString s;
	bool f = true;

	if ( d == false ) {
		dccOff();
		s = QString::fromAscii( "disabled" );
	}
	else {
		s = QString::fromAscii( "enabled" );
	}

	p->config->writeEntry( QString::fromAscii( "useDcc" ), s );

	if ( p->session_->isConnected() && d ) {
		dccOn();
	}
	kDebug( 14100 ) << "s: "<<s;

	return f;
}

void
GaduAccount::saveFriendsMode( bool i )
{
	p->config->writeEntry( QString::fromAscii( "forFriends" ),
			i == true ? QString::fromAscii( "1" ) : QString::fromAscii( "0" ) );
}


bool
GaduAccount::loadFriendsMode()
{
	QString s;
	bool r;
	int n;

	s = p->config->readEntry( QString::fromAscii( "forFriends" ), QString() );
	n = s.toInt( &r );

	if ( n ) {
		return true;
	}

	return false;

}

bool
GaduAccount::exportListOnChange()
{
	return p->exportListMode;
}

void
GaduAccount::setExportListOnChange( bool i )
{
	p->exportListMode = i;
	p->config->writeEntry( QString::fromAscii( "exportListOnChange" ),
			i == true ? QString::fromAscii( "1" ) : QString::fromAscii( "0" ) );

	// stop timer and do not export until next change
	p->exportTimer_->stop();
	p->exportUserlist = false;

}

bool
GaduAccount::loadExportListOnChange()
{
	QString s;
	bool r;
	int n;

	s = p->config->readEntry( QString::fromAscii( "exportListOnChange" ), QString("1") );
	n = s.toInt( &r );

	if ( n ) {
		return true;
	}

	return false;
}

bool
GaduAccount::importListOnLogin()
{
	return p->importListMode;
}

void
GaduAccount::setImportListOnLogin( bool i )
{
	p->importListMode = i;
	p->config->writeEntry( QString::fromAscii( "importListOnLogin" ),
			i == true ? QString::fromAscii( "1" ) : QString::fromAscii( "0" ) );
}

bool
GaduAccount::loadImportListOnLogin()
{
	QString s;
	bool r;
	int n;

	s = p->config->readEntry( QString::fromAscii( "importListOnLogin" ), QString("1") );
	n = s.toInt( &r );

	if ( n ) {
		return true;
	}

	return false;
}


// might be bit inconsistent with what I used in DCC, but hell, it is so much easier to parse :-)
bool
GaduAccount::ignoreAnons()
{
	QString s;
	bool r;
	int n;

	s = p->config->readEntry( QString( "ignoreAnons" ), QString() );
	n = s.toInt( &r );

	if ( n ) {
		return true;
	}

	return false;

}

void
GaduAccount::setIgnoreAnons( bool i )
{
	p->ignoreAnons = i;
	p->config->writeEntry( QString::fromAscii( "ignoreAnons" ),
			i == true ? QString::fromAscii( "1" ) : QString::fromAscii( "0" ) );
}

GaduAccount::tlsConnection
GaduAccount::useTls()
{
	QString s;
	bool c;
	unsigned int oldC;
	tlsConnection Tls;

	s = p->config->readEntry( QString::fromAscii( "useEncryptedConnection" ), QString() );
	oldC = s.toUInt( &c );
	// we have old format
	if ( c ) {
		kDebug( 14100 ) << "old format for param useEncryptedConnection, value " <<
				oldC << " will be converted to new string value" << endl;
		setUseTls( (tlsConnection) oldC );
		// should be string now, unless there was an error reading
		s = p->config->readEntry( QString::fromAscii( "useEncryptedConnection" ), QString() );
		kDebug( 14100 ) << "new useEncryptedConnection value : " << s;
	}

	Tls = TLS_no;
	if ( s == "TLS_ifAvaliable" ) {
		Tls = TLS_ifAvaliable;
	}
	if ( s == "TLS_only" ) {
		Tls = TLS_only;
	}

	return Tls;
}

void
GaduAccount::setUseTls( tlsConnection ut )
{
	QString s;
	switch( ut ) {
		case TLS_ifAvaliable:
			s = "TLS_ifAvaliable";
		break;

		case TLS_only:
			s = "TLS_only";
		break;

		default:
		case TLS_no:
			s = "TLS_no";
		break;
	}

	p->config->writeEntry( QString::fromAscii( "useEncryptedConnection" ), s );
}

#include "gaduaccount.moc"
