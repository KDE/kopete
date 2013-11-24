// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadudcc.cpp
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

#include "gadudcc.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <kdebug.h>

#include "gadudccserver.h"
#include "gadudcctransaction.h"
#include "gaduaccount.h"

#include "libgadu.h"

#include <qsocketnotifier.h>
#include <qhostaddress.h>
#include <qmutex.h>
#include <qmap.h>
#include <qstring.h>

volatile unsigned int GaduDCC::referenceCount = 0;

GaduDCCServer* GaduDCC::dccServer = NULL;

static QMutex initmutex;

typedef QMap< unsigned int, GaduAccount* > gaduAccounts;
static gaduAccounts accounts;

GaduDCC::GaduDCC( QObject* parent )
:QObject( parent )
{
}

bool
GaduDCC::unregisterAccount()
{
	return unregisterAccount( accountId );
}

GaduAccount*
GaduDCC::account( unsigned int uin )
{
	return accounts[ uin ];
}

bool
GaduDCC::unregisterAccount( unsigned int id )
{
	initmutex.lock();

	if ( id == 0 ) {
		kDebug(14100) << "ID nan";
		initmutex.unlock();
		return false;
	}

	if ( !accounts.contains( id ) ) {
		kDebug(14100) << "attempt to unregister not registered account";
		initmutex.unlock();
		return false;
	}

	accounts.remove( id );

	if ( --referenceCount <= 0 ) {
		kDebug(14100) << "closing dcc socket";
		referenceCount = 0;
		delete dccServer;
		dccServer = NULL;
	}
	kDebug(14100)  << "reference count " << referenceCount;
	initmutex.unlock();

	return true;
}

bool
GaduDCC::registerAccount( GaduAccount* account )
{
	unsigned int aid;

	if ( !account ) {
		return false;
	}

	if ( account->accountId().isEmpty() ) {
		kDebug(14100) << "attempt to register account with empty ID";
		return false;
	}

	initmutex.lock();

	aid = account->accountId().toInt();

	if ( accounts.contains( aid ) ) {
		kDebug(14100) << "attempt to register already registered account";
		initmutex.unlock();
		return false;
	}

	accountId = aid;
	kDebug( 14100 ) << " attempt to register " << accountId;

	accounts[ accountId ] = account;

	referenceCount++;

	if ( !dccServer) {
		dccServer = new GaduDCCServer();
	}

	connect( dccServer, SIGNAL(incoming(gg_dcc*,bool&)), SLOT(slotIncoming(gg_dcc*,bool&)) );

	initmutex.unlock();

	return true;
}
void
GaduDCC::slotIncoming( gg_dcc* incoming, bool& handled )
{
	gg_dcc* newdcc;
	GaduDCCTransaction* dt;

	kDebug( 14100 ) << "slotIncomming for UIN: " << incoming->uin;

	// no uin? I'm so sorry
	// this screws file receiving (using kadu 0.4.x as peer) for me
//	if ( !incoming->uin ) {
//		return;
//	}

	handled = true;
	// TODO: limit number of connections per contact, or maybe even use parametr for that
	newdcc = new gg_dcc;
	memcpy( newdcc, incoming, sizeof( gg_dcc ) );
	dt = new GaduDCCTransaction( this );
	if ( dt->setupIncoming( newdcc ) == false ) {
		// FIXME: write something to user, maybe, or not...
		delete dt;
	}
}

GaduDCC::~GaduDCC()
{
	if ( accounts.contains( accountId ) ) {
		kDebug( 14100 ) << "unregister account " << accountId << "  in destructor ";
		unregisterAccount( accountId );
	}
}

unsigned int
GaduDCC::listeingPort()
{
	if ( dccServer ) {
		return dccServer->listeingPort();
	}
	return 0;
}

#include "gadudcc.moc"
