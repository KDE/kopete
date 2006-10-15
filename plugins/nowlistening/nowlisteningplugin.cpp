/*
    nowlisteningplugin.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2005-2006           by Michaël Larouche <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2006      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>
#include <qstringlist.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kaction.h>

#include "config.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopetecommandhandler.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "kopeteaccountmanager.h"

#include "nowlisteningconfig.h"
#include "nowlisteningplugin.h"
#include "nlmediaplayer.h"
#include "nlkscd.h"
#include "nlnoatun.h"
#include "nljuk.h"
#include "nlamarok.h"
#include "nlkaffeine.h"
#include "nowlisteningguiclient.h"

#if defined Q_WS_X11 && !defined K_WS_QTONLY && defined HAVE_XMMS
#include "nlxmms.h"
#endif

class NowListeningPlugin::Private
{
public:
	Private() : m_currentMediaPlayer(0L), m_client(0L), m_currentChatSession(0L), m_currentMetaContact(0L), 
				advertTimer(0L)
	{}

	// abstracted media player interfaces
	QPtrList<NLMediaPlayer> m_mediaPlayerList;
	NLMediaPlayer *m_currentMediaPlayer;

	// Needed for DCOP interprocess communication
	DCOPClient *m_client;
	Kopete::ChatSession *m_currentChatSession;
	Kopete::MetaContact *m_currentMetaContact;

	// Used when using automatic advertising to know who has already gotten
	// the music information
	QStringList m_musicSentTo;

	// Used when advertising to status message.
	QTimer *advertTimer;
};

typedef KGenericFactory<NowListeningPlugin> NowListeningPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_nowlistening, NowListeningPluginFactory( "kopete_nowlistening" )  )

NowListeningPlugin::NowListeningPlugin( QObject *parent, const char* name, const QStringList& /*args*/ )
: Kopete::Plugin( NowListeningPluginFactory::instance(), parent, name )
{
	if ( pluginStatic_ )
		kdDebug( 14307 )<<"####"<<"Now Listening already initialized"<<endl;
	else
		pluginStatic_ = this;

	d = new Private;

	kdDebug(14307) << k_funcinfo << endl;

	// Connection for the "/media" command (always needed)
	connect( Kopete::ChatSessionManager::self(), SIGNAL(
			chatSessionCreated( Kopete::ChatSession * )) , SLOT( slotNewKMM(
			Kopete::ChatSession * ) ) );

	// If autoadvertising is on...
	connect(Kopete::ChatSessionManager::self(),
			SIGNAL(aboutToSend(Kopete::Message&)),
			this,
			SLOT(slotOutgoingMessage(Kopete::Message&)));

	QValueList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();
	for (QValueListIterator<Kopete::ChatSession*> it= sessions.begin(); it!=sessions.end() ; ++it)
	  slotNewKMM( *it );

	// get a pointer to the dcop client
	d->m_client = kapp->dcopClient(); //new DCOPClient();

	// set up known media players
	d->m_mediaPlayerList.setAutoDelete( true );
	d->m_mediaPlayerList.append( new NLKscd( d->m_client ) );
	d->m_mediaPlayerList.append( new NLNoatun( d->m_client ) );
	d->m_mediaPlayerList.append( new NLJuk( d->m_client ) );
	d->m_mediaPlayerList.append( new NLamaroK( d->m_client ) );
	d->m_mediaPlayerList.append( new NLKaffeine( d->m_client ) );

#if defined Q_WS_X11 && !defined K_WS_QTONLY && HAVE_XMMS
	d->m_mediaPlayerList.append( new NLXmms() );
#endif

	// User has selected a specific mediaPlayer so update the currentMediaPlayer pointer.
	if( NowListeningConfig::self()->useSpecifiedMediaPlayer() )
	{
		updateCurrentMediaPlayer();
	}

	// watch for '/media' getting typed
	Kopete::CommandHandler::commandHandler()->registerCommand(
		this,
		"media",
		SLOT( slotMediaCommand( const QString &, Kopete::ChatSession * ) ),
		i18n("USAGE: /media - Displays information on current song"),
		0
	);

	connect ( this , SIGNAL( settingsChanged() ) , this , SLOT( slotSettingsChanged() ) );

	// Advert the accounts with the current listened track.
	d->advertTimer = new QTimer(this);
	connect(d->advertTimer, SIGNAL( timeout() ), this, SLOT( slotAdvertCurrentMusic() ) );
	d->advertTimer->start(5000); // Update every 5 seconds
}

NowListeningPlugin::~NowListeningPlugin()
{
	//kdDebug( 14307 ) << k_funcinfo << endl;

	delete d;

	pluginStatic_ = 0L;
}

void NowListeningPlugin::slotNewKMM(Kopete::ChatSession *KMM)
{
	new NowListeningGUIClient( KMM, this );
}

NowListeningPlugin* NowListeningPlugin::plugin()
{
	return pluginStatic_ ;
}

void NowListeningPlugin::slotMediaCommand( const QString &args, Kopete::ChatSession *theChat )
{
	QString advert = mediaPlayerAdvert();
	if ( advert.isEmpty() )
	{
		// Catch no players/no track playing message case:
		// Since we can't stop a message send in a plugin, add some message text to
		// prevent us sending an empty message
		advert = i18n("Message from Kopete user to another user; used when sending media information even though there are no songs playing or no media players running", "Now Listening for Kopete - it would tell you what I am listening to, if I was listening to something on a supported media player.");
	}
	
	Kopete::Message msg( theChat->myself(),
			theChat->members(),
			advert + " " + args,
			Kopete::Message::Outbound,
			Kopete::Message::RichText
	);
	
	theChat->sendMessage( msg );
}

void NowListeningPlugin::slotOutgoingMessage(Kopete::Message& msg)
{
	// Only do stuff if autoadvertising is on
	if(!NowListeningConfig::self()->chatAdvertising())
		return;

	QString originalBody = msg.plainBody();

	// If it is a /media message, don't process it
	if(originalBody.startsWith(NowListeningConfig::self()->header()))
		return;

	// What will be sent
	QString newBody;

	// Getting the list of contacts the message will be sent to to determine if at least
	// one of them has never gotten the current music information.
	Kopete::ContactPtrList dest = msg.to();
	bool mustSendAnyway = false;
	for( Kopete::Contact *c = dest.first() ; c ; c = dest.next() )
	{
		const QString& cId = c->contactId();
		if( 0 == d->m_musicSentTo.contains( cId ) )
		{
			mustSendAnyway = true;

			// The contact will get the music information so we put it in the list.
			d->m_musicSentTo.push_back( cId );
		}
	}

	bool newTrack = newTrackPlaying();

	// We must send the music information if someone has never gotten it or the track(s)
	// has changed since it was last sent.
	if ( mustSendAnyway || newTrack )
	{
		QString advert = mediaPlayerAdvert(false); // false since newTrackPlaying() did the update
		if( !advert.isEmpty() )
			newBody = originalBody + "<br>" + advert;

		// If we send because the information has changed since it was last sent, we must
		// rebuild the list of contacts the latest information was sent to.
		if( newTrack )
		{
			d->m_musicSentTo.clear();
			for( Kopete::Contact *c = dest.first() ; c ; c = dest.next() )
			{
				d->m_musicSentTo.push_back( c->contactId() );
			}
		}
	}

	// If the body has been modified, change the message
	if( !newBody.isEmpty() )
	{
		msg.setBody( newBody, Kopete::Message::RichText );
 	}
}

void NowListeningPlugin::slotAdvertCurrentMusic()
{
	// Do anything when statusAdvertising is off.
	if( !NowListeningConfig::self()->statusAdvertising() && !NowListeningConfig::self()->appendStatusAdvertising() )
		return; 

	// This slot is called every 5 seconds, so we check if we have a new track playing.
	if( newTrackPlaying() )
	{
		QString advert;

		QPtrList<Kopete::Account> accountsList = Kopete::AccountManager::self()->accounts();
		for( Kopete::Account* a = accountsList.first(); a; a = accountsList.next() )
		{
			/*
				NOTE:
				MSN status message(personal message) use a special tag to advert the current music playing. 
				So, we don't send the all formatted string, send a special string seperated by ";".
				
				Also, do not use MSN hack in appending mode.
			*/
			if( a->protocol()->pluginId() == "MSNProtocol" && !NowListeningConfig::self()->appendStatusAdvertising() )
			{
				QString track, artist, album, mediaList;
				bool isPlaying=false;

				if( NowListeningConfig::self()->useSpecifiedMediaPlayer() && d->m_currentMediaPlayer )
				{
					if( d->m_currentMediaPlayer->playing() )
					{
						track = d->m_currentMediaPlayer->track();
						artist = d->m_currentMediaPlayer->artist();
						album = d->m_currentMediaPlayer->album();
						mediaList = track + ";" + artist + ";" + album;
						isPlaying = true;
					}
				}
				else
				{
					for ( NLMediaPlayer* i = d->m_mediaPlayerList.first(); i; i = d->m_mediaPlayerList.next() )
					{
						if( i->playing() )
						{
							track = i->track();
							artist = i->artist();
							album = i->album();
							mediaList = track + ";" + artist + ";" + album;
							isPlaying = true;
						}
					}
				}

				// KDE4 TODO: Use the new status message framework, and remove this "hack".
				if( isPlaying )
				{
					advert = QString("[Music]%1").arg(mediaList);
				}

			}
			else
			{
				if( NowListeningConfig::self()->appendStatusAdvertising() )
				{
					// Check for the now listening message in parenthesis, 
					// include the header to not override other messages in parenthesis.
					QRegExp statusSong( QString(" \\(%1.*\\)$").arg( NowListeningConfig::header()) );
					
					// HACK: Don't keep appending the now listened song. Replace it in the status message.
					advert = a->myself()->property( Kopete::Global::Properties::self()->awayMessage() ).value().toString();
					// Remove the braces when they are no listened song.
					QString mediaAdvert = mediaPlayerAdvert(false);
					if(!mediaAdvert.isEmpty())
					{
						if(statusSong.search(advert) != -1)
						{
							advert = advert.replace(statusSong, QString(" (%1)").arg(mediaPlayerAdvert(false)) );
						}
						else
						{
							advert += QString(" (%1)").arg( mediaPlayerAdvert(false) );
						}
					}
					else
					{
						advert = advert.replace(statusSong, "");
					}
				}
				else
				{
					advert = mediaPlayerAdvert(false); // newTrackPlaying has done the update.
				}
			}

			a->setOnlineStatus(a->myself()->onlineStatus(), advert);
		}
	}
}

QString NowListeningPlugin::mediaPlayerAdvert(bool update)
{
	// generate message for all players
	QString message;
	
	if( NowListeningConfig::self()->useSpecifiedMediaPlayer() && d->m_currentMediaPlayer != 0L )
	{
		buildTrackMessage(message, d->m_currentMediaPlayer, update);
	}
	else
	{
		for ( NLMediaPlayer* i = d->m_mediaPlayerList.first(); i; i = d->m_mediaPlayerList.next() )
		{
			buildTrackMessage(message, i, update);
		}
	}

	kdDebug( 14307 ) << k_funcinfo << message << endl;
			
	return message;
}

void NowListeningPlugin::buildTrackMessage(QString &message, NLMediaPlayer *player, bool update)
{
	QString perTrack = NowListeningConfig::self()->perTrack();

	if(update)
		player->update();
	if ( player->playing() )
	{
		kdDebug( 14307 ) << k_funcinfo << player->name() << " is playing" << endl;
		if ( message.isEmpty() )
			message = NowListeningConfig::self()->header();
	
		if ( message != NowListeningConfig::self()->header() ) // > 1 track playing!
			message = message + NowListeningConfig::self()->conjunction();
		message = message + substDepthFirst( player, perTrack, false );
	}
}

bool NowListeningPlugin::newTrackPlaying(void) const
{
	if( NowListeningConfig::self()->useSpecifiedMediaPlayer() && d->m_currentMediaPlayer != 0L )
	{
		d->m_currentMediaPlayer->update();
		if( d->m_currentMediaPlayer->newTrack() )
		return true;
	}
	else
	{
		for ( NLMediaPlayer* i = d->m_mediaPlayerList.first(); i; i = d->m_mediaPlayerList.next() )
		{
			i->update();
			if( i->newTrack() )
			return true;
		}
	}
	return false;
}

QString NowListeningPlugin::substDepthFirst( NLMediaPlayer *player,
		QString in, bool inBrackets ) const
{
	QString track = player->track();
	QString artist = player->artist();
	QString album = player->album();
	QString playerName = player->name();

	for ( unsigned int i = 0; i < in.length(); i++ )
	{
		QChar c = in.at( i );
		//kdDebug(14307) << "Now working on:" << in << " char is: " << c << endl;
		if ( c == '(' )
		{
			// find matching bracket
			int depth = 0;
			//kdDebug(14307) << "Looking for ')'" << endl;
			for ( unsigned int j = i + 1; j < in.length(); j++ )
			{
				QChar d = in.at( j );
				//kdDebug(14307) << "Got " << d << endl;
				if ( d == '(' )
					depth++;
				if ( d == ')' )
				{
					// have we found the match?
					if ( depth == 0 )
					{
						// recursively replace contents of matching ()
						QString substitution = substDepthFirst( player,
								in.mid( i + 1, j - i - 1), true ) ;
						in.replace ( i, j - i + 1, substitution );
						// perform substitution and return the result
						i = i + substitution.length() - 1;
						break;
					}
					else
						depth--;
				}
			}
		}
	}
	// no () found, perform substitution!
	// get each string (to) to substitute for (from)
	bool done = false;
	if ( in.contains ( "%track" ) )
	{
		if ( track.isEmpty() )
			track = i18n("Unknown track");

		in.replace( "%track", track );
		done = true;
	}

	if ( in.contains ( "%artist" ) && !artist.isEmpty() )
	{
		if ( artist.isEmpty() )
			artist = i18n("Unknown artist");
		in.replace( "%artist", artist );
		done = true;
	}
	if ( in.contains ( "%album" ) && !album.isEmpty() )
	{
		if ( album.isEmpty() )
			album = i18n("Unknown album");
		in.replace( "%album", album );
		done = true;
	}
	if ( in.contains ( "%player" ) && !playerName.isEmpty() )
	{
		if ( playerName.isEmpty() )
			playerName = i18n("Unknown player");
		in.replace( "%player", playerName );
		done = true;
	}
	// make whether we return anything dependent on whether we
	// were in brackets and if we were, if a substitution was made.
	if ( inBrackets && !done )
		return "";

	return in;
}

void NowListeningPlugin::advertiseToChat( Kopete::ChatSession *theChat, QString message )
{
	Kopete::ContactPtrList pl = theChat->members();

	// get on with it
	kdDebug(14307) << k_funcinfo <<
		( pl.isEmpty() ? "has no " : "has " ) << "interested recipients: " << endl;
/*	for ( pl.first(); pl.current(); pl.next() )
		kdDebug(14307) << "NowListeningPlugin::advertiseNewTracks() " << pl.current()->displayName() << endl; */
	// if no-one in this KMM wants to be advertised to, don't send
	// any message
	if ( pl.isEmpty() )
		return;
	Kopete::Message msg( theChat->myself(),
			pl,
			message,
			Kopete::Message::Outbound,
			Kopete::Message::RichText );
	theChat->sendMessage( msg );
}

void NowListeningPlugin::updateCurrentMediaPlayer()
{
	kdDebug(14307) << k_funcinfo << "Update current media player (single mode)" << endl;

	d->m_currentMediaPlayer = d->m_mediaPlayerList.at( NowListeningConfig::self()->selectedMediaPlayer() );
}

void NowListeningPlugin::slotSettingsChanged()
{
	// Force reading config
	NowListeningConfig::self()->readConfig();

	// Update the currentMediaPlayer, because config has changed.
	if( NowListeningConfig::useSpecifiedMediaPlayer() )
		updateCurrentMediaPlayer();

	disconnect(Kopete::ChatSessionManager::self(),
			   SIGNAL(aboutToSend(Kopete::Message&)),
			   this,
			   SLOT(slotOutgoingMessage(Kopete::Message&)));

	d->advertTimer->stop();
	disconnect(d->advertTimer, SIGNAL(timeout()), this, SLOT(slotAdvertCurrentMusic()));

	if( NowListeningConfig::self()->chatAdvertising() ) 
	{
		kdDebug(14307) << k_funcinfo << "Now using chat window advertising." << endl;

		connect(Kopete::ChatSessionManager::self(),
				SIGNAL(aboutToSend(Kopete::Message&)),
				this,
				SLOT(slotOutgoingMessage(Kopete::Message&)));
	}
	else if( NowListeningConfig::self()->statusAdvertising() || NowListeningConfig::self()->appendStatusAdvertising() )
	{
		kdDebug(14307) << k_funcinfo << "Now using status message advertising." << endl;

		connect(d->advertTimer, SIGNAL(timeout()), this, SLOT(slotAdvertCurrentMusic()));
		d->advertTimer->start(5000);
	}
}

NowListeningPlugin* NowListeningPlugin::pluginStatic_ = 0L;

#include "nowlisteningplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
