 /*
  * jabberfiletransfer.cpp
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include <kdebug.h>
#include <im.h>
#include <xmpp.h>
#include "jabberfiletransfer.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetetransfermanager.h"
#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"
#include "jabbercontactpool.h"
#include "jabberbasecontact.h"
#include "jabbercontact.h"

JabberFileTransfer::JabberFileTransfer ( JabberAccount *account, XMPP::FileTransfer *incomingTransfer )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "New incoming transfer from " << incomingTransfer->peer().full () << ", filename " << incomingTransfer->fileName () << ", size " << QString::number ( incomingTransfer->fileSize () ) << endl;

	mAccount = account;
	mXMPPTransfer = incomingTransfer;

	// try to locate an exact match in our pool first
	JabberBaseContact *contact = mAccount->contactPool()->findExactMatch ( mXMPPTransfer->peer () );

	if ( !contact )
	{
		// we have no exact match, try a broader search
		contact = mAccount->contactPool()->findRelevantRecipient ( mXMPPTransfer->peer () );
	}

	if ( !contact )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "No matching local contact found, creating a new one." << endl;

		Kopete::MetaContact *metaContact = new Kopete::MetaContact ();

		metaContact->setTemporary (true);

		contact = mAccount->contactPool()->addContact ( mXMPPTransfer->peer (), metaContact, false );

		Kopete::ContactList::self ()->addMetaContact ( metaContact );
	}

	connect ( Kopete::TransferManager::transferManager (), SIGNAL ( accepted ( Kopete::Transfer *, const QString & ) ),
			  this, SLOT ( slotIncomingTransferAccepted ( Kopete::Transfer *, const QString & ) ) );
	connect ( Kopete::TransferManager::transferManager (), SIGNAL ( refused ( const Kopete::FileTransferInfo & ) ),
			  this, SLOT ( slotTransferRefused ( const Kopete::FileTransferInfo & ) ) );

	initializeVariables ();

	mTransferId = Kopete::TransferManager::transferManager()->askIncomingTransfer ( contact,
																				  mXMPPTransfer->fileName (),
																				  mXMPPTransfer->fileSize (),
																				  mXMPPTransfer->description () );

}

JabberFileTransfer::JabberFileTransfer ( JabberAccount *account, JabberBaseContact *contact, const QString &file )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "New outgoing transfer for " << contact->contactId() << ": " << file << endl;

	mAccount = account;
	mLocalFile.setName ( file );
	mLocalFile.open ( IO_ReadOnly );

	mKopeteTransfer = Kopete::TransferManager::transferManager()->addTransfer ( contact,
																			  mLocalFile.name (),
																			  mLocalFile.size (),
																			  contact->contactId (),
																			  Kopete::FileTransferInfo::Outgoing );

 	connect ( mKopeteTransfer, SIGNAL ( result ( KIO::Job * ) ), this, SLOT ( slotTransferResult () ) );

	mXMPPTransfer = mAccount->client()->fileTransferManager()->createTransfer ();

	initializeVariables ();

	connect ( mXMPPTransfer, SIGNAL ( connected () ), this, SLOT ( slotOutgoingConnected () ) );
	connect ( mXMPPTransfer, SIGNAL ( bytesWritten ( int ) ), this, SLOT ( slotOutgoingBytesWritten ( int ) ) );
	connect ( mXMPPTransfer, SIGNAL ( error ( int ) ), this, SLOT ( slotTransferError ( int ) ) );

	mXMPPTransfer->sendFile ( XMPP::Jid ( contact->fullAddress () ), KURL(file).fileName (), mLocalFile.size (), "" );

}

JabberFileTransfer::~JabberFileTransfer ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Destroying Jabber file transfer object." << endl;

	mLocalFile.close ();

	mXMPPTransfer->close ();
	delete mXMPPTransfer;

}

void JabberFileTransfer::initializeVariables ()
{

	mTransferId = -1;
	mBytesTransferred = 0;
	mBytesToTransfer = 0;
	mXMPPTransfer->setProxy ( XMPP::Jid ( mAccount->configGroup()->readEntry ( "ProxyJID" ) ) );

}

void JabberFileTransfer::slotIncomingTransferAccepted ( Kopete::Transfer *transfer, const QString &fileName )
{

	if ( (long)transfer->info().transferId () != mTransferId )
		return;

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Accepting transfer for " << mXMPPTransfer->peer().full () << endl;

	mKopeteTransfer = transfer;
	mLocalFile.setName ( fileName );

	bool couldOpen = false;
	Q_LLONG offset = 0;
	Q_LLONG length = 0;

	mBytesTransferred = 0;
	mBytesToTransfer = mXMPPTransfer->fileSize ();

	if ( mXMPPTransfer->rangeSupported () && mLocalFile.exists () )
	{
		KGuiItem resumeButton ( i18n ( "&Resume" ) );
		KGuiItem overwriteButton ( i18n ( "Over&write" ) );

		switch ( KMessageBox::questionYesNoCancel ( Kopete::UI::Global::mainWidget (),
													i18n ( "The file %1 already exists, do you want to resume or overwrite it?" ).arg ( fileName ),
													i18n ( "File Exists: %1" ).arg ( fileName ),
													resumeButton, overwriteButton ) )
		{
			case KMessageBox::Yes:		// resume
										couldOpen = mLocalFile.open ( IO_ReadWrite );
										if ( couldOpen )
										{
											offset = mLocalFile.size ();
											length = mXMPPTransfer->fileSize () - offset;
											mBytesTransferred = offset;
											mBytesToTransfer = length;
											mLocalFile.at ( mLocalFile.size () );
										}
										break;

			case KMessageBox::No:		// overwrite
										couldOpen = mLocalFile.open ( IO_WriteOnly );
										break;

			default:					// cancel
										deleteLater ();
										return;
		}
	}
	else
	{
		// overwrite by default
		couldOpen = mLocalFile.open ( IO_WriteOnly );
	}

	if ( !couldOpen )
	{
		transfer->slotError ( KIO::ERR_COULD_NOT_WRITE, fileName );

		deleteLater ();
	}
	else
	{
		connect ( mKopeteTransfer, SIGNAL ( result ( KIO::Job * ) ), this, SLOT ( slotTransferResult () ) );
		connect ( mXMPPTransfer, SIGNAL ( readyRead ( const QByteArray& ) ), this, SLOT ( slotIncomingDataReady ( const QByteArray & ) ) );
		connect ( mXMPPTransfer, SIGNAL ( error ( int ) ), this, SLOT ( slotTransferError ( int ) ) );
		mXMPPTransfer->accept ( offset, length );
	}

}

void JabberFileTransfer::slotTransferRefused ( const Kopete::FileTransferInfo &transfer )
{

	if ( (long)transfer.transferId () != mTransferId )
		return;

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Local user refused transfer from " << mXMPPTransfer->peer().full () << endl;

	deleteLater ();

}

void JabberFileTransfer::slotTransferResult ()
{

	if ( mKopeteTransfer->error () == KIO::ERR_USER_CANCELED )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Transfer with " << mXMPPTransfer->peer().full () << " has been canceled." << endl;
		mXMPPTransfer->close ();
		deleteLater ();
	}

}

void JabberFileTransfer::slotTransferError ( int errorCode )
{

	switch ( errorCode )
	{
		case XMPP::FileTransfer::ErrReject:
			// user rejected the transfer request
			mKopeteTransfer->slotError ( KIO::ERR_ACCESS_DENIED,
										 mXMPPTransfer->peer().full () );
			break;

		case XMPP::FileTransfer::ErrNeg:
			// unable to negotiate a suitable connection for the file transfer with the user
			mKopeteTransfer->slotError ( KIO::ERR_COULD_NOT_LOGIN,
										 mXMPPTransfer->peer().full () );
			break;

		case XMPP::FileTransfer::ErrConnect:
			// could not connect to the user
			mKopeteTransfer->slotError ( KIO::ERR_COULD_NOT_CONNECT,
										 mXMPPTransfer->peer().full () );
			break;

		case XMPP::FileTransfer::ErrStream:
			// data stream was disrupted, probably cancelled
			mKopeteTransfer->slotError ( KIO::ERR_CONNECTION_BROKEN,
										 mXMPPTransfer->peer().full () );
			break;

		default:
			// unknown error
			mKopeteTransfer->slotError ( KIO::ERR_UNKNOWN,
										 mXMPPTransfer->peer().full () );
			break;
	}

	deleteLater ();

}

void JabberFileTransfer::slotIncomingDataReady ( const QByteArray &data )
{

	mBytesTransferred += data.size ();
	mBytesToTransfer -= data.size ();

	mKopeteTransfer->slotProcessed ( mBytesTransferred );

	mLocalFile.writeBlock ( data );

	if ( mBytesToTransfer <= 0 )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Transfer from " << mXMPPTransfer->peer().full () << " done." << endl;

		mKopeteTransfer->slotComplete ();

		deleteLater ();
	}

}

void JabberFileTransfer::slotOutgoingConnected ()
{

	kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Outgoing data connection is open." << endl;

	mBytesTransferred = mXMPPTransfer->offset ();
	mLocalFile.at ( mXMPPTransfer->offset () );
	mBytesToTransfer = ( mXMPPTransfer->fileSize () > mXMPPTransfer->length () ) ? mXMPPTransfer->length () : mXMPPTransfer->fileSize ();

	slotOutgoingBytesWritten ( 0 );

}

void JabberFileTransfer::slotOutgoingBytesWritten ( int nrWritten )
{

	mBytesTransferred += nrWritten;
	mBytesToTransfer -= nrWritten;

	mKopeteTransfer->slotProcessed ( mBytesTransferred );

	if ( mBytesToTransfer )
	{
		int nrToWrite = mXMPPTransfer->dataSizeNeeded ();

		QByteArray readBuffer ( nrToWrite );

		mLocalFile.readBlock ( readBuffer.data (), nrToWrite );

		mXMPPTransfer->writeFileData ( readBuffer );
	}
	else
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Transfer to " << mXMPPTransfer->peer().full () << " done." << endl;

		mKopeteTransfer->slotComplete ();

		deleteLater ();
	}

}

#include "jabberfiletransfer.moc"
