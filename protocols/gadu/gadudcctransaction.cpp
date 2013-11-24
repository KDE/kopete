// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadudcctransaction.cpp
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

#include "gadudcctransaction.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetetransfermanager.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"

#include <qsocketnotifier.h>
#include <qfile.h>

#include "gaducontact.h"
#include "gaduaccount.h"
#include "gadudcc.h"

#include "libgadu.h"

GaduDCCTransaction::GaduDCCTransaction( GaduDCC* parent )
:QObject( parent ), gaduDCC_( parent )
{
	read_		= NULL;
	write_		= NULL;
	contact		= NULL;
	transfer_	= NULL;
	dccSock_	= NULL;
	peer		= 0;
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
GaduDCCTransaction::setupOutgoing( GaduContact* peerContact, QString& filePath )
{
	GaduContact* me;
	GaduAccount* metoo;

	if ( !peerContact ) {
		return false;
	}

	me = static_cast<GaduContact*>( peerContact->account()->myself() );

	QString aaa =  peerContact->contactIp().toString();
	kDebug( 14100 ) << "slotOutgoin for UIN: " << peerContact->uin() << " port " << peerContact->contactPort() << " ip " <<aaa;
	kDebug( 14100 ) << "File path is " << filePath;

	if ( peerContact->contactPort() >= 10 ) {  
		dccSock_ = gg_dcc_send_file( htonl( peerContact->contactIp().toIPv4Address() ), peerContact->contactPort(), me->uin(), peerContact->uin() );
		gg_dcc_fill_file_info(dccSock_,filePath.toAscii());
		transfer_ = Kopete::TransferManager::transferManager()->addTransfer ( peerContact,
		filePath,  dccSock_->file_info.size, peerContact->metaContact()->displayName(),	Kopete::FileTransferInfo::Outgoing );
		createNotifiers( true );
		enableNotifiers( dccSock_->check  );
	}
	else {
		kDebug( 14100 ) << "Peer " << peerContact->uin() << " is passive, requesting reverse connection";
		metoo = static_cast<GaduAccount*>( me->account() );
		gaduDCC_->requests[peerContact->uin()]=filePath;
		metoo->dccRequest( peerContact );
	}

	return false;
}

bool
GaduDCCTransaction::setupIncoming( const unsigned int uin, GaduContact* peerContact )
{

	if ( !peerContact ) {
		kDebug( 14100 ) << "setupIncoming called with peerContact == NULL ";
		return false;
		return false;
	}

	QString aaa =  peerContact->contactIp().toString();
	kDebug( 14100 ) << "setupIncoming for UIN: " << uin << " port " << peerContact->contactPort() << " ip " <<aaa;

	peer = peerContact->uin();
	dccSock_ = gg_dcc_get_file( htonl( peerContact->contactIp().toIPv4Address() ), peerContact->contactPort(), uin, peer );

	contact = peerContact;
	return setupIncoming( dccSock_ );

}

bool
GaduDCCTransaction::setupIncoming( gg_dcc* dccS )
{
	if ( !dccS ) {
		kDebug(14100) << "gg_dcc_get_file failed in GaduDCCTransaction::setupIncoming";
		return false;
	}

	dccSock_ = dccS;

	peer = dccS->uin;
	
	connect ( Kopete::TransferManager::transferManager(), SIGNAL(accepted(Kopete::Transfer*,QString)),
			  this, SLOT(slotIncomingTransferAccepted(Kopete::Transfer*,QString)) );
	connect ( Kopete::TransferManager::transferManager(), SIGNAL(refused(Kopete::FileTransferInfo)),
			  this, SLOT(slotTransferRefused(Kopete::FileTransferInfo)) );

	incoming = true;
	createNotifiers( true );
	enableNotifiers( dccSock_->check  );

	return true;
}


void
GaduDCCTransaction::closeDCC()
{
	kDebug(14100) << "closeDCC()";

	disableNotifiers();
	destroyNotifiers();
	gg_dcc_free( dccSock_ );
	dccSock_ = NULL;
}

void
GaduDCCTransaction::destroyNotifiers()
{
	disableNotifiers();
	delete read_;
	read_ = NULL;
	delete write_;
	write_ = NULL;
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
		QObject::connect( read_, SIGNAL(activated(int)), SLOT(watcher()) );
		QObject::connect( write_, SIGNAL(activated(int)), SLOT(watcher()) );
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
GaduDCCTransaction::slotIncomingTransferAccepted ( Kopete::Transfer* transfer, const QString& fileName )
{

	if ( (long)transfer->info().transferId () != transferId_ ) {
		return;
	}

	transfer_ = transfer;
	localFile_.setFileName( fileName );

	if ( localFile_.exists() ) {
		KGuiItem resumeButton( i18n ( "&Resume" ) );
		KGuiItem overwriteButton( i18n ( "Over&write" ) );
		switch ( KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget (),
							i18n( "The file %1 already exists, do you want to resume or overwrite it?", fileName ),
							i18n( "File Exists: %1", fileName ), resumeButton, overwriteButton ) )
		{
			// resume
			case KMessageBox::Yes:
				if ( localFile_.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
					dccSock_->offset  = localFile_.size();
					dccSock_->file_fd = localFile_.handle();
				}
			break;
			// overwrite
			case KMessageBox::No:
				if ( localFile_.open( QIODevice::ReadWrite ) ) {
					dccSock_->offset  = 0;
					dccSock_->file_fd = localFile_.handle();
				}
			break;

			// cancel
			default:
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
		if ( localFile_.open( QIODevice::ReadWrite ) == false ) {
			transfer->slotError ( KIO::ERR_COULD_NOT_WRITE, fileName );
			closeDCC();
			deleteLater ();
			return;
		}
		dccSock_->offset  = 0;
		dccSock_->file_fd = localFile_.handle();
	}

	connect ( transfer, SIGNAL(result(KJob*)), this, SLOT(slotTransferResult()) );

	// reenable notifiers
	enableNotifiers( dccSock_->check );

}

void
GaduDCCTransaction::slotTransferResult()
{
	if ( transfer_->error() == KIO::ERR_USER_CANCELED ) {
		closeDCC();
		deleteLater();
	}
}

void
GaduDCCTransaction::slotTransferRefused ( const Kopete::FileTransferInfo&  transfer )
{
	if ( (long)transfer.transferId () != transferId_ )
		return;
	closeDCC();
	deleteLater();
}

void
GaduDCCTransaction::askIncommingTransfer()
{

	transferId_ = Kopete::TransferManager::transferManager()->askIncomingTransfer ( contact,
		QString( (const char*)dccSock_->file_info.filename ),  dccSock_->file_info.size );

}

void
GaduDCCTransaction::watcher() {

	gg_event* dccEvent;
	GaduAccount* account;

	disableNotifiers();

	dccEvent = gg_dcc_watch_fd( dccSock_ );
	if ( ! dccEvent ) {
		// connection is fucked
		closeDCC();
		return;
	}
	switch ( dccEvent->type ) {
		case GG_EVENT_DCC_CLIENT_ACCEPT:
			kDebug(14100) << " GG_EVENT_DCC_CLIENT_ACCEPT ";
			// check dccsock->peer_uin, if unknown, fuck it;

			// is it for us ?
			account = gaduDCC_->account( dccSock_->uin );
			if ( !account ) {
				kDebug( 14100 ) << " this dcc transaction is for uin " << dccSock_->uin << ", which is not quite for me... closing";
				// unknown 'to' ?, we're off
				gg_free_event( dccEvent );
				closeDCC();
				deleteLater();
				return;
			}

			if ( !peer ) {
				contact = static_cast<GaduContact*> (account->contacts().value( QString::number( dccSock_->peer_uin ) ));
			}
			else {
				contact = static_cast<GaduContact*> (account->contacts().value( QString::number( peer ) ));
			}

			if ( contact == NULL ) {
				// refusing, contact on the list
				kDebug(14100) << " dcc connection from " << dccSock_->peer_uin << " refused, UIN not on the list ";
				gg_free_event( dccEvent );
				closeDCC();
				// emit error
				deleteLater();
				return;
			}
			else {
				// ask user to accept that transfer
				kDebug(14100) <<  " dcc accepted from " << dccSock_->peer_uin;
			}

			break;
		case GG_EVENT_DCC_CALLBACK:
			kDebug(14100) << "GG_DCC_EVENT_CALLBACK";
			break;
		case GG_EVENT_NONE:
			kDebug(14100) << " GG_EVENT_NONE";
			// update gui with progress
			if ( transfer_ ) {
				transfer_->slotProcessed( dccSock_->offset );
			}
			break;

		case GG_EVENT_DCC_NEED_FILE_ACK:
			kDebug(14100) << " GG_EVENT_DCC_NEED_FILE_ACK ";
			gg_free_event( dccEvent );
			askIncommingTransfer();
			return;
			break;
		case GG_EVENT_DCC_NEED_FILE_INFO:
			if (gaduDCC_->requests.contains(dccSock_->peer_uin)) {
			    QString filePath = gaduDCC_->requests[dccSock_->peer_uin];
			    kDebug() << "Callback request found. Sending " << filePath;
			    gaduDCC_->requests.remove(dccSock_->peer_uin);
		    	    gg_dcc_fill_file_info(dccSock_,filePath.toAscii());
			    transfer_ = Kopete::TransferManager::transferManager()->addTransfer ( contact,
			    filePath,  dccSock_->file_info.size, contact->metaContact()->displayName(),	Kopete::FileTransferInfo::Outgoing );
			} else {
				gg_free_event( dccEvent );
				closeDCC();
				deleteLater();
				return;
			}
			break;

		case GG_EVENT_DCC_ERROR:
			kDebug(14100) << " GG_EVENT_DCC_ERROR :" << dccEvent->event.dcc_error;
			if ( transfer_ ) {
				switch( dccEvent->event.dcc_error ) {

					case GG_ERROR_DCC_REFUSED:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "Connection to peer was refused; it possibly does not listen for incoming connections." ) );
					break;

					case GG_ERROR_DCC_EOF:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "File transfer transaction was not agreed by peer." ) );
					break;

					case GG_ERROR_DCC_HANDSHAKE:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "File-transfer handshake failure." ) );
					break;
					case GG_ERROR_DCC_FILE:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "File transfer had problems with the file." ) );
					break;
					case GG_ERROR_DCC_NET:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "There was network error during file transfer." ) );
					break;
					default:
						transfer_->slotError( KIO::ERR_SLAVE_DEFINED, i18n( "Unknown File-Transfer error." ) );
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
			if ( dccEvent )
				gg_free_event( dccEvent );
			return;

		default:
			kDebug(14100) << "unknown/unhandled DCC EVENT: " << dccEvent->type;
			break;
	}

	if ( dccEvent ) {
		gg_free_event( dccEvent );
	}

	enableNotifiers( dccSock_->check );
}

#include "gadudcctransaction.moc"
