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
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetetransfermanager.h"
#include "jabberaccount.h"
#include "jabberprotocol.h"
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

		KopeteMetaContact *metaContact = new KopeteMetaContact ();

		metaContact->setTemporary (true);

		contact = mAccount->contactPool()->addContact ( mXMPPTransfer->peer (), metaContact, false );

		KopeteContactList::contactList ()->addMetaContact ( metaContact );
	}

	connect ( KopeteTransferManager::transferManager (), SIGNAL ( accepted ( KopeteTransfer *, const QString & ) ),
			  this, SLOT ( slotIncomingTransferAccepted ( KopeteTransfer *, const QString & ) ) );
	connect ( KopeteTransferManager::transferManager (), SIGNAL ( refused ( const KopeteFileTransferInfo & ) ),
			  this, SLOT ( slotTransferRefused ( const KopeteFileTransferInfo & ) ) );

	initializeVariables ();

	mTransferId = KopeteTransferManager::transferManager()->askIncomingTransfer ( contact,
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

	mKopeteTransfer = KopeteTransferManager::transferManager()->addTransfer ( contact,
																			  mLocalFile.name (),
																			  mLocalFile.size (),
																			  contact->contactId (),
																			  KopeteFileTransferInfo::Outgoing );

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
	mXMPPTransfer->setProxy ( XMPP::Jid ( mAccount->pluginData ( mAccount->protocol (), "ProxyJID" ) ) );

}

void JabberFileTransfer::slotIncomingTransferAccepted ( KopeteTransfer *transfer, const QString &fileName )
{

	if ( (long)transfer->info().transferId () != mTransferId )
		return;

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Accepting transfer for " << mXMPPTransfer->peer().full () << endl;

	mKopeteTransfer = transfer;
	mLocalFile.setName ( fileName );

	if ( !mLocalFile.open ( IO_WriteOnly ) )
	{
		transfer->slotError ( KopeteTransfer::Other, i18n("Could not write to the local file.") );

		deleteLater ();
	}
	else
	{
	 	connect ( mKopeteTransfer, SIGNAL ( result ( KIO::Job * ) ), this, SLOT ( slotTransferResult () ) );
		connect ( mXMPPTransfer, SIGNAL ( readyRead ( const QByteArray& ) ), this, SLOT ( slotIncomingDataReady ( const QByteArray & ) ) );
		connect ( mXMPPTransfer, SIGNAL ( error ( int ) ), this, SLOT ( slotTransferError ( int ) ) );
		mXMPPTransfer->accept ();
	}

}

void JabberFileTransfer::slotTransferRefused ( const KopeteFileTransferInfo &transfer )
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
			mKopeteTransfer->slotError ( KIO::ERR_ACCESS_DENIED,
										 // "Access denied to"...
										 i18n("The user %1 rejected the transfer request.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrNeg:
			mKopeteTransfer->slotError ( KIO::ERR_COULD_NOT_LOGIN,
										 i18n("Unable to negotiate a suitable connection for the file transfer with %1.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrConnect:
			mKopeteTransfer->slotError ( KIO::ERR_COULD_NOT_CONNECT,
										 i18n("Could not connect to %1.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrStream:
			mKopeteTransfer->slotError ( KIO::ERR_CONNECTION_BROKEN,
										 i18n("The data stream with %1 was disrupted. (probably cancelled)").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		default:
			mKopeteTransfer->slotError ( KIO::ERR_UNKNOWN,
										 i18n("Unknown error while transferring a file with %1.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;
	}

	deleteLater ();

}

void JabberFileTransfer::slotIncomingDataReady ( const QByteArray &data )
{

	mBytesTransferred += data.size ();

	mKopeteTransfer->slotProcessed ( mBytesTransferred );

	mLocalFile.writeBlock ( data );

	if ( mBytesTransferred >= mXMPPTransfer->fileSize () )
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
