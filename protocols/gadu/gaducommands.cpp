#include "gaducommands.h"
#include <qsocketnotifier.h>
#include <klocale.h>
#include <qregexp.h>
#include <kdebug.h>
#include <errno.h>

GaduCommand::GaduCommand( QObject* parent, const char* name )
    : QObject( parent, name ), read_(0), write_(0)
{
}

GaduCommand::~GaduCommand()
{
    //QSocketNotifiers are children and will
    //be deleted anyhow
}

bool
GaduCommand::done() const
{
    return done_;
}

char*
GaduCommand::qstrToChar( const QString& str )
{
    return (str.isEmpty()?
            NULL:
            (char*)(str.latin1()));
}

void
GaduCommand::checkSocket( int fd, int checkWhat )
{
    read_ = new QSocketNotifier( fd, QSocketNotifier::Read, this );
    read_->setEnabled( false );
    QObject::connect( read_, SIGNAL(activated(int)),
                      SLOT(forwarder()) );

    write_ = new QSocketNotifier( fd, QSocketNotifier::Write, this );
    write_->setEnabled( false );
    QObject::connect( write_, SIGNAL(activated(int)),
                      SLOT(forwarder()) );

    if( checkWhat & GG_CHECK_READ ) {
        read_->setEnabled( true );
    }
    if( checkWhat & GG_CHECK_WRITE ) {
        write_->setEnabled( true );
    }
}

void
GaduCommand::enableNotifiers( int checkWhat )
{
    if( checkWhat & GG_CHECK_READ ) {
        read_->setEnabled( true );
    }
    if( checkWhat & GG_CHECK_WRITE ) {
        write_->setEnabled( true );
    }
}

void
GaduCommand::disableNotifiers()
{
    read_->setEnabled( false );
    write_->setEnabled( false );
}

void
GaduCommand::forwarder()
{
    emit socketReady();
}


SearchCommand::SearchCommand( QObject* parent, const char* name )
    : GaduCommand( parent, name ), request_(0), session_(0)
{
}

SearchCommand::~SearchCommand()
{
}

void
SearchCommand::searchMode0( const QString& nickname, const QString& firstName,
                            const QString& lastName, const QString& city,
                            int gender, int min_birth, int max_birth, int active, int start )
{
    request_ = gg_search_request_mode_0( qstrToChar(nickname),
                                         qstrToChar(firstName),
                                         qstrToChar(lastName),
                                         qstrToChar(city),
                                         gender, min_birth, max_birth, active, start );
}

void
SearchCommand::searchMode1( const QString& email, int active, int start )
{
    request_ = gg_search_request_mode_1( qstrToChar(email), active, start );
}

void
SearchCommand::searchMode2( const QString& phone, int active, int start )
{
    request_ = gg_search_request_mode_2( qstrToChar(phone), active, start );
}

void
SearchCommand::searchMode3( uin_t uin, int active, int start )
{
    request_ = gg_search_request_mode_3( uin, active, start );
}

void
SearchCommand::execute()
{
    session_ = gg_search( request_ , 1);

    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
SearchCommand::watcher()
{
    disableNotifiers();

    if ( gg_search_watch_fd( session_ ) == -1 ) {
        gg_free_search( session_ );
        emit error( i18n("Connection error"),
                    i18n("Unknown connection error") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_free_search( session_ );
        emit error( i18n("Searching error"),
                    i18n("There was an unknown searching error") );
        switch( session_->error )
        {
        case GG_ERROR_RESOLVING:
            kdDebug(713)<<"Resolving error."<<endl;
            break;
        case GG_ERROR_CONNECTING:
            kdDebug(713)<<"Connecting error."<<endl;
            break;
        case GG_ERROR_READING:
            kdDebug(713)<<"Reading error."<<endl;
            break;
        case GG_ERROR_WRITING:
            kdDebug(713)<<"Writting error."<<endl;
            break;
        default:
            kdDebug(713)<<"Freaky error = "<<session_->state<<" "<<strerror(errno)<<endl;

        }
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE ) {
        emit done( static_cast<struct gg_search*>(session_->data) );
        gg_free_search( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}

RegisterCommand::RegisterCommand( QObject* parent, const char* name )
    :GaduCommand( parent, name ), session_(0)
{
}

RegisterCommand::RegisterCommand( const QString& email, const QString& password, QObject* parent, const char* name )
    :GaduCommand(parent, name), email_(email), password_(password), session_(0)
{
}

RegisterCommand::~RegisterCommand()
{
}

void
RegisterCommand::setUserinfo( const QString& email, const QString& password )
{
    email_ = email;
    password_ = password;
}

void
RegisterCommand::execute()
{
    session_ = gg_register( email_.latin1(), password_.latin1(), 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
RegisterCommand::watcher()
{
    disableNotifiers();

    if ( gg_register_watch_fd( session_ ) == -1 ) {
        gg_free_register( session_ );
        emit error( i18n("Connection error"),
                    i18n("Unknown connection error while registering") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_free_register( session_ );
        emit error( i18n("Registration error"),
                    i18n("There was an unknown registration error") );
        switch( session_->error )
        {
        case GG_ERROR_RESOLVING:
            kdDebug(713)<<"Resolving error."<<endl;
            break;
        case GG_ERROR_CONNECTING:
            kdDebug(713)<<"Connecting error."<<endl;
            break;
        case GG_ERROR_READING:
            kdDebug(713)<<"Reading error."<<endl;
            break;
        case GG_ERROR_WRITING:
            kdDebug(713)<<"Writting error."<<endl;
            break;
        default:
            kdDebug(713)<<"Freaky error = "<<session_->state<<" "<<strerror(errno)<<endl;
            break;
        }
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE ) {
        emit done( i18n("Registration complete"), i18n("Registration has completed successfully. You will receive an email with a confirmation shortly.") );
        gg_free_register( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}

RemindPasswordCommand::RemindPasswordCommand( QObject* parent, const char* name )
    : GaduCommand(parent, name), uin_(0), session_(0)
{
}

RemindPasswordCommand::RemindPasswordCommand( uin_t uin, QObject* parent, const char* name )
    : GaduCommand(parent, name), uin_(uin), session_(0)
{
}

RemindPasswordCommand::~RemindPasswordCommand()
{
    if ( session_ )
        gg_remind_passwd_free( session_ );
}

void
RemindPasswordCommand::setUIN( uin_t uin )
{
    uin_ = uin;
}

void
RemindPasswordCommand::execute()
{
    session_ = gg_remind_passwd( uin_, 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
RemindPasswordCommand::watcher()
{
    disableNotifiers();

    if (gg_remind_passwd_watch_fd( session_ ) == -1) {
        gg_free_remind_passwd( session_ );
        emit error( i18n("Connection error"),
                    i18n("Password reminding finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_free_remind_passwd( session_ );
        emit error( i18n("Connection error"),
                    i18n("Password reminding finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE) {
        struct gg_pubdir *p = static_cast<struct gg_pubdir *>(session_->data);
        QString finished = (p->success)?i18n("Successfully"):i18n("Unsuccessfully. Please retry.");
        emit done( i18n("Remind password"),
                   i18n("Remind password finished: ") + finished );
        gg_free_remind_passwd( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}

ChangePasswordCommand::ChangePasswordCommand( QObject* parent, const char* name )
    : GaduCommand( parent, name ), session_(0)
{
}

ChangePasswordCommand::~ChangePasswordCommand()
{
    if ( session_ )
        gg_change_passwd_free( session_ );
}

void
ChangePasswordCommand::setInfo( uin_t uin, const QString& passwd, const QString& newpasswd,
                                const QString& newemail )
{
    uin_ = uin;
    passwd_ = passwd;
    newpasswd_ = newpasswd;
    newemail_ = newemail;
}

void
ChangePasswordCommand::execute()
{
    session_ = gg_remind_passwd( uin_, 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
ChangePasswordCommand::watcher()
{
    disableNotifiers();

    if (gg_pubdir_watch_fd( session_ ) == -1) {
        gg_change_passwd_free( session_ );
        emit error( i18n("Connection error"),
                    i18n("Password changing finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_free_change_passwd( session_ );
        emit error( i18n("State error."),
                    i18n("Password changing finished prematurely due to a session related problem (try again later).") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE) {
        emit done( i18n("Changed password"),
                   i18n("Your password has been changed.") );
        gg_free_change_passwd( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}


ChangeInfoCommand::ChangeInfoCommand( QObject* parent, const char* name )
    :GaduCommand( parent, name ), session_(0)
{
}

ChangeInfoCommand::~ChangeInfoCommand()
{
    if ( session_ )
        gg_change_pubdir_free( session_ );
}

void
ChangeInfoCommand::setInfo( uin_t uin, const QString& passwd,
                            const QString& firstName, const QString& lastName,
                            const QString& nickname, const QString& email,
                            int born, int gender, const QString& city )
{
    memset( &info_, 0, sizeof(struct gg_change_info_request) );
    uin_ = uin;
    passwd_ = passwd;
    info_.first_name = (char*)firstName.latin1();
    info_.last_name = (char*)lastName.latin1();
    info_.nickname = (char*)nickname.latin1();
    info_.email = (char*)email.latin1();
    info_.born = born;
    info_.gender = gender;
    info_.city = (char*)city.latin1();
}

void
ChangeInfoCommand::execute()
{
    session_ = gg_change_info( uin_, passwd_.latin1(), &info_, 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
ChangeInfoCommand::watcher()
{
    disableNotifiers();

    if ( gg_change_pubdir_watch_fd( session_ ) == -1 ) {
        gg_change_pubdir_free( session_ );
        emit error( i18n("Connection error"),
                    i18n("User info changing finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_change_pubdir_free( session_ );
        emit error( i18n("State error."),
                    i18n("User info changing finished prematurely due to a session related problem (try again later).") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE) {
        emit done( i18n("Changed user info"),
                   i18n("Your info has been changed.") );
        gg_change_pubdir_free( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}

UserlistPutCommand::UserlistPutCommand( QObject* parent, const char* name )
    :GaduCommand( parent, name )
{
}

UserlistPutCommand::UserlistPutCommand( uin_t uin, const QString& password, const QStringList& contacts,
                                        QObject* parent, const char* name )
    :GaduCommand( parent, name ), session_(0), uin_(uin), password_(password), contacts_(contacts)
{
}

UserlistPutCommand::~UserlistPutCommand()
{
    if ( session_ )
        gg_userlist_put_free( session_ );
}

void
UserlistPutCommand::setInfo( uin_t uin, const QString& password, const QStringList& contacts )
{
    uin_ = uin;
    password_ = password;
    contacts_ = contacts;
}

void
UserlistPutCommand::execute()
{
    session_ = gg_userlist_put( uin_, password_.latin1(), contacts_.join( "\r\n" ).latin1(), 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    checkSocket( session_->fd, session_->check );
}

void
UserlistPutCommand::watcher()
{
    disableNotifiers();

    if ( gg_userlist_put_watch_fd( session_ ) == -1 ) {
        gg_userlist_put_free( session_ );
        emit error( i18n("Connection error"),
                    i18n("Exporting of userlist to the server finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_userlist_put_free( session_ );
        emit error( i18n("State error."),
                    i18n("Exporting of userlist to the server finished prematurely due to a session related problem (try again later).") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE) {
        emit done( i18n("Userlist exported"),
                   i18n("Your userlist has been exported to the server.") );
        gg_userlist_put_free( session_ );
        done_ = true;
        delete this;
        return;
    }

    enableNotifiers( session_->check );
}

UserlistGetCommand::UserlistGetCommand( QObject* parent, const char* name )
    : GaduCommand( parent, name ), session_(0)
{
    kdDebug()<<"Userlist created"<<endl;
}

UserlistGetCommand::~UserlistGetCommand()
{
    if ( session_ )
        gg_userlist_get_free( session_ );
}

void
UserlistGetCommand::setInfo( uin_t uin, const QString& password )
{
    uin_ = uin;
    password_ = password;
}

void
UserlistGetCommand::execute()
{
    session_ = gg_userlist_get( uin_, password_.latin1(), 1 );
    connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
    kdDebug()<<"userlist executing"<<endl;
    checkSocket( session_->fd, session_->check );
    kdDebug()<<"userlist ending"<<endl;
}


void
UserlistGetCommand::watcher()
{
    disableNotifiers();
    kdDebug()<<"Watching start"<<endl;

    if ( gg_userlist_get_watch_fd( session_ ) == -1 ) {
        gg_userlist_get_free( session_ );
        emit error( i18n("Connection error"),
                    i18n("Importing of userlist from the server finished prematurely due to a connection error.") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_ERROR ) {
        gg_userlist_get_free( session_ );
        emit error( i18n("State error."),
                    i18n("Importing of userlist from the server finished prematurely due to a session related problem (try again later).") );
        done_ = true;
        delete this;
        return;
    }
    if ( session_->state == GG_STATE_DONE) {
        QString data = QString( static_cast<char*>(session_->data) );
        QStringList result;
        QString name;
        QString group;
        QString uin;
        QStringList::iterator it;
        QStringList::iterator itNext;
        int idx = 6;

        if ( data.contains( '\n' ) ) {
            result = QStringList::split( ";", data, true );
        } else {
            QStringList strList = QStringList::split( ";", data, true );
            for( it = strList.begin(); it != strList.end(); ++it, --idx ) {
                if ( idx == 5 )
                    name = (*it);
                if ( idx == 1 )
                    group = (*it);
                if ( idx == 0 ) {
                    idx = 6;
                    itNext = it;
                    ++itNext;
                    if ( (*it).endsWith( (*itNext) ) ) {
                        uin = (*it).replace( QRegExp( (*itNext) ), "" );
                    } else {
                        //super bad
                    }
                    result.append( name + ";" + name + ";" + name + ";" + name + ";" + ";" + group + ";" + uin );
                }
            }
        }
        emit done( result );
        done_ = true;
        delete this;
        return;
    }

    kdDebug()<<"Watching end"<<endl;
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

