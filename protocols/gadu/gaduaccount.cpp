
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
#include "kopeteuiglobal.h"

#include <kpassdlg.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <ktempfile.h>
#include <kio/netaccess.h>

#include <qapplication.h>
#include <qdialog.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <netinet/in.h>

// FIXME: use dynamic cache please, i consider this as broken resolution of this problem
// FIXME: needs to be resolved before 0.8/kde3.2 will be out !
const int NUM_SERVERS = 5;
const char* const servers_ip[ NUM_SERVERS ] = {
	"217.17.41.88",
 	"217.17.41.85",
	"217.17.41.87",
	"217.17.41.86",
	"217.17.41.84",
};

 GaduAccount::GaduAccount( KopeteProtocol* parent, const QString& accountID,const char* name )
: KopeteAccount( parent, accountID, name ), pingTimer_( 0 ), saveListDialog( NULL ), loadListDialog( NULL )
{
	QHostAddress ip;

	textcodec_ = QTextCodec::codecForName( "CP1250" );
	session_ = new GaduSession( this, "GaduSession" );
	KGlobal::config()->setGroup( "Gadu" );

	setMyself( new GaduContact(  accountId().toInt(), accountId(), this, new KopeteMetaContact() ) );

	status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL );
	lastDescription = QString::null;

	for ( int i = 0; i < NUM_SERVERS; i++ ) {
		ip.setAddress( QString( servers_ip[i] ) );
		servers_.append( ip );
	}
	currentServer = -1;
	serverIP = 0;

	initActions();
	initConnections();
}

void
GaduAccount::initActions()
{
	searchAction		= new KAction( i18n( "&Search for Friends" ), "", 0,
							this, SLOT( slotSearch() ), this, "actionSearch" );
	listputAction		= new KAction( i18n( "Export Contacts on Server" ), "", 0,
							this, SLOT( slotExportContactsList() ), this, "actionListput" );
	listToFileAction	= new KAction( i18n( "Export Contacts to file" ), "", 0,
							this, SLOT( slotExportContactsListToFile() ), this, "actionListputFile" );
	listFromFileAction	= new KAction( i18n( "Import Contacts from file" ), "", 0,
							this, SLOT( slotImportContactsFromFile() ), this, "actionListgetFile" );
}

void
GaduAccount::initConnections()
{
	QObject::connect( session_, SIGNAL( error( const QString&, const QString& ) ),
				SLOT( error( const QString&, const QString& ) ) );
	QObject::connect( session_, SIGNAL( messageReceived( KGaduMessage* ) ),
				SLOT( messageReceived( KGaduMessage* ) )  );
	QObject::connect( session_, SIGNAL( notify( KGaduNotifyList* ) ),
				SLOT( notify( KGaduNotifyList* ) ) );
	QObject::connect( session_, SIGNAL( contactStatusChanged( KGaduNotify* ) ),
				SLOT( contactStatusChanged( KGaduNotify* ) ) );
	QObject::connect( session_, SIGNAL( connectionFailed( gg_failure_t )),
				SLOT( connectionFailed( gg_failure_t ) ) );
	QObject::connect( session_, SIGNAL( connectionSucceed( ) ),
				SLOT( connectionSucceed( ) ) );
	QObject::connect( session_, SIGNAL( disconnect() ),
				SLOT( slotSessionDisconnect() ) );
	QObject::connect( session_, SIGNAL( ackReceived( unsigned int ) ),
				SLOT( ackReceived( unsigned int ) ) );
	QObject::connect( session_, SIGNAL( pubDirSearchResult( const searchResult& ) ),
				SLOT( slotSearchResult( const searchResult& ) ) );
	QObject::connect( session_, SIGNAL( userListExported() ),
				SLOT( userListExportDone() ) );
}

void
GaduAccount::loaded()
{
	QString nick;
	nick	= pluginData( protocol(), QString::fromAscii( "nickName" ) );
	if ( !nick.isNull() ) {
		myself()->rename( nick );
	}
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


KActionMenu*
GaduAccount::actionMenu()
{
	kdDebug(14100) << "actionMenu() " << endl;

	actionMenu_ = new KActionMenu( accountId(), myself()->onlineStatus().iconFor( this ), this );
	actionMenu_->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ), i18n( "%1 <%2> " ).

#if QT_VERSION < 0x030200
	arg( myself()->displayName() ).arg( accountId() ) );
#else
	arg( myself()->displayName(), accountId() ) );
#endif

	if ( session_->isConnected() ) {
		searchAction->setEnabled( TRUE );
		listputAction->setEnabled( TRUE );
	}
	else {
		searchAction->setEnabled( FALSE );
		listputAction->setEnabled( FALSE );
	}
	kdDebug( 14100 ) << "nr of contacts - " << contacts().count() <<endl ;
	
	if ( contacts().count() > 1 ) {
		if ( saveListDialog ) {
			listToFileAction->setEnabled( FALSE );
		}	
		else {
			listToFileAction->setEnabled( TRUE );
		}
	
		listToFileAction->setEnabled( TRUE );
	}
	else {
		listToFileAction->setEnabled( FALSE );
	}
	
	if ( loadListDialog ) {
		listFromFileAction->setEnabled( FALSE );
	}
	else {
		listFromFileAction->setEnabled( TRUE );
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
	actionMenu_->insert( listToFileAction );
	actionMenu_->insert( listFromFileAction );
	
	return actionMenu_;
}

void
GaduAccount::connect()
{
	slotGoOnline();
}

void
GaduAccount::disconnect()
{
	slotGoOffline();
	connectWithSSL=true;
}

bool
GaduAccount::addContactToMetaContact( const QString& contactId, const QString& displayName,
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
			if ( useTls() != TLS_no ) {
				connectWithSSL = true;
			}
			else {
				connectWithSSL = false;
			}
			serverIP = 0;
			currentServer = -1;
			status_ = status;
			kdDebug(14100) << "#### Connecting..., tls option "<< (int)useTls() << " " << endl;
			lastDescription = descr;
			slotLogin( status.internalStatus(), descr );
			return;
		}
		else {
			status_ = status;
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

	myself()->setOnlineStatus( status );
	myself()->setProperty( "awayMessage", descr );

	if ( status.internalStatus() == GG_STATUS_NOT_AVAIL || status.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
		if ( pingTimer_ ){
			pingTimer_->stop();
		}
	}
}

void
GaduAccount::slotLogin( int status, const QString& dscr )
{
	lastDescription	= dscr;

	myself()->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING ));
	myself()->setProperty( "awayMessage", dscr );


	if ( !session_->isConnected() ) {
		if ( password().isEmpty() ) {
			connectionFailed( GG_FAILURE_PASSWORD );
		}
		else {
				session_->login( accountId().toInt(), password(), connectWithSSL, status, dscr, serverIP );
		}
	}
	else {
		session_->changeStatus( status );
	}
}

void
GaduAccount::slotLogoff()
{
	if ( session_->isConnected() || status_ == GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING )) {
		status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		changeStatus( status_ );
		session_->logoff();
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
	if ( session_->isConnected() ) {
		session_->sendMessage( recipient, msg, msgClass );
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
	KopeteContactPtrList contactsListTmp;

	// FIXME:check for ignored users list
	// FIXME:anonymous (those not on the list) users should be ignored, as an option

	if ( gaduMessage->sender_id == 0 ) {
		//system message, display them or not?
		kdDebug(14100) << "####" << " System Message " << gaduMessage->message << endl;
		return;
	}

	contact = static_cast<GaduContact*> ( contacts()[ QString::number( gaduMessage->sender_id ) ] );

	if ( !contact ) {
		KopeteMetaContact* metaContact = new KopeteMetaContact ();
		metaContact->setTemporary ( true );
		contact = new GaduContact( gaduMessage->sender_id,
				QString::number( gaduMessage->sender_id ), this, metaContact );
		KopeteContactList::contactList ()->addMetaContact( metaContact );
		addNotify( gaduMessage->sender_id );
	}

	contactsListTmp.append( myself() );
	KopeteMessage msg( gaduMessage->sendTime, contact, contactsListTmp, gaduMessage->message, KopeteMessage::Inbound );
	contact->messageReceived( msg );
}

void
GaduAccount::ackReceived( unsigned int recipient  )
{
	GaduContact* contact;

	contact = static_cast<GaduContact*> ( contacts()[ QString::number( recipient ) ] );
	if ( contact ) {
		kdDebug(14100) << "####" << "Received an ACK from " << contact->uin() << endl;
		contact->messageAck();
	}
	else {
		kdDebug(14100) << "####" << "Received an ACK from an unknown user : " << recipient << endl;
	}
}


void
GaduAccount::notify( KGaduNotifyList* notifyList )
{
	GaduContact* contact;
	QPtrListIterator<KGaduNotify>notifyListIterator( *notifyList );
	unsigned int i;

// FIXME:store this info in GaduContact, be usefull in dcc and custom images
//		n->remote_ip;
//		n->remote_port;
//		n->version;
//		n->image_size;

	for ( i = notifyList->count() ; i-- ; ++notifyListIterator ) {
		kdDebug(14100) << "### NOTIFY " << (*notifyListIterator)->contact_id << " " << (*notifyListIterator)->status << endl;
		contact = static_cast<GaduContact*> ( contacts()[ QString::number( (*notifyListIterator)->contact_id ) ] );

		if ( !contact ) {
			kdDebug(14100) << "Notify not in the list " << (*notifyListIterator)->contact_id << endl;
			session_->removeNotify((*notifyListIterator)->contact_id );
			continue;
		}

		if ( (*notifyListIterator)->description.isNull() ) {
			contact->setOnlineStatus(  GaduProtocol::protocol()->convertStatus( (*notifyListIterator)->status ) );
			contact->removeProperty( "awayMessage" );

		}
		else {
			contact->setOnlineStatus( GaduProtocol::protocol()->convertStatus( (*notifyListIterator)->status ) );
			contact->setProperty( "awayMessage", (*notifyListIterator)->description );
		}
	}
}

void
GaduAccount::contactStatusChanged( KGaduNotify* gaduNotify )
{
	kdDebug(14100) << "####" << " contact's status changed, uin:" << gaduNotify->contact_id <<endl;

	GaduContact* contact;

	contact = static_cast<GaduContact*>( contacts()[ QString::number( gaduNotify->contact_id ) ] );
	if( !contact ) {
		return;
	}

	if ( gaduNotify->description.isEmpty() ) {
		contact->setOnlineStatus( GaduProtocol::protocol()->convertStatus( gaduNotify->status ) );
		contact->removeProperty( "awayMessage" );
	}
	else {
		contact->setOnlineStatus( GaduProtocol::protocol()->convertStatus( gaduNotify->status ) );
		contact->setProperty( "awayMessage", gaduNotify->description );
	}

/// FIXME: again, store this information
//	e->event.status60.r emote_ip;
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
GaduAccount::pingServer()
{
	kdDebug(14100) << "####" << " Ping..." << endl;
	session_->ping();
}

void
GaduAccount::connectionFailed( gg_failure_t failure )
{
	bool tryReconnect = false;
	QString pass;


	switch (failure) {
		case GG_FAILURE_PASSWORD:
			pass = password( true );
			if ( pass.isEmpty() ) {
				slotCommandDone( QString::null, i18n( "Please set password, empty passwords are not supported by Gadu-Gadu"  ) );
				// and set status disconnected, so icon on toolbar won't blink
				status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
				myself()->setOnlineStatus( status_ );
				return;
			}
			if ( pass.isNull() ){
				// user pressed CANCEL
				status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
				myself()->setOnlineStatus( status_ );
				return;
			}
			tryReconnect = true;
		break;
		default:
			if ( connectWithSSL ) {
				if ( useTls() != TLS_only ) {
					slotCommandDone( QString::null, i18n( "connection using SSL was not possible, retrying without." ) );
					kdDebug( 14100 ) << "try without tls now" << endl;
					connectWithSSL = false;
					tryReconnect = true;
					currentServer = -1;
					serverIP = 0;
					break;
				}
			}
			else {
				if ( currentServer == NUM_SERVERS-1 ) {
					serverIP = 0;
					currentServer = -1;
				}
				else {
					serverIP = htonl( servers_[ ++currentServer ].ip4Addr() );
					kdDebug(14100) << "trying : " << currentServer << endl;
					tryReconnect = true;
				}
			}
		break;
	}

	if ( tryReconnect ) {
			slotLogin( status_.internalStatus() , lastDescription );
	}
	else {
		error( i18n( "unable to connect to the Gadu-Gadu server(\"%1\")." ).arg( GaduSession::failureDescription( failure ) ),
				i18n( "Connection Error" ) );
		status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		myself()->setOnlineStatus( status_ );
	}
}

void
GaduAccount::connectionSucceed( )
{
	kdDebug(14100) << "#### Gadu-Gadu connected! " << endl;
	status_ =  GaduProtocol::protocol()->convertStatus( session_->status() );
	myself()->setOnlineStatus( status_ );
	myself()->setProperty( "awayMessage", lastDescription );
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

	QDictIterator<KopeteContact> kopeteContactsList( contacts() );

	uin_t* userlist = 0;
	userlist = new uin_t[ contacts().count() ];

	for( i=0 ; kopeteContactsList.current() ; ++kopeteContactsList ) {
		userlist[i++] = static_cast<GaduContact*> ((*kopeteContactsList))->uin();
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

	status = myself()->onlineStatus().internalStatus();
	if ( status != GG_STATUS_NOT_AVAIL || status!= GG_STATUS_NOT_AVAIL_DESCR ) {
		myself()->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
	}
}


void
GaduAccount::userlist( const QString& contactsListString )
{
	kdDebug(14100)<<"### Got userlist - gadu account"<<endl;

	gaduContactsList contactsList;
	QString contactName;
	QStringList groups;
	GaduContact* contact;
	KopeteMetaContact* metaContact;
	int i;

	// FIXME: give feedback about error
	if ( session_->stringToContacts( contactsList, contactsListString ) == false ) {
		kdDebug(14100) << "couldn't convert it, something is wrong" << endl;
		return;
	}

	QPtrListIterator< contactLine >contactLine( contactsList );

	for ( i = contactsList.count() ; i-- ;  ) {
		kdDebug(14100) << "uin " << (*contactLine)->uin << endl;

		if ( (*contactLine)->uin.isNull() ) {
			kdDebug(14100) << "no Uin, strange.. "<<endl;
			goto next_cont;
		}

		if ( contacts()[ (*contactLine)->uin ] ) {
			kdDebug(14100) << "UIN already exists in contacts "<< (*contactLine)->uin << endl;
		}
		else {
			if ( (*contactLine)->displayname.length() ) {
				contactName = (*contactLine)->displayname;
			}

			// if there is no nickname
			if ( (*contactLine)->displayname.isNull() ) {
				// no name either
				if ( (*contactLine)->nickname.isNull() ) {
					// maybe we can use fistname + surname ?
					if ( (*contactLine)->firstname.isNull() && (*contactLine)->surname.isNull() ) {
						contactName = (*contactLine)->uin;
					}
					// what a shame, i have to use UIN than :/
					else {
						if ( (*contactLine)->firstname.isNull() ) {
							contactName = (*contactLine)->surname;
						}
						else {
							if ( (*contactLine)->surname.isNull() ) {
								contactName = (*contactLine)->firstname;
							}
							else {
								contactName = (*contactLine)->firstname+" "+(*contactLine)->surname;
							}
						}
					}
				}
				else {
					contactName = (*contactLine)->nickname;
				}
			}
			else {
				contactName = (*contactLine)->displayname;
			}

			bool s = addContact( (*contactLine)->uin, contactName, 0L, KopeteAccount::DontChangeKABC, QString::null, false );
			if ( s == false ) {
				kdDebug(14100) << "There was a problem adding UIN "<< (*contactLine)->uin << "to users list" << endl;
				goto next_cont;
			}
		}
		contact = static_cast<GaduContact*>( contacts()[ (*contactLine)->uin ] );
		if ( contact == NULL ) {
			kdDebug(14100) << "oops, no KopeteContact in contacts()[] for some reason, for \"" << (*contactLine)->uin << "\"" << endl;
			goto next_cont;
		}
		
		// update/add infor for contact
		
		contact->setProperty( "emailAddress", (*contactLine)->email );
		contact->setProperty( "firstName", (*contactLine)->firstname );
		contact->setProperty( "lastName", (*contactLine)->surname );
		contact->setProperty( "privPhoneNum", (*contactLine)->phonenr );
		contact->setProperty( "ignored", i18n( "ignored" ), (*contactLine)->ignored ? "true" : "false" );
		if ( (*contactLine)->displayname.isEmpty() ) {
			if ( contact->contactId().isEmpty() ) {
				kdDebug(14100) << (*contactLine)->uin << "nick name: " << contactName << endl;
				contact->rename( contactName );
			}
		}
		else {
		
		}
		

		if ( !( (*contactLine)->group.isEmpty() ) ) {
			// FIXME: libkopete bug i guess, by default contact goes to top level group
			// if user desrired to see contact somewhere else, remove it from top level one
			metaContact = contact->metaContact();
			metaContact->removeFromGroup( KopeteGroup::topLevel() );
			// put him in all desired groups:
			groups = QStringList::split( ",", (*contactLine)->group );
			for ( QStringList::Iterator groupsIterator = groups.begin(); groupsIterator != groups.end(); ++groupsIterator ) {
				metaContact->addToGroup( KopeteContactList::contactList ()->getGroup ( *groupsIterator) );
			}
		}

next_cont:
		// next contact line
		++contactLine;
	}
}

void
GaduAccount::userListExportDone()
{
	slotCommandDone( QString::null, i18n( "Contacts exported to the server.") );
}


// FIXME: make loading and saving nonblocking (at the moment KFileDialog stops plugin/kopete)

void
GaduAccount::slotExportContactsListToFile()
{
	KTempFile tempFile;
	
	if ( saveListDialog ) {
		kdDebug( 14100 ) << " save contacts to file: alread waiting for input " << endl ;
		return;
	}
	
	saveListDialog = new KFileDialog( "::kopete-gadu" + accountId(), QString::null, 
					Kopete::UI::Global::mainWidget(), "gadu-list-save", false ); 
	saveListDialog->setCaption( i18n(" Save Contacts list for account %1 as ...").arg( myself()->displayName() ) );
	
	if ( saveListDialog->exec() == QDialog::Accepted ) {
		
		QTextCodec* textcodec = QTextCodec::codecForName( "CP1250" );
		QCString list = textcodec->fromUnicode( session_->contactsToString( userlist() ) );
		
		if ( tempFile.status() ) {
			// say cheese, can't create file.....			
			error( i18n( "Unable to create temporary file" ), i18n( "Save Contacts list failed" ) );
		}
		else {
			QTextStream* tempStream = tempFile.textStream();
			(*tempStream) << list.data();
			tempFile.close();
			
			bool res = KIO::NetAccess::upload( 
								tempFile.name() , 
								saveListDialog->selectedURL() , 
								Kopete::UI::Global::mainWidget() 
								);
			if ( !res ) {
				// say it failed
				error( KIO::NetAccess::lastErrorString(), i18n( "Save Contacts list failed" ) );
			}
		}

	}
	delete saveListDialog;
	saveListDialog = NULL;
}

void
GaduAccount::slotImportContactsFromFile()
{

	if ( loadListDialog ) {
		kdDebug( 14100 ) << "load contacts from file: alread waiting for input " << endl ;
		return;
	}

	loadListDialog = new KFileDialog( "::kopete-gadu" + accountId(), QString::null, 
					Kopete::UI::Global::mainWidget(), "gadu-list-load", true ); 
	loadListDialog->setCaption( i18n(" Load Contacts list for account %1 as ...").arg( myself()->displayName() ) );
	
	if ( loadListDialog->exec() == QDialog::Accepted ) {
		
		QTextCodec* textcodec = QTextCodec::codecForName( "CP1250" );
		QCString list;

		KURL url = loadListDialog->selectedURL();
		QString oname;
		kdDebug(14100) << "a:"<<url<<"\nb:" << oname << endl;
		if ( KIO::NetAccess::download(	url, 
						oname,
						Kopete::UI::Global::mainWidget() 
						) ) {
						
			QFile tempFile( oname );
			if ( tempFile.open( IO_ReadOnly ) ) {
				QTextStream tempStream( &tempFile );
				tempStream >> list;
				tempFile.close();
				// and store it
				kdDebug( 14100 ) << "loaded list:" << endl;
				kdDebug( 14100 ) << list << endl;
				kdDebug( 14100 ) << " --------------- " << endl;
				userlist( textcodec->toUnicode( list ) );
			}
			else {
				error( tempFile.errorString(),
					i18n( "Contacts list Load has failed" ) );
			}
		}
		else {
			// say, it failed misourably
			error( KIO::NetAccess::lastErrorString(), 
				i18n( "Contacts list Load has failed" ) );
		}

	}
	delete loadListDialog;
	loadListDialog = NULL;
}

void
GaduAccount::slotExportContactsList()
{
	session_->exportContactsOnServer( userlist() );
}


gaduContactsList*
GaduAccount::userlist()
{
	GaduContact* contact;
	gaduContactsList* contactsList = new gaduContactsList;
	int i;

	if ( !contacts().count() ) {
		return contactsList;
	}

	QDictIterator<KopeteContact> contactsIterator( contacts() );

	for( i=0 ; contactsIterator.current() ; ++contactsIterator ) {
		contact = static_cast<GaduContact*>( *contactsIterator );
		if ( contact->uin() != static_cast<GaduContact*>( myself() )->uin() ) {
			contactsList->append( contact->contactDetails() );
		}
	}

	return contactsList;
}

void
GaduAccount::slotSearch()
{
	new GaduPublicDir( this );
}

void
GaduAccount::slotSearch( int uin )
{
	new GaduPublicDir( this, uin );
}

void
GaduAccount::slotChangePassword()
{
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

bool
GaduAccount::pubDirSearch( QString& name, QString& surname, QString& nick,
			    int UIN, QString& city, int gender,
			    int ageFrom, int ageTo, bool onlyAlive )
{
	return session_->pubDirSearch( name, surname, nick, UIN, city, gender,
							ageFrom, ageTo, onlyAlive );
}

void
GaduAccount::pubDirSearchClose()
{
	session_->pubDirSearchClose();
}

void
GaduAccount::slotSearchResult( const searchResult& result )
{
	emit pubDirSearchResult( result );
}


GaduAccount::tlsConnection
GaduAccount::useTls()
{
	tlsConnection Tls = (tlsConnection) ( pluginData( protocol(), QString::fromAscii( "useEncryptedConnection" ) ).toInt() );
	if ( Tls != TLS_ifAvaliable && Tls != TLS_only && Tls != TLS_no ) {
		// default
		Tls = TLS_no;
	}
	return Tls;
}

void
GaduAccount::setUseTls( tlsConnection ut )
{
	if ( ut < 0 || ut > 2 ) {
		return;
	}
	setPluginData( protocol(), QString::fromAscii( "useEncryptedConnection" ), QString::number( (int) ut ) );
}

#include "gaduaccount.moc"
