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
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "New incoming transfer from " << incomingTransfer->peer().full () << endl;

	mAccount = account;
	mXMPPTransfer = incomingTransfer;
	mTransferId = -1;

	mBytesTransferred = 0;

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

	mTransferId = KopeteTransferManager::transferManager()->askIncomingTransfer ( contact,
																				  mXMPPTransfer->fileName (),
																				  mXMPPTransfer->fileSize () );

}

JabberFileTransfer::JabberFileTransfer ( JabberAccount *account, JabberBaseContact *contact, const QString &file )
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "New outgoing transfer for " << contact->contactId() << ": " << file << endl;

	mAccount = account;
	mLocalFile.setName ( file );
	mLocalFile.open ( IO_ReadOnly );
	mBytesTransferred = 0;

	mKopeteTransfer = KopeteTransferManager::transferManager()->addTransfer ( contact,
																			  mLocalFile.name (),
																			  mLocalFile.size (),
																			  contact->contactId (),
																			  KopeteFileTransferInfo::Outgoing );

 	connect ( mKopeteTransfer, SIGNAL ( transferCanceled () ), this, SLOT ( slotTransferCanceled () ) );

	mXMPPTransfer = mAccount->client()->fileTransferManager()->createTransfer ();

	mXMPPTransfer->setProxy ( XMPP::Jid ( account->pluginData ( account->protocol (), "ProxyJID" ) ) );

	connect ( mXMPPTransfer, SIGNAL ( connected () ), this, SLOT ( slotOutgoingConnected () ) );
	connect ( mXMPPTransfer, SIGNAL ( bytesWritten ( int ) ), this, SLOT ( slotOutgoingBytesWritten ( int ) ) );
	connect ( mXMPPTransfer, SIGNAL ( error ( int ) ), this, SLOT ( slotTransferError ( int ) ) );

	mXMPPTransfer->sendFile ( XMPP::Jid ( contact->fullAddress () ), KURL(file).fileName (), mLocalFile.size () );

}

JabberFileTransfer::~JabberFileTransfer ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Destroying Jabber file transfer object." << endl;

	mLocalFile.close ();

	mXMPPTransfer->close ();
	delete mXMPPTransfer;

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

void JabberFileTransfer::slotTransferCanceled ()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Transfer with " << mXMPPTransfer->peer().full () << " has been canceled." << endl;

	deleteLater ();

}

void JabberFileTransfer::slotTransferError ( int errorCode )
{

	switch ( errorCode )
	{
		case XMPP::FileTransfer::ErrReject:
			mKopeteTransfer->slotError ( KopeteTransfer::Refused,
										 i18n("The user %1 rejected the transfer request.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrNeg:
			mKopeteTransfer->slotError ( KopeteTransfer::Other,
										 i18n("Unable to negotiate a suitable connection for the file transfer with %1.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrConnect:
			mKopeteTransfer->slotError ( KopeteTransfer::Other,
										 i18n("Could not connect to %1.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		case XMPP::FileTransfer::ErrStream:
			mKopeteTransfer->slotError ( KopeteTransfer::Other,
										 i18n("The data stream with %1 was disrupted.").
										 arg(mXMPPTransfer->peer().full ()) );
			break;

		default:
			mKopeteTransfer->slotError ( KopeteTransfer::Other,
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

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Received " << data.size () << " bytes from " << mXMPPTransfer->peer().full () << ", now at a total of " << mBytesTransferred << endl;

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

	slotOutgoingBytesWritten ( 0 );

}

void JabberFileTransfer::slotOutgoingBytesWritten ( int nrWritten )
{

	mBytesTransferred += nrWritten;

	mKopeteTransfer->slotProcessed ( mBytesTransferred );

	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Wrote " << nrWritten << " bytes to " << mXMPPTransfer->peer().full () << " in this pass, now at a total of " << mBytesTransferred << endl;

	if ( mBytesTransferred < mXMPPTransfer->fileSize () )
	{
		int nrToWrite = mXMPPTransfer->dataSizeNeeded ();

		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Sending " << nrToWrite << " bytes to " << mXMPPTransfer->peer().full () << endl;

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
