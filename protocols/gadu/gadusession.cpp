#include <klocale.h>

#include <qsocketnotifier.h>
#include <qobject.h>
#include <kdebug.h>
#include <errno.h>

#include "gadusession.h"


GaduSession::GaduSession( QObject *parent, const char* name )
    : QObject( parent, name )
{
    session_ = 0;
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

void
GaduSession::login( const struct gg_login_params& p )
{
    if ( !isConnected() ) {
	if ( !(session_ = gg_login( &p ))) {
            emit connectionFailed( 0L );
            gg_free_session( session_ );
            session_ = 0;
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
	if( session_->check & GG_CHECK_READ ) {
            read_->setEnabled( true );
	}
	if( session_->check & GG_CHECK_WRITE ) {
            write_->setEnabled( true );
	}
    }
}

void
GaduSession::enableNotifiers( int checkWhat )
{
    if( checkWhat & GG_CHECK_READ ) {
        read_->setEnabled( true );
    }
    if( checkWhat & GG_CHECK_WRITE ) {
        write_->setEnabled( true );
    }
}

void
GaduSession::disableNotifiers()
{
    read_->setEnabled( false );
    write_->setEnabled( false );
}

void
GaduSession::login( uin_t uin, const QString& password,
                    int status, const QString& statusDescr )
{
    struct gg_login_params p;

    memset( &p, 0, sizeof(p) );
    p.uin = uin;
    p.password = password.latin1();
    p.status = status;
    p.status_descr = statusDescr.latin1();
    p.async = 1;
    login( p );
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
                                msg.latin1() );
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
                                     msg.latin1(),
                                     msg.length() );
    else
        emit error( i18n("Not Connected..."),
                    i18n("You are not connected to the server!") );
    return 1;
}

int
GaduSession::changeStatus( int status )
{
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
        return gg_change_status_descr( session_, status, descr.latin1() );
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
            delete read_;
            delete write_;
            read_ = 0;
            write_ = 0;
            gg_free_session( session_ );
            session_ = 0L;
        }
        emit connectionFailed( e );
        break;
    case GG_EVENT_DISCONNECT:
        if ( session_ ) {
            delete read_;
            delete write_;
            read_ = 0;
            write_ = 0;
            gg_free_session( session_ );
            session_ = 0L;
        }
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
	kdDebug()<<"GaduGadu Event = "<<e->type<<endl;
        break;
    }

    if ( e ) gg_free_event( e );

    enableNotifiers( session_->check );
}

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
