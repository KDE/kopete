/*
    kopetetransfermanager.cpp

    Copyright (c) 2002-2003 by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Richard Smith <kopete@metafoo.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <klocale.h>
#include <kstaticdeleter.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kmessagebox.h>
#include <kio/observer.h>

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopeteuiglobal.h"

#include "kopetetransfermanager.h"
#include "kopetefileconfirmdialog.h"

/***************************
 *  Kopete::FileTransferInfo *
 ***************************/

Kopete::FileTransferInfo::FileTransferInfo(  Kopete::Contact *contact, const QString& file, const unsigned long size, const QString &recipient, KopeteTransferDirection di, const unsigned int id, QString internalId)
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
 *     Kopete::Transfer      *
 ***************************/


Kopete::Transfer::Transfer( const Kopete::FileTransferInfo &kfti, const QString &localFile, bool showProgressInfo)
	: KIO::Job(showProgressInfo), mInfo(kfti)
{
	KURL targ; targ.setPath( localFile );
	init( targ, showProgressInfo );
}

Kopete::Transfer::Transfer( const Kopete::FileTransferInfo &kfti, const Kopete::Contact *contact, bool showProgressInfo)
	: KIO::Job(showProgressInfo), mInfo(kfti)
{
	// TODO: use mInfo.url().fileName() after move to protocol-aware filetransfers
	KURL targ; targ.setPath( mInfo.file() );
	init( displayURL( contact, targ.fileName() ), showProgressInfo );
}

void Kopete::Transfer::init( const KURL &target, bool showProgressInfo )
{
	mTarget = target;

	if( showProgressInfo )
		Observer::self()->slotCopying( this, sourceURL(), destinationURL() );

	connect( this, SIGNAL( result( KIO::Job* ) ), SLOT( slotResultEmitted() ) );

	setAutoErrorHandlingEnabled( true, 0 );
}

Kopete::Transfer::~Transfer()
{
}

KURL Kopete::Transfer::displayURL( const Kopete::Contact *contact, const QString &file )
{
	KURL url;
	url.setProtocol( QString::fromLatin1("kopete") );

	QString host;
	if( !contact )
		host = QString::fromLatin1("unknown origin");
	else if( contact->metaContact() )
		host = contact->metaContact()->displayName();
	else
		host = contact->contactId();
	url.setHost(host);

	// url.setPath( contact->protocol()->displayName() );

	url.setFileName( file );
	return url;
}

// TODO: add possibility of network file transfers;
//  call mInfo->url() not file()
KURL Kopete::Transfer::sourceURL()
{
	if( mInfo.direction() == Kopete::FileTransferInfo::Incoming )
		return displayURL( mInfo.contact(), mInfo.file() );
	else
	{
		KURL url; url.setPath( mInfo.file() );
		return url;
	}
}

KURL Kopete::Transfer::destinationURL()
{
	return mTarget;
}

void Kopete::Transfer::slotProcessed(unsigned int bytes)
{
	emitPercent( bytes, mInfo.size() );
}

void Kopete::Transfer::slotComplete()
{
	emitResult();
}

void Kopete::Transfer::slotError( int error, const QString &errorText )
{
	m_error = error;
	m_errorText = errorText;

	emitResult();
}

void Kopete::Transfer::slotResultEmitted()
{
	if( error() == KIO::ERR_USER_CANCELED )
		emit transferCanceled();
}

/***************************
 *  Kopete::TransferManager  *
 ***************************/

static KStaticDeleter<Kopete::TransferManager> deleteManager;
Kopete::TransferManager *Kopete::TransferManager::s_transferManager = 0;

Kopete::TransferManager* Kopete::TransferManager::transferManager()
{
	if(!s_transferManager)
		deleteManager.setObject(s_transferManager, new Kopete::TransferManager(0));

	return s_transferManager;
}

Kopete::TransferManager::TransferManager( QObject *parent ) : QObject( parent )
{
	nextID = 0;
}

Kopete::Transfer* Kopete::TransferManager::addTransfer(  Kopete::Contact *contact, const QString& file, const unsigned long size, const QString &recipient , Kopete::FileTransferInfo::KopeteTransferDirection di)
{
//	if (nextID != 0)
		nextID++;
	Kopete::FileTransferInfo info(contact, file, size, recipient,di,  nextID);
	Kopete::Transfer *trans = new Kopete::Transfer(info, contact);
	connect(trans, SIGNAL(result(KIO::Job *)), this, SLOT(slotComplete(KIO::Job *)));
	mTransfersMap.insert(nextID, trans);
	return trans;
}

void Kopete::TransferManager::slotAccepted(const Kopete::FileTransferInfo& info, const QString& filename)
{
	Kopete::Transfer *trans = new Kopete::Transfer(info, filename);
	connect(trans, SIGNAL(result(KIO::Job *)), this, SLOT(slotComplete(KIO::Job *)));
	mTransfersMap.insert(info.transferId(), trans);
	emit accepted(trans,filename);
}

int Kopete::TransferManager::askIncomingTransfer(  Kopete::Contact *contact, const QString& file, const unsigned long size, const QString& description, QString internalId)
{
//	if (nextID != 0)
		nextID++;
		
	QString dn= contact ? (contact->metaContact() ? contact->metaContact()->displayName() : contact->contactId()) : i18n("<unknown>");

	Kopete::FileTransferInfo info(contact, file, size, dn, Kopete::FileTransferInfo::Incoming , nextID , internalId);

	//FIXME!!! this will not be deleted if it's still open when kopete exits
	KopeteFileConfirmDialog *diag= new KopeteFileConfirmDialog(info, description , 0 )  ;

	connect( diag, SIGNAL( accepted(const Kopete::FileTransferInfo&, const QString&)) , this, SLOT( slotAccepted(const Kopete::FileTransferInfo&, const QString&) ) );
	connect( diag, SIGNAL( refused(const Kopete::FileTransferInfo&)) , this, SIGNAL( refused(const Kopete::FileTransferInfo&) ) );
	diag->show();
	return nextID;
}

void Kopete::TransferManager::removeTransfer( unsigned int id )
{
	mTransfersMap.remove(id);
	//we don't need to delete the job, the job get deleted itself
}

void Kopete::TransferManager::slotComplete(KIO::Job *job)
{
	Kopete::Transfer *transfer=dynamic_cast<Kopete::Transfer*>(job);
	if(!transfer)
		return;

	emit done(transfer);

	for( QMap<unsigned, Kopete::Transfer*>::Iterator it = mTransfersMap.begin();
	     it != mTransfersMap.end(); ++it )
	{
		if( it.data() == transfer )
		{
			removeTransfer(it.key());
			break;
		}
	}
}

void Kopete::TransferManager::sendFile( const KURL &file, const QString &fname, unsigned long sz,
	 bool mustBeLocal,	QObject *sendTo, const char *slot )
{
	KURL url(file);
	QString filename;
	unsigned int size = 0;

	//If the file location is null, then get it from a file open dialog
	if( !url.isValid() )
		url = KFileDialog::getOpenURL( QString::null, QString::fromLatin1("*"), 0l, i18n( "Kopete File Transfer" ));
	else
	{
		filename = fname;
		size = sz;
	}

	if( filename.isEmpty() )
		filename = url.fileName();

	if( size == 0 )
	{
		KFileItem finfo(KFileItem::Unknown, KFileItem::Unknown, url);
		size = (unsigned long)finfo.size();
	}

	if( !url.isEmpty() )
	{
		if( mustBeLocal && !url.isLocalFile() )
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
				i18n( "Sorry, sending files which are not stored locally is not yet supported by this protocol.\n"
				"Please copy this file to your computer and try again." ) );
		}
		else
		{
			connect( this, SIGNAL(sendFile(const KURL&, const QString&, unsigned int)), sendTo, slot );
			emit sendFile( url, filename, size );
			disconnect( this, SIGNAL(sendFile(const KURL&, const QString&, unsigned int)), sendTo, slot );
		}
	}
}

#include "kopetetransfermanager.moc"

