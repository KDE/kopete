/*
    nlquodlibet.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2006,2007 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002,2003,2004,2005,2006,2007 by the Kopete developers  <kopete-devel@kde.org>

    Purpose:
    This class abstracts the interface to the Quod Libet music player by
    implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "nlquodlibet.h"

#include <stdlib.h>
#include <stdio.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include <KDebug>
#include <KDirWatch>
#include <KLocale>

NLQuodLibet::NLQuodLibet() : NLMediaPlayer()
{
	m_name = "Quod Libet";
	m_playing = false;
	m_watch = new KDirWatch( this );
	connect( m_watch, SIGNAL(created(QString)),
			 SLOT(fileChanged(QString)) );
	connect( m_watch, SIGNAL(deleted(QString)),
			 SLOT(fileChanged(QString)) );
	connect( m_watch, SIGNAL(created(QString)),
			 SLOT(fileChanged(QString)) );
	m_watch->addFile( currentTrackPath() );
}

NLQuodLibet::~NLQuodLibet()
{

}

void NLQuodLibet::update()
{
	//look for running QL
	// see if the ~/.quodlibet/current exists
	//   if yes
    //   parse for artist, album, title
	//   m_playing = true;
    // else
    //   m_playing = false;

	// assume we have no data
	m_artist = i18n( "Unknown artist" );
	m_album = i18n( "Unknown album" );
	m_track = i18n( "Unknown track" );

	QString path = currentTrackPath();
	QFile currentTrackFile( path );
	if ( currentTrackFile.exists() )
	{
		m_playing = true;
		QFileInfo info( currentTrackFile );
		m_newTrack = ( info.lastModified() > m_timestamp );
		if ( m_newTrack )
			m_timestamp = info.lastModified();

		parseFile( currentTrackFile );
	}
	else
		m_playing = false;
}

QString NLQuodLibet::currentTrackPath() const
{
	return QDir::homePath() + QLatin1String( "/.quodlibet/current" );
}

void NLQuodLibet::parseFile( QFile & file )
{
	if ( file.open( QIODevice::ReadOnly ) ) {
		QTextStream stream( &file );
		QString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine(); // line of text excluding '\n'
			parseLine( line );
		}
		file.close();
	}
}

void NLQuodLibet::parseLine( const QString & line )
{
	QStringList parts = line.split( '=', QString::KeepEmptyParts );
	if ( parts.count() == 2 )
	{
		if ( parts[0] == "album" ) {
			kDebug() << "found QL album: " << parts[1];
			m_album = parts[1];
		}
		if ( parts[0] == "artist" ) {
			kDebug() << "found QL artist: " << parts[1];
			m_artist = parts[1];
		}
		if ( parts[0] == "title" ) {
			kDebug() << "found QL track: " << parts[1];
			m_track = parts[1];
		}
	}
}

void NLQuodLibet::fileChanged( const QString & file )
{
	if ( file == currentTrackPath() )
		update();
}

#include "nlquodlibet.moc"

// vim: set noet ts=4 sts=4 sw=4:
