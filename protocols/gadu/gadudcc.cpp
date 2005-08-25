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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <kdebug.h>

#include "gadudccserver.h"
#include "gadudcc.h"
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

GaduDCC::GaduDCC( QObject* parent, const char* name )
:QObject( parent, name )
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
		kdDebug(14100) << "ID nan" << endl;
		initmutex.unlock();
		return false;
	}

	if ( !accounts.contains( id ) ) {
		kdDebug(14100) << "attempt to unregister not registered account" << endl;
		initmutex.unlock();
		return false;
	}

	accounts.remove( id );

	if ( --referenceCount <= 0 ) {
		kdDebug(14100) << "closing dcc socket" << endl;
		referenceCount = 0;
		if ( dccServer ) {
			delete dccServer;
			dccServer = NULL;
		}
	}
	kdDebug(14100)  << "reference count " << referenceCount << endl;
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
		kdDebug(14100) << "attempt to register account with empty ID" << endl;
		return false;
	}

	initmutex.lock();

	aid = account->accountId().toInt();

	if ( accounts.contains( aid ) ) {
		kdDebug(14100) << "attempt to register already registered account" << endl;
		initmutex.unlock();
		return false;
	}

	accountId = aid;
	kdDebug( 14100 ) << " attempt to register " << accountId << endl;

	accounts[ accountId ] = account;

	referenceCount++;

	if ( !dccServer) {
		dccServer = new GaduDCCServer();
	}

	connect( dccServer, SIGNAL( incoming( gg_dcc*, bool& ) ), SLOT( slotIncoming( gg_dcc*, bool& ) ) );

	initmutex.unlock();

	return true;
}
void
GaduDCC::slotIncoming( gg_dcc* incoming, bool& handled )
{
	gg_dcc* newdcc;
	GaduDCCTransaction* dt;

	kdDebug( 14100 ) << "slotIncomming for UIN: " << incoming->uin  << endl;

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
		kdDebug( 14100 ) << "unregister account " << accountId << "  in destructor " << endl;
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
