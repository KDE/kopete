/*
    oscarversionupdater.cpp  -  Version Updater

    Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarversionupdater.h"

#include <qdom.h>
#include <qmutex.h>

#include <kdebug.h>
#include <kio/job.h>
#include <kconfig.h>
#include <kglobal.h>


QMutex updateMutex;
OscarVersionUpdater *OscarVersionUpdater::versionUpdaterStatic = 0L;

OscarVersionUpdater::OscarVersionUpdater()
: mStamp( 1 ), mUpdating( false )
{
	initICQVersionInfo();
	initAIMVersionInfo();
}

OscarVersionUpdater::~OscarVersionUpdater()
{
}

OscarVersionUpdater *OscarVersionUpdater::self()
{
	if ( !versionUpdaterStatic )
		versionUpdaterStatic = new OscarVersionUpdater();
	
	return versionUpdaterStatic;
}

bool OscarVersionUpdater::update( unsigned int stamp )
{
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << endl;
	bool doUpdate = false;
	bool isUpdating = false;
	
	updateMutex.lock();
	if ( !mUpdating && stamp == mStamp )
	{
		doUpdate = true;
		mUpdating = true;
	}
	isUpdating = mUpdating;
	updateMutex.unlock();
	
	if ( doUpdate )
	{
		mVersionData.resize( 0 );
		
		KConfigGroup config( KGlobal::config(), "Oscar" );
		QString url = config.readEntry( "NewVersionURL", "http://kopete.kde.org/oscarversions.xml" );
		mTransferJob = KIO::get ( url );
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Download version info from server."<< endl;
		
		connect ( mTransferJob, SIGNAL ( result ( KIO::Job* ) ),
		          this, SLOT ( slotTransferResult ( KIO::Job* ) ) );
		connect ( mTransferJob, SIGNAL ( data ( KIO::Job*, const QByteArray& ) ),
		          this, SLOT ( slotTransferData ( KIO::Job*, const QByteArray& ) ) );
	}
	return isUpdating;
}

unsigned int OscarVersionUpdater::stamp() const
{
	return mStamp;
}

void OscarVersionUpdater::initICQVersionInfo()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	KConfigGroup config( KGlobal::config(), "ICQVersion" );
	
	mICQVersion.clientString = config.readEntry( "ClientString", "ICQBasic" );
	mICQVersion.clientId = config.readEntry( "ClientId", "0x010A" ).toUShort( 0, 0 );
	mICQVersion.major = config.readEntry( "Major", "0x0006" ).toUShort( 0, 0 );
	mICQVersion.minor = config.readEntry( "Minor", "0x0000" ).toUShort( 0, 0 );
	mICQVersion.point = config.readEntry( "Point", "0x0000" ).toUShort( 0, 0 );
	mICQVersion.build = config.readEntry( "Build", "0x17AB" ).toUShort( 0, 0 );
	mICQVersion.other = config.readEntry( "Other", "0x00007535" ).toUInt( 0, 0 );
	mICQVersion.country = config.readEntry( "Country", "us" );
	mICQVersion.lang = config.readEntry( "Lang", "en" );
}

void OscarVersionUpdater::initAIMVersionInfo()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	
	KConfigGroup config( KGlobal::config(), "AIMVersion" );
	
	mAIMVersion.clientString = config.readEntry( "ClientString", "AOL Instant Messenger (SM), version 5.1.3036/WIN32" );
	mAIMVersion.clientId = config.readEntry( "ClientId", "0x0109" ).toUShort( 0, 0 );
	mAIMVersion.major = config.readEntry( "Major", "0x0005" ).toUShort( 0, 0 );
	mAIMVersion.minor = config.readEntry( "Minor", "0x0001" ).toUShort( 0, 0 );
	mAIMVersion.point = config.readEntry( "Point", "0x0000" ).toUShort( 0, 0 );
	mAIMVersion.build = config.readEntry( "Build", "0x0bdc" ).toUShort( 0, 0 );
	mAIMVersion.other = config.readEntry( "Other", "0x000000d2" ).toUInt( 0, 0 );
	mAIMVersion.country = config.readEntry( "Country", "us" );
	mAIMVersion.lang = config.readEntry( "Lang", "en" );
}

void OscarVersionUpdater::printDebug()
{
	kdDebug(OSCAR_RAW_DEBUG) << "*************** AIM VERSION INFO ***************" << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "client string: " << mAIMVersion.clientString << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "client id: "  << QString::number( mAIMVersion.clientId, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "major: "  << QString::number( mAIMVersion.major, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "minor: "  << QString::number( mAIMVersion.minor, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "point: "  << QString::number( mAIMVersion.point, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "build: "  << QString::number( mAIMVersion.build, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "other: "  << QString::number( mAIMVersion.other, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "country: "  << mAIMVersion.country << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "lang: "  << mAIMVersion.lang << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "************************************************" << endl;
	
	kdDebug(OSCAR_RAW_DEBUG) << "*************** ICQ VERSION INFO ***************" << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "client string: " << mICQVersion.clientString << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "client id: "  << QString::number( mICQVersion.clientId, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "major: "  << QString::number( mICQVersion.major, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "minor: "  << QString::number( mICQVersion.minor, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "point: "  << QString::number( mICQVersion.point, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "build: "  << QString::number( mICQVersion.build, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "other: "  << QString::number( mICQVersion.other, 16 ) << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "country: "  << mICQVersion.country << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "lang: "  << mICQVersion.lang << endl;
	kdDebug(OSCAR_RAW_DEBUG) << "************************************************" << endl;
}

void OscarVersionUpdater::slotTransferData ( KIO::Job */*job*/, const QByteArray &data )
{
	unsigned oldSize = mVersionData.size();
	mVersionData.resize ( oldSize + data.size() );
	memcpy ( &mVersionData.data()[oldSize], data.data(), data.size() );
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Data size " << mVersionData.size() << endl;
}

void OscarVersionUpdater::slotTransferResult ( KIO::Job *job )
{
	bool bUpdate = false;
	if ( job->error() || mTransferJob->isErrorPage() )
	{
		//TODO show error
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Download of version info has faild!" << endl;
	}
	else
	{
		kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Updating version info" << endl;
		
		QDomDocument doc;
		if ( doc.setContent ( mVersionData ) )
		{
			Oscar::ClientVersion tmpICQ = mICQVersion;
			Oscar::ClientVersion tmpAIM = mAIMVersion;
			
			parseDocument( doc );
			
			if ( !equal( tmpICQ, mICQVersion ) )
			{
				storeVersionInfo( "ICQVersion", mICQVersion );
				bUpdate = true;
			}
			
			if ( !equal( tmpAIM, mAIMVersion ) )
			{
				storeVersionInfo( "AIMVersion", mAIMVersion );
				bUpdate = true;
			}
		}
	}
	
	// clear
	mVersionData.resize( 0 );
	mTransferJob = 0;
	
	updateMutex.lock();
	if ( bUpdate )
		mStamp++;
	mUpdating = false;
	updateMutex.unlock();
}

void OscarVersionUpdater::parseDocument( QDomDocument& doc )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	
	QDomElement root = doc.documentElement();
	if ( root.tagName() != "oscar" )
		return;
	
	QDomElement versionElement = root.firstChild().toElement();
	while( !versionElement.isNull() )
	{
		if ( versionElement.tagName() == "icq" )
			parseVersion( mICQVersion, versionElement );
		else if ( versionElement.tagName() == "aim" )
			parseVersion( mAIMVersion, versionElement );
		
		versionElement = versionElement.nextSibling().toElement();
	}
}

bool OscarVersionUpdater::parseVersion( Oscar::ClientVersion& version, QDomElement& element )
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	
	// clear structure
	version.clientString = QString::null;
	version.clientId = 0x0000;
	version.major = 0x0000;
	version.minor = 0x0000;
	version.point = 0x0000;
	version.build = 0x0000;
	version.other = 0x00000000;
	version.country = QString::null;
	version.lang = QString::null;
	
	QDomElement versionChild = element.firstChild().toElement();
	while ( !versionChild.isNull() )
	{
		if ( versionChild.tagName() == "client" )
			version.clientString = versionChild.text();
		else if ( versionChild.tagName() == "clientId" )
			version.clientId = versionChild.text().toUShort( 0, 0);
		else if ( versionChild.tagName() == "major" )
			version.major = versionChild.text().toUShort( 0, 0 );
		else if ( versionChild.tagName() == "minor" )
			version.minor = versionChild.text().toUShort( 0, 0 );
		else if ( versionChild.tagName() == "point" )
			version.point = versionChild.text().toUShort( 0, 0 );
		else if ( versionChild.tagName() == "build" )
			version.build = versionChild.text().toUShort( 0, 0 );
		else if ( versionChild.tagName() == "other" )
			version.other = versionChild.text().toUInt( 0, 0 );
		else if ( versionChild.tagName() == "country" )
			version.country = versionChild.text();
		else if ( versionChild.tagName() == "lang" )
			version.lang = versionChild.text();
		
		versionChild = versionChild.nextSibling().toElement();
	}
	
	return true;
}

void OscarVersionUpdater::storeVersionInfo( const QString& group, const Oscar::ClientVersion& version ) const
{
	kdDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Storing version info to group: " << group << endl;
	KConfigGroup config( KGlobal::config(), group );
	
	config.writeEntry( "ClientString", version.clientString );
	config.writeEntry( "ClientId", version.clientId );
	config.writeEntry( "Major", version.major );
	config.writeEntry( "Minor", version.minor );
	config.writeEntry( "Point", version.point );
	config.writeEntry( "Build", version.build );
	config.writeEntry( "Other", version.other );
	config.writeEntry( "Country", version.country );
	config.writeEntry( "Lang", version.lang );
	config.sync();
}

bool OscarVersionUpdater::equal( const Oscar::ClientVersion& a, const Oscar::ClientVersion& b ) const
{
	if ( a.clientString != b.clientString || a.clientId != b.clientId ||
	     a.major != b.major|| a.minor != b.minor ||
	     a.point != b.point || a.build != b.build ||
	     a.other != b.other || a.country != b.country ||
	     a.lang != b.lang )
	{
		return false;
	}
	
	return true;
}

#include "oscarversionupdater.moc"
