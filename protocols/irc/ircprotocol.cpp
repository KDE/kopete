/*
    ircprotocol - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircprotocol.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qregexp.h>
#include <qdict.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "ircadd.h"
#include "ircidentity.h"
#include "ircaddcontactpage.h"
#include "ircpreferences.h"
#include "kopetemetacontact.h"
#include "ircchannelcontact.h"
#include "kopeteidentitymanager.h"
#include "irceditidentitywidget.h"
#include "kirc.h"

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol::IRCProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	kdDebug(14120) << k_funcinfo << endl;
	// Load all ICQ icons from KDE standard dirs

	setStatusIcon( "irc_protocol_offline" );

	new IRCPreferences("irc_protocol", this);

	KConfig *cfg = KGlobal::config();
        cfg->setGroup("IRC");

	//Migration code
	if( cfg->hasKey("Nickname") )
	{
		createNewIdentity( cfg->readEntry("Nickname") + "@" + cfg->readEntry("Server") + ":" + cfg->readEntry("Port") );

		cfg->deleteEntry("Nickname");
		cfg->deleteEntry("Server");
		cfg->deleteEntry("Port");
		cfg->deleteEntry("AutoConnect");
		cfg->sync();
	}
}

IRCProtocol::~IRCProtocol()
{
	//delete identity;
}

KActionMenu* IRCProtocol::protocolActions()
{
	KActionMenu *mActionMenu;
	QDict<KopeteIdentity> mIdentities = KopeteIdentityManager::manager()->identities(this);
	QDictIterator<KopeteIdentity> it( mIdentities );

	if( mIdentities.count() == 1 )
		mActionMenu = static_cast<IRCIdentity*>( it.current() )->actionMenu();
	else
	{
		mActionMenu = new KActionMenu( "IRC", this );
		for( ; it.current(); ++it )
		{
			mActionMenu->insert( static_cast<IRCIdentity*>( it.current() )->actionMenu() );
		}
	}

	return mActionMenu;
}

const QString IRCProtocol::protocolIcon()
{
	return "irc_protocol_small";
}

void IRCProtocol::init()
{

}

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent)
{
	return new IRCAddContactPage(this,parent);
}

EditIdentityWidget *IRCProtocol::createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent)
{
	return new IRCEditIdentityWidget(this, static_cast<IRCIdentity*>(identity),parent);
}

KopeteIdentity *IRCProtocol::createNewIdentity(const QString &identityId)
{
	kdDebug(14120) << k_funcinfo << endl;

	IRCIdentity *id = new IRCIdentity( identityId, this );
	mIdentityMap[ identityId.section('@',1) ] = id;

	return id;
}

void IRCProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	kdDebug(14120) << k_funcinfo << endl;

	QString contactId = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];
	QStringList identities  = QStringList::split( ",", serializedData[ "identities" ] );

	if( displayName.isEmpty() )
		displayName = contactId;

	if( !identities.isEmpty() )
	{
		for( uint i=0 ; i<identities.count(); i++ )
		{
			kdDebug(14120) << k_funcinfo << i << endl;
			QString server = identities[i].section('@',1);
			if( mIdentityMap.contains( server ) )
				mIdentityMap[ server ]->addContact( contactId, displayName, metaContact );
			else
				mIdentityMap.begin().data()->addContact( contactId, displayName, metaContact );
		}
	}
	else
	{
		//This guy does not have an identity. Must be old data.
		//Just add him to the first server we have, which would be the default server if
		//the migration code was run at the beginning
		mIdentityMap.begin().data()->addContact( contactId, displayName, metaContact );
	}
}

#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

