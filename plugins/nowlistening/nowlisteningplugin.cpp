/*
    nowlisteningplugin.cpp

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

#include <qtimer.h>
#include <qdict.h>
#if QT_VERSION < 0x030100
#include <qregexp.h>
#endif

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kaction.h>
#include <kstdguiitem.h>

//#include "kopete.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "nowlisteningpreferences.h"
#include "nowlisteningplugin.h"
#include "nlmediaplayer.h"
#include "nlkscd.h"
#include "nlnoatun.h"
#include "nlxmms.h"

K_EXPORT_COMPONENT_FACTORY(  kopete_nowlistening, KGenericFactory<NowListeningPlugin> );

NowListeningPlugin::NowListeningPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
	: KopetePlugin( parent, name )
{
	kdDebug(14307) << "NowListeningPlugin::NowListeningPlugin()" << endl;
	// make these pointers safe until init'd
	m_actionCollection = 0L;
	m_actionWantsAdvert = 0L;
	m_currentMetaContact = 0L;
	m_currentMessageManager = 0L;

	// initialise preferences
	
	m_prefs = new NowListeningPreferences( "", this );
	connect ( m_prefs, SIGNAL( saved() ), this, SLOT( slotSettingsChanged() ) );
	// get a pointer to the dcop client
	
	m_client = kapp->dcopClient();
	m_client->attach();
	
	// set up known media players
	m_mediaPlayer = new QPtrList<NLMediaPlayer>;
	m_mediaPlayer->setAutoDelete( true );
	m_mediaPlayer->append( new NLKscd( m_client ) );
	m_mediaPlayer->append( new NLNoatun( m_client ) );
	m_mediaPlayer->append( new NLXmms() );
	
	// set up poll timers
	m_pollTimer = new QTimer();
	
	// connect timer signals to dcop poll methods
	connect( m_pollTimer, SIGNAL( timeout() ),
			this, SLOT( slotChangesToAllChats() ) );
	m_pollTimer->start( m_prefs->pollFrequency() * 1000 );

	// watch for '/media' getting typed
	connect(  KopeteMessageManagerFactory::factory(),
			SIGNAL(  aboutToSend(  KopeteMessage & ) ),
			SLOT(  slotOutgoingMessage(  KopeteMessage & ) ) );
}

NowListeningPlugin::~NowListeningPlugin()
{
	kdDebug(14307) << "NowListeningPlugin::~NowListeningPlugin()" << endl;

	delete m_mediaPlayer;
	
	// cleanly end DCOP
	m_client->detach();
}

KActionCollection *NowListeningPlugin::customContextMenuActions( KopeteMetaContact *m )
{
	kdDebug(14307) << "NowListeningPlugin::customContextMenuActions() - for " << 
		m->displayName() << endl;
	delete m_actionCollection;
	m_actionCollection = new KActionCollection( this );
	m_actionWantsAdvert = new KToggleAction( "Now Listening", 0, this, 
			SLOT( slotContactWantsToggled() ), m_actionCollection, "actionWantsAdvert" );
	m_actionWantsAdvert->setChecked( m->pluginData( this ).first() == "true" );
	m_actionCollection->insert( m_actionWantsAdvert );
	m_currentMetaContact = m;
	return m_actionCollection;
}

KActionCollection *NowListeningPlugin::customChatActions(  KopeteMessageManager* kmm )
{
	kdDebug(14307) << "NowListeningPlugin::customChatActions()" << endl;
	delete m_actionCollection;
	m_actionCollection = new KActionCollection( this );
	KAction *actionSendAdvert = new KAction( "Send Media Info", 0, this,
			SLOT( slotAdvertToCurrentChat() ), m_actionCollection, "actionSendAdvert" );
	m_actionCollection->insert( actionSendAdvert );
	m_currentMessageManager = kmm;
	return m_actionCollection;
}

void NowListeningPlugin::slotContactWantsToggled()
{
	kdDebug(14307) << "NowListeningPlugin::slotContactsWantsToggled()" << endl;
	if ( m_actionWantsAdvert && m_currentMetaContact )
	{
		m_currentMetaContact->setPluginData( this, ( m_actionWantsAdvert->isChecked() 
			? "true" : "false" ) );
	}
	return;
}

void NowListeningPlugin::slotAdvertToCurrentChat()
{
	//advertise  to a single chat
	if ( m_currentMessageManager )
		advertiseToChat( m_currentMessageManager, allPlayerAdvert() );
	return;
}	

void NowListeningPlugin::slotChangesToAllChats()
{
	kdDebug(14307) << "NowListeningPlugin::slotChangesToAllChats()" << endl;
	//bool sthToAdvertise = false;
	QString message = changesOnlyAdvert();
	// tell each active chat about the new tracks
	if ( !message.isEmpty() )
	{
		// get the list of active chats
		KopeteMessageManagerDict allsessions = 
			KopeteMessageManagerFactory::factory()->sessions();
		// for each active chat
		for ( QIntDictIterator<KopeteMessageManager> it( allsessions );
				it.current();
				++it )
		{
			advertiseToChat( it.current(), message );
		}
	}
	return;
}

void NowListeningPlugin::slotOutgoingMessage( KopeteMessage& msg )
{
	//NB DOESN'T RESPECT USER PREFS"!!!
	QString originalBody = msg.plainBody();
	// look for messages that we've generated and ignore them
	if ( !originalBody.startsWith( m_prefs->header() ) )
	{
		// look for the string '/media'
		if ( originalBody.startsWith( "/media" ) )
		{
			// replace it with media advert
			QString newBody = allPlayerAdvert() + originalBody.right( 
					originalBody.length() - 6 );
			msg.setBody( newBody, KopeteMessage::RichText );
		}	
		return;
	}
}

QString NowListeningPlugin::allPlayerAdvert() const
{
	// generate message for all players
	QString message = m_prefs->header();
	QString perTrack = m_prefs->perTrack();
	
	for ( NLMediaPlayer* i = m_mediaPlayer->first(); i; i = m_mediaPlayer->next() )
	{
		i->update();
		if ( i->playing() )
		{
			if (  message != m_prefs->header() ) // > 1 track playing!
				message = message + m_prefs->conjunction();
			message = message + substDepthFirst( i, perTrack, false );
		}
	}
	return message;
}
	
QString NowListeningPlugin::changesOnlyAdvert() const
{	
	QString message = "";
	QString perTrack = m_prefs->perTrack();

	// see if there is something new
	for ( NLMediaPlayer* i = m_mediaPlayer->first(); i; i = m_mediaPlayer->next() )
	{
		i->update();
		if ( i->playing() && i->newTrack() )
		{
			if ( message.isEmpty() )
				message = m_prefs->header();
			// generate a message 
			//kdDebug(14307) << m_mediaPlayer[ i ]->name() << " says it's playing "
			//	<< "a " << ( m_mediaPlayer[ i ]->newTrack() ? "new" : "old" )  << " track" << endl;
			// CLEVER SUBSTITUTION WITH DEPTH FIRST SEARCH FOR 
			// INCLUSION CONDITIONAL ON SUBSTITUTION BEING MADE
			if (  message != m_prefs->header() ) // > 1 track playing!
				message = message + m_prefs->conjunction();
			//kdDebug(14307) << i->track() << i->artist() << i->album() << i->name() << endl;
			message = message + substDepthFirst( i, perTrack, false );
		}
	}
	return message;
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
	if ( in.contains ( "%track" ) && !track.isEmpty() )
	{
#if QT_VERSION < 0x030100
		in.replace( QRegExp( "%track" ), track );
#else
		in.replace( "%track", track );
#endif
		done = true;
	}
	if ( in.contains ( "%artist" ) && !artist.isEmpty() )
	{
#if QT_VERSION < 0x030100
		in.replace( QRegExp( "%artist" ), artist );
#else
		in.replace( "%artist", artist );
#endif
		done = true;
	}
	if ( in.contains ( "%album" ) && !album.isEmpty() )
	{
#if QT_VERSION < 0x030100
		in.replace( QRegExp( "%album" ), album );
#else
		in.replace( "%album", album );
#endif
		done = true;
	}
	if ( in.contains ( "%player" ) && !playerName.isEmpty() )
	{
#if QT_VERSION < 0x030100
		in.replace( QRegExp( "%player" ), playerName );
#else
		in.replace( "%player", playerName );
#endif
		done = true;
	}
	//kdDebug(14307) << "Result is: " << in << endl;
	// make whether we return anything dependent on whether we
	// were in brackets and if we were, if a substitution was made.
	if ( inBrackets && !done )
		return "";
	else
		return in;
}

void NowListeningPlugin::advertiseToChat( KopeteMessageManager *theChat, QString message )
{
	if ( theChat->widget() != 0L )
	{
		// then we have a live session?

		// reduce the recipients to the set of members who are interested
		// in our output
		KopeteContactPtrList pl = theChat->members();
		QStringList myData;
		// avoid skipping one member when removing 
		// (old version's pl.remove(); pl.next() skipped because 
		// remove() moves on to next for you.
		pl.first();
		while ( pl.current() )
		{
			myData = pl.current()->metaContact()->pluginData( this );
			if ( myData.isEmpty() || myData.first() != "true" )
				pl.remove();
			else
				pl.next();
		}
		// get on with it
		kdDebug(14307) << "NowListeningPlugin::advertiseNewTracks() - " <<
			( pl.isEmpty() ? "has no " : "has " ) << "interested recipients: " << endl;
		for ( pl.first(); pl.current(); pl.next() )
			kdDebug(14307) << "NowListeningPlugin::advertiseNewTracks() " << pl.current()->displayName() << endl;
		// if no-one in this KMM wants to be advertised to, don't send
		// any message
		if ( pl.isEmpty() )
			return;
		KopeteMessage msg( theChat->user(),
				pl,
				message,
				KopeteMessage::Outbound,
				KopeteMessage::RichText );
		theChat->slotMessageSent( msg );
	}
	return;
}
void NowListeningPlugin::slotSettingsChanged()
{
	m_pollTimer->start( m_prefs->pollFrequency() * 1000 );
}

#include "nowlisteningplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
