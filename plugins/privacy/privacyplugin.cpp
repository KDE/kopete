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
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetemessage.h"
#include "kopetemessageevent.h"
#include "kopetechatsessionmanager.h"
#include "kopeteprotocol.h"
#include "privacymessagehandler.h"
#include "privacyconfig.h"

#include <kgenericfactory.h>

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

void PrivacyPlugin::slotSettingsChanged()
{
	PrivacyConfig::self()->readConfig();
}

void PrivacyPlugin::slotIncomingMessage( Kopete::MessageEvent *event )
{
	kDebug(14313) << k_funcinfo << endl;

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
			event->discard();
			return;
		}
	}
	else if( PrivacyConfig::sender_AllowAllButBlackList() )
	{
		if( PrivacyConfig::blackList().contains( msg.from()->protocol()->pluginId() + ":" + msg.from()->contactId() ) )
		{
			kDebug(14313) << k_funcinfo << "Message from " << msg.from()->protocol()->pluginId() << ":" << msg.from()->contactId() << " dropped (blacklisted)" << endl;
			event->discard();
			return;
		}
	}
	else if( PrivacyConfig::sender_AllowNoneButContactList() )
	{
		if( msg.from()->metaContact()->isTemporary() )
		{
			kDebug(14313) << k_funcinfo << "Message from " << msg.from()->contactId() << " dropped (not on the contactlist)" << endl;
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
			event->discard();
			return;
		}
	}
}

#include "privacyplugin.moc"
