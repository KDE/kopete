/*
    Privacy Plugin - Filter messages 

    Copyright (c) 2006 by Andre Duffeck <andre@duffeck.de>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kgenericfactory.h>
#include <kicon.h>
#include <knotification.h>

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetemessage.h"
#include "kopetemessageevent.h"
#include "kopetechatsessionmanager.h"
#include "kopeteprotocol.h"
#include "kopetecontactlist.h"
#include "kopetepluginmanager.h"
#include "privacymessagehandler.h"
#include "privacyconfig.h"

#include "privacyplugin.h"

typedef KGenericFactory<PrivacyPlugin> PrivacyPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_privacy, PrivacyPluginFactory( "kopete_privacy" )  )
PrivacyPlugin * PrivacyPlugin::pluginStatic_ = 0L;

PrivacyPlugin::PrivacyPlugin( QObject *parent, const QStringList & )
: Kopete::Plugin( PrivacyPluginFactory::instance(), parent )
{
	kDebug(14313) << k_funcinfo << endl;
	if( !pluginStatic_ )
		pluginStatic_ = this;

	KAction *addToWhiteList = new KAction( KIcon("privacy_whitelist"), i18n("Add to WhiteList" ),
		actionCollection(), "addToWhiteList" );
	connect(addToWhiteList, SIGNAL(triggered(bool)), this, SLOT(slotAddToWhiteList()));
	KAction *addToBlackList = new KAction( KIcon("privacy_blacklist"), i18n("Add to BlackList" ),
		actionCollection(), "addToBlackList" );
	connect(addToBlackList, SIGNAL(triggered(bool)), this, SLOT(slotAddToBlackList()));

	setXMLFile("privacyui.rc");

	m_inboundHandler = new PrivacyMessageHandlerFactory( Kopete::Message::Inbound,
		Kopete::MessageHandlerFactory::InStageStart, this, SLOT( slotIncomingMessage( Kopete::MessageEvent * ) ) );

	connect( this, SIGNAL( settingsChanged() ), this, SLOT( slotSettingsChanged() ) );
}


PrivacyPlugin::~PrivacyPlugin()
{
	kDebug(14313) << k_funcinfo << endl;
	pluginStatic_ = 0L;
	delete m_inboundHandler;
}

PrivacyPlugin *PrivacyPlugin::plugin()
{
	return pluginStatic_ ;
}

PrivacyConfig *PrivacyPlugin::config()
{
	return PrivacyConfig::self();
}

void PrivacyPlugin::slotSettingsChanged()
{
	PrivacyConfig::self()->readConfig();
}

void PrivacyPlugin::slotAddToWhiteList()
{	
	QStringList whitelist = PrivacyConfig::whiteList();
	foreach( Kopete::MetaContact *metacontact, Kopete::ContactList::self()->selectedMetaContacts() )
	{
		foreach( Kopete::Contact *contact, metacontact->contacts() )
		{
			QString entry( contact->protocol()->pluginId() + ":" + contact->contactId() );
			if( !whitelist.contains( entry ) )
				whitelist.append( entry );
		}
	}
	PrivacyConfig::setWhiteList( whitelist );
	PrivacyConfig::self()->writeConfig();
}

void PrivacyPlugin::slotAddToBlackList()
{	
	QStringList blacklist = PrivacyConfig::blackList();
	foreach( Kopete::MetaContact *metacontact, Kopete::ContactList::self()->selectedMetaContacts() )
	{
		foreach( Kopete::Contact *contact, metacontact->contacts() )
		{
			QString entry( contact->protocol()->pluginId() + ":" + contact->contactId() );
			if( !blacklist.contains( entry ) )
				blacklist.append( entry );
		}
	}
	PrivacyConfig::setBlackList( blacklist );
	PrivacyConfig::self()->writeConfig();
}

void PrivacyPlugin::slotIncomingMessage( Kopete::MessageEvent *event )
{
	Kopete::Message msg = event->message();

	if( msg.direction() == Kopete::Message::Outbound ||
		msg.direction() == Kopete::Message::Internal )
		return;

	// Verify sender
	if( PrivacyConfig::sender_AllowNoneButWhiteList() )
	{
		if( !PrivacyConfig::whiteList().contains( msg.from()->protocol()->pluginId() + ":" + msg.from()->contactId() ) )
		{
			kDebug(14313) << k_funcinfo << "Message from " << msg.from()->protocol()->pluginId() << ":" << msg.from()->contactId() << " dropped (not whitelisted)" << endl;
			KNotification::event( "message_dropped", i18n("A message from %1 was dropped, because this contact is not on your whitelist.").arg(msg.from()->contactId()) );
			event->discard();
			return;
		}
	}
	else if( PrivacyConfig::sender_AllowAllButBlackList() )
	{
		if( PrivacyConfig::blackList().contains( msg.from()->protocol()->pluginId() + ":" + msg.from()->contactId() ) )
		{
			kDebug(14313) << k_funcinfo << "Message from " << msg.from()->protocol()->pluginId() << ":" << msg.from()->contactId() << " dropped (blacklisted)" << endl;
			KNotification::event( "message_dropped", i18n("A message from %1 was dropped, because this contact is on your blacklist.").arg(msg.from()->contactId()) );
			event->discard();
			return;
		}
	}
	else if( PrivacyConfig::sender_AllowNoneButContactList() )
	{
		if( msg.from()->metaContact()->isTemporary() )
		{
			kDebug(14313) << k_funcinfo << "Message from " << msg.from()->contactId() << " dropped (not on the contactlist)" << endl;
			KNotification::event( "message_dropped", i18n("A message from %1 was dropped, because this contact is not on your contactlist.").arg(msg.from()->contactId()) );
			event->discard();
			return;
		}
	}

	// Verify content
	if( PrivacyConfig::content_DropIfAny() )
	{
		foreach(QString word, PrivacyConfig::dropIfAny().split(',') )
		{
			if( msg.plainBody().contains( word ) )
			{
				kDebug(14313) << k_funcinfo << "Message dropped because it contained: " << word << endl;
				KNotification::event( "message_dropped", i18n("A message from %1 was dropped, because it contained a blacklisted word.").arg(msg.from()->contactId()) );
				event->discard();
				return;
			}
		}
	}

	if( PrivacyConfig::content_DropIfAll() )
	{
		bool drop = true;
		foreach(QString word, PrivacyConfig::dropIfAll().split(',') )
		{
			if( !msg.plainBody().contains( word ) )
			{
				drop = false;
				break;
			}
		}
		if( drop )
		{
			kDebug(14313) << k_funcinfo << "Message dropped because it contained blacklisted words." << endl;
				KNotification::event( "message_dropped", i18n("A message from %1 was dropped, because it contained blacklisted words.").arg(msg.from()->contactId()) );
			event->discard();
			return;
		}
	}
}

#include "privacyplugin.moc"
