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

#include "jabberfiletransfer.h"
#include <QBuffer>
#include <QTimer>
#include <kdebug.h>
#include <im.h>
#include <xmpp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kcodecs.h>
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
#include "xmpp_tasks.h"

JabberFileTransfer::JabberFileTransfer ( JabberAccount *account, XMPP::FileTransfer *incomingTransfer )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "New incoming transfer from " << incomingTransfer->peer().full () << ", filename " << incomingTransfer->fileName () << ", size " << QString::number ( incomingTransfer->fileSize () );

	mAccount = account;
	mXMPPTransfer = incomingTransfer;

	// try to locate an exact match in our pool first
	mContact = mAccount->contactPool()->findExactMatch ( mXMPPTransfer->peer () );

	if ( !mContact )
	{
		// we have no exact match, try a broader search
		mContact = mAccount->contactPool()->findRelevantRecipient ( mXMPPTransfer->peer () );
	}

	if ( !mContact )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "No matching local contact found, creating a new one.";

		Kopete::MetaContact *metaContact = new Kopete::MetaContact ();

		metaContact->setTemporary (true);

		mContact = mAccount->contactPool()->addContact ( mXMPPTransfer->peer (), metaContact, false );

		Kopete::ContactList::self ()->addMetaContact ( metaContact );
	}

	connect ( Kopete::TransferManager::transferManager (), SIGNAL (accepted(Kopete::Transfer*,QString)),
			  this, SLOT (slotIncomingTransferAccepted(Kopete::Transfer*,QString)) );
	connect ( Kopete::TransferManager::transferManager (), SIGNAL (refused(Kopete::FileTransferInfo)),
			  this, SLOT (slotTransferRefused(Kopete::FileTransferInfo)) );

	initializeVariables ();

	if (!mXMPPTransfer->thumbnail().isNull()) {
		JT_BitsOfBinary *task = new JT_BitsOfBinary ( mAccount->client()->rootTask() );
		connect ( task, SIGNAL(finished()), this, SLOT(slotThumbnailReceived()) );
		task->get ( mXMPPTransfer->peer(), QString(mXMPPTransfer->thumbnail().data) );
		task->go ( true );
		QTimer::singleShot ( 5000, this, SLOT(askIncomingTransfer()) ); // Wait for thumbnail max 5s
	} else {
		askIncomingTransfer ();
	}
}

void JabberFileTransfer::slotThumbnailReceived ()
{
	JT_BitsOfBinary *task = static_cast<JT_BitsOfBinary *> ( sender() );
	askIncomingTransfer ( task->data().data() );
}

void JabberFileTransfer::askIncomingTransfer ( const QByteArray &thumbnail )
{
	if (mTransferId != -1)
		return;

	QPixmap preview;
	if (!thumbnail.isNull())
	{
		preview.loadFromData ( thumbnail );
	}

	mTransferId = Kopete::TransferManager::transferManager()->askIncomingTransfer ( mContact,
																				  mXMPPTransfer->fileName (),
																				  mXMPPTransfer->fileSize (),
																				  mXMPPTransfer->description () ,
																				QString(),
																				  preview);

}

JabberFileTransfer::JabberFileTransfer ( JabberAccount *account, JabberBaseContact *contact, const QString &file )
{
	kDebug(JABBER_DEBUG_GLOBAL) << "New outgoing transfer for " << contact->contactId() << ": " << file;

	mAccount = account;
	mContact = contact;
	mLocalFile.setFileName ( file );
	bool canOpen=mLocalFile.open ( QIODevice::ReadOnly );
	
	mKopeteTransfer = Kopete::TransferManager::transferManager()->addTransfer ( contact,
																			  mLocalFile.fileName (),
																			  mLocalFile.size (),
																			  contact->contactId (),
																			  Kopete::FileTransferInfo::Outgoing );


	connect ( mKopeteTransfer, SIGNAL (result(KJob*)), this, SLOT (slotTransferResult()) );

	mXMPPTransfer = mAccount->client()->fileTransferManager()->createTransfer ();

	initializeVariables ();

	connect ( mXMPPTransfer, SIGNAL (connected()), this, SLOT (slotOutgoingConnected()) );
	connect ( mXMPPTransfer, SIGNAL (bytesWritten(qint64)), this, SLOT (slotOutgoingBytesWritten(qint64)) );
	connect ( mXMPPTransfer, SIGNAL (error(int)), this, SLOT (slotTransferError(int)) );
	
	FTThumbnail preview;
	QImage img=QImage(mLocalFile.fileName());
	if(!img.isNull())
	{
		img=img.scaled(64,64,Qt::KeepAspectRatio);
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		img.save(&buffer, "PNG"); // writes image into ba in PNG format
		preview= FTThumbnail(ba,QString("image/png"),img.width(),img.height());
	}

	  
	if(canOpen) {
		mXMPPTransfer->sendFile ( XMPP::Jid ( contact->fullAddress () ), KUrl(file).fileName (), mLocalFile.size (), "", preview);
	} else {
		mKopeteTransfer->slotError ( KIO::ERR_CANNOT_OPEN_FOR_READING, file );
	}

}

JabberFileTransfer::~JabberFileTransfer ()
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Destroying Jabber file transfer object.";

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

	kDebug(JABBER_DEBUG_GLOBAL) << "Accepting transfer for " << mXMPPTransfer->peer().full ();

	mKopeteTransfer = transfer;
	mLocalFile.setFileName ( fileName );

	bool couldOpen = false;
	qlonglong offset = 0;
	qlonglong length = 0;

	mBytesTransferred = 0;
	mBytesToTransfer = mXMPPTransfer->fileSize ();

	if ( mXMPPTransfer->rangeSupported () && mLocalFile.exists () )
	{
		KGuiItem resumeButton ( i18n ( "&Resume" ) );
		KGuiItem overwriteButton ( i18n ( "Over&write" ) );

		switch ( KMessageBox::questionYesNoCancel ( Kopete::UI::Global::mainWidget (),
													i18n ( "The file %1 already exists, do you want to resume or overwrite it?", fileName ),
													i18n ( "File Exists: %1", fileName ),
													resumeButton, overwriteButton ) )
		{
			case KMessageBox::Yes:		// resume
										couldOpen = mLocalFile.open ( QIODevice::ReadWrite );
										if ( couldOpen )
										{
											offset = mLocalFile.size ();
											length = mXMPPTransfer->fileSize () - offset;
											mBytesTransferred = offset;
											mBytesToTransfer = length;
											mLocalFile.seek ( mLocalFile.size () );
										}
										break;

			case KMessageBox::No:		// overwrite
										couldOpen = mLocalFile.open ( QIODevice::WriteOnly );
										break;

			default:					// cancel
										deleteLater ();
										return;
		}
	}
	else
	{
		// overwrite by default
		couldOpen = mLocalFile.open ( QIODevice::WriteOnly );
	}

	if ( !couldOpen )
	{
		transfer->slotError ( KIO::ERR_COULD_NOT_WRITE, fileName );

		deleteLater ();
	}
	else
	{
		connect ( mKopeteTransfer, SIGNAL (result(KJob*)), this, SLOT (slotTransferResult()) );
		connect ( mXMPPTransfer, SIGNAL (readyRead(QByteArray)), this, SLOT (slotIncomingDataReady(QByteArray)) );
		connect ( mXMPPTransfer, SIGNAL (error(int)), this, SLOT (slotTransferError(int)) );
		mXMPPTransfer->accept ( offset, length );
	}

}

void JabberFileTransfer::slotTransferRefused ( const Kopete::FileTransferInfo &transfer )
{

	if ( (long)transfer.transferId () != mTransferId )
		return;

	kDebug(JABBER_DEBUG_GLOBAL) << "Local user refused transfer from " << mXMPPTransfer->peer().full ();

	deleteLater ();

}

void JabberFileTransfer::slotTransferResult ()
{

	if ( mKopeteTransfer->error () == KIO::ERR_USER_CANCELED )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Transfer with " << mXMPPTransfer->peer().full () << " has been canceled.";
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

	mLocalFile.write ( data );

	if ( mBytesToTransfer <= 0 )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Transfer from " << mXMPPTransfer->peer().full () << " done.";

		mKopeteTransfer->slotComplete ();

		deleteLater ();
	}

}

void JabberFileTransfer::slotOutgoingConnected ()
{

	kDebug ( JABBER_DEBUG_GLOBAL ) << "Outgoing data connection is open.";

	mBytesTransferred = mXMPPTransfer->offset ();
	mLocalFile.seek ( mXMPPTransfer->offset () );
	mBytesToTransfer = ( mXMPPTransfer->fileSize () > mXMPPTransfer->length () ) ? mXMPPTransfer->length () : mXMPPTransfer->fileSize ();

	slotOutgoingBytesWritten ( 0 );

}

void JabberFileTransfer::slotOutgoingBytesWritten ( qint64 nrWritten )
{

	mBytesTransferred += nrWritten;
	mBytesToTransfer -= nrWritten;

	mKopeteTransfer->slotProcessed ( mBytesTransferred );

	if ( mBytesToTransfer )
	{
		int nrToWrite = mXMPPTransfer->dataSizeNeeded ();

		QByteArray readBuffer ( nrToWrite , '\0' );

		mLocalFile.read ( readBuffer.data (), nrToWrite );

		mXMPPTransfer->writeFileData ( readBuffer );
	}
	else
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Transfer to " << mXMPPTransfer->peer().full () << " done.";

		mKopeteTransfer->slotComplete ();

		deleteLater ();
	}

}

#include "jabberfiletransfer.moc"
