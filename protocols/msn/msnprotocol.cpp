/*
    msnprotocol.cpp - Kopete MSN Protocol Plugin

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kaboutdata.h>

#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"

#include "msnaddcontactpage.h"
#include "msneditaccountwidget.h"
#include "msncontact.h"
#include "msnaccount.h"
#include "msnprotocol.h"
#include "msnmessagemanager.h"

typedef KGenericFactory<MSNProtocol> MSNProtocolFactory;
#if KDE_IS_VERSION(3,2,90)
static const KAboutData aboutdata("kopete_msn", I18N_NOOP("MSN Messenger") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( libkopete_msn_shared, MSNProtocolFactory( &aboutdata ) )
#else
K_EXPORT_COMPONENT_FACTORY( libkopete_msn_shared, MSNProtocolFactory( "kopete_msn" ) )
#endif

MSNProtocol *MSNProtocol::s_protocol = 0L;

MSNProtocol::MSNProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: KopeteProtocol( MSNProtocolFactory::instance(), parent, name ),

	NLN( KopeteOnlineStatus::Online,    25, this, 1, QString::null,   i18n( "Set O&nline" ),        i18n( "Online" ) ),
	BSY( KopeteOnlineStatus::Away,      20, this, 2, "msn_busy",      i18n( "Set &Busy" ),          i18n( "Busy" ) ),
	BRB( KopeteOnlineStatus::Away,      22, this, 3, "msn_brb",       i18n( "Set Be &Right Back" ), i18n( "Be Right Back" ) ),
	AWY( KopeteOnlineStatus::Away,      18, this, 4, "msn_away",      i18n( "Set &Away" ),          i18n( "Away From Computer" ) ),
	PHN( KopeteOnlineStatus::Away,      12, this, 5, "msn_phone",     i18n( "Set On The &Phone" ),  i18n( "On the Phone" ) ),
	LUN( KopeteOnlineStatus::Away,      15, this, 6, "msn_lunch",     i18n( "Set Out To &Lunch" ),  i18n( "Out to Lunch" ) ),
	FLN( KopeteOnlineStatus::Offline,    0, this, 7, QString::null,   i18n( "Set &Offline" ),       i18n( "Offline" ) ),
	HDN( KopeteOnlineStatus::Away,       3, this, 8, "msn_invisible", i18n( "Set &Invisible" ),     i18n( "Invisible" ) ), //We use away because we don't want to see this state changed when autoaway.
	IDL( KopeteOnlineStatus::Away,      10, this, 9, "msn_away",      "FIXME: Make this unselectable", i18n( "Idle" ) ),
	UNK( KopeteOnlineStatus::Unknown,   25, this, 0, "status_unknown","FIXME: Make this unselectable", i18n( "Status not available" ) ),
	CNT( KopeteOnlineStatus::Connecting, 2, this, 10,"msn_connecting","FIXME: Make this unselectable", i18n( "Connecting" ) ),
	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propPhoneHome(Kopete::Global::Properties::self()->privatePhone()),
	propPhoneWork(Kopete::Global::Properties::self()->workPhone()),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone())
{
	s_protocol = this;

	addAddressBookField( "messaging/msn", KopetePlugin::MakeIndexField );

	setRichTextCapabilities( KopeteProtocol::BaseFgColor | KopeteProtocol::BaseFont );

	// m_status = m_unknownStatus = UNK;
}

KopeteContact *MSNProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId   = serializedData[ "contactId" ] ;
	QString accountId   = serializedData[ "accountId" ] ;
	QString lists = serializedData[ "lists" ];
	QStringList groups  = QStringList::split( ",", serializedData[ "groups" ] );

	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );

	KopeteAccount *account = accounts[ accountId ];
	if( !account )
		account = createNewAccount( accountId );

	// Create MSN contact
	MSNContact *c = new MSNContact( account, contactId, metaContact );

	for( QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
		c->contactAddedToGroup( ( *it ).toUInt(), 0L  /* FIXME - m_groupList[ ( *it ).toUInt() ]*/ );

	c->setInfo( "PHH" , serializedData[ "PHH" ] );
	c->setInfo( "PHW" , serializedData[ "PHW" ] );
	c->setInfo( "PHM" , serializedData[ "PHM" ] );

	c->setBlocked(  (bool)(lists.contains('B')) );
	c->setAllowed(  (bool)(lists.contains('A')) );
	c->setReversed( (bool)(lists.contains('R')) );
	return c;
}

AddContactPage *MSNProtocol::createAddContactWidget(QWidget *parent , KopeteAccount *i)
{
	return (new MSNAddContactPage(i->isConnected(),parent));
}

KopeteEditAccountWidget *MSNProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new MSNEditAccountWidget(this,account,parent);
}

KopeteAccount *MSNProtocol::createNewAccount(const QString &accountId)
{
	return new MSNAccount(this, accountId);
}


// NOTE: CALL THIS ONLY BEING CONNECTED
void MSNProtocol::slotSyncContactList()
{
/*	if ( ! mIsConnected )
	{
		return;
	}
	// First, delete D marked contacts
	QStringList localcontacts;

	contactsFile->setGroup("Default");

	contactsFile->readListEntry("Contacts",localcontacts);
	QString tmpUin;
	tmpUin.sprintf("%d",uin);
	tmp.append(tmpUin);
	cnt=contactsFile->readNumEntry("Count",0);
*/
}

MSNProtocol* MSNProtocol::protocol()
{
	return s_protocol;
}

bool MSNProtocol::validContactId(const QString& userid)
{
	return ( userid.contains('@') ==1 && userid.contains('.') >=1 && userid.contains(' ') == 0);
}

#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

