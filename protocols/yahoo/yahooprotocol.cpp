/*
    yahooprotocol.cpp - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

/* QT Includes */
#include <qapplication.h>
#include <qcursor.h>
#include <qwidget.h>
#include <qobject.h>
#include <qtimer.h>

/* KDE Includes */
#include <kdebug.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

/* Local Includes */
#include "yahoodebug.h"
#include "yahoostatus.h"
#include "yahooprotocol.h"
#include "yahooaccount.h"
#include "yahoocontact.h"
#include "yahooaddcontact.h"
#include "yahooeditaccount.h"

/* Kopete Includes */
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"

K_EXPORT_COMPONENT_FACTORY( kopete_yahoo, KGenericFactory<YahooProtocol> );

class KPopupMenu;

YahooProtocol::YahooProtocol( QObject *parent, const char *name, const QStringList & ) : KopeteProtocol( parent, name )
{
	kdDebug(14180) << "YahooProtocol::YahooProtocol()" << endl;

	if ( !s_protocolStatic_ )
		s_protocolStatic_ = this;
	else
		kdDebug(14180) << "YahooProtocol already initialized" << endl;

	/* Init actions and icons and create the status bar icon */
//	initActions();

//	setStatusIcon( "yahoo_offline" );

	/* Create preferences menu */
	m_prefs = new YahooPreferences("yahoo_protocol", this);

	/* Call slotSettingsChanged() to get it all registered. */
	slotSettingsChanged();

	QObject::connect( m_prefs, SIGNAL(saved(void)), this, SLOT(slotSettingsChanged(void)));

//	m_isConnected = false;

	addAddressBookField( "messaging/yahoo", KopetePlugin::MakeIndexField );
}

YahooProtocol::~YahooProtocol()
{
	kdDebug(14180) << "YahooProtocol::~YahooProtocol()" << endl;
	delete m_prefs;
	s_protocolStatic_ = 0L;
}

YahooProtocol* YahooProtocol::s_protocolStatic_ = 0L;

/***************************************************************************
 *                                                                         *
 *   Re-implementation of Plugin class methods                             *
 *                                                                         *
 ***************************************************************************/

YahooProtocol *YahooProtocol::protocol()
{
	return s_protocolStatic_;
}

void YahooProtocol::deserializeContact( KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData, const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId = serializedData[ "contactId" ];
/*	if(contact(contactId))
	{
		kdDebug( 14180 ) << k_funcinfo << "User " << contactId << " already in contacts map" << endl;
		return;
	}
*/	// TODO: this might have done something useful - perhaps it should be reimplemented?

	addContact( contactId, serializedData[ "displayName" ], metaContact, serializedData[ "group" ] );
}

void YahooProtocol::init()
{
}

bool YahooProtocol::unload()
{
	return true;
}

void YahooProtocol::slotSettingsChanged()
{
	kdDebug(14180) << "YahooProtocol::slotSettingsChanged()" <<endl;
	m_server   = KGlobal::config()->readEntry("Server", "cs.yahoo.com");
	m_port     = KGlobal::config()->readNumEntry("Port", 5050);
	m_logAll   = KGlobal::config()->readBoolEntry("LogAll", true);
}

AddContactPage *YahooProtocol::createAddContactWidget( QWidget * parent )
{
	kdDebug(14180) << "YahooProtocol::createAddContactWidget(<parent>)" << endl;
	return new YahooAddContact(this, parent);
	return 0L;
}

EditAccountWidget *YahooProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new YahooEditAccount(this, account, parent);
}

KopeteAccount *YahooProtocol::createNewAccount(const QString &accountId)
{
	return new YahooAccount(this, accountId);
}

#include "yahooprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

