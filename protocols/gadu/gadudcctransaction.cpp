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
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetetransfermanager.h"
#include "kopeteuiglobal.h"

#include <qsocketnotifier.h>
#include <qfile.h>

#include "gadudcctransaction.h"
#include "gaducontact.h"
#include "gaduaccount.h"
#include "gadudcc.h"

#include "libgadu.h"

GaduDCCTransaction::GaduDCCTransaction( gg_dcc* socket, GaduDCC* parent, const char* name )
:QObject( parent, name ), read_( NULL ), write_( NULL ), account( NULL ), contact( NULL ), transfer_( NULL ), peer( 0 ) 
{
	dccSock_ = socket;
	gaduDCC_ = parent;
}

GaduDCCTransaction::GaduDCCTransaction( gg_dcc* socket, GaduContact* cont, GaduDCC* parent, const char* name )
:QObject( parent, name), read_( NULL ), write_( NULL ), account( NULL ), contact( cont ), transfer_( NULL ), peer( 0 )
{
	dccSock_ = socket;
	gaduDCC_ = parent;
}

GaduDCCTransaction::~GaduDCCTransaction()
{
	closeDCC();
}

unsigned int
GaduDCCTransaction::recvUIN()
{
	if ( dccSock_ ) {
		return dccSock_->uin;
	}
	return 0;
}

unsigned int
GaduDCCTransaction::peerUIN()
{
	if ( dccSock_ ) {
		return dccSock_->peer_uin;
	}
	return 0;
}

bool 
GaduDCCTransaction::setupIncoming( unsigned int p )
{
	if ( dccSock_ == NULL ){
		kdDebug(14100) << "attempt to initialize gadu-dcc transaction with NULL dccsocket " << endl;
		return false;
	}

	connect ( KopeteTransferManager::transferManager(), SIGNAL( accepted( KopeteTransfer *, const QString & ) ),
			  this, SLOT( slotIncomingTransferAccepted ( KopeteTransfer *, const QString & ) ) );
	connect ( KopeteTransferManager::transferManager(), SIGNAL( refused( const KopeteFileTransferInfo & ) ),
			  this, SLOT( slotTransferRefused( const KopeteFileTransferInfo & ) ) );

	incoming = true;
	peer = p;

	createNotifiers( true );
	enableNotifiers( dccSock_->check  );

	return true;
}


void
GaduDCCTransaction::closeDCC()
{
	kdDebug(14100) << "closeDCC()" << endl;

	disableNotifiers();
	destroyNotifiers();
	gg_dcc_free( dccSock_ );
	dccSock_ = NULL;
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
	if ( !dccSock_ ){
		return;
	}

	read_ = new QSocketNotifier( dccSock_->fd, QSocketNotifier::Read, this );
	read_->setEnabled( false );

	write_ = new QSocketNotifier( dccSock_->fd, QSocketNotifier::Write, this );
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
GaduDCCTransaction::slotIncomingTransferAccepted ( KopeteTransfer* transfer, const QString& fileName )
{

	if ( (long)transfer->info().transferId () != transferId_ ) {
		return;
	}
	
	// to string fools, strings used here are from jabber protocol
	
	transfer_ = transfer;
	localFile_.setName( fileName );

	if ( localFile_.exists() ) {
		KGuiItem resumeButton( i18n ( "&Resume" ) );
		KGuiItem overwriteButton( i18n ( "Over&write" ) );
		switch ( KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget (),
							i18n( "The file %1 already exists, do you want to resume or overwrite it?" ).arg( fileName ),
							i18n( "File Exists: %1" ).arg( fileName ), resumeButton, overwriteButton ) )
		{
			case KMessageBox::Yes:		// resume
				if ( localFile_.open( IO_WriteOnly | IO_Append ) ) {
					dccSock_->offset  = localFile_.size();
					dccSock_->file_fd = localFile_.handle();
				}
			break;

			case KMessageBox::No:		// overwrite
				if ( localFile_.open( IO_ReadWrite ) ) {
					dccSock_->offset  = 0;
					dccSock_->file_fd = localFile_.handle();
				}
			break;

			default:					// cancel
				closeDCC();
				deleteLater();
				return;
			break;
		}
		if ( localFile_.handle() < 1 ) {
			closeDCC();
			deleteLater();
			return;								
		}
	}
	else {
		// overwrite by default
		if ( localFile_.open( IO_ReadWrite ) == FALSE ) {
			transfer->slotError ( KIO::ERR_COULD_NOT_WRITE, fileName );
			closeDCC();
			deleteLater ();
			return;
		}
		dccSock_->offset  = 0;
		dccSock_->file_fd = localFile_.handle();
	}
	
	connect ( transfer, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotTransferResult() ) );

	// reenable notifiers
	enableNotifiers( dccSock_->check );

}

void
GaduDCCTransaction::slotTransferResult()
{
	if ( transfer_->error() == KIO::ERR_USER_CANCELED ) {
		if ( transfer_ ) {
			transfer_->setError( KopeteTransfer::CanceledLocal );
		}
		closeDCC();
		deleteLater();
	}
}

void 
GaduDCCTransaction::slotTransferRefused ( const KopeteFileTransferInfo&  transfer )
{
	if ( (long)transfer.transferId () != transferId_ )
		return;
	closeDCC();
	deleteLater();
}

void
GaduDCCTransaction::askIncommingTransfer()
{
	
	transferId_ = KopeteTransferManager::transferManager()->askIncomingTransfer ( contact,
		QString( (const char*)dccSock_->file_info.filename ),  dccSock_->file_info.size );

}

void
GaduDCCTransaction::watcher() {

	gg_event* dccEvent;

	disableNotifiers();

	dccEvent = gg_dcc_watch_fd(dccSock_ );
	if ( ! dccEvent ) {
		// connection is fucked
		closeDCC();
		return;
	}
	switch ( dccEvent->type ) {
		case GG_EVENT_DCC_CLIENT_ACCEPT:
			kdDebug(14100) << " GG_EVENT_DCC_CLIENT_ACCEPT " << endl;
			// check dccsock->peer_uin, if unknown, fuck it;
			
			// is it for us ?

			account = gaduDCC_->account( dccSock_->uin );
			if ( !account ) {
				kdDebug( 14100 ) << " this dcc transaction is for uin " << dccSock_->uin << ", which is not quite for me... closing"  << endl;
				// unknown 'to' ?, we're off
				gg_free_event( dccEvent );
				closeDCC();
				deleteLater();
				return;
			}
			
			if ( !peer ) {
				contact = static_cast<GaduContact*> (account->contacts()[ QString::number( dccSock_->peer_uin ) ]);
			}
			else {
				contact = static_cast<GaduContact*> (account->contacts()[ QString::number( peer ) ]);
			}
			
			if ( contact == NULL ) {
				// refusing, contact on the list
				kdDebug(14100) << " dcc connection from " << dccSock_->peer_uin << " refused, UIN not on the list " <<endl;
				gg_free_event( dccEvent );
				closeDCC();
				// emit error
				deleteLater();
				return;
			} 
			else {
				// ask user to accept that transfer
				kdDebug(14100) <<  " dcc accepted from " << dccSock_->peer_uin << endl;
			}

			break;
		case GG_EVENT_NONE:
			kdDebug(14100) << " GG_EVENT_NONE" << endl;
			if ( transfer_ ) {
				transfer_->slotProcessed( dccSock_->offset );
			}
			// update gui with progress
			break;
		
		case GG_EVENT_DCC_NEED_FILE_ACK:
			kdDebug(14100) << " GG_EVENT_DCC_NEED_FILE_ACK " << endl;
			gg_free_event( dccEvent );
			askIncommingTransfer();
			return;
			break;
		
		case GG_EVENT_DCC_ERROR:
			kdDebug(14100) << " GG_EVENT_DCC_ERROR :" << dccEvent->event.dcc_error  << endl;
			if ( transfer_ ) {
				switch( dccEvent->event.dcc_error ) {
					
					case GG_ERROR_DCC_REFUSED:
						transfer_->setError( KopeteTransfer::Refused );
					break;
					
					case GG_ERROR_DCC_EOF:
						transfer_->setError( KopeteTransfer::CanceledRemote );
					break;
					
					case GG_ERROR_DCC_HANDSHAKE:
					case GG_ERROR_DCC_FILE:
					case GG_ERROR_DCC_NET:
					default:
						transfer_->setError( KopeteTransfer::Other );
					break;
				}
			}
			gg_free_event( dccEvent );
			closeDCC();
			deleteLater();
			return;
			
		case GG_EVENT_DCC_DONE:
			if ( transfer_ ) {
				transfer_->slotComplete();
			}
			closeDCC();
			deleteLater();
			return;
			
		default:
			kdDebug(14100) << "unknown/unhandled DCC EVENT: " << dccEvent->type << endl;
			break;
	}

	if ( dccEvent ) {
		gg_free_event( dccEvent );
	}

	enableNotifiers( dccSock_->check );
}

#include "gadudcctransaction.moc"
