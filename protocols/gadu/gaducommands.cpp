// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003	 Zack Rusin 	<zack@kde.org>
//
// gaducommands.cpp - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any of these).
// These delete themselves, meaning you don't
//  have to/can't delete them explicitly and have to create
//  them dynamically (via the 'new' call).
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

#include "gaducommands.h"
#include "gadusession.h"

#include <qsocketnotifier.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qimage.h>
#include <QPixmap>

#include <klocale.h>
#include <kdebug.h>

#include <errno.h>

GaduCommand::GaduCommand( QObject* parent )
: QObject( parent ), read_( 0 ), write_( 0 )
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

void
GaduCommand::checkSocket( int fd, int checkWhat )
{
	read_ = new QSocketNotifier( fd, QSocketNotifier::Read, this );
	read_->setEnabled( false );
	QObject::connect( read_, SIGNAL(activated(int)), SLOT(forwarder()) );

	write_ = new QSocketNotifier( fd, QSocketNotifier::Write, this );
	write_->setEnabled( false );
	QObject::connect( write_, SIGNAL(activated(int)), SLOT(forwarder()) );

	enableNotifiers( checkWhat );
}

void
GaduCommand::enableNotifiers( int checkWhat )
{
	if ( read_ ) {
		if( checkWhat & GG_CHECK_READ ) {
			read_->setEnabled( true );
		}
	}

	if ( write_ ) {
		if( checkWhat & GG_CHECK_WRITE ) {
			write_->setEnabled( true );
		}
	}
}

void
GaduCommand::disableNotifiers()
{
	if ( read_ ) {
		read_->setEnabled( false );
	}
	if ( write_ ) {
		write_->setEnabled( false );
	}
}

void
GaduCommand::deleteNotifiers()
{
	delete read_;
	read_ = NULL;
	delete write_;
	write_ = NULL;
}

void
GaduCommand::forwarder()
{
	emit socketReady();
}

RegisterCommand::RegisterCommand( QObject* parent )
:GaduCommand( parent ), state( RegisterStateNoToken ), session_( 0 ), uin( 0 )
{
}

RegisterCommand::RegisterCommand( const QString& email, const QString& password, QObject* parent )
:GaduCommand( parent ), state( RegisterStateNoToken ), email_( email ), password_( password ), session_( 0 ), uin( 0 )
{
}

RegisterCommand::~RegisterCommand()
{
}

unsigned int RegisterCommand::newUin()
{
	if ( state == RegisterStateDone ) {
		return uin;
	}
//	else
	return 0;
}
void
RegisterCommand::requestToken()
{
	kDebug( 14100 ) << "requestToken Initialisation";
	state = RegisterStateWaitingForToken;

	if ( !( session_ = gg_token( 1 ) ) ) {
		emit error( i18n( "Gadu-Gadu" ), i18n( "Unable to retrieve token." ) );
		state = RegisterStateNoToken;
		return;
	}

	connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
	checkSocket( session_->fd, session_->check );

	return;
}

void
RegisterCommand::setUserinfo( const QString& email, const QString& password, const QString& token )
{
	email_ = email;
	password_ = password;
	tokenString = token;
}

void
RegisterCommand::cancel()
{
	deleteNotifiers();
	session_ = NULL;

}

void
RegisterCommand::execute()
{
	if ( state != RegisterStateGotToken || email_.isEmpty() || password_.isEmpty() || tokenString.isEmpty() ) {
		// get token first || fill information
		kDebug(14100) << "not enough info to run execute, state: " << state << " , email: " << email_ << ", password present " << !password_.isEmpty() << ", token string:" << tokenString;
		return;
	}
	session_ = gg_register3( email_.toAscii(), password_.toAscii(), tokenId.toAscii(), tokenString.toAscii(), 1 );
	if ( !session_ ) {
		error( i18n( "Gadu-Gadu" ), i18n( "Registration FAILED" ) );
		return;
	}
	state = RegisterStateWaitingForNumber;
	connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
	checkSocket( session_->fd, session_->check );
}

void RegisterCommand::watcher()
{
	gg_pubdir* pubDir;

	if ( state == RegisterStateWaitingForToken  ) {
		disableNotifiers();
		if ( gg_token_watch_fd( session_ ) == -1 ) {
			deleteNotifiers();
			emit error( i18n( "Gadu-Gadu" ), i18n( "Unknown connection error while retrieving token." ) );
			gg_token_free( session_ );
			session_ = NULL;
			state = RegisterStateNoToken;
			return;
		}

		pubDir = (struct gg_pubdir *)session_->data;
		emit operationStatus( i18n( "Token retrieving status: %1", GaduSession::stateDescription( session_->state ) ) );
		switch ( session_->state ) {
			case GG_STATE_CONNECTING:
				kDebug( 14100 ) << "Recreating notifiers ";
				deleteNotifiers();
				checkSocket( session_->fd, 0);
				break;
			case GG_STATE_ERROR:
				deleteNotifiers();
				emit error( i18n( "Gadu-Gadu token retrieve problem" ), GaduSession::errorDescription(  session_->error )  );
				gg_token_free( session_ );
				session_ = NULL;
				state = RegisterStateNoToken;
				return;
				break;
			case GG_STATE_DONE:
				struct gg_token* sp = ( struct gg_token* )session_->data;
				tokenId = (char *)sp->tokenid;
				kDebug( 14100 ) << "got Token!, ID: " << tokenId;
				deleteNotifiers();
				if ( pubDir->success ) {
					QPixmap tokenImg;
					tokenImg.loadFromData( (const unsigned char *)session_->body, session_->body_size );
					state = RegisterStateGotToken;
					emit tokenRecieved( tokenImg, tokenId );
				}
				else {
					emit error( i18n( "Gadu-Gadu" ), i18n( "Unable to retrieve token." ) );
					state = RegisterStateNoToken;
					deleteLater();
				}
				gg_token_free( session_ );
				session_ = NULL;
				disconnect( this, SLOT(watcher()) );
				return;
				break;
		}
		enableNotifiers( session_->check );
	}
	if ( state == RegisterStateWaitingForNumber ) {
		disableNotifiers();
		if ( gg_register_watch_fd( session_ ) == -1 ) {
			deleteNotifiers();
			emit error( i18n( "Gadu-Gadu" ), i18n( "Unknown connection error while registering." ) );
			gg_free_register( session_ );
			session_ = NULL;
			state = RegisterStateGotToken;
			return;
		}
		pubDir = (gg_pubdir*) session_->data;
		emit operationStatus( i18n( "Registration status: %1", GaduSession::stateDescription( session_->state ) ) );
		switch ( session_->state ) {
			case GG_STATE_CONNECTING:
				kDebug( 14100 ) << "Recreating notifiers ";
				deleteNotifiers();
				checkSocket( session_->fd, 0);
				break;
			case GG_STATE_ERROR:
				deleteNotifiers();
				emit error( i18n( "Gadu-Gadu Registration Error" ), GaduSession::errorDescription(  session_->error ) );
				gg_free_register( session_ );
				session_ = NULL;
				state = RegisterStateGotToken;
				return;
				break;

			case GG_STATE_DONE:
				deleteNotifiers();
				if ( pubDir->success && pubDir->uin ) {
					uin= pubDir->uin;
					state = RegisterStateDone;
					emit done( i18n( "Registration Finished" ), i18n( "Registration has been completed successfully." ) );
				}
				else {
					emit error( i18n( "Registration Error" ), i18n( "Incorrect data sent to server." ) );
					state = RegisterStateGotToken;
				}
				gg_free_register( session_ );
				session_ = NULL;
				disconnect( this, SLOT(watcher()) );
				deleteLater();
				return;
				break;
		}
		enableNotifiers( session_->check );
		return;
	}
}

RemindPasswordCommand::RemindPasswordCommand( QObject* parent )
: GaduCommand( parent ), uin_( 0 ), session_( 0 )
{
}

RemindPasswordCommand::RemindPasswordCommand( uin_t uin, QObject* parent )
: GaduCommand( parent ), uin_( uin ), session_( 0 )
{
}

RemindPasswordCommand::~RemindPasswordCommand()
{
}

void
RemindPasswordCommand::setUIN( uin_t uin )
{
	uin_ = uin;
}

void
RemindPasswordCommand::execute()
{
}

void
RemindPasswordCommand::watcher()
{
	disableNotifiers();

	if ( gg_remind_passwd_watch_fd( session_ ) == -1 ) {
		gg_free_remind_passwd( session_ );
		emit error( i18n( "Connection Error" ), i18n( "Password reminding finished prematurely due to a connection error." ) );
		done_ = true;
		deleteLater();
		return;
	}

	if ( session_->state == GG_STATE_ERROR ) {
		gg_free_remind_passwd( session_ );
		emit error( i18n( "Connection Error" ), i18n( "Password reminding finished prematurely due to a connection error." ) );
		done_ = true;
		deleteLater();
		return;
	}

	if ( session_->state == GG_STATE_DONE ) {
		struct gg_pubdir* p = static_cast<struct gg_pubdir*>( session_->data );
		QString finished = (p->success) ? i18n( "Success" ) : i18n( "Unsuccessful. Please retry." );
		emit done( i18n( "Remind Password" ), i18n( "Remind password finished: " ) + finished );
		gg_free_remind_passwd( session_ );
		done_ = true;
		deleteLater();
		return;
	}
	enableNotifiers( session_->check );
}

ChangePasswordCommand::ChangePasswordCommand( QObject* parent )
: GaduCommand( parent ), session_( 0 )
{
}

ChangePasswordCommand::~ChangePasswordCommand()
{
}

void
ChangePasswordCommand::setInfo( uin_t uin, const QString& passwd, const QString& newpasswd, const QString& newemail )
{
	uin_			= uin;
	passwd_		= passwd;
	newpasswd_	= newpasswd;
	newemail_		= newemail;
}

void
ChangePasswordCommand::execute()
{
}

void
ChangePasswordCommand::watcher()
{
	disableNotifiers();

	if (  gg_pubdir_watch_fd( session_ ) == -1 ) {
		gg_change_passwd_free( session_ );
		emit error( i18n( "Connection Error" ), i18n( "Password changing finished prematurely due to a connection error." ) );
		done_ = true;
		deleteLater();
		return;
	}

	if ( session_->state == GG_STATE_ERROR ) {
		gg_free_change_passwd( session_ );
		emit error( i18n( "State Error" ),
				i18n( "Password changing finished prematurely due to a session related problem (try again later)." ) );
		done_ = true;
		deleteLater();
		return;
	}

	if ( session_->state == GG_STATE_DONE ) {
		emit done( i18n( "Changed Password" ),  i18n( "Your password has been changed." ) );
		gg_free_change_passwd( session_ );
		done_ = true;
		deleteLater();
		return;
	}

	enableNotifiers( session_->check );
}


#include "gaducommands.moc"
