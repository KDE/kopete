// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Current author and maintainer: Grzegorz Jaskiewicz
//				gj at pointblue.com.pl
//
// Copyright (C) 	2002-2003	 Zack Rusin <zack@kde.org>
//
// gaducommands.h - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any
//  of these). These delete themselves, meaning you don't
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include "gaducommands.h"
#include <qsocketnotifier.h>
#include <klocale.h>
#include <qregexp.h>
#include <qtextcodec.h>
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


RegisterCommand::RegisterCommand( QObject* parent, const char* name )
	:GaduCommand( parent, name ), session_(0)
{
    uin=0;
}

RegisterCommand::RegisterCommand( const QString& email, const QString& password, QObject* parent, const char* name )
	:GaduCommand(parent, name), email_(email), password_(password), session_(0)
{
    uin=0;
}


unsigned int RegisterCommand::newUin()
{
	return uin;
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
	session_ = gg_register( email_.local8Bit(), password_.local8Bit(), 1 );
	connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
	checkSocket( session_->fd, session_->check );
}

void RegisterCommand::watcher()
{
	disableNotifiers();
	gg_pubdir *gg_pub;


	if ( gg_register_watch_fd( session_ ) == -1 ) {
		gg_free_register( session_ );
		emit error( i18n("Connection Error"),
				i18n("Unknown connection error while registering") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_ERROR ) {
		gg_free_register( session_ );
		emit error( i18n("Registration Error"),
				i18n("There was an unknown registration error.") );
		switch( session_->error )
		{
		case GG_ERROR_RESOLVING:
			kdDebug(14100713)<<"Resolving error."<<endl;
			break;
		case GG_ERROR_CONNECTING:
			kdDebug(14100713)<<"Connecting error."<<endl;
			break;
		case GG_ERROR_READING:
			kdDebug(14100713)<<"Reading error."<<endl;
			break;
		case GG_ERROR_WRITING:
			kdDebug(14100713)<<"Writing error."<<endl;
			break;
		default:
			kdDebug(14100713)<<"Freaky error = "<<session_->state<<" "<<strerror(errno)<<endl;
			break;
		}
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_DONE ) {

		gg_pub=(gg_pubdir *)session_->data;
		if (gg_pub){
		    uin= gg_pub->uin;
//		    kdDebug(14100713)<<"wylosowany numerek to:"<< uin << endl;
		    emit done( i18n("Registration Finished"), i18n("Registration has completed successfully.") );
		}
		else{
		    emit error( i18n("Registration Error"),
				i18n("Data send to server were invalid.") );
		}

		gg_free_register( session_ );
		done_ = true;
		deleteLater();
		return;
	}

	enableNotifiers( session_->check );
	return;
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
		emit error( i18n("Connection Error"),
								i18n("Password reminding finished prematurely due to a connection error.") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_ERROR ) {
		gg_free_remind_passwd( session_ );
		emit error( i18n("Connection Error"),
								i18n("Password reminding finished prematurely due to a connection error.") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_DONE) {
		struct gg_pubdir *p = static_cast<struct gg_pubdir *>(session_->data);
		QString finished = (p->success)?i18n("Successfully"):i18n("Unsuccessful. Please retry.");
		emit done( i18n("Remind Password"),
							 i18n("Remind password finished: ") + finished );
		gg_free_remind_passwd( session_ );
		done_ = true;
		deleteLater();
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
	session_ = gg_change_passwd2( uin_,
						passwd_.latin1(),
						newpasswd_.latin1(),
						newemail_.latin1(),
						newemail_.latin1(), 1 );
	connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
	checkSocket( session_->fd, session_->check );
}

void
ChangePasswordCommand::watcher()
{
	disableNotifiers();

	if (gg_pubdir_watch_fd( session_ ) == -1) {
		gg_change_passwd_free( session_ );
		emit error( i18n("Connection Error"),
								i18n("Password changing finished prematurely due to a connection error.") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_ERROR ) {
		gg_free_change_passwd( session_ );
		emit error( i18n("State Error"),
								i18n("Password changing finished prematurely due to a session related problem (try again later).") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_DONE) {
		emit done( i18n("Changed Password"),
							 i18n("Your password has been changed.") );
		gg_free_change_passwd( session_ );
		done_ = true;
		deleteLater();
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
	info_.first_name = firstName.local8Bit().data();
	info_.last_name = lastName.local8Bit().data();
	info_.nickname = nickname.local8Bit().data();
	info_.email = email.local8Bit().data();
	info_.born = born;
	info_.gender = gender;
	info_.city = city.local8Bit().data();
}

void
ChangeInfoCommand::execute()
{
	session_ = gg_change_info( uin_, passwd_.local8Bit(), &info_, 1 );
	connect( this, SIGNAL(socketReady()), SLOT(watcher()) );
	checkSocket( session_->fd, session_->check );
}

void
ChangeInfoCommand::watcher()
{
	disableNotifiers();

	if ( gg_change_pubdir_watch_fd( session_ ) == -1 ) {
		gg_change_pubdir_free( session_ );
		emit error( i18n("Connection Error"),
								i18n("User info changing finished prematurely due to a connection error.") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_ERROR ) {
		gg_change_pubdir_free( session_ );
		emit error( i18n("State Error"),
								i18n("User info changing finished prematurely due to a session related problem (try again later).") );
		done_ = true;
		deleteLater();
		return;
	}
	if ( session_->state == GG_STATE_DONE) {
		emit done( i18n("Changed User Info"),
							 i18n("Your info has been changed.") );
		gg_change_pubdir_free( session_ );
		done_ = true;
		deleteLater();
		return;
	}

	enableNotifiers( session_->check );
}

#include "gaducommands.moc"

