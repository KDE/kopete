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

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kapp.h>
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

#define NO_PLAYERS 3

K_EXPORT_COMPONENT_FACTORY(  kopete_nowlistening, KGenericFactory<NowListeningPlugin> );

NowListeningPlugin::NowListeningPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
	: KopetePlugin( parent, name )
{
	kdDebug() << "NowListeningPlugin::NowListeningPlugin()" << endl;
	// make these pointers safe until init'd
	m_actionCollection = 0L;
	m_actionWantsAdvert = 0L;
	m_currentMetaContact = 0L;

	// initialise preferences
	
	m_prefs = new NowListeningPreferences( "", this );
	connect ( m_prefs, SIGNAL( saved() ), this, SLOT( slotSettingsChanged() ) );
	// get a pointer to the dcop client
	
	m_client = kapp->dcopClient();
	m_client->attach();
	
	// set up known media players
	m_mediaPlayer = new NLMediaPlayer*[ NO_PLAYERS ];
	m_mediaPlayer[ 0 ] = new NLKscd( m_client );
	m_mediaPlayer[ 1 ] = new NLNoatun( m_client );
	m_mediaPlayer[ 2 ] = new NLXmms();
	
	// set up poll timers
	m_pollTimer = new QTimer();
	
	// connect timer signals to dcop poll methods
	connect( m_pollTimer, SIGNAL( timeout() ),
			this, SLOT( slotPollPlayers() ) );
	m_pollTimer->start( m_prefs->pollFrequency() * 1000 );
	
	// set up the greeting
	m_message = m_prefs->message();
}

KActionCollection *NowListeningPlugin::customContextMenuActions( KopeteMetaContact *m )
{
	kdDebug() << "NowListeningPlugin::customContextMenuActions() - for " << 
		m->displayName() << endl;
	delete m_actionCollection;
	m_actionCollection = new KActionCollection( this );
	m_actionWantsAdvert = new KToggleAction( "Now Listening", KStdGuiItem::ok().iconName(),
			0, m_actionCollection, "actionWantsAdvert" );
	m_actionWantsAdvert->setChecked( m->pluginData( this ).first() == "true" );
	connect( m_actionWantsAdvert, SIGNAL( toggled( bool ) ),
			this, SLOT( slotContactWantsToggled( bool ) ) );
	m_actionCollection->insert( m_actionWantsAdvert );
	m_currentMetaContact = m;
	return m_actionCollection;
}

void NowListeningPlugin::slotContactWantsToggled( bool on )
{
	kdDebug() << "NowListeningPlugin::slotContactsWantsToggled()" << endl;
	if ( m_actionWantsAdvert && m_currentMetaContact )
	{
		/*
		kdDebug() << "NowListeningPlugin::slotContactsWantsToggled()" << 
			" - setting PLD for " << m_currentMetaContact->displayName() << " to " << ( on ? "true" : "false" ) << endl;
		*/
		m_currentMetaContact->setPluginData( this, ( on ? "true" : "false" ) );
		/*
		//kdDebug() << "NowListeningPlugin::slotContactsWantsToggled()" <<
			" - PLD reads as " << m_currentMetaContact->pluginData( this ).first() << endl;
		*/
	}
}

void NowListeningPlugin::slotPollPlayers()
{
	kdDebug() << "NowListeningPlugin::slotPollPlayers()" << endl;
	bool sthToAdvertise = false;
	QString message = m_message;
	// use m_message for header, for now
	QString header = m_message;
	// temporary prototypes, to be made configurable
	QString conjunction = ", and ";
	QString perTrack = "%track by %artist on $album";

	// see if there is something new
	for ( int i = 0; i < NO_PLAYERS; i++ )
	{
		m_mediaPlayer[ i ]->update();
		if ( m_mediaPlayer[ i ]->playing() && m_mediaPlayer[ i ]->newTrack() )
		{
			sthToAdvertise = true;
			// SIMPLE MESSAGE WITH STATIC HEADER, NO SUBSTITUTION
			// generate a message 
			kdDebug() << m_mediaPlayer[ i ]->name() << " says it's playing "
				<< " a " << ( m_mediaPlayer[ i ]->newTrack() ? " new" : "old" )  << " track" << endl;
			/*if ( message != m_message ) // > 1 track playing!
				message = message + "\nand: ";
			message = message + m_mediaPlayer[ i ]->track();
			if ( !m_mediaPlayer[ i ]->artist().isEmpty() )
				message += " by " + m_mediaPlayer[ i ]->artist();
			if ( !m_mediaPlayer[ i ]->album().isEmpty( ) )
				message = message + " on " + m_mediaPlayer[ i ]->album();
			*/
			// SUBSTITUTION MESSAGE
			// get the prototype (header, per track, and conjunction)
			// substitutable strings are
			// %artist, %track, %album
			if ( message != header ) // > 1 track playing!
				message = message + conjunction;
			QString thisTrack = perTrack;
			thisTrack.replace( "%track", m_mediaPlayer[  i ]->track() );
			thisTrack.replace( "%artist", m_mediaPlayer[  i ]->artist() );
			thisTrack.replace( "%album", m_mediaPlayer[  i ]->album() );
			message = message + thisTrack;
		}
	}
	// tell anyone who is interested
	if ( sthToAdvertise )
		advertiseNewTracks( message );
}

void NowListeningPlugin::advertiseNewTracks(QString message)
{
	
	// tell each active chat about the new song 
	
	// get the list of active chats
	KopeteMessageManagerDict allsessions = 
		KopeteMessageManagerFactory::factory()->sessions();
	// for each active chat
	for ( QIntDictIterator<KopeteMessageManager> it( allsessions );
			it.current();
			++it )
	{
		if ( it.current()->widget() != 0L )
		{
			// then we have a live session?
			kdDebug() << "NowListeningPlugin::advertiseNewTracks() " <<
				it.currentKey() << " : " << it.current() << endl;
			// send message to all contacts (except self?)
			// Debug the to list
			/*KopeteContactPtrList pl = it.current()->members();
			QString tolist;
			for ( pl.first(); pl.current(); pl.next() )
				tolist = tolist + (long)pl.current();
			kdDebug() << "NowListeningPlugin::advertiseNewTracks() to list is " << tolist << endl;
			*/
			// make the destination the set of members who are interested
			// in our output
			KopeteContactPtrList pl = it.current()->members();
			for ( pl.first(); pl.current(); pl.next() )
			{ 
				QStringList myData = pl.current()->metaContact()->pluginData( this );
				if ( !myData.isEmpty( ) && myData.first() != "true" )
					pl.remove();
			}
			// get on with it
			kdDebug() << "NowListeningPlugin::advertiseNewTracks() - " <<
				( pl.isEmpty() ? "has no " : "has " ) << "interested recipients: " << endl;
			for ( pl.first(); pl.current(); pl.next() )
				kdDebug() << "NowListeningPlugin::advertiseNewTracks() " << pl.current()->displayName() << endl;
			// if no-one in this KMM wants to be advertised to, don't send
			// any message
			if ( pl.isEmpty() )
				break;
			KopeteMessage msg( it.current()->user(),
					/*it.current()->members(),*/
					pl,
					message,
					KopeteMessage::Outbound);
			it.current()->slotMessageSent( msg );
		}
	}
	// for now, just print a debug message
	//kdDebug() << "NowListeningPlugin::advertiseNewTracks() - " << 
		//message << endl;
}

void NowListeningPlugin::slotSettingsChanged()
{
	m_pollTimer->start( m_prefs->pollFrequency() * 1000 );
	m_message = m_prefs->message();
}

NowListeningPlugin::~NowListeningPlugin()
{
	kdDebug() << "NowListeningPlugin::~NowListeningPlugin()" << endl;
	// clean up our array of media player instances
	for ( int i = 0; i < NO_PLAYERS; i++ )
	{
		delete m_mediaPlayer[ i ];
	}
	delete m_mediaPlayer;
	
	// cleanly end DCOP
	m_client->detach();
}

//#include "nowlisteningplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
