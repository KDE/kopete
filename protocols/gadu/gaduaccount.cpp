// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA

#include "gaduaccount.h"
#include "gaducontact.h"
#include "gaduprotocol.h"
#include "gaduaway.h"
#include "gadupubdir.h"

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"

#include <kpassdlg.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knotifyclient.h>

#include <qapplication.h>
#include <qdialog.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qptrlist.h>

GaduAccount::GaduAccount( KopeteProtocol* parent, const QString& accountID,const char* name )
: KopeteAccount( parent, accountID, name ), pingTimer_( 0 )
{
	textcodec_ = QTextCodec::codecForName( "CP1250" );
	session_ = new GaduSession( this, "GaduSession" );
	KGlobal::config()->setGroup( "Gadu" );
	// Connect if possible using SSL
	isTls = 0;
	connectWithSSL=true;
	myself_ = new GaduContact(  accountId().toInt(), accountId(), this, new KopeteMetaContact() );
	lastStatus = GG_STATUS_AVAIL;
	lastDescription = QString::null;

	initActions();
	initConnections();
}

void
GaduAccount::initActions()
{
	searchAction	= new KAction( i18n( "&Search for friends" ), "", 0,
							this, SLOT( slotSearch() ), this, "actionSearch" );
	listputAction	= new KAction( i18n( "Export contacts on server" ), "", 0,
							this, SLOT( slotExportContactsList() ), this, "actionListput" );
}

void
GaduAccount::initConnections()
{
	QObject::connect( session_, SIGNAL( error( const QString&, const QString& ) ),
				SLOT( error( const QString&, const QString& ) ) );
	QObject::connect( session_, SIGNAL( messageReceived( struct gg_event* ) ),
				SLOT( messageReceived( struct gg_event*) )  );
	QObject::connect( session_, SIGNAL( notify( struct gg_event* ) ),
				SLOT( notify( struct gg_event* ) ) );
	QObject::connect( session_, SIGNAL( statusChanged( struct gg_event* ) ),
				SLOT( statusChanged( struct gg_event* ) ) );
	QObject::connect( session_, SIGNAL( connectionFailed( gg_failure_t )),
				SLOT( connectionFailed( gg_failure_t ) ) );
	QObject::connect( session_, SIGNAL( connectionSucceed( struct gg_event* ) ),
				SLOT( connectionSucceed( struct gg_event* ) ) );
	QObject::connect( session_, SIGNAL( disconnect() ),
				SLOT( slotSessionDisconnect() ) );
	QObject::connect( session_, SIGNAL( ackReceived( struct gg_event* ) ),
				SLOT( ackReceived( struct gg_event* ) ) );
	QObject::connect( session_, SIGNAL( pubDirSearchResult( const searchResult& ) ),
				SLOT( slotSearchResult( const searchResult& ) ) );
	QObject::connect( session_, SIGNAL( userListExported() ),
				SLOT( userListExportDone() ) );
}

void GaduAccount::loaded()
{
	QString nick;
	nick	= pluginData( protocol(), QString::fromAscii( "nickName" ) );
	isTls	= (bool) ( pluginData( protocol(), QString::fromAscii( "useEncryptedConnection" ) ).toInt() );
	if ( isTls > 2 || isTls < 0 ) {
		isTls = 0;
	}
	if ( !nick.isNull() ) {
		myself_->rename( nick );
	}
}

void GaduAccount::setAway( bool isAway, const QString& awayMessage )
{
	uint status;

	if ( isAway ) {
		status = ( awayMessage.isEmpty() ) ? GG_STATUS_BUSY : GG_STATUS_BUSY_DESCR;
	}
	else{
		status = ( awayMessage.isEmpty() ) ? GG_STATUS_AVAIL : GG_STATUS_AVAIL_DESCR;
	}
	changeStatus( GaduProtocol::protocol()->convertStatus( status ), awayMessage );
}

KopeteContact* GaduAccount::myself() const
{
	return myself_;
}

KActionMenu* GaduAccount::actionMenu()
{
	kdDebug(14100) << "actionMenu() " << endl;

	actionMenu_ = new KActionMenu( accountId(), myself()->onlineStatus().iconFor( this ), this );
	actionMenu_->popupMenu()->insertTitle( myself_->onlineStatus().iconFor( myself_ ), i18n( "%1 <%2> " ).

#if QT_VERSION < 0x030200
	arg( myself_->displayName() ).arg( accountId() ) );
#else
	arg( myself_->displayName(), accountId() ) );
#endif

	if ( session_->isConnected() ) {
		searchAction->setEnabled( TRUE );
		listputAction->setEnabled( TRUE );
	}
	else {
		searchAction->setEnabled( FALSE );
		listputAction->setEnabled( FALSE );
	}

	actionMenu_->insert( new KAction( i18n( "Go O&nline" ),
			GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL ).iconFor( this ),
			0, this, SLOT( slotGoOnline() ), this, "actionGaduConnect" ) );
	actionMenu_->insert( new KAction( i18n( "Set &Busy" ),
			GaduProtocol::protocol()->convertStatus( GG_STATUS_BUSY ).iconFor( this ),
			0, this, SLOT( slotGoBusy() ), this, "actionGaduConnect" ) );
	actionMenu_->insert( new KAction( i18n( "Set &Invisible" ),
			GaduProtocol::protocol()->convertStatus( GG_STATUS_INVISIBLE ).iconFor( this ),
			0, this, SLOT( slotGoInvisible() ), this, "actionGaduConnect" ) );
	actionMenu_->insert( new KAction( i18n( "Go &Offline" ),
			GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ).iconFor( this ),
			0, this, SLOT( slotGoOffline() ), this, "actionGaduConnect" ) );
	actionMenu_->insert( new KAction( i18n( "Set &Description" ),
			"info",
			0, this, SLOT( slotDescription() ), this, "actionGaduDescription" ) );

	actionMenu_->popupMenu()->insertSeparator();

	actionMenu_->insert( listputAction );
	actionMenu_->insert( searchAction );

//  actionMenu_->insert( new KAction( i18n( "Change Password" ), "", 0, this,
//		SLOT( slotChangePassword() ), this, "actionChangePassword" ) );

	return actionMenu_;
}

void GaduAccount::connect()
{
	connectWithSSL = true;
	slotGoOnline();
}

void GaduAccount::disconnect()
{
	slotGoOffline();
	connectWithSSL=true;
}

bool GaduAccount::addContactToMetaContact( const QString& contactId, const QString& displayName,
					 KopeteMetaContact* parentContact )
{
	kdDebug(14100) << "addContactToMetaContact " << contactId << endl;

	uin_t uinNumber = contactId.toUInt();
	GaduContact* newContact = new GaduContact( uinNumber, displayName, this, parentContact );
	newContact->setParentIdentity( accountId() );
	addNotify( uinNumber );

	return true;
}

void
GaduAccount::changeStatus( const KopeteOnlineStatus& status, const QString& descr )
{
	kdDebug() << "### Status = " << session_->isConnected() << endl;

	if ( GG_S_NA( status.internalStatus() ) ) {
		if ( !session_->isConnected() ) {
			return;//already logged off
		}
		else {
			 if ( status.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
				if ( session_->changeStatusDescription( status.internalStatus(), descr ) != 0 ) {
					return;
				}
			}
		}
		session_->logoff();
	}
	else {
		if ( !session_->isConnected() ) {
			connectWithSSL = true;
			slotLogin( status.internalStatus(), descr );
			status_ = status;
			return;
		}
		else {
			if ( descr.isEmpty() ) {
				if ( session_->changeStatus( status.internalStatus() ) != 0 )
					return;
			}
			else {
				if ( session_->changeStatusDescription( status.internalStatus(), descr ) != 0 )
					return;
			}
		}
	}

	status_ = status;
	myself_->setOnlineStatus( status_, descr );

	if ( status_.internalStatus() == GG_STATUS_NOT_AVAIL || status_.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
		if ( pingTimer_ ){
			pingTimer_->stop();
		}
	}
}

void
GaduAccount::slotLogin( int status, const QString& dscr, bool lastAttemptFailed )
{
	QString pass;

	lastStatus		= status;
	lastDescription	= dscr;

	myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING ), dscr );

	if ( !session_->isConnected() || lastAttemptFailed ) {
		pass = password( lastAttemptFailed );
		if ( pass.isEmpty() ) {
			slotCommandDone( QString::null,
					i18n( "Please set password, empty passwords are not supported by Gadu-Gadu"  ) );
			// and set status disconnected, so icon on toolbar won't blink
			myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
			return;
		}
		if ( pass.isNull() && lastAttemptFailed ){
			// user pressed CANCEL
			myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
			return;
		}
		session_->login( accountId().toInt(), pass, connectWithSSL, status, dscr );
	}
	else {
		session_->changeStatus( status );
	}
}

void
GaduAccount::slotLogoff()
{
	if ( session_->isConnected() ) {
		status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		changeStatus( status_ );
		session_->logoff();
	}
	connectWithSSL = true;
}

void
GaduAccount::slotGoOnline()
{
	if ( !session_->isConnected() ) {
	    connectWithSSL = true;
		kdDebug(14100) << "#### Connecting..." << endl;
		slotLogin();
	}
	else{
		changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL ) );
	}
}
void
GaduAccount::slotGoOffline()
{
	slotLogoff();
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
		session_->removeNotify( u );
	}
}

void
GaduAccount::addNotify( uin_t uin )
{
	if ( session_->isConnected() ) {
		session_->addNotify( uin );
	}
}

void
GaduAccount::notify( uin_t* userlist, int count )
{
	if ( session_->isConnected() ) {
		session_->notify( userlist, count );
	}
}

void
GaduAccount::sendMessage( uin_t recipient, const QString& msg, int msgClass )
{
	QString sendMsg, cpMsg;

	if ( session_->isConnected() ) {
		sendMsg = msg;
		sendMsg.replace( QString::fromAscii( "\n" ), QString::fromAscii( "\r\n" ) );
		cpMsg = textcodec_->fromUnicode( sendMsg );
		session_->sendMessage( recipient, (unsigned char *)cpMsg.ascii(), msgClass );
	}
}

void
GaduAccount::error( const QString& title, const QString& message )
{
	KMessageBox::error( 0, title, message );
}

void
GaduAccount::messageReceived( struct gg_event* e )
{
	GaduContact* c = 0;
	KopeteContactPtrList tmp;
	KopeteContactPtrList tmpPtrList;
	QString message;

	if ( !e->event.msg.message ) {
		return;
	}

	// FIXME:check for ignored users list
	// FIXME:anonymous (those not on the list) users should be ignored, as an option

	message = textcodec_->toUnicode( (const char*) e->event.msg.message );

	if ( e->event.msg.sender == 0 ) {
		//system message, display them or not?
		kdDebug(14100) << "####" << " System Message " << message << endl;
		return;
	}

	c = static_cast<GaduContact*> ( contacts()[ QString::number( e->event.msg.sender ) ] );

	if ( !c ) {
		KopeteMetaContact* metaContact = new KopeteMetaContact ();
		metaContact->setTemporary ( true );
		c = new GaduContact( e->event.msg.sender,
				QString::number( e->event.msg.sender ), this, metaContact );
		KopeteContactList::contactList ()->addMetaContact( metaContact );
		addNotify( e->event.msg.sender );
	}

	tmpPtrList.append( myself_ );
	KopeteMessage msg( c, tmpPtrList, message, KopeteMessage::Inbound );
	c->messageReceived( msg );
}

void
GaduAccount::ackReceived( struct gg_event* e  )
{
	GaduContact* c;

	c = static_cast<GaduContact*> ( contacts()[ QString::number( e->event.ack.recipient ) ] );
	if ( c ) {
		kdDebug(14100) << "####" << "Received an ACK from " << c->uin() << endl;
		c->messageAck();
	}
	else {
		kdDebug(14100) << "####" << "Received an ACK from an unknown user : " << e->event.ack.recipient << endl;
	}
}


void
GaduAccount::notify( struct gg_event* e )
{
	GaduContact* c;
	unsigned int n;

	for( n=0 ; e->event.notify60[n].uin ; n++ ) {
		kdDebug(14100) << "### NOTIFY " << e->event.notify60[n].uin << " " << e->event.notify60[n].status << endl;
		c = static_cast<GaduContact*> ( contacts()[ QString::number( e->event.notify60[n].uin ) ] );

		if ( !c ) {
			kdDebug(14100) << "Notify not in the list " << e->event.notify60[n].uin << endl;
			session_->removeNotify( e->event.notify60[n].uin );
			continue;
		}

// FIXME:store this info in GaduContact, be usefull in dcc and custom images
//		n->remote_ip;
//		n->remote_port;
//		n->version;
//		n->image_size;

		if ( e->event.notify60[n].descr ) {
			c->setDescription( textcodec_->toUnicode( e->event.notify60[n].descr )  );
			c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( e->event.notify60[n].status ), c->description() );
		}
		else {
			c->setDescription( QString::null );
			c->setOnlineStatus(  GaduProtocol::protocol()->convertStatus( e->event.notify60[n].status ) );
		}
	}
}

void
GaduAccount::statusChanged( struct gg_event* e )
{
	kdDebug(14100) << "####" << " status changed, uin:" << e->event.status60.uin <<endl;

	GaduContact* c;

	c = static_cast<GaduContact*>( contacts()[ QString::number( e->event.status60.uin ) ] );
	if( !c ) {
		return;
	}

	if ( e->event.status60.descr ) {
		c->setDescription( textcodec_->toUnicode( e->event.status60.descr ) );
		c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( e->event.status60.status ), c->description()  );
	}
	else {
		c->setDescription( QString::null );
		c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( e->event.status60.status )  );
	}

/// FIXME: again, store this information
//	e->event.status60.remote_ip;
//	e->event.status60.remote_port;
//	e->event.status60.version;
//	e->event.status60.image_size;
}

void
GaduAccount::pong()
{
	kdDebug(14100) << "####" << " Pong..." << endl;
}

void
GaduAccount::connectionFailed( gg_failure_t failure )
{
	status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
	myself_->setOnlineStatus( status_ );

	if ( failure == GG_FAILURE_PASSWORD ) {
		slotLogin( lastStatus, lastDescription, true );
		return;
	}
	if ( failure == GG_FAILURE_TLS || ( connectWithSSL && failure == GG_FAILURE_CONNECTING ) ) {
		if ( isTls == 0 && connectWithSSL) {
			connectWithSSL = false;
			slotCommandDone( QString::null, i18n( "connection using SSL was not possible, retrying without." ) );
			slotLogin( lastStatus, lastDescription );
			return;
		}
	}

	error( i18n( "unable to connect to the Gadu-Gadu server(\"%1\")." ).arg( GaduSession::failureDescription( failure ) ),
			i18n( "Connection Error" ) );

}

void
GaduAccount::connectionSucceed( struct gg_event* /*e*/ )
{
	kdDebug(14100) << "#### Gadu-Gadu connected! " << endl;
	status_ =  GaduProtocol::protocol()->convertStatus( session_->status() );
	myself_->setOnlineStatus( status_ );
	startNotify();

	QObject::connect( session_, SIGNAL( userListRecieved( const QString& ) ),
					SLOT( userlist( const QString& ) ) );
	session_->requestContacts();

	if ( !pingTimer_ ) {
		pingTimer_ = new QTimer( this );
		QObject::connect( pingTimer_, SIGNAL( timeout() ),
		SLOT( pingServer() ) );
	}
	pingTimer_->start( 180000 );//3 minute timeout
}

void
GaduAccount::startNotify()
{
	int i = 0;
	if ( !contacts().count() ) {
		return;
	}

	QDictIterator<KopeteContact> it( contacts() );

	uin_t* userlist = 0;
	userlist = new uin_t[ contacts().count() ];

	for( i=0 ; it.current() ; ++it ) {
		userlist[i++] = static_cast<GaduContact*> ((*it))->uin();
	}

	session_->notify( userlist, contacts().count() );
}

void
GaduAccount::slotSessionDisconnect()
{
	uin_t status;

	kdDebug(14100) << "Disconnecting" << endl;

	if (pingTimer_) {
		pingTimer_->stop();
	}
	QDictIterator<KopeteContact> it( contacts() );

	for ( ; it.current() ; ++it ) {
		static_cast<GaduContact*>((*it))->setOnlineStatus(
				GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
	}

	status = myself_->onlineStatus().internalStatus();
	if ( status != GG_STATUS_NOT_AVAIL || status!= GG_STATUS_NOT_AVAIL_DESCR ) {
		myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
	}
}


void
GaduAccount::userlist( const QString& list )
{
	kdDebug(14100)<<"### Got userlist - gadu account"<<endl;

	gaduContactsList u;
	QString contactname;
	QStringList groupsl;
	GaduContact* ucontact;
	KopeteMetaContact* metac;
	bool s;
	int i;

	// FIXME: give feedback about error
	if ( session_->stringToContacts( u , list ) == false ) {
		return;
	}

	QPtrListIterator< contactLine > loo( u );

	for ( i=u.count() ; i-- ;  ) {
		kdDebug(14100) << "uin " << (*loo)->uin << endl;

		if ( (*loo)->uin.isNull() ) {
			kdDebug(14100) << "no Uin, strange.. "<<endl;
			goto next_cont;
		}

		if ( contacts()[ (*loo)->uin ] ) {
			kdDebug(14100) << "UIN already exists in contacts "<< (*loo)->uin << endl;
		}
		else {
			if ( (*loo)->name.length() ) {
				contactname = (*loo)->name;
			}

			// if there is no nickname
			if ( (*loo)->nickname.isNull() ) {
				// no name either
				if ( (*loo)->name.isNull() ) {
					// maybe we can use fistname + surname ?
					if ( (*loo)->firstname.isNull() && (*loo)->surname.isNull() ) {
						contactname = (*loo)->uin;
					}
					// what a shame, i have to use UIN than :/
					else {
						if ( (*loo)->firstname.isNull() ) {
							contactname = (*loo)->surname;
						}
						else {
							if ( (*loo)->surname.isNull() ) {
								contactname = (*loo)->firstname;
							}
							else {
								contactname = (*loo)->firstname+" "+(*loo)->surname;
							}
						}
					}
				}
				else {
					contactname = (*loo)->name;
				}
			}
			else {
				contactname = (*loo)->nickname;
			}

			s = addContact( (*loo)->uin, contactname, 0L, KopeteAccount::DontChangeKABC, QString::null, false );
			if ( s == false ) {
				kdDebug(14100) << "There was a problem adding UIN "<< (*loo)->uin << "to users list" << endl;
				goto next_cont;
			}
		}
		ucontact = static_cast<GaduContact*>( contacts()[ (*loo)->uin ] );

		// update/add infor for contact
		ucontact->setInfo( (*loo)->email, (*loo)->firstname, (*loo)->surname, (*loo)->nickname, (*loo)->phonenr );

		if ( !( (*loo)->group.isEmpty() ) ) {
			// FIXME: libkopete bug i guess, by default contact goes to top level group
			// if user desrired to see contact somewhere else, remove it from top level one
			metac = ucontact->metaContact();
			metac->removeFromGroup( KopeteGroup::topLevel() );
			// put him in all desired groups:
			groupsl = QStringList::split( ",", (*loo)->group );
			for ( QStringList::Iterator g = groupsl.begin(); g != groupsl.end(); ++g ) {
				metac->addToGroup( KopeteContactList::contactList ()->getGroup ( *g ) );
			}
		}

next_cont:
		++loo;
	}
}

void
GaduAccount::userListExportDone()
{
	slotCommandDone( QString::null, i18n( "Contacts exported to the server.") );
}

void
GaduAccount::slotExportContactsList()
{
	session_->exportContacts( userlist() );
}


gaduContactsList*
GaduAccount::userlist()
{
	GaduContact* contact;
	gaduContactsList* gaducontactslist = new gaduContactsList;
	contactLine cl;
	int i;

	if ( !contacts().count() ) {
		return gaducontactslist;
	}

	QDictIterator<KopeteContact> it( contacts() );

	for( i=0 ; it.current() ; ++it ) {
		contact = static_cast<GaduContact*>(*it);
		if ( contact->uin() != myself_->uin() ) {
			gaducontactslist->append( contact->contactDetails() );
		}
	}

	return gaducontactslist;
}

void
GaduAccount::pingServer()
{
	session_->ping();
}

void
GaduAccount::slotSearch()
{
	new GaduPublicDir( this, NULL, "Gadu Public user directory search" );
}

void
GaduAccount::slotChangePassword()
{
/*
	QCString password;
	int result = KPasswordDialog::getPassword( password, i18n("Enter new password") );

//	if ( result == KPasswordDialog::Accepted ) {
//		ChangePasswordCommand *cmd = new ChangePasswordCommand( this, "changePassCmd" );
//		cmd->setInfo( myself_->uin(), password(), password, "zackrat@att.net" );
//		QObject::connect( cmd, SIGNAL(done(const QString&, const QString&)),
//											SLOT(slotCommandDone(const QString&, const QString&)) );
//		QObject::connect( cmd, SIGNAL(error(const QString&, const QString&)),
//											SLOT(slotCommandError(const QString&, const QString&)) );
//	}
*/
}

void
GaduAccount::slotCommandDone( const QString& /*title*/, const QString& what )
{
	//FIXME: any chance to have my own title in event popup ?
	KNotifyClient::userEvent( 0, what,
			KNotifyClient::PassivePopup, KNotifyClient::Notification  );
}

void
GaduAccount::slotCommandError(const QString& title, const QString& what )
{
	error( title, what );
}

void
GaduAccount::slotDescription()
{
	GaduAway* away = new GaduAway( this );

	if( away->exec() == QDialog::Accepted ) {
		changeStatus( GaduProtocol::protocol()->convertStatus( away->status() ),
					away->awayText() );
	}
	delete away;
}

bool GaduAccount::pubDirSearch( QString& name, QString& surname, QString& nick,
			    int UIN, QString& city, int gender,
			    int ageFrom, int ageTo, bool onlyAlive )
{
    return session_->pubDirSearch( name, surname, nick, UIN, city, gender,
							ageFrom, ageTo, onlyAlive );
}

void GaduAccount::pubDirSearchClose()
{
	session_->pubDirSearchClose();
}

void GaduAccount::slotSearchResult( const searchResult& result )
{
	emit pubDirSearchResult( result );
}


int GaduAccount::isConnectionEncrypted()
{
	return isTls;
}

void GaduAccount::useTls( int ut )
{
	if ( ut < 0 || ut > 2 ) {
		return;
	}
	isTls = ut;
	setPluginData( protocol(), QString::fromAscii( "useEncryptedConnection" ), QString::number( ut ) );
}

#include "gaduaccount.moc"
