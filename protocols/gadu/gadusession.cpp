#include "gadusession.h"

#include <klocale.h>
#include <kdebug.h>

#include <qsocketnotifier.h>

#include <netinet/in.h>
#include <errno.h>
#include <string.h>

const int NUM_SERVERS = 7;
const char* const gg_servers_ip[NUM_SERVERS] = {"217.17.41.82", "217.17.41.83",
                                                "217.17.41.84", "217.17.41.85",
                                                "217.17.41.86", "217.17.41.87",
                                                "217.17.41.88"};

GaduSession::GaduSession( QObject *parent, const char* name )
	: QObject( parent, name ), session_(0), currentServer_(-1)
{
  QHostAddress ip;
	for ( int i = 0; i < NUM_SERVERS; i++ ) {
		ip.setAddress( QString( gg_servers_ip[i] ) );
		servers_.append( ip );
  }
}

GaduSession::~GaduSession()
{
	if ( isConnected() ) {
		logoff();
	}
	if( session_ ) {
		gg_free_session( session_ );
		delete read_;
		delete write_;
		read_ = 0;
		write_ = 0;
		session_ = 0;
	}
}

bool
GaduSession::isConnected() const
{
	if ( session_ )
		return (session_->state & GG_STATE_CONNECTED);
	return false;
}

int
GaduSession::status() const
{
  kdDebug(14100)<<"Status = " << session_->status <<", initial = "<< session_->initial_status <<endl;
  if ( session_ )
    return session_->status;
  return GG_STATUS_NOT_AVAIL;
}

void
GaduSession::login( struct gg_login_params& p )
{
  kdDebug()<<"Login"<<endl;
	if ( !isConnected() ) {
    if ( currentServer_++ != -1 ) {
      p.server_addr = htonl( servers_[ currentServer_ ].ip4Addr() );
      p.server_port = 8074;
    }
		if ( !(session_ = gg_login( &p ))) {
			gg_free_session( session_ );
			session_ = 0;
      if ( currentServer_ == NUM_SERVERS )
        emit connectionFailed( 0L );
      else
        login( params_ );
			return;
		}
		read_ = new QSocketNotifier( session_->fd, QSocketNotifier::Read, this );
		read_->setEnabled( false );
		QObject::connect( read_, SIGNAL(activated(int)),
											SLOT(checkDescriptor()) );

		write_ = new QSocketNotifier( session_->fd, QSocketNotifier::Write, this );
		write_->setEnabled( false );
		QObject::connect( write_, SIGNAL(activated(int)),
											SLOT(checkDescriptor()) );

		enableNotifiers( session_->check );
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
	if ( read_ )
		read_->setEnabled( false );
	if ( write_ )
		write_->setEnabled( false );
}

void
GaduSession::login( uin_t uin, const QString& password,
                    int status, const QString& statusDescr )
{
  memset( &params_, 0, sizeof(params_) );
	params_.uin = uin;
	params_.password = const_cast<char*>( password.latin1() );
	params_.status = status;
	params_.status_descr = statusDescr.local8Bit().data();
  params_.client_version = GG_DEFAULT_CLIENT_VERSION;
	params_.async = 1;
	login( params_ );
}

void
GaduSession::logoff()
{
	if ( isConnected() ) {
		gg_logoff( session_ );
		QObject::disconnect( this, SLOT(checkDescriptor()) );
		delete read_;
		delete write_;
		read_ = 0;
		write_ = 0;
		gg_free_session( session_ );
		session_ = 0;
	}
}

int
GaduSession::notify( uin_t *userlist, int count )
{
	if ( isConnected() )
		return gg_notify( session_, userlist, count );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

int
GaduSession::addNotify( uin_t uin )
{
	if ( isConnected() )
		return gg_add_notify( session_, uin );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

int
GaduSession::removeNotify( uin_t uin )
{
	if ( isConnected() )
		gg_remove_notify( session_, uin );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

int
GaduSession::sendMessage( uin_t recipient, const QString& msg,
                          int msgClass )
{
	if ( isConnected() )
		return gg_send_message( session_,
														msgClass,
														recipient,
														reinterpret_cast<const unsigned char *>(msg.local8Bit().data()) );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

int
GaduSession::sendMessageCtcp( uin_t recipient, const QString& msg,
                              int msgClass )
{
	if ( isConnected() )
		return gg_send_message_ctcp( session_,
																 msgClass,
																 recipient,
																 reinterpret_cast<const unsigned char *>( msg.local8Bit().data() ),
																 msg.length() );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

int
GaduSession::changeStatus( int status )
{
  kdDebug()<<"## Changing to "<<status<<endl;
	if ( isConnected() )
		return gg_change_status( session_, status );
	else
		emit error( i18n("Not Connected..."),
								i18n("You have to be connected to the server to change your status!") );
	return 1;
}

int
GaduSession::changeStatusDescription( int status, const QString& descr )
{
	if ( isConnected() )
		return gg_change_status_descr( session_, status, descr.local8Bit() );
	else
		emit error( i18n("Not Connected..."),
								i18n("You have to be connected to the server to change your status!") );
	return 1;

}

int
GaduSession::ping()
{
	if ( isConnected() )
		return gg_ping( session_ );

	return 1;
}

int
GaduSession::dccRequest( uin_t uin )
{
	if ( isConnected() )
		return gg_dcc_request( session_, uin );
	else
		emit error( i18n("Not Connected..."),
								i18n("You are not connected to the server!") );
	return 1;
}

void
GaduSession::checkDescriptor()
{
	disableNotifiers();

	struct gg_event *e;

	if (!(e = gg_watch_fd(session_))) {
		emit error( i18n("Connection broken!"),
								i18n(strerror(errno)) );
		QObject::disconnect( this, SLOT(checkDescriptor()) );
		delete read_;
		delete write_;
		read_ = 0;
		write_ = 0;
		gg_free_session( session_ );
		emit disconnect();
		return;
	}

	switch( e->type ) {
	case GG_EVENT_MSG:
		emit messageReceived( e );
		break;
	case GG_EVENT_ACK:
		emit ackReceived( e );
		break;
	case GG_EVENT_STATUS:
		emit statusChanged( e );
		break;
	case GG_EVENT_NOTIFY:
		emit notify( e );
		break;
	case GG_EVENT_NOTIFY_DESCR:
		emit notifyDescription( e );
		break;
	case GG_EVENT_CONN_SUCCESS:
		emit connectionSucceed( e );
		break;
	case GG_EVENT_CONN_FAILED:
		if ( session_ ) {
			gg_free_session( session_ );
			session_ = 0L;
		}
		QObject::disconnect( this, SLOT(checkDescriptor()) );
		delete read_;
		delete write_;
		read_ = 0;
		write_ = 0;
    if ( currentServer_ == NUM_SERVERS )
      emit connectionFailed( e );
    else
      login( params_ );
		break;
	case GG_EVENT_DISCONNECT:
		if ( session_ ) {
			gg_free_session( session_ );
			session_ = 0L;
		}
		QObject::disconnect( this, SLOT(checkDescriptor()) );
		delete read_;
		delete write_;
		read_ = 0;
		write_ = 0;
		emit disconnect();
		break;
	case GG_EVENT_PONG:
		emit pong();
		break;
	case GG_EVENT_NONE:
		break;
	default:
		emit error( i18n("Unknown event..."),
								i18n("Can't handle an event. Please report this to zack@kde.org") );
		kdDebug(14100)<<"GaduGadu Event = "<<e->type<<endl;
		break;
	}

	if ( e ) gg_free_event( e );

	if ( session_ )
		enableNotifiers( session_->check );
}

#include "gadusession.moc"
