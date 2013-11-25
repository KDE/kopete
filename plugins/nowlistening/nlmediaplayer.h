/*
    nlmediaplayer.h

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	Represents a generic media player
    and abstracts real media players' actual interfaces (DCOP for KDE apps,
	otherwise anything goes!

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NLMEDIAPLAYER_H
#define NLMEDIAPLAYER_H

#include <QString>

class NLMediaPlayer
{
	public:
		enum NLMediaType { Audio, Video };
		NLMediaPlayer() { m_playing = false; m_artist = ""; m_album = ""; m_track = ""; m_newTrack = false; }
		virtual ~NLMediaPlayer() {}
		/**
		 * This communicates with the actual mediaplayer and updates
		 * the model of its state in this class
		 */
		virtual void update() = 0;
		bool playing() const { return m_playing; }
		bool newTrack() const { return m_newTrack; }
		QString artist() const { return m_artist; }
		QString album() const { return m_album; }
		QString track() const { return m_track; }
		QString name() const{ return m_name; }
		NLMediaType mediaType() const { return m_type; }
	protected:
		// The name of the application
		QString m_name;
		bool m_playing;
		bool m_newTrack;
		QString m_artist;
		QString m_album;
		QString m_track;
		NLMediaType m_type;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
