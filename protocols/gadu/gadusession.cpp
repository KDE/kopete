// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 	2003	 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
// Copyright (C) 	2002	 Zack Rusin <zack@kde.org>
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#include "ctime"

#include "gadusession.h"

#include <klocale.h>
#include <kdebug.h>

#include <qsocketnotifier.h>
#include <qtextcodec.h>
#include <qdatetime.h>

#include <errno.h>
#include <string.h>

GaduSession::GaduSession( QObject* parent, const char* name )
: QObject( parent, name ), session_( 0 ), searchSeqNr_( 0 )
{
	textcodec = QTextCodec::codecForName( "CP1250" );
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
	kdDebug(14100)<<"Status = " << session_->status <<", initial = "<< session_->initial_status <<endl;
	if ( session_ ) {
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
//		gg_debug_level=GG_DEBUG_MISC;

		kdDebug(14100) << "Login" << endl;

		if ( !( session_ = gg_login( p ) ) ) {
			destroySession();
			kdDebug( 14100 ) << "libgadu internal error " << endl;
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
	if ( read_ ) {
		delete read_;
		read_ = NULL;
	}
	if ( write_ ) {
		delete write_;
		write_ = NULL;
	}
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
		QObject::connect( read_, SIGNAL( activated( int ) ), SLOT( checkDescriptor() ) );
		QObject::connect( write_, SIGNAL( activated( int ) ), SLOT( checkDescriptor() ) );
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
GaduSession::login( uin_t uin, const QString& password, bool useTls,
					int status, const QString& statusDescr, unsigned int server, bool forFriends )
{
	memset( &params_, 0, sizeof(params_) );

	params_.uin		= uin;
	params_.password	= (char *)password.ascii();
	params_.status		= status | ( forFriends ? GG_STATUS_FRIENDS_MASK : 0);
	params_.status_descr	= ( textcodec->fromUnicode( statusDescr ).data() );
	params_.async		= 1;
	params_.tls		= useTls;
	params_ .server_addr	= server;

	if ( useTls ) {
		params_.server_port = GG_HTTPS_PORT;
	}
	else {
		if ( server ) {
			params_.server_port = GG_DEFAULT_PORT;
		}
	}

	kdDebug(14100)<<"gadusession::login, server ( " << server << " ), tls(" << useTls << ") " <<endl;
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
GaduSession::logoff()
{
	destroySession();
	emit disconnect();
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
GaduSession::sendMessage( uin_t recipient, const QString& msg, int msgClass )
{
	QString sendMsg;
	QCString cpMsg;

	if ( isConnected() ) {
		sendMsg = msg;
		sendMsg.replace( QString::fromAscii( "\n" ), QString::fromAscii( "\r\n" ) );
		cpMsg = textcodec->fromUnicode( sendMsg );

		return gg_send_message( session_, msgClass, recipient, (const unsigned char *)cpMsg.data() );
	}
	else {
		emit error( i18n("Not Connected"), i18n("You are not connected to the server.") );
	}

	return 1;
}

int
GaduSession::changeStatus( int status, bool forFriends )
{
	kdDebug()<<"## Changing to "<<status<<endl;
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
	QCString ndescr;

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

bool
GaduSession::pubDirSearch(QString& name, QString& surname, QString& nick, int UIN, QString& city,
                            int gender, int ageFrom, int ageTo, bool onlyAlive)
{
	QString bufYear;
	gg_pubdir50_t searchRequest_;

	if ( !session_ ) {
		return false;
	}

	searchRequest_ = gg_pubdir50_new( GG_PUBDIR50_SEARCH_REQUEST );
	if ( !searchRequest_ ) {
		return false;
	}

	if ( !UIN ) {
		if (name.length()) {
			gg_pubdir50_add( searchRequest_, GG_PUBDIR50_FIRSTNAME,
						(const char*)textcodec->fromUnicode( name ) );
		}
		if ( surname.length() ) {
			gg_pubdir50_add( searchRequest_, GG_PUBDIR50_LASTNAME,
						(const char*)textcodec->fromUnicode( surname ) );
		}
		if ( nick.length() ) {
			gg_pubdir50_add( searchRequest_, GG_PUBDIR50_NICKNAME,
						(const char*)textcodec->fromUnicode( nick ) );
		}
		if ( city.length() ) {
			gg_pubdir50_add( searchRequest_, GG_PUBDIR50_CITY,
						(const char*)textcodec->fromUnicode( city ) );
		}
		if ( ageFrom || ageTo ) {
			QString yearFrom = QString::number( QDate::currentDate().year() - ageFrom );
			QString yearTo = QString::number( QDate::currentDate().year() - ageTo );

			if ( ageFrom && ageTo ) {
				gg_pubdir50_add( searchRequest_, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearFrom + " " + yearTo ) );
			}
			if ( ageFrom ) {
				gg_pubdir50_add( searchRequest_, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearFrom ) );
			}
			else {
				gg_pubdir50_add( searchRequest_, GG_PUBDIR50_BIRTHYEAR,
							(const char*)textcodec->fromUnicode( yearTo ) );
			}
		}

		switch ( gender ) {
			case 1:
				gg_pubdir50_add( searchRequest_, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_MALE );
			break;
			case 2:
				gg_pubdir50_add( searchRequest_, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_FEMALE );
			break;
		}

		if ( onlyAlive ) {
			gg_pubdir50_add( searchRequest_, GG_PUBDIR50_ACTIVE, GG_PUBDIR50_ACTIVE_TRUE );
		}
	}
	// otherwise we are looking only for one fellow with this nice UIN
	else{
		gg_pubdir50_add( searchRequest_, GG_PUBDIR50_UIN, QString::number( UIN ).ascii() );
	}

	gg_pubdir50_add( searchRequest_, GG_PUBDIR50_START, QString::number(searchSeqNr_).ascii() );

	gg_pubdir50( session_, searchRequest_ );

	gg_pubdir50_free( searchRequest_ );

	return true;
}

void
GaduSession::sendResult( gg_pubdir50_t result )
{
	int i, count, age;
	ResLine resultLine;
	SearchResult sres;

	count = gg_pubdir50_count( result );

	for ( i = 0; i < count; i++ ) {

		resultLine.uin		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_UIN ) );
		resultLine.firstname	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_FIRSTNAME ));
		resultLine.surname	= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_LASTNAME ));
		resultLine.nickname	= textcodec->toUnicode( gg_pubdir50_get(result, i, GG_PUBDIR50_NICKNAME ) );
		resultLine.age		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_BIRTHYEAR ) );
		resultLine.city		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_CITY ) );
		QString stat		= textcodec->toUnicode( gg_pubdir50_get( result, i, GG_PUBDIR50_STATUS ) );
		resultLine.status	= stat.toInt();
		age = resultLine.age.toInt();
		if ( age ) {
			resultLine.age = QString::number( QDate::currentDate().year() - age );
		}
		else {
			resultLine.age.truncate( 0 );
		}
		sres.append( resultLine );
		kdDebug(14100) << "found line "<< resultLine.uin << " " << resultLine.firstname << endl;
	}
	
	searchSeqNr_ = gg_pubdir50_next( result );

	emit pubDirSearchResult( sres );
}

void
GaduSession::requestContacts()
{
	if ( !session_ || session_->state != GG_STATE_CONNECTED ) {
		kdDebug(14100) <<" you need to be connected to send " << endl;
		return;
	}

	if ( gg_userlist_request( session_, GG_USERLIST_GET, NULL ) == -1 ) {
		kdDebug(14100) <<" userlist export ERROR " << endl;
		return;
	}
	kdDebug( 14100 ) << "Contacts list import..started " << endl;
}

QString
GaduSession::contactsToString( gaduContactsList* contactsList )
{
	QPtrListIterator<contactLine>contactsListIterator( *contactsList );
	unsigned int i;
	QString contacts;

	for ( i = contactsList->count() ; i-- ; ++contactsListIterator ) {
		if ( (*contactsListIterator)->ignored ) {
			contacts +=
				"i;;;;;;" + (*contactsListIterator)->uin+
				"\n";
		}
		else {
//	name;surname;nick;displayname;telephone;group(s);uin;email;0;;0;
			contacts +=
				(*contactsListIterator)->firstname +";"+
				(*contactsListIterator)->surname+";"+
				(*contactsListIterator)->nickname+";"+
				(*contactsListIterator)->displayname+";"+
				(*contactsListIterator)->phonenr+";"+
				(*contactsListIterator)->group+";"+
				(*contactsListIterator)->uin+";"+
				(*contactsListIterator)->email+
				";0;;0;\n";
		}
	}

	return contacts;
}

void
GaduSession::exportContactsOnServer( gaduContactsList* contactsList )
{
	QCString plist;

	if ( !session_ || session_->state != GG_STATE_CONNECTED ) {
		kdDebug( 14100 ) << "you need to connect to export Contacts list " << endl;
		return;
	}


	plist = textcodec->fromUnicode( contactsToString( contactsList ) );
	kdDebug(14100) <<"--------------------userlists\n" << plist << endl;
	kdDebug(14100) << "----------------------------" << endl;

	if ( gg_userlist_request( session_, GG_USERLIST_PUT, plist.data() ) == -1 ) {
		kdDebug( 14100 ) << "export contact list failed " << endl;
		return;
	}
	kdDebug( 14100 ) << "Contacts list export..started " << endl;
}


bool
GaduSession::stringToContacts( gaduContactsList& gaducontactsList , const QString& sList )
{
	QStringList::iterator stringIterator;
	QStringList strList ;
	contactLine* cl = NULL;
	bool email;

	if ( sList.isEmpty() || sList.isNull() ) {
		return false;
	}

	if ( ( !sList.contains( '\n' ) && sList.contains( ';' ) )  || !sList.contains( ';' ) ) {
		// basicaly, server stores contact list as it is
		// even if i will send him windows 2000 readme file
		// he will accept it, and he will return it on each contact import
		// so, if you have any problems with contact list
		// this is probably not my fault, only previous client issue
		// iow: i am not bothered :-)
		kdDebug(14100)<<"you have to retype your contacts, and export list again"<<endl;
		kdDebug(14100)<<"send this line to author, if you think it should work"<<endl;
		kdDebug(14100)<<"------------------------------------------------------"<<endl;
		kdDebug(14100)<<"\"" << sList << "\""<<endl;
		kdDebug(14100)<<"------------------------------------------------------"<<endl;
		return false;
	}

	QStringList ln  = QStringList::split( QChar( '\n' ),  sList, true );
	QStringList::iterator lni = ln.begin( );

	while( lni != ln.end() ) {
		QString cline = (*lni);
		if ( cline.isNull() ) {
			break;
		}
		kdDebug(14100)<<"\""<< cline << "\"" << endl;

		strList  = QStringList::split( QChar( ';' ), cline, true );

		stringIterator = strList.begin();

		if ( cl == NULL ) {
			cl = new contactLine;
		}

		// ignored contact
		if ( strList.count() == 7 ) {
			 // well, this is probably it - you're d00med d\/de :-)
			if ( (*stringIterator) == QString( "i" ) ) {
				cl->ignored	= true;
				cl->uin		= strList[6];
				kdDebug(14100) << " ignored :\"" << cl->uin << "\"" << endl;
				++lni;
				continue;
			}
			else {
				kdDebug(14100) << "This line of contacts is incorrect, send it to me if you think it is ok:" << strList.count() << " LINE:\"" << cline << "\"" << endl;
				++lni;
				continue;
			}
		}

		if ( strList.count() == 12 ) {
			email = true;
		}
		else {
			email = false;
			if ( strList.count() != 8 ) {
				kdDebug(14100) << "This line of contacts is incorrect, send it to me if you think it is ok:" <<strList.count() << " LINE:\"" << cline << "\"" << endl;
				++lni;
				continue;
			}
		}


//each line ((firstname);(secondname);(nickname);(displayname);(tel);(group);(uin);

		stringIterator = strList.begin();

		if ( cl == NULL ) {
			cl = new contactLine;
		}

		cl->ignored		= false;
		cl->firstname		= (*stringIterator);
		cl->surname		= (*++stringIterator);
		cl->nickname		= (*++stringIterator);
		cl->displayname		= (*++stringIterator);
		cl->phonenr		= (*++stringIterator);
		cl->group		= (*++stringIterator);
		cl->uin			= (*++stringIterator);
		if ( email ) {
			cl->email	= (*++stringIterator);
        	}
		else {
			 cl->email	= "";
		}

		++lni;

		if ( cl->uin.isNull() ) {
			kdDebug(14100) << "no Uin, strange "<<endl;
			kdDebug(14100) << "LINE:" << cline <<endl;
			// FIXME: maybe i should consider this as an fatal error, and return false
			continue;
		}

		gaducontactsList.append( cl );
		kdDebug(14100) << "adding to list ,uin:" << cl->uin <<endl;

		cl = NULL;
	}

	// happends only when last contact was without UIN, and was at the end of list...
	// rather rare, but don't leak please :-)
	delete cl;

	return true;
}

void
GaduSession::handleUserlist( gg_event* event )
{
	QString ul;
	switch( event->event.userlist.type ) {
		case GG_USERLIST_GET_REPLY:
			if ( event->event.userlist.reply ) {
				ul = event->event.userlist.reply;
				kdDebug( 14100 ) << "Got Contacts list  OK " << endl;
			}
			else {
				kdDebug( 14100 ) << "Got Contacts list  FAILED/EMPTY " << endl;
				// FIXME: send failed?
			}
			emit userListRecieved( ul );
			break;

		case GG_USERLIST_PUT_REPLY:
			kdDebug( 14100 ) << "Contacts list exported  OK " << endl;
			emit userListExported();
			break;

	}
}

QString
GaduSession::stateDescription( int state )
{
	switch( state ) {
		case GG_STATE_IDLE:
			return i18n( "idle" );
			break;
		case GG_STATE_RESOLVING:
			return i18n( "resolving host" );
			break;
		case GG_STATE_CONNECTING:
			return i18n( "connecting" );
			break;
		case GG_STATE_READING_DATA:
			return i18n( "reading data" );
			break;
		case GG_STATE_ERROR:
			return i18n( "error" );
			break;
		case GG_STATE_CONNECTING_HUB:
			return i18n( "connecting to hub" );
			break;
		case GG_STATE_CONNECTING_GG:
			return i18n( "connecting to server" );
			break;
		case GG_STATE_READING_KEY:
			return i18n( "retriving key" );
			break;
		case GG_STATE_READING_REPLY:
			return i18n( "waiting for reply" );
			break;
		case GG_STATE_CONNECTED:
			return i18n( "connected" );
			break;
		case GG_STATE_SENDING_QUERY:
			return i18n( "sending query" );
			break;
		case GG_STATE_READING_HEADER:
			return i18n( "reading header" );
			break;
		case GG_STATE_PARSING:
			return i18n( "parse data" );
			break;
		case GG_STATE_DONE:
			return i18n( "done" );
			break;
		case GG_STATE_TLS_NEGOTIATION:
			return i18n( "Tls connection negotiation" );
			break;
	}
	return i18n( "unknown" );
}
QString
GaduSession::errorDescription( int err )
{
	switch( err ){
		case GG_ERROR_RESOLVING:
			return i18n( "Resolving error." );
		break;
		case GG_ERROR_CONNECTING:
			return i18n( "Connecting error." );
		break;
		case GG_ERROR_READING:
			return i18n( "Reading error." );
		break;
		case GG_ERROR_WRITING:
			return i18n( "Writing error." );
		break;
	}
	return i18n( "Unknown error number %1." ).arg( QString::number( (unsigned int)err ) );
}

QString
GaduSession::failureDescription( gg_failure_t f )
{
	switch( f ) {
		case GG_FAILURE_RESOLVING:
			return i18n( "Unable to resolve server address. DNS failure." );
			break;
		case GG_FAILURE_CONNECTING:
			return i18n( "Unable to connect to server." );
			break;
		case GG_FAILURE_INVALID:
			return i18n( "Server send incorrect data. Protocol error." );
			break;
		case GG_FAILURE_READING:
			return i18n( "Problem reading data from server." );
			break;
		case GG_FAILURE_WRITING:
			return i18n( "Problem sending data to server." );
			break;
		case GG_FAILURE_PASSWORD:
			return i18n( "Incorrect password." );
			break;
		case GG_FAILURE_404:
			return QString::fromAscii( "404." );
			break;
		case GG_FAILURE_TLS:
			return i18n( "Unable to connect over encrypted channel.\nTry to turn off encryption support in Gadu account settings and reconnect." );
			break;
	}

	return i18n( "Unknown error number %1." ).arg( QString::number( (unsigned int)f ) );
}

void
GaduSession::notify60( gg_event* event )
{
	KGaduNotifyList nl;
	KGaduNotify* gn;
	unsigned int n;
	nl.setAutoDelete( TRUE );


	for( n=0 ; event->event.notify60[n].uin ; n++ ) {
		gn = new KGaduNotify;
		gn->contact_id	= event->event.notify60[n].uin;
		gn->status	= event->event.notify60[n].status;
		gn->remote_ip	= event->event.notify60[n].remote_ip;
		gn->remote_port	= event->event.notify60[n].remote_port;
		gn->version	= event->event.notify60[n].version;
		gn->image_size	= event->event.notify60[n].image_size;
		gn->description	= textcodec->toUnicode( event->event.notify60[n].descr );
		nl.append( gn );
	}
	if ( n ) {
		emit notify( &nl );
	}
}

void
GaduSession::checkDescriptor()
{
	disableNotifiers();

	struct gg_event* event;
	KGaduMessage	gaduMessage;
	KGaduNotify	gaduNotify;

	if ( !( event = gg_watch_fd( session_ ) ) ) {
		kdDebug(14100)<<"Connection was broken for some reason"<<endl;
		destroyNotifiers();
		logoff();
		return;
	}

	// FD changed, recreate socket notifiers
	if ( session_->state == GG_STATE_CONNECTING_HUB || session_->state == GG_STATE_CONNECTING_GG ) {
		kdDebug(14100)<<"recreating notifiers"<<endl;
		destroyNotifiers();
		createNotifiers( true );
	}

	switch( event->type ) {
		case GG_EVENT_MSG:
			if ( event->event.msg.msgclass == GG_CLASS_CTCP ) {
				// TODO: DCC CONNECTION
			}
			else {
				gaduMessage.message =
					textcodec->toUnicode((const char*)event->event.msg.message);
				gaduMessage.sender_id = event->event.msg.sender;
				gaduMessage.sendTime.setTime_t( event->event.msg.time, Qt::LocalTime );
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
				gaduNotify.description = QString::null;
			}
			gaduNotify.remote_ip	= 0;
			gaduNotify.remote_port	= 0;
			gaduNotify.version	= 0;
			gaduNotify.image_size	= 0;
			gaduNotify.time		= 0;

			emit contactStatusChanged( &gaduNotify );
		break;
		case GG_EVENT_STATUS60:
			gaduNotify.status	= event->event.status60.status;
			gaduNotify.contact_id	= event->event.status60.uin;
			if ( event->event.status60.descr ) {
				gaduNotify.description = textcodec->toUnicode( event->event.status60.descr );
			}
			else {
				gaduNotify.description = QString::null;
			}
			gaduNotify.remote_ip	= event->event.status60.remote_ip;
			gaduNotify.remote_port	= event->event.status60.remote_port;
			gaduNotify.version	= event->event.status60.version;
			gaduNotify.image_size	= event->event.status60.image_size;
			gaduNotify.time		= event->event.status60.time;

			emit contactStatusChanged( &gaduNotify );
		break;
		case GG_EVENT_NOTIFY60:
			notify60( event );
		break;
		case GG_EVENT_CONN_SUCCESS:
			emit connectionSucceed();
		break;
		case GG_EVENT_CONN_FAILED:
			destroySession();
			kdDebug(14100) << "emit connection failed(" << event->event.failure << ") signal" << endl;
			emit connectionFailed( (gg_failure_t)event->event.failure );
			break;
		case GG_EVENT_DISCONNECT:
			kdDebug(14100)<<"event Disconnected"<<endl;
			logoff();
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
			kdDebug(14100)<<"Unprocessed GaduGadu Event = "<<event->type<<endl;
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
