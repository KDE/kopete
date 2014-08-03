/*
    kopetetransfermanager.cpp

    Copyright (c) 2002-2003 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002-2003 by Richard Smith          <kopete@metafoo.co.uk>
    Copyright (c) 2008      by Roman Jarosz           <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetetransfermanager.h"

#include <QtCore/QTimerEvent>
#include <QtGui/QTextDocument>

#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kmessagebox.h>
#include <kio/jobuidelegate.h>
#include <kuiserverjobtracker.h>

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "kopetechatsession.h"
#include "kopeteuiglobal.h"

/***************************
 *  Kopete::FileTransferInfo *
 ***************************/

Kopete::FileTransferInfo::FileTransferInfo()
{
	mId = 0;
	mContact = 0;
	mSize = 0;
	mSaveToDirectory = false;
}

Kopete::FileTransferInfo::FileTransferInfo(  Kopete::Contact *contact, const QStringList& files, const unsigned long size, const QString &recipient, KopeteTransferDirection di, const unsigned int id, QString internalId, const QPixmap &preview, bool saveToDirectory )
{
	mContact = contact;
	mFiles = files;
	mId = id;
	mSize = size;
	mRecipient = recipient;
	m_intId= internalId;
	mDirection= di;
	mPreview = preview;
	mSaveToDirectory = saveToDirectory;
}

/***************************
 *     Kopete::Transfer      *
 ***************************/

static const int TransferRateWindowLength = 10;
static const int TransferRateTimerDelay = 1000;

class Kopete::Transfer::Private
{
public:
	Private( const Kopete::FileTransferInfo &ftInfo )
		: info( ftInfo ), transferRateTimer( 0 )
	{
		memset( transferRate, 0, sizeof(transferRate) );
	}

	FileTransferInfo info;
	KUrl target;

	//if ft has one file then localUrl is file otherwise it's directory
	KUrl localUrl;

	int transferRate[TransferRateWindowLength];
	int transferRateTimer;
};

Kopete::Transfer::Transfer( const Kopete::FileTransferInfo &kfti, const QString &localFile, bool showProgressInfo)
	: KIO::Job(), d( new Private(kfti) )
{
	connect( kfti.contact(), SIGNAL(destroyed(QObject*)), this, SLOT(slotContactDestroyed()) );
	this->setUiDelegate(new KIO::JobUiDelegate());
	if(showProgressInfo)
		KIO::getJobTracker()->registerJob(this);

	KUrl targ; targ.setPath( localFile );
	d->localUrl = targ;
	init( targ, showProgressInfo );
}

Kopete::Transfer::Transfer( const Kopete::FileTransferInfo &kfti, bool showProgressInfo)
	: KIO::Job(), d( new Private(kfti) )
{
	connect( kfti.contact(), SIGNAL(destroyed(QObject*)), this, SLOT(slotContactDestroyed()) );
	this->setUiDelegate(new KIO::JobUiDelegate());
	if(showProgressInfo)
		KIO::getJobTracker()->registerJob(this);

	// TODO: use mInfo.url().fileName() after move to protocol-aware filetransfers
	KUrl targ; targ.setPath( d->info.file() );
	init( displayURL( d->info.contact(), targ.fileName() ), showProgressInfo );
}

void Kopete::Transfer::init( const KUrl &target, bool showProgressInfo )
{
	d->target = target;
	setTotalAmount( KJob::Files, d->info.files().count() );
	setTotalAmount( KJob::Bytes, d->info.size() );

	if( showProgressInfo )
		emitCopying( sourceURL(), destinationURL() );

	connect( this, SIGNAL(result(KJob*)), SLOT(slotResultEmitted()) );

	ui()->setAutoErrorHandlingEnabled( false );
}

Kopete::Transfer::~Transfer()
{
	stopTransferRateTimer();

	delete d;
}

const Kopete::FileTransferInfo &Kopete::Transfer::info() const
{
	return d->info;
}

KUrl Kopete::Transfer::displayURL( const Kopete::Contact *contact, const QString &file )
{
	KUrl url;
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

void Kopete::Transfer::slotNextFile( const QString &sourceFile, const QString &destinationFile )
{
	KUrl src;
	KUrl dest;

	kDebug() << "source: " << sourceFile << " destination: " << destinationFile;
	if( d->info.direction() == Kopete::FileTransferInfo::Incoming )
	{
		KUrl url( sourceFile );
		src = displayURL( d->info.contact(), url.fileName() );
		dest.setPath( destinationFile );
	}
	else
	{
		src.setPath( sourceFile );
		KUrl url( destinationFile );
		dest = displayURL( d->info.contact(), url.fileName() );
	}

	setProcessedAmount( KJob::Files, processedAmount(KJob::Files) + 1 );
	emit description(this, i18n("Copying"),
	                 qMakePair(i18n("Source"), src.prettyUrl()),
	                 qMakePair(i18n("Destination"), dest.prettyUrl()));
}

// TODO: add possibility of network file transfers;
//  call mInfo->url() not file()
KUrl Kopete::Transfer::sourceURL()
{
	if( d->info.direction() == Kopete::FileTransferInfo::Incoming )
		return displayURL( d->info.contact(), d->info.file() );
	else
	{
		KUrl url; url.setPath( d->info.file() );
		return url;
	}
}

KUrl Kopete::Transfer::destinationURL()
{
	return d->target;
}

void Kopete::Transfer::emitCopying(const KUrl &src, const KUrl &dest)
{
    emit description(this, i18n("Copying"),
                     qMakePair(i18n("Source"), src.prettyUrl()),
                     qMakePair(i18n("Destination"), dest.prettyUrl()));
}

void Kopete::Transfer::slotProcessed( unsigned int bytes )
{
	if ( !d->transferRateTimer )
		d->transferRateTimer = startTimer( TransferRateTimerDelay );

	d->transferRate[0] += (bytes - processedAmount(KJob::Bytes));

	setProcessedAmount( KJob::Bytes, bytes );
	emitPercent( bytes, d->info.size() );
}

void Kopete::Transfer::timerEvent( QTimerEvent *event )
{
	if ( event->timerId() != d->transferRateTimer )
	{
		KIO::Job::timerEvent( event );
		return;
	}

	// Calculate average transferRate
	qint64 bytesPerSecond = 0;
	for ( int i = 0; i < TransferRateWindowLength; ++i )
		bytesPerSecond += d->transferRate[i];

	bytesPerSecond /= qint64( TransferRateWindowLength );

	for ( int i = TransferRateWindowLength - 2; i >= 0; --i )
		d->transferRate[i + 1] = d->transferRate[i];

	d->transferRate[0] = 0;
	emitSpeed( bytesPerSecond );

	// Stop the timer if there is no activity.
	if ( bytesPerSecond == 0 )
		stopTransferRateTimer();
}

void Kopete::Transfer::slotComplete()
{
	stopTransferRateTimer();
	setError( KJob::NoError );
	showHtmlMessage( i18n("File transfer %1 completed.", fileForMessage() ) );
	emitResult();
}

void Kopete::Transfer::slotError( int error, const QString &errorText )
{
	stopTransferRateTimer();
	setError(error);
	setErrorText(errorText);

	showHtmlMessage( i18n("File transfer %1 failed.", fileForMessage() ) );
	emitResult();
}

void Kopete::Transfer::slotResultEmitted()
{
	if( error() == KIO::ERR_USER_CANCELED )
	{
		stopTransferRateTimer();
		showHtmlMessage( i18n("You cancelled file transfer %1", fileForMessage() ) );
		emit transferCanceled();
	}
}

void Kopete::Transfer::slotContactDestroyed()
{
	setError( KIO::ERR_USER_CANCELED );
	emitResult();
}

void Kopete::Transfer::slotCancelled()
{
	stopTransferRateTimer();

	// If cancel button was pressed suppress notification because it was show already in slotResultEmitted()
	if ( error() != KIO::ERR_USER_CANCELED )
	{
		showHtmlMessage( i18n("File transfer %1 cancelled.", fileForMessage() ) );
		emitResult();
	}
}

bool Kopete::Transfer::showMessage( QString text ) const
{
	Kopete::ChatSession *cs = chatSession();
	if ( !cs )
		return false;

	Kopete::Message msg;
	msg.setPlainBody( text );
	cs->appendMessage( msg );
	return true;
}

bool Kopete::Transfer::showHtmlMessage( QString text ) const
{
	Kopete::ChatSession *cs = chatSession();
	if (! cs)
		return false;
	
	Kopete::Message msg;
	msg.setHtmlBody( text );
	cs->appendMessage( msg );
	return true;
}

QString Kopete::Transfer::fileForMessage() const
{
	if( d->info.direction() == Kopete::FileTransferInfo::Incoming )
		return QString( "<a href=\"%1\">%2</a>" ).arg( d->localUrl.url(), Qt::escape( d->localUrl.toLocalFile() ) );
	else
		return Qt::escape( d->info.file() );
}

void Kopete::Transfer::stopTransferRateTimer()
{
	if ( d->transferRateTimer )
	{
		killTimer( d->transferRateTimer );
		d->transferRateTimer = 0;
	}
}

Kopete::ChatSession* Kopete::Transfer::chatSession() const
{
	Kopete::Contact *c = d->info.contact();
	return ( c ) ? c->manager() : 0;
}

/***************************
 *  Kopete::TransferManager  *
 ***************************/

Kopete::TransferManager* Kopete::TransferManager::transferManager()
{
	static TransferManager s(0L);
	return &s;
}

Kopete::TransferManager::TransferManager( QObject *parent ) : QObject( parent )
{
}

Kopete::Transfer* Kopete::TransferManager::addTransfer( Kopete::Contact *contact, const QString& file, const unsigned long size, const QString &recipient, Kopete::FileTransferInfo::KopeteTransferDirection di)
{
	return addTransfer( contact, QStringList(file), size, recipient, di );
}

Kopete::Transfer* Kopete::TransferManager::addTransfer( Kopete::Contact *contact, const QStringList& files, const unsigned long size, const QString &recipient, Kopete::FileTransferInfo::KopeteTransferDirection di)
{
	// Use message id to make file transfer id unique because we already use it for incoming file transfer.
	uint id = Kopete::Message::nextId();
	Kopete::FileTransferInfo info(contact, files, size, recipient, di, id);
	Kopete::Transfer *trans = new Kopete::Transfer(info);
	connect(trans, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)));
	mTransfersMap.insert(id, trans);
	return trans;
}

unsigned int Kopete::TransferManager::askIncomingTransfer( Kopete::Contact *contact, const QString& file, const unsigned long size, const QString& description, QString internalId, const QPixmap &preview )
{
	return askIncomingTransfer( contact, QStringList( file ), size, description, internalId, preview );
}

unsigned int Kopete::TransferManager::askIncomingTransfer( Kopete::Contact *contact, const QStringList& files, const unsigned long size, const QString& description, QString internalId, const QPixmap &preview )
{
	Kopete::ChatSession *cs = contact->manager( Kopete::Contact::CanCreate );
	if ( !cs )
		return 0;
	
	const QString dn = contact->metaContact() ? contact->metaContact()->displayName() : contact->contactId();
	
	QString msgFileName;
	foreach ( const QString &file, files )
	{
		QString trimmedFile = file.trimmed();
		if ( !trimmedFile.isEmpty() )
			msgFileName += trimmedFile + ", ";
	}

	// Remove ", " from end
	if ( msgFileName.size() >= 2 )
		msgFileName = msgFileName.left( msgFileName.size() - 2 );

	Kopete::Message msg( contact, cs->myself() );
	msg.setType( Kopete::Message::TypeFileTransferRequest );
	msg.setDirection( Kopete::Message::Inbound );
	msg.setPlainBody( description );
	msg.setFileName( msgFileName );
	msg.setFileSize( size );
	msg.setFilePreview( preview );
	
	Kopete::FileTransferInfo info( contact, files, size, dn, Kopete::FileTransferInfo::Incoming,
	                               msg.id(), internalId, preview, (files.count() > 1) );
	mTransferRequestInfoMap.insert( msg.id(), info );
	
	cs->appendMessage( msg );
	
	return msg.id();
}

void Kopete::TransferManager::saveIncomingTransfer( unsigned int id )
{
	Kopete::FileTransferInfo info = mTransferRequestInfoMap.value( id );
	if ( !info.isValid() )
		return;

	KConfigGroup cg( KGlobal::config(), "File Transfer" );
	const QString defaultPath = cg.readEntry( "defaultPath", QDir::homePath() );
	KUrl url = QString(defaultPath + QLatin1String( "/" ) + info.file());

	if ( info.saveToDirectory() )
		url = getSaveDir( url );
	else
		url = getSaveFile( url );

	if ( url.isEmpty() )
		return;

	if ( !url.isValid() )
	{
		emit askIncomingDone( id );
		emit refused( info );
		mTransferRequestInfoMap.remove( id );
		return;
	}

	const QString directory = ( info.saveToDirectory() ) ? url.toLocalFile() : url.directory();
	if( !directory.isEmpty() )
		cg.writeEntry( "defaultPath", directory );

	Kopete::Transfer *trans = new Kopete::Transfer( info, url.toLocalFile() );
	connect( trans, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)) );
	mTransfersMap.insert( info.transferId(), trans );
	emit askIncomingDone( id );
	emit accepted( trans, url.toLocalFile() );
	mTransferRequestInfoMap.remove( id );
}

void Kopete::TransferManager::cancelIncomingTransfer( unsigned int id )
{
	Kopete::FileTransferInfo info = mTransferRequestInfoMap.value( id );
	if ( !info.isValid() )
		return;

	emit askIncomingDone( id );
	emit refused( info );
	mTransferRequestInfoMap.remove( id );
}

void Kopete::TransferManager::slotComplete(KJob *job)
{
	Kopete::Transfer *transfer=dynamic_cast<Kopete::Transfer*>(job);
	if(!transfer)
		return;

	emit done(transfer);

	for( QMap<unsigned, Kopete::Transfer*>::Iterator it = mTransfersMap.begin();
	     it != mTransfersMap.end(); ++it )
	{
		if( it.value() == transfer )
		{
			removeTransfer(it.key());
			break;
		}
	}
}

void Kopete::TransferManager::sendFile( const KUrl &file, const QString &fname, unsigned long sz,
	 bool mustBeLocal,	QObject *sendTo, const char *slot )
{
	KUrl url = file;
	QString filename;
	unsigned int size = 0;

	//If the file location is null, then get it from a file open dialog
	if( !url.isValid() )
		url = KFileDialog::getOpenUrl( KUrl(), QString::fromLatin1("*"), 0l, i18n( "Kopete File Transfer" ));
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
			connect( this, SIGNAL(sendFile(KUrl,QString,uint)), sendTo, slot );
			emit sendFile( url, filename, size );
			disconnect( this, SIGNAL(sendFile(KUrl,QString,uint)), sendTo, slot );
		}
	}
}

void Kopete::TransferManager::removeTransfer( unsigned int id )
{
	mTransfersMap.remove(id);
	//we don't need to delete the job, the job get deleted itself
}

KUrl Kopete::TransferManager::getSaveFile( const KUrl& startDir ) const
{
	KUrl url = startDir;
	for ( ;; )
	{
		url = KFileDialog::getSaveUrl( url, QLatin1String( "*" ), 0, i18n( "File Transfer" ) );
		if ( !url.isValid() )
			return url;

		if ( !url.isLocalFile() )
		{
			KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "You must provide a valid local filename" ) );
			continue;
		}

		QFileInfo fileInfo( url.toLocalFile() );
		if ( fileInfo.exists() )
		{
			if ( !fileInfo.isWritable() )
			{
				KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "You do not have permission to write to selected file" ) );
				continue;
			}

			int ret = KMessageBox::warningContinueCancel( 0, i18n( "The file '%1' already exists.\nDo you want to overwrite it ?", url.toLocalFile() ),
			                                              i18n( "Overwrite File" ), KStandardGuiItem::save() );
			if ( ret == KMessageBox::Cancel )
				continue;
		}
		else
		{
			QFileInfo dirInfo( url.directory() );
			if ( !dirInfo.isDir() || !dirInfo.exists() )
			{
				KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "The directory %1 does not exist", dirInfo.fileName() ) );
				continue;
			}
			else if ( !dirInfo.isWritable() )
			{
				KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "You do not have permission to write to selected directory" ) );
				continue;
			}
			
		}
		break;
	}
	return url;
}

KUrl Kopete::TransferManager::getSaveDir( const KUrl& startDir ) const
{
	KUrl url = startDir;
	for ( ;; )
	{
		url = KFileDialog::getExistingDirectoryUrl( url, 0, i18n( "File Transfer" ) );
		if ( !url.isValid() )
			return url;

		if ( !url.isLocalFile() )
		{
			KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "You must provide a valid local directory" ) );
			continue;
		}

		QFileInfo dirInfo( url.toLocalFile() );
		if ( !dirInfo.isDir() || !dirInfo.exists() )
		{
			KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "The directory %1 does not exist", dirInfo.filePath() ) );
			continue;
		}
		else if ( !dirInfo.isWritable() )
		{
			KMessageBox::messageBox( 0, KMessageBox::Sorry, i18n( "You do not have permission to write to selected directory" ) );
			continue;
		}
		break;
	}
	return url;
}

#include "kopetetransfermanager.moc"

