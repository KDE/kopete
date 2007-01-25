/*
    nlquodlibet.h

    Kopete Now Listening To plugin

    Copyright (c) 2006 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2006 by the Kopete developers  <kopete-devel@kde.org>
	
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

#ifndef NLQUODLIBET_H
#define NLQUODLIBET_H

#include <qdatetime.h>
#include <qfile.h>
#include <qobject.h>

#include "nlmediaplayer.h"

class KDirWatch;

class NLQuodLibet :  public QObject, public NLMediaPlayer
{
Q_OBJECT
	public:
		NLQuodLibet();
		virtual void update();
	protected:
		QString currentTrackPath() const;
		void parseFile( QFile & );
		void parseLine( const QString & );
	protected slots:
		void slotFileChanged( const QString & path );
	private:
		QDateTime m_timestamp;
		KDirWatch * m_watch;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
