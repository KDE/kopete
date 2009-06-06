/*
    nlquodlibet.h

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

#ifndef NLQUODLIBET_H
#define NLQUODLIBET_H

#include <QDateTime>
#include <QFile>
#include <QObject>

#include "nlmediaplayer.h"

class KDirWatch;

class NLQuodLibet :  public QObject, public NLMediaPlayer
{
Q_OBJECT
	public:
		NLQuodLibet();
		virtual ~NLQuodLibet();
		virtual void update();
	protected:
		QString currentTrackPath() const;
		void parseFile( QFile & );
		void parseLine( const QString & );
	protected Q_SLOTS:
		void fileChanged( const QString & path );
	private:
		QDateTime m_timestamp;
		KDirWatch * m_watch;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
