// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadurichtextformat.cpp
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

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <kdebug.h>

#include "gadudcctransaction.h"
#include "libgadu.h"

#include <qsocketnotifier.h>

GaduDCCTransaction::GaduDCCTransaction( gg_dcc* socket, QObject* parent, const char* name )
:QObject( parent, name )
{
	dccSock = socket;
}
/*
GaduDCCTransaction::GaduDCCTransaction( QObject* parent, const char* name )
:QObject( parent, name )
{
	dccSock = NULL;
}
*/
//unsigned int ip, unsigned int port, unsigned int myUin, unsigned int peerUin,

unsigned int
GaduDCCTransaction::recvUIN()
{
	if ( dccSock ) {
		return dccSock->uin;
	}
	return 0;
}

unsigned int
GaduDCCTransaction::peerUIN()
{
	if ( dccSock ) {
		return dccSock->peer_uin;
	}
	return 0;
}

bool GaduDCCTransaction::setup()
{
	if ( dccSock == NULL ){
		kdDebug(14100) << "attempt to initialize gadu-dcc transaction with NULL dccsocket " << endl;
		return false;
	}

	createNotifiers( true );
	enableNotifiers( dccSock->check  );

	return true;
}


GaduDCCTransaction::~GaduDCCTransaction()
{
	closeDCC();
}

void
GaduDCCTransaction::closeDCC()
{
	disableNotifiers();
	destroyNotifiers();
	gg_dcc_free( dccSock );
}

void
GaduDCCTransaction::destroyNotifiers()
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
GaduDCCTransaction::createNotifiers( bool connect )
{
	if ( !dccSock ){
		return;
	}

	read_ = new QSocketNotifier( dccSock->fd, QSocketNotifier::Read, this );
	read_->setEnabled( false );

	write_ = new QSocketNotifier( dccSock->fd, QSocketNotifier::Write, this );
	write_->setEnabled( false );

	if ( connect ) {
		QObject::connect( read_, SIGNAL( activated( int ) ), SLOT( watcher() ) );
		QObject::connect( write_, SIGNAL( activated( int ) ), SLOT( watcher() ) );
	}
}

void
GaduDCCTransaction::enableNotifiers( int checkWhat )
{
	if( (checkWhat & GG_CHECK_READ) && read_ ) {
		read_->setEnabled( true );
	}
	if( (checkWhat & GG_CHECK_WRITE) && write_ ) {
		write_->setEnabled( true );
	}
}

void
GaduDCCTransaction::disableNotifiers()
{
	if ( read_ ) {
		read_->setEnabled( false );
	}
	if ( write_ ) {
		write_->setEnabled( false );
	}
}

void
GaduDCCTransaction::watcher() {

	gg_event *dccEvent;

	disableNotifiers();

	dccEvent = gg_dcc_watch_fd(dccSock);
	if ( ! dccEvent ) {
		// connection is fucked
		closeDCC();
		return;
	}
	switch ( dccEvent->type ) {
		default:
kdDebug(14100) << "unknown/unhandled DCC EVENT: " << dccEvent->type << endl;
			break;
	}

	if ( dccEvent ) {
		gg_free_event( dccEvent );
	}

	enableNotifiers( dccSock->check );
}

#include "gadudcctransaction.moc"
