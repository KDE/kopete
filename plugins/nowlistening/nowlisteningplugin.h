/*
    nowlisteningplugin.h

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NOWLISTENINGPLUGIN_H
#define NOWLISTENINGPLUGIN_H

#include <kaction.h>
#include "kopeteplugin.h"
#include "nowlisteningpreferences.h"
#include "nlmediaplayer.h"

class QTimer;
class DCOPClient;

/**
 * @author Will Stephenson
 */
class NowListeningPlugin : public KopetePlugin
{
		Q_OBJECT
		public:
			NowListeningPlugin(  QObject *parent, const char *name, const QStringList &args );
			virtual ~NowListeningPlugin();
			virtual KActionCollection *customContextMenuActions( KopeteMetaContact* );
//			virtual KActionCollection *customChatActions( KopeteMessageManager* );


		public slots:
			/**
			 * Apply updated settings from preferences instance
			 */
			void slotSettingsChanged();
			void slotContactWantsToggled( bool on );
		protected:
			/** 
			 * Constructs and sends the message
			 */
			void advertiseNewTracks( QString message );
		protected slots:
			/**
			 * Polls all players for current state and sends "now listening" message
			 */
			 void slotPollPlayers();
		private:
			// Points to the preferences instance
			NowListeningPreferences *m_prefs;
			// Array of pointers to media player interfaces
			NLMediaPlayer **m_mediaPlayer;
			// Triggers slotPollPlayers
			QTimer *m_pollTimer;
			// The initial part of the "now listening" message
			QString m_message;
			// Needed for DCOP interprocess communication
			DCOPClient *m_client;
			KActionCollection *m_actionCollection;
			KToggleAction *m_actionWantsAdvert;
			KopeteMetaContact *m_currentMetaContact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
