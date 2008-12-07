/*
    nowlisteningplugin.h

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2005           by Michaël Larouche <larouche@kde.org>

    Kopete    (c) 2002-2005      by the Kopete developers  <kopete-devel@kde.org>

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

#include <QVariantList>

namespace Kopete { class ChatSession; class Message; }

class NLMediaPlayer;

/**
 * @author Will Stephenson
 * @author Michaël Larouche
 */
class NowListeningPlugin : public Kopete::Plugin
{
	Q_OBJECT

friend class NowListeningGUIClient;

	public:
		NowListeningPlugin(  QObject *parent, const QVariantList &args );
		virtual ~NowListeningPlugin();
		static NowListeningPlugin* plugin();

	public slots:
		void slotMediaCommand( const QString &, Kopete::ChatSession *theChat );
		void slotOutgoingMessage(Kopete::Message&);
		void slotAdvertCurrentMusic();

	protected:
		/**
		 * Constructs a string containing the track information.
		 * @param update Whether the players must update their data. It can be
		 *               useful to set it to false if one already has called
		 *               update somewhere else, for instance in newTrackPlaying().
		 */
		QString mediaPlayerAdvert(bool update = true);
		/**
		 * @internal Build the message for @ref mediaPlayerAdvert
		 * @param message Reference to the messsage, because return QString cause data loss.
		 * @param player Pointer to the current Media Player. 
		 *               Used to get the information about the current track playing.
		 * @param update Whether the players must update their data. It can be
		 *               useful to set it to false if one already has called
		 *               update somewhere else, for instance in newTrackPlaying().
		 */
		void buildTrackMessage(QString &message, NLMediaPlayer *player, bool update);
		/**
		 * @return true if one of the players has changed track since the last message.
		 */
		bool newTrackPlaying(void) const;
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
		void advertiseToChat( Kopete::ChatSession* theChat, QString message );
		/**
		 * Update the currentMedia pointer on config change.
		 */
		void updateCurrentMediaPlayer();

	protected slots:
		/**
		 * Reacts to a new chat starting and adds actions to its GUI
		 */
		void slotNewKMM( Kopete::ChatSession* );
		
		/**
		 * Reacts to the plugin's settings changed signal, originating from the KCModule dispatcher
		 */ 
		void slotSettingsChanged();

	private:
		class Private;
		Private * const d;

		static NowListeningPlugin* pluginStatic_;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
