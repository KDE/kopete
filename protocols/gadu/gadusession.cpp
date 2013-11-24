// vim: set noet ts=4 sts=4 sw=4 :
// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004	 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
// Copyright (C) 2002	 	Zack Rusin <zack@kde.org>
//
// gadusession.cpp
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
//

#include "gadusession.h"

#include <ctime>

#include <klocale.h>
#include <kdebug.h>
#include "kopetemessage.h"

#include <qsocketnotifier.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <QByteArray>
#include "gadurichtextformat.h"

#include <errno.h>
#include <string.h>
#include <netinet/in.h>

GaduSession::GaduSession( QObject* parent )
: QObject( parent ), session_( 0 ), searchSeqNr_( 0 ), deletingUserList( false )
{
	textcodec = QTextCodec::codecForName( "Windows-1250" );
	rtf = new GaduRichTextFormat;
}

GaduSession::~GaduSession()
{
	logoff();
}

bool
GaduSession::isConnected() const
{
	if ( session_ ) {
		return ( session_->state & GG_STATE_CONNECTED );
	}
	return false;
}

int
GaduSession::status() const
{
	if ( session_ ) {
		kDebug(14100)<<"Status = " << session_->status <<", initial = "<< session_->initial_status;
		return session_->status & ( ~GG_STATUS_FRIENDS_MASK );
	}
	return GG_STATUS_NOT_AVAIL;
}

void
GaduSession::login( struct gg_login_params* p )
{
	if ( !isConnected() ) {

// turn on in case you have any problems, and  you want
// to report it better. libgadu needs to be recompiled with debug enabled
//		gg_debug_level=GG_DEBUG_MISC|GG_DEBUG_FUNCTION;

		kDebug(14100) << "Login";

		if ( !( session_ = gg_login( p ) ) ) {
			destroySession();
			kDebug( 14100 ) << "libgadu internal error ";
			emit connectionFailed(  GG_FAILURE_CONNECTING );
			return;
		}

		createNotifiers( true );
		enableNotifiers( session_->check );
		searchSeqNr_=0;
	}
}

void
GaduSession::destroyNotifiers()
{
	disableNotifiers();
	delete read_;
	read_ = NULL;
	delete write_;
	write_ = NULL;
}

void
GaduSession::createNotifiers( bool connect )
{
	if ( !session_ ){
		return;
	}

	read_ = new QSocketNotifier( session_->fd, QSocketNotifier::Read, this );
	read_->setEnabled( false );

	write_ = new QSocketNotifier( session_->fd, QSocketNotifier::Write, this );
	write_->setEnabled( false );

	if ( connect ) {
		QObject::connect( read_, SIGNAL(activated(int)), SLOT(checkDescriptor()) );
		QObject::connect( write_, SIGNAL(activated(int)), SLOT(checkDescriptor()) );
	}
}

void
GaduSession::enableNotifiers( int checkWhat )
{
	if( (checkWhat & GG_CHECK_READ) && read_ ) {
		read_->setEnabled( true );
	}
	if( (checkWhat & GG_CHECK_WRITE) && write_ ) {
		write_->setEnabled( true );
	}
}

void
GaduSession::disableNotifiers()
{
	if ( read_ ) {
		read_->setEnabled( false );
	}
	if ( write_ ) {
		write_->setEnabled( false );
	}
}

void
GaduSession::dccRequest( const unsigned int uin )
{
	if ( session_ ) {
		gg_dcc_request( session_, uin );
	}
}

void
GaduSession::login( KGaduLoginParams* loginp )
{
	QByteArray desc = textcodec->fromUnicode( loginp->statusDescr );

	memset( &params_, 0, sizeof(params_) );

	params_.status_descr	= (char*)desc.data();

	params_.uin		= loginp->uin;
	params_.password	= loginp->password.data();
	params_.status		= loginp->status | ( loginp->forFriends ? GG_STATUS_FRIENDS_MASK : 0 );
	params_.async		= 1;
	params_.tls		= loginp->useTls;
	params_.server_addr	= loginp->server;
	params_.client_addr	= loginp->client_addr;
	params_.client_port	= loginp->client_port;

	kDebug(14100) << "LOGIN IP: " << loginp->client_addr;

	if ( loginp->useTls ) {
		params_.server_port = GG_HTTPS_PORT;
	}
	else {
		if ( loginp->server ) {
			params_.server_port = GG_DEFAULT_PORT;
		}
	}

	kDebug(14100)<<"gadusession::login, server ( " << loginp->server << " ), tls(" << loginp->useTls << ") ";
	login( &params_ );

}

void
GaduSession::destroySession()
{
	if ( session_ ) {
		destroyNotifiers();
		gg_free_session( session_ );
		session_ = 0;
	}
}

void
GaduSession::logoff( Kopete::Account::DisconnectReason reason )
{
	destroySession();
	emit disconnect( reason );
}

int
GaduSession::notify( uin_t* userlist, int count )
{
	if ( isConnected() ) {
		return gg_notify( session_, userlist, count );
	}
	else {
		emit error( i18n("Not Connected"), i18n("You are not connected to the server.") );
	}

	return 1;
}

int
GaduSession::addNotify( uin_t uin )
{
	if ( isConnected() ) {
		return gg_add_notify( session_, uin );
	}
	else {
		emit error( i18n("Not Connected"), i18n("You are not connected to the server.") );
	}
	return 1;
}

int
GaduSession::removeNotify( uin_t uin )
{
	if ( isConnected() ) {
		gg_remove_notify( session_, uin );
	}
	else {
		emit error( i18n("Not Connected"), i18n("You are not connected to the server.") );
	}

	return 1;
}

int
GaduSession::sendMessage( uin_t recipient, const Kopete::Message& msg, int msgClass )
{
	QString sendMsg;
	QByteArray cpMsg;
	KGaduMessage* gadumessage;

	if ( isConnected() ) {
		gadumessage = rtf->convertToGaduMessage( msg );
		if ( gadumessage ) {
			const void* data = (const void*)gadumessage->rtf.data();
			cpMsg = textcodec->fromUnicode( gadumessage->message );
			int o;
			o = gg_send_message_richtext( session_, msgClass, recipient, (const unsigned char *)cpMsg.data(), (const unsigned char*) data, gadumessage->rtf.size() );
			gadumessage->rtf.resize(0);
			delete gadumessage;
			return o;
		}
		else {
			sendMsg = msg.plainBody();
			sendMsg.replace( QLatin1Char( '\n' ), QString::fromAscii( "\r\n" ) );
			cpMsg = textcodec->fromUnicode( sendMsg );

			return gg_send_message( session_, msgClass, recipient, (const unsigned char *)cpMsg.data() );
		}
	}
	else {
		emit error( i18n("Not Connected"), i18n("You are not connected to the server.") );
	}

	return 1;
}

int
GaduSession::changeStatus( int status, bool forFriends )
{
	kDebug(14101)<<"## Changing to "<<status;
	if ( isConnected() ) {
		return gg_change_status( session_, status | ( forFriends ? GG_STATUS_FRIENDS_MASK : 0) );
	}
	else {
		emit error( i18n("Not Connected"),  i18n("You have to be connected to the server to change your status.") );
	}

	return 1;
}

int
GaduSession::changeStatusDescription( int status, const QString& descr, bool forFriends )
{
	QByteArray ndescr;

	ndescr= textcodec->fromUnicode(descr);

	if ( isConnected() ) {
		return gg_change_status_descr( session_,
				status | ( forFriends ? GG_STATUS_FRIENDS_MASK : 0), ndescr.data() );
	}
	else {
		emit error( i18n("Not Connected"), i18n("You have to be connected to the server to change your status.") );
	}

	return 1;
}

int
GaduSession::ping()
{
	if ( isConnected() ) {
		return gg_ping( session_ );
	}

	return 1;
}

void
GaduSession::pubDirSearchClose()
{
	searchSeqNr_=0;
}

unsigned int
GaduSession::getPersonalInformation()
{
	gg_pubdir50_t searchRequest;
	unsigned int seqNr;

	if ( isConnected() == false ) {
		return 0;
	}

	searchRequest = gg_pubdir50_new( GG_PUBDIR50_READ );
	if ( !searchRequest ) {
		return 0;
	}

	seqNr = gg_pubdir50( session_, searchRequest );
	gg_pubdir50_free( searchRequest );

	return seqNr;
}

bool
GaduSession::publishPersonalInformation( ResLine& d )
{
	gg_pubdir50_t r;
	
	if ( !session_ ) {
		return 0;
        }		
	
	r = gg_pubdir50_new( GG_PUBDIR50_WRITE );

	if ( d.firstname.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_FIRSTNAME, 
			 (const char *)((const char*)textcodec->fromUnicode( d.firstname ) ) );
        if ( d.surname.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_LASTNAME,
			(const char *)((const char*)textcodec->fromUnicode( d.surname ) ) );
	if ( d.nickname.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_NICKNAME,
			(const char *)((const char*)textcodec->fromUnicode( d.nickname ) ) );
	if ( d.age.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_BIRTHYEAR,
			(const char *)((const char*)textcodec->fromUnicode( d.age ) ) );
        if ( d.city.length() )
        	gg_pubdir50_add( r, GG_PUBDIR50_CITY,
			(const char *)((const char*)textcodec->fromUnicode( d.city ) ) );
	if ( d.meiden.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_FAMILYNAME,
			(const char *)((const char*)textcodec->fromUnicode( d.meiden ) ) );
        if ( d.orgin.length() )
		gg_pubdir50_add( r, GG_PUBDIR50_FAMILYCITY,
			(const char *)((const char*)textcodec->fromUnicode( d.orgin ) ) );
	if ( d.gender.length() == 1 )
		gg_pubdir50_add( r, GG_PUBDIR50_GENDER, 
			(const char *)((const char*)textcodec->fromUnicode( d.gender ) ) );
        
	gg_pubdir50( session_, r );
        
	gg_pubdir50_free( r );

	return true;
}

unsigned int
GaduSession::pubDirSearch(  ResLine& query, int ageFrom, int ageTo, bool onlyAlive )
{
	QString bufYear;
	unsigned int reqNr;
	gg_pubdir50_t searchRequest;

	if ( !session_ ) {
		return 0;
	}

	searchRequest = gg_pubdir50_new( GG_PUBDIR50_SEARCH_REQUEST );
	if ( !searchRequest ) {
		return 0;
	}

	if ( query.uin == 0 ) {
		if (query.firstname.length()) {
			gg_pubdir50_add( searchRequest, GG_PUBDIR50_FIRSTNAME,
						(const char*)textcodec->fromUnicode( query.firstname ) );
		}
		if ( query.surname.length() ) {
			gg_pubdir50_add( searchRequest, GG_PUBDIR50_LASTNAME,
						(const char*)textcodec->fromUnicode( query.surname ) );
		}
		if ( query.nickname.length() ) {
			gg_pubdir50_add( searchRequest, GG_PUBDIR50_NICKNAME,
						(const char*)textcodec->fromUnicode( query.nickname ) );
		}
		if ( query.city.length() ) {
			gg_pubdir50_add( searchRequest, GG_PUBDIR50_CITY,
						(const char*)textcodec->fromUnicode( query.city ) );
		}
		if ( ageFrom || ageTo ) {
			QString yearFrom = QString::number( QDate::currentDate().year() - ageFrom );
			QString yearTo = QString::number( QDate::currentDate().year() - ageTo );

			if ( ageFrom && ageTo ) {
				gg_pubdir50_add( searchRequest, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearFrom + ' ' + yearTo ) );
			}
			if ( ageFrom ) {
				gg_pubdir50_add( searchRequest, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearFrom ) );
			}
			else {
				gg_pubdir50_add( searchRequest, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearTo ) );
			}
		}

		if ( query.gender.length() == 1 ) {
                	gg_pubdir50_add( searchRequest, GG_PUBDIR50_GENDER,
				(const char *)((const char*)textcodec->fromUnicode( query.gender ) ) );
		}

		if ( onlyAlive ) {
			gg_pubdir50_add( searchRequest, GG_PUBDIR50_ACTIVE, GG_PUBDIR50_ACTIVE_TRUE );
		}
	}
	// otherwise we are looking only for one fellow with this nice UIN
	else{
		gg_pubdir50_add( searchRequest, GG_PUBDIR50_UIN, QString::number( query.uin ).toAscii() );
	}

	gg_pubdir50_add( searchRequest, GG_PUBDIR50_START, QString::number( searchSeqNr_ ).toAscii() );
	reqNr = gg_pubdir50( session_, searchRequest );
	gg_pubdir50_free( searchRequest );

	return reqNr;
}

void
GaduSession::sendResult( gg_pubdir50_t result )
{
	int i, count, age;
	ResLine resultLine;
	SearchResult sres;

	count = gg_pubdir50_count( result );

	if ( !count ) {
		kDebug(14100) << "there was nothing found in public directory for requested details";
	}

	for ( i = 0; i < count; i++ ) {
		resultLine.uin		= QString( gg_pubdir50_get( result, i, GG_PUBDIR50_UIN ) ).toInt();
		resultLine.firstname	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_FIRSTNAME ) );
		resultLine.surname	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_LASTNAME ) );
		resultLine.nickname	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_NICKNAME ) );
		resultLine.age		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_BIRTHYEAR ) );
		resultLine.city		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_CITY ) );
		QString stat		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_STATUS ) );
		resultLine.orgin	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_FAMILYCITY ) );
		resultLine.meiden	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_FAMILYNAME ) );
		resultLine.gender	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_GENDER ) );

		resultLine.status	= stat.toInt();
		age = resultLine.age.toInt();
		if ( age ) {
			resultLine.age = QString::number( QDate::currentDate().year() - age );
		}
		else {
			resultLine.age.truncate( 0 );
		}
		sres.append( resultLine );
		kDebug(14100) << "found line "<< resultLine.uin << ' ' << resultLine.firstname;
	}

	searchSeqNr_ = gg_pubdir50_next( result );
	emit pubDirSearchResult( sres, gg_pubdir50_seq( result ) );
}

void
GaduSession::requestContacts()
{
	if ( !session_ || session_->state != GG_STATE_CONNECTED ) {
		kDebug(14100) <<" you need to be connected to send ";
		return;
	}

	if ( gg_userlist_request( session_, GG_USERLIST_GET, NULL ) == -1 ) {
		kDebug(14100) <<" userlist export ERROR ";
		return;
	}
	kDebug( 14100 ) << "Contacts list import..started ";
}

void
GaduSession::exportContactsOnServer( GaduContactsList* contactsList )
{
	QByteArray plist;

	if ( !session_ || session_->state != GG_STATE_CONNECTED ) {
		kDebug( 14100 ) << "you need to connect to export Contacts list ";
		return;
	}
	if ( deletingUserList) {
		kDebug( 14100 ) << "you are currently deleting list ";
		return;
	}
	plist = textcodec->fromUnicode( contactsList->asString() );
	kDebug(14100) <<"--------------------userlists\n" << plist;
	kDebug(14100) << "----------------------------";
	if ( gg_userlist_request( session_, GG_USERLIST_PUT, plist.data() ) == -1 ) {
		kDebug( 14100 ) << "export contact list failed ";
		return;
	}
	kDebug( 14100 ) << "Contacts list export..started ";
}


void
GaduSession::deleteContactsOnServer( )
{

	if ( !session_ || session_->state != GG_STATE_CONNECTED ) {
		kDebug( 14100 ) << "you need to connect to delete Contacts list ";
		return;
	}

	if ( gg_userlist_request( session_, GG_USERLIST_PUT, " " ) == -1 ) {
		kDebug( 14100 ) << "delete contact list failed ";
		return;
	}
	deletingUserList=true;
	kDebug( 14100 ) << "Contacts list delete... started ";
}


void
GaduSession::handleUserlist( gg_event* event )
{
	QString ul;
	switch( event->event.userlist.type ) {
		case GG_USERLIST_GET_REPLY:
			if ( event->event.userlist.reply ) {
				ul = textcodec->toUnicode(event->event.userlist.reply);
				kDebug( 14100 ) << "Got Contacts list  OK ";
			}
			else {
				kDebug( 14100 ) << "Got Contacts list  FAILED/EMPTY ";
				// FIXME: send failed?
			}
			emit userListRecieved( ul );
			break;

		case GG_USERLIST_PUT_REPLY:
			if ( deletingUserList ) {
				deletingUserList = false;
				kDebug( 14100 ) << "Contacts list deleted  OK ";
				emit userListDeleted();
			}
			else {
				kDebug( 14100 ) << "Contacts list exported  OK ";
				emit userListExported();
			}
			break;

	}
}

QString
GaduSession::stateDescription( int state )
{
	switch( state ) {
		case GG_STATE_IDLE:
			return i18n( "idle" );
		case GG_STATE_RESOLVING:
			return i18n( "resolving host" );
		case GG_STATE_CONNECTING:
			return i18n( "connecting" );
		case GG_STATE_READING_DATA:
			return i18n( "reading data" );
		case GG_STATE_ERROR:
			return i18n( "error" );
		case GG_STATE_CONNECTING_HUB:
			return i18n( "connecting to hub" );
		case GG_STATE_CONNECTING_GG:
			return i18n( "connecting to server" );
		case GG_STATE_READING_KEY:
			return i18n( "retrieving key" );
		case GG_STATE_READING_REPLY:
			return i18n( "waiting for reply" );
		case GG_STATE_CONNECTED:
			return i18n( "connected" );
		case GG_STATE_SENDING_QUERY:
			return i18n( "sending query" );
		case GG_STATE_READING_HEADER:
			return i18n( "reading header" );
		case GG_STATE_PARSING:
			return i18n( "parsing data" );
		case GG_STATE_DONE:
			return i18n( "done" );
		case GG_STATE_TLS_NEGOTIATION:
			return i18n( "TLS connection negotiation" );
		default:
			return i18n( "unknown" );
	}
}
QString
GaduSession::errorDescription( int err )
{
	switch( err ){
		case GG_ERROR_RESOLVING:
			return i18n( "Resolving error." );
		case GG_ERROR_CONNECTING:
			return i18n( "Connecting error." );
		case GG_ERROR_READING:
			return i18n( "Reading error." );
		case GG_ERROR_WRITING:
			return i18n( "Writing error." );
		default:
			return i18n( "Unknown error number %1." , err );
	}
}

QString
GaduSession::failureDescription( gg_failure_t f )
{
	switch( f ) {
		case GG_FAILURE_RESOLVING:
			return i18n( "Unable to resolve server address. DNS failure." );
		case GG_FAILURE_CONNECTING:
			return i18n( "Unable to connect to server." );
		case GG_FAILURE_INVALID:
			return i18n( "Server sent incorrect data. Protocol error." );
		case GG_FAILURE_READING:
			return i18n( "Problem reading data from server." );
		case GG_FAILURE_WRITING:
			return i18n( "Problem sending data to server." );
		case GG_FAILURE_PASSWORD:
			return i18n( "Incorrect password." );
		case GG_FAILURE_404:
			return QString::fromAscii( "404." );
		case GG_FAILURE_TLS:
			return i18n( "Unable to connect over an encrypted channel.\nTry to turn off encryption support in the Gadu account settings, then reconnect." );
		default:
			return i18n( "Unknown error number %1." , f );
	}
}

void
GaduSession::notify60( gg_event* event )
{
	KGaduNotify* gn = NULL;
	unsigned int n;
	
	if ( event->event.notify60[0].uin ) {
		gn = new KGaduNotify;
	}
	else {
		return;
	}

	for( n=0 ; event->event.notify60[n].uin ; n++ ) {
		gn->contact_id	= event->event.notify60[n].uin;
		gn->status	= event->event.notify60[n].status;
		gn->remote_ip.setAddress( ntohl( event->event.notify60[n].remote_ip ) );
		gn->remote_port	= event->event.notify60[n].remote_port;
		if ( event->event.notify60[n].remote_ip && gn->remote_port > 10 ) {
			gn->fileCap = true;
		}
		else {
			gn->fileCap = false;
		}
		gn->version	= event->event.notify60[n].version;
		gn->image_size	= event->event.notify60[n].image_size;
		gn->description	= textcodec->toUnicode( event->event.notify60[n].descr );
		emit contactStatusChanged( gn );	
	}
	delete gn;
}

void
GaduSession::checkDescriptor()
{
	disableNotifiers();

	struct gg_event* event;
//	struct gg_dcc*   dccSock;
	KGaduMessage	gaduMessage;
	KGaduNotify	gaduNotify;

	if ( !( event = gg_watch_fd( session_ ) ) ) {
		kDebug(14100)<<"Connection was broken for some reason";
		destroyNotifiers();
		logoff( Kopete::Account::ConnectionReset );
		return;
	}

	// FD changed, recreate socket notifiers
	if ( session_->state == GG_STATE_CONNECTING_HUB || session_->state == GG_STATE_CONNECTING_GG ) {
		kDebug(14100)<<"recreating notifiers";
		destroyNotifiers();
		createNotifiers( true );
	}

	switch( event->type ) {
		case GG_EVENT_MSG:
			kDebug(14100) << "incoming message:class:" << event->event.msg.msgclass;
			if ( event->event.msg.msgclass & GG_CLASS_CTCP ) {
				kDebug( 14100 ) << "incoming ctcp ";
				// TODO: DCC CONNECTION
				emit incomingCtcp( event->event.msg.sender );
			}

			if ( (event->event.msg.msgclass & GG_CLASS_MSG) || (event->event.msg.msgclass & GG_CLASS_CHAT) ) {
				gaduMessage.message =
					textcodec->toUnicode((const char*)event->event.msg.message);
				gaduMessage.sender_id = event->event.msg.sender;
				gaduMessage.sendTime.setTime_t( event->event.msg.time );
				gaduMessage.message = rtf->convertToHtml( gaduMessage.message, event->event.msg.formats_length, event->event.msg.formats );
				emit messageReceived( &gaduMessage );
			}
		break;
		case GG_EVENT_ACK:
			emit ackReceived( event->event.ack.recipient );
		break;
		case GG_EVENT_STATUS:
			gaduNotify.status = event->event.status.status;
			gaduNotify.contact_id = event->event.status.uin;
			if ( event->event.status.descr ) {
				gaduNotify.description = textcodec->toUnicode( event->event.status.descr );
			}
			else {
				gaduNotify.description.clear();
			}
			gaduNotify.remote_port	= 0;
			gaduNotify.version	= 0;
			gaduNotify.image_size	= 0;
			gaduNotify.time		= 0;
			gaduNotify.fileCap	= false;

			emit contactStatusChanged( &gaduNotify );
		break;
		case GG_EVENT_STATUS60:
			gaduNotify.status	= event->event.status60.status;
			gaduNotify.contact_id	= event->event.status60.uin;
			if ( event->event.status60.descr ) {
				gaduNotify.description = textcodec->toUnicode( event->event.status60.descr );
			}
			else {
				gaduNotify.description.clear();
			}
			gaduNotify.remote_ip.setAddress( ntohl( event->event.status60.remote_ip ) );
			gaduNotify.remote_port	= event->event.status60.remote_port;
			gaduNotify.version	= event->event.status60.version;
			gaduNotify.image_size	= event->event.status60.image_size;
			gaduNotify.time		= event->event.status60.time;
			if ( event->event.status60.remote_ip && gaduNotify.remote_port > 10 ) {
				gaduNotify.fileCap = true;
			}
			else {
				gaduNotify.fileCap = false;
			}

			emit contactStatusChanged( &gaduNotify );
		break;
		case GG_EVENT_NOTIFY60:
			notify60( event );
		break;
		case GG_EVENT_CONN_SUCCESS:
			kDebug(14100) << "success server: " << session_->server_addr;
			emit connectionSucceed();
		break;
		case GG_EVENT_CONN_FAILED:
			kDebug(14100) << "failed server: " << session_->server_addr;
			destroySession();
			kDebug(14100) << "emit connection failed(" << event->event.failure << ") signal";
			emit connectionFailed( (gg_failure_t)event->event.failure );
			break;
		case GG_EVENT_DISCONNECT:
			kDebug(14100)<<"event Disconnected";
			// it should be called either when we requested disconnect, or when other client connects with our UID
			logoff( Kopete::Account::Manual );
		break;
		case GG_EVENT_PONG:
			emit pong();
		break;
		case GG_EVENT_NONE:
			break;
		case GG_EVENT_PUBDIR50_SEARCH_REPLY:
		case GG_EVENT_PUBDIR50_WRITE:
		case GG_EVENT_PUBDIR50_READ:
			sendResult( event->event.pubdir50 );
	        break;
		case GG_EVENT_USERLIST:
			handleUserlist( event );
		break;
		default:
			kDebug(14100)<<"Unprocessed GaduGadu Event = "<<event->type;
		break;
	}

	if ( event ) {
		gg_free_event( event );
	}

	if ( session_ ) {
		enableNotifiers( session_->check );
	}
}

#include "gadusession.moc"
