/*
    kopeteavatarmanager.cpp - Global avatar manager

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteavatarmanager.h"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QDir>

// KDE includes
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kcodecs.h>
#include <kurl.h>
#include <kio/job.h>
#include <klocale.h>

// Kopete includes
#include <kopetecontact.h>
#include <kopeteprotocol.h>

namespace Kopete
{

//BEGIN AvatarManager
AvatarManager *AvatarManager::s_self = 0;

AvatarManager *AvatarManager::self()
{
	if( !s_self )
	{
		s_self = new AvatarManager;
	}
	return s_self;
}

class AvatarManager::Private
{
public:
	KUrl baseDir;

	/**
	 * Create directory if needed
	 * @param directory URL of the directory to create
	 */
	void createDirectory(const KUrl &directory);

	/**
	 * Scale the given image to 96x96.
	 * @param source Original image
	 */
	QImage scaleImage(const QImage &source);
};

static const QString UserDir("User");
static const QString ContactDir("Contacts");
static const QString AvatarConfig("avatarconfig.rc");


AvatarManager::AvatarManager(QObject *parent)
 : QObject(parent), d(new Private)
{
	// Locate avatar data dir on disk
	d->baseDir = KUrl( KStandardDirs::locateLocal("appdata", QLatin1String("avatars") ) );

	// Create directory on disk, if necessary
	d->createDirectory( d->baseDir );
}

AvatarManager::~AvatarManager()
{
	delete d;
}

void AvatarManager::add(Kopete::AvatarManager::AvatarEntry newEntry)
{
	Q_ASSERT(!newEntry.name.isEmpty());
	
	KUrl avatarUrl(d->baseDir);

	// First find where to save the file
	switch(newEntry.category)
	{
		case AvatarManager::User:
			avatarUrl.addPath( UserDir );
			d->createDirectory( avatarUrl );
			break;
		case AvatarManager::Contact:
			avatarUrl.addPath( ContactDir );
			d->createDirectory( avatarUrl );
			// Use the plugin name for protocol sub directory
			if( newEntry.contact && newEntry.contact->protocol() )
			{
				QString protocolName = newEntry.contact->protocol()->pluginId();
				avatarUrl.addPath( protocolName );
				d->createDirectory( avatarUrl );
			}
			break;
		default:
			break;
	}

	kDebug(14010) << k_funcinfo << "Base directory: " << avatarUrl.path() << endl;

	// Second, open the avatar configuration in current directory.
	KUrl configUrl = avatarUrl;
	configUrl.addPath( AvatarConfig );
	
	QImage avatar;
	if( !newEntry.path.isEmpty() && newEntry.image.isNull() )
	{
		avatar = QImage(newEntry.path);
	}
	else
	{
		avatar = newEntry.image;
	}

	// Scale avatar
	avatar = d->scaleImage(avatar);

	QString avatarFilename;
	// Find MD5 hash for filename
	// MD5 always contain ASCII caracteres so it is more save for a filename.
	// Name can use UTF-8 caracters.
	QByteArray tempArray;
	QBuffer tempBuffer(&tempArray);
	tempBuffer.open( QIODevice::WriteOnly );
	avatar.save(&tempBuffer, "PNG");
	KMD5 context(tempArray);
	avatarFilename = context.hexDigest() + QLatin1String(".png");

	// Save image on disk	
	kDebug(14010) << k_funcinfo << "Saving " << avatarFilename << " on disk." << endl;
	avatarUrl.addPath( avatarFilename );

	if( !avatar.save( avatarUrl.path(), "PNG") )
	{
		kDebug(14010) << k_funcinfo << "Saving of " << avatarUrl.path() << " failed !" << endl;
		return;
	}
	else
	{
		// Save metadata of image
		KConfigGroup avatarConfig(KSharedConfig::openConfig( configUrl.path(), KConfig::OnlyLocal), newEntry.name );
	
		avatarConfig.writeEntry( "Filename", avatarFilename );
		avatarConfig.writeEntry( "Category", int(newEntry.category) );

		avatarConfig.sync();
	
		// Add final path to the new entry for avatarAdded signal
		newEntry.path = avatarUrl.path();

		emit avatarAdded(newEntry);
	}
}

void AvatarManager::remove(Kopete::AvatarManager::AvatarEntry entryToRemove)
{
	Q_UNUSED(entryToRemove);
	emit avatarRemoved(entryToRemove);
}

void AvatarManager::Private::createDirectory(const KUrl &directory)
{
	if( !QFile::exists(directory.path()) )
	{
		kDebug(14010) << k_funcinfo << "Creating directory: " << directory.path() << endl;
		KIO::Job *job = KIO::mkdir(directory);
		job->exec();
		if( job->error() )
		{
			kDebug(14010) << "Directory creating failed: " << job->errorText() << endl;
		}
	}
}

QImage AvatarManager::Private::scaleImage(const QImage &source)
{
	QImage result;

	if( source.width() > 96 || source.height() > 96 )
	{
		// Scale and crop the picture.
		result = source.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
		// crop image if not square
		if( result.width() < result.height() )
			result = result.copy( (result.width()-result.height())/2, 0, 96, 96 );
		else if( result.width() > result.height() )
			result = result.copy( 0, (result.height()-result.width())/2, 96, 96 );
	}
	else if( source.width() < 32 || source.height() < 32 )
	{
		// Scale and crop the picture.
		result = source.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
		// crop image if not square
		if( result.width() < result.height() )
			result = result.copy( (result.width()-result.height())/2, 0, 32, 32 );
		else if( result.width() > result.height() )
			result = result.copy( 0, (result.height()-result.width())/2, 32, 32 );
	}
	else if( source.width() != source.height() )
	{
		if(source.width() < source.height())
			result = source.copy((source.width()-source.height())/2, 0, source.height(), source.height());
		else if (source.width() > source.height())
			result = source.copy(0, (source.height()-source.width())/2, source.height(), source.height());
	}

	return result;
}

//END AvatarManager

//BEGIN AvatarQueryJob
class AvatarQueryJob::Private
{
public:
	Private(AvatarQueryJob *parent)
	 : queryJob(parent), category(AvatarManager::All)
	{}

	QPointer<AvatarQueryJob> queryJob;
	AvatarManager::AvatarCategory category;
	QList<AvatarManager::AvatarEntry> avatarList;
	KUrl baseDir;

	void listAvatarDirectory(const QString &path);
};

AvatarQueryJob::AvatarQueryJob(QObject *parent)
 : KJob(parent), d(new Private(this))
{

}

AvatarQueryJob::~AvatarQueryJob()
{
	delete d;
}

void AvatarQueryJob::setQueryFilter(Kopete::AvatarManager::AvatarCategory category)
{
	d->category = category;
}

void AvatarQueryJob::start()
{
	d->baseDir = KUrl( KStandardDirs::locateLocal("appdata", QLatin1String("avatars") ) );

	if( d->category & Kopete::AvatarManager::User )
	{
		d->listAvatarDirectory( UserDir );
	}
	if( d->category & Kopete::AvatarManager::Contact )
	{
		KUrl contactUrl(d->baseDir);
		contactUrl.addPath( ContactDir );

		QDir contactDir(contactUrl.path());
		QStringList subdirsList = contactDir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
		QString subdir;
		foreach(subdir, subdirsList)
		{
			d->listAvatarDirectory( ContactDir + QDir::separator() + subdir );
		}
	}

	// Finish the job
	emitResult();
}

QList<Kopete::AvatarManager::AvatarEntry> AvatarQueryJob::avatarList() const
{
	return d->avatarList;
}

void AvatarQueryJob::Private::listAvatarDirectory(const QString &relativeDirectory)
{
	KUrl avatarDirectory = baseDir;
	avatarDirectory.addPath(relativeDirectory);

	kDebug(14010) << k_funcinfo << "Listing avatars in " << avatarDirectory.path() << endl;

	// Look for Avatar configuration
	KUrl avatarConfigUrl = avatarDirectory;
	avatarConfigUrl.addPath( AvatarConfig );
	if( QFile::exists(avatarConfigUrl.path()) )
	{
		KConfig *avatarConfig = new KConfig( avatarConfigUrl.path(), KConfig::OnlyLocal);
		// Each avatar entry in configuration is a group
		QStringList groupEntryList = avatarConfig->groupList();
		QString groupEntry;
		foreach(groupEntry, groupEntryList)
		{
			avatarConfig->setGroup(groupEntry);

			Kopete::AvatarManager::AvatarEntry listedEntry;
			listedEntry.name = groupEntry;
			listedEntry.category = static_cast<Kopete::AvatarManager::AvatarCategory>( avatarConfig->readEntry("Category", 0) );

			QString filename = avatarConfig->readEntry( "Filename", QString() );
			KUrl avatarPath(avatarDirectory);
			avatarPath.addPath( filename );
			listedEntry.path = avatarPath.path();

			avatarList << listedEntry;
		}
	}
	else
	{
		queryJob->setError( UserDefinedError );
		queryJob->setErrorText( i18n("Avatar configuration hasn't been found on disk for category %1.", relativeDirectory) );
	}
}

//END AvatarQueryJob

}

#include "kopeteavatarmanager.moc"
