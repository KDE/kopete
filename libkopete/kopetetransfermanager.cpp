/***************************************************************************
                          kopetetransfermanager.cpp  -  description
                             -------------------
    begin                : Sat Aug 3 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetetransfermanager.h"
#include "kopetefileconfirmdialog.h"

#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <kdialogbase.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qfontmetrics.h>

#include <kio/observer.h>

/***************************
 *  KopeteFileTransferInfo *
 ***************************/

KopeteFileTransferInfo::KopeteFileTransferInfo(  KopeteContact *contact, const QString& file, const unsigned long size, const QString &recipient, KopeteTransferDirection di, const unsigned int id, void *internalId)
{
	mContact = contact;
	mFile = file;
	mId = id;
	mSize = size;
	mRecipient = recipient;
	m_intId= internalId;
	mDirection= di;
}

/***************************
 *     KopeteTransfer      *
 ***************************/


KopeteTransfer::KopeteTransfer( const KopeteFileTransferInfo &kfti, const QString &localFile, bool showProgressInfo)
	: KIO::Job(showProgressInfo), mInfo(kfti)
{
	KURL targ; targ.setPath( localFile );
	init( targ, showProgressInfo );
}

KopeteTransfer::KopeteTransfer( const KopeteFileTransferInfo &kfti, const KopeteContact *contact, bool showProgressInfo)
	: KIO::Job(showProgressInfo), mInfo(kfti)
{
	// TODO: use mInfo.url().fileName() after move to protocol-aware filetransfers
	KURL targ; targ.setPath( mInfo.file() );
	init( displayURL( contact, targ.fileName() ), showProgressInfo );
}

void KopeteTransfer::init( const KURL &target, bool showProgressInfo )
{
	mTarget = target;

	if( showProgressInfo )
		Observer::self()->slotCopying( this, sourceURL(), destinationURL() );

	connect( this, SIGNAL( result( KIO::Job* ) ), SLOT( slotResultEmitted() ) );

	setAutoErrorHandlingEnabled( true, 0 );
}

KopeteTransfer::~KopeteTransfer()
{
}

KURL KopeteTransfer::displayURL( const KopeteContact *contact, const QString &file )
{
	KURL url;
	url.setProtocol( QString::fromLatin1("kopete") );

	QString host;
	if( !contact )
		host = QString::fromLatin1("unknown origin");
	else if( contact->metaContact() )
		host = contact->metaContact()->displayName();
	else
		host = contact->displayName();
	url.setHost(host);

	// url.setPath( contact->protocol()->displayName() );

	url.setFileName( file );
	return url;
}

// TODO: add possibility of network file transfers;
//  call mInfo->url() not file()
KURL KopeteTransfer::sourceURL()
{
	if( mInfo.direction() == KopeteFileTransferInfo::Incoming )
		return displayURL( mInfo.contact(), mInfo.file() );
	else
	{
		KURL url; url.setPath( mInfo.file() );
		return url;
	}
}

KURL KopeteTransfer::destinationURL()
{
	return mTarget;
}

void KopeteTransfer::slotPercentCompleted(unsigned int percent)
{
	// horribly inaccurate, but scheduled to be removed.
	slotProcessed(percent * (mInfo.size() / 100));
	if( percent >= 100 )
		slotComplete();
}

void KopeteTransfer::slotProcessed(unsigned int bytes)
{
	emitPercent( bytes, mInfo.size() );
}

void KopeteTransfer::slotComplete()
{
	emitResult();
}

void KopeteTransfer::setError(KopeteTransferError kopeteError)
{
	QString errorText;
	int error;

	switch (kopeteError)
	{
		case CanceledLocal:
			errorText = i18n("Aborted");
			error = KIO::ERR_USER_CANCELED;
			break;
		case CanceledRemote:
			errorText = i18n("Remote user aborted");
			error = KIO::ERR_ABORTED;
			break;
		case Timeout:
			errorText = i18n("Connection timed out");
			error = KIO::ERR_CONNECTION_BROKEN;
			break;
		default:
			errorText = i18n("Unknown error occurred");
			error = KIO::ERR_UNKNOWN;
			break;
	}

	slotError( error, errorText );
}

void KopeteTransfer::slotError( int error, const QString &errorText )
{
	m_error = error;
	m_errorText = errorText;

	emitResult();
}

void KopeteTransfer::slotResultEmitted()
{
	if( error() == KIO::ERR_USER_CANCELED )
		emit transferCanceled();
}

/***************************
 *  KopeteTransferManager  *
 ***************************/

KopeteTransferManager* KopeteTransferManager::transferManager()
{
	static KopeteTransferManager transferManager(0);

	return &transferManager;
}

KopeteTransferManager::KopeteTransferManager( QObject *parent ) : QObject( parent )
{
	nextID = 0;
}

KopeteTransfer* KopeteTransferManager::addTransfer(  KopeteContact *contact, const QString& file, const unsigned long size, const QString &recipient , KopeteFileTransferInfo::KopeteTransferDirection di)
{
//	if (nextID != 0)
		nextID++;
	KopeteFileTransferInfo info(contact, file, size, recipient,di,  nextID);
	KopeteTransfer *trans = new KopeteTransfer(info, contact);
	connect(trans, SIGNAL(done(KopeteTransfer *)), this, SIGNAL(slotComplete(KopeteTransfer *)));
	mTransfersMap.insert(nextID, trans);
	return trans;
}

void KopeteTransferManager::slotAccepted(const KopeteFileTransferInfo& info, const QString& filename)
{
	KopeteTransfer *trans = new KopeteTransfer(info, filename);
	connect(trans, SIGNAL(result(KIO::Job *)), this, SIGNAL(slotComplete(KopeteTransfer *)));
	mTransfersMap.insert(info.transferId(), trans);
	emit accepted(trans,filename);
}

int KopeteTransferManager::askIncomingTransfer(  KopeteContact *contact, const QString& file, const unsigned long size, const QString& description, void *internalId)
{
//	if (nextID != 0)
		nextID++;
	KopeteFileTransferInfo info(contact, file, size, contact->metaContact()->displayName(), KopeteFileTransferInfo::Incoming , nextID , internalId);

	//FIXME!!! this will not be deleted if it's still open when kopete exits
	KopeteFileConfirmDialog *diag= new KopeteFileConfirmDialog(info, description , 0 )  ;

	connect( diag, SIGNAL( accepted(const KopeteFileTransferInfo&, const QString&)) , this, SLOT( slotAccepted(const KopeteFileTransferInfo&, const QString&) ) );
	connect( diag, SIGNAL( refused(const KopeteFileTransferInfo&)) , this, SIGNAL( refused(const KopeteFileTransferInfo&) ) );
	diag->show();
	return nextID;
}

void KopeteTransferManager::removeTransfer( unsigned int id )
{
	KopeteTransfer *trans = mTransfersMap[id];
//	mTransfersMap.remove(id);
	delete trans;
}

void KopeteTransferManager::slotComplete(KopeteTransfer *transfer)
{
	emit done(transfer);

	for( QMap<unsigned, KopeteTransfer*>::Iterator it = mTransfersMap.begin();
	     it != mTransfersMap.end(); ++it )
	{
		if( it.data() == transfer )
		{
			mTransfersMap.remove( it.key() );
			break;
		}
	}
}

#include "kopetetransfermanager.moc"

