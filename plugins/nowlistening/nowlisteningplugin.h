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


#include "kopeteplugin.h"


class QTimer;
class DCOPClient;
class KActionCOllection;
class KToggleAction;
class NowListeningPreferences;
class NLMediaPlayer;

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
		virtual KActionCollection *customChatActions( KopeteMessageManager* );

	public slots:
		/**
		 * Apply updated settings from preferences instance
		 */
		void slotSettingsChanged();
		/**
		 * Perform any string substitution needed on outgoing messages
		 */
		void slotOutgoingMessage( KopeteMessage& msg );
	
	protected:
		/** 
		 * Constructs a string containing the track information for all
		 * players
		 */
		QString allPlayerAdvert() const;
		/**
		 * Constructs a string containing any changes to the media being played
		 */
		QString changesOnlyAdvert() const;
		/**  
		 * Creates the string for a single player
		 * @p player - the media player we're using
		 * @p in - the source format string
		 * @p bool - is this call within a set of brackets for conditional expansion?
		 */
		QString substDepthFirst( NLMediaPlayer *player, QString in, bool inBrackets) const;
		/**
		 * Sends a message to a single chat
		 */
		void advertiseToChat( KopeteMessageManager* theChat, QString message );
	
	protected slots:
		/**
		 * called to change whether a contact should receive adverts
		 */
		void slotContactWantsToggled( bool on );
		/**
		 * called to reactively send an advert
		 */
		void slotAdvertToCurrentChat();
		/** 
		 * informs all active chats of any changes since last
		 */
		void slotChangesToAllChats();
	
	private:
		// used to access the GUI settings
		NowListeningPreferences *m_prefs;
		// abstracted media player interfaces
		NLMediaPlayer **m_mediaPlayer;
		// Triggers slotChangesToAllChats
		QTimer *m_pollTimer;
		// Needed for DCOP interprocess communication
		DCOPClient *m_client;
		// Support GUI actions
		KActionCollection *m_actionCollection;
		KopeteMessageManager *m_currentMessageManager;
		KToggleAction *m_actionWantsAdvert;
		KopeteMetaContact *m_currentMetaContact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
