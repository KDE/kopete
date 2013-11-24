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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "gadudccserver.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <kdebug.h>

#include "libgadu.h"
#include "gaduaccount.h"

#include <qobject.h>
#include <qsocketnotifier.h>
#include <qhostaddress.h>

GaduDCCServer::GaduDCCServer( QHostAddress* dccIp, unsigned int port )
:QObject()
{
	kDebug( 14100 ) << "dcc socket NULL, creating new liteining socket ";

	// don't care about UIN at that point
	dccSock = gg_dcc_socket_create( (unsigned int)-1, port );

	if ( dccSock == NULL ){
		kDebug(14100) << "attempt to initialize gadu-dcc listeing socket FAILED";
		return;
	}

	kDebug(14100) << "attempt to initialize gadu-dcc listeing socket success";

	// using global variables sucks, don't have too much choice thou
	if ( dccIp == NULL ) {
		gg_dcc_ip = 0xffffffff; // 255.255.255.255
	}
	else {
		gg_dcc_ip = htonl( dccIp->toIPv4Address() );
	}
	gg_dcc_port = dccSock->port;

	createNotifiers( true );
	enableNotifiers( dccSock->check  );
}

GaduDCCServer::~GaduDCCServer()
{
	kDebug( 14100 ) << "gadu dcc server destructor ";
	closeDCC();
}

void
GaduDCCServer::closeDCC()
{
	if ( dccSock ) {
		disableNotifiers();
		destroyNotifiers();
		gg_dcc_free( dccSock );
		dccSock = NULL;
		gg_dcc_ip = 0;
		gg_dcc_port = 0;
	}

}

unsigned int
GaduDCCServer::listeingPort()
{
	if ( dccSock == NULL ) {
		return 0;
	}
// else
	return dccSock->port;
}

void
GaduDCCServer::destroyNotifiers()
{
	disableNotifiers();
	delete read_;
	read_ = NULL;
	delete write_;
	write_ = NULL;
}

void
GaduDCCServer::createNotifiers( bool connect )
{
	if ( !dccSock ){
		return;
	}

	read_ = new QSocketNotifier( dccSock->fd, QSocketNotifier::Read, this );
	read_->setEnabled( false );

	write_ = new QSocketNotifier( dccSock->fd, QSocketNotifier::Write, this );
	write_->setEnabled( false );

	if ( connect ) {
		QObject::connect( read_, SIGNAL(activated(int)), SLOT(watcher()) );
		QObject::connect( write_, SIGNAL(activated(int)), SLOT(watcher()) );
	}
}

void
GaduDCCServer::enableNotifiers( int checkWhat )
{
	if( (checkWhat & GG_CHECK_READ) && read_ ) {
		read_->setEnabled( true );
	}
	if( (checkWhat & GG_CHECK_WRITE) && write_ ) {
		write_->setEnabled( true );
	}
}

void
GaduDCCServer::disableNotifiers()
{
	if ( read_ ) {
		read_->setEnabled( false );
	}
	if ( write_ ) {
		write_->setEnabled( false );
	}
}

void
GaduDCCServer::watcher() {

	gg_event* dccEvent;
	bool handled = false;

	disableNotifiers();

	dccEvent = gg_dcc_watch_fd( dccSock );
	if ( ! dccEvent ) {
		// connection is fucked
		// we should try to reenable it
//		closeDCC();
		return;
	}
	switch ( dccEvent->type ) {
		case GG_EVENT_NONE:
			break;
		case GG_EVENT_DCC_ERROR:
			kDebug( 14100 ) << " dcc error occurred ";
			break;
		case GG_EVENT_DCC_NEW:
			// I do expect receiver to set this boolean to true if he handled signal
			// if so, no other receiver should be bothered with it, and I shall not close it
			// otherwise connection is closed as not handled
			emit incoming( dccEvent->event.dcc_new, handled );
			if ( !handled ) {
				if ( dccEvent->event.dcc_new->file_fd > 0) {
					close( dccEvent->event.dcc_new->file_fd );
				}
				gg_dcc_free( dccEvent->event.dcc_new );
			}
			break;
		default:
			kDebug(14100) << "unknown/unhandled DCC EVENT: " << dccEvent->type;
			break;
	}

	if ( dccEvent ) {
		gg_free_event( dccEvent );
	}

	enableNotifiers( dccSock->check );
}
#include "gadudccserver.moc"
