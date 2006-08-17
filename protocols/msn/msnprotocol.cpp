/*
    msnprotocol.cpp - Kopete MSN Protocol Plugin

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart @ kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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

#include <qimage.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kaboutdata.h>

#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"

#include "msnaddcontactpage.h"
#include "msneditaccountwidget.h"
#include "msncontact.h"
#include "msnaccount.h"
#include "msnprotocol.h"
#include "msnchatsession.h"

typedef KGenericFactory<MSNProtocol> MSNProtocolFactory;
#if KDE_IS_VERSION(3,2,90)
static const KAboutData aboutdata("kopete_msn", I18N_NOOP("MSN Messenger") , "1.0" );
K_EXPORT_COMPONENT_FACTORY( libkopete_msn_shared, MSNProtocolFactory( &aboutdata ) )
#else
K_EXPORT_COMPONENT_FACTORY( libkopete_msn_shared, MSNProtocolFactory( "kopete_msn" ) )
#endif

MSNProtocol *MSNProtocol::s_protocol = 0L;

MSNProtocol::MSNProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: Kopete::Protocol( MSNProtocolFactory::instance(), parent, name ),
	NLN( Kopete::OnlineStatus::Online,    25, this, 1, QString::null,               i18n( "Online" ) ,           i18n( "O&nline" ),        Kopete::OnlineStatusManager::Online,Kopete::OnlineStatusManager::HasAwayMessage ),
	BSY( Kopete::OnlineStatus::Away,      20, this, 2, "msn_busy",                  i18n( "Busy" ) ,             i18n( "&Busy" ),          Kopete::OnlineStatusManager::Busy,  Kopete::OnlineStatusManager::HasAwayMessage ),
	BRB( Kopete::OnlineStatus::Away,      22, this, 3, "msn_brb",                   i18n( "Be Right Back" ),     i18n( "Be &Right Back" ) ,  0  ,                              Kopete::OnlineStatusManager::HasAwayMessage ),
	AWY( Kopete::OnlineStatus::Away,      18, this, 4, "contact_away_overlay",      i18n( "Away From Computer" ),i18n( "&Away" ),          Kopete::OnlineStatusManager::Away,  Kopete::OnlineStatusManager::HasAwayMessage ),
	PHN( Kopete::OnlineStatus::Away,      12, this, 5, "contact_phone_overlay",     i18n( "On the Phone" ) ,     i18n( "On The &Phone" )  ,  0  ,                              Kopete::OnlineStatusManager::HasAwayMessage ),
	LUN( Kopete::OnlineStatus::Away,      15, this, 6, "contact_food_overlay",      i18n( "Out to Lunch" ) ,     i18n( "Out To &Lunch" )  ,  0  ,                              Kopete::OnlineStatusManager::HasAwayMessage ),
	FLN( Kopete::OnlineStatus::Offline,    0, this, 7, QString::null,               i18n( "Offline" ) ,          i18n( "&Offline" ),       Kopete::OnlineStatusManager::Offline,Kopete::OnlineStatusManager::DisabledIfOffline ),
	HDN( Kopete::OnlineStatus::Invisible,  3, this, 8, "contact_invisible_overlay", i18n( "Invisible" ) ,        i18n( "&Invisible" ),     Kopete::OnlineStatusManager::Invisible ), 
	IDL( Kopete::OnlineStatus::Away,      10, this, 9, "contact_away_overlay",      i18n( "Idle" ) ,             i18n( "&Idle" ),          Kopete::OnlineStatusManager::Idle , Kopete::OnlineStatusManager::HideFromMenu ),
	UNK( Kopete::OnlineStatus::Unknown,   25, this, 0, "status_unknown",            i18n( "Status not available" ) ),
	CNT( Kopete::OnlineStatus::Connecting, 2, this, 10,"msn_connecting",            i18n( "Connecting" ) ),
	propEmail(Kopete::Global::Properties::self()->emailAddress()),
	propPhoneHome(Kopete::Global::Properties::self()->privatePhone()),
	propPhoneWork(Kopete::Global::Properties::self()->workPhone()),
	propPhoneMobile(Kopete::Global::Properties::self()->privateMobilePhone()),
	propClient("client", i18n("Remote Client"), 0, false),
	propGuid("guid", i18n("Contact GUID"), 0, true),
	propPersonalMessage(Kopete::Global::Properties::self()->awayMessage())
{
	s_protocol = this;

	addAddressBookField( "messaging/msn", Kopete::Plugin::MakeIndexField );

	setCapabilities( Kopete::Protocol::BaseFgColor | Kopete::Protocol::BaseFont | Kopete::Protocol::BaseFormatting );

	// m_status = m_unknownStatus = UNK;
}

Kopete::Contact *MSNProtocol::deserializeContact( Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId   = serializedData[ "contactId" ] ;
	QString accountId   = serializedData[ "accountId" ] ;
	QString lists = serializedData[ "lists" ];
	QStringList groups  = QStringList::split( ",", serializedData[ "groups" ] );
	QString contactGuid = serializedData[ "contactGuid" ] ;

	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( this );

	Kopete::Account *account = accounts[ accountId ];
	if( !account )
		account = createNewAccount( accountId );

	// Create MSN contact
	MSNContact *c = new MSNContact( account, contactId, metaContact );

	for( QStringList::Iterator it = groups.begin() ; it != groups.end(); ++it )
		c->contactAddedToGroup( *it, 0L  /* FIXME - m_groupList[ ( *it ).toUInt() ]*/ );

	c->m_obj= serializedData[ "obj" ];
	c->setInfo( "PHH" , serializedData[ "PHH" ] );
	c->setInfo( "PHW" , serializedData[ "PHW" ] );
	c->setInfo( "PHM" , serializedData[ "PHM" ] );
	c->setProperty( propGuid, contactGuid );

	c->setBlocked(  (bool)(lists.contains('B')) );
	c->setAllowed(  (bool)(lists.contains('A')) );
	c->setReversed( (bool)(lists.contains('R')) );

	return c;
}

AddContactPage *MSNProtocol::createAddContactWidget(QWidget *parent , Kopete::Account *i)
{
	return (new MSNAddContactPage(i->isConnected(),parent));
}

KopeteEditAccountWidget *MSNProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new MSNEditAccountWidget(this,account,parent);
}

Kopete::Account *MSNProtocol::createNewAccount(const QString &accountId)
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

QImage MSNProtocol::scalePicture(const QImage &picture)
{
	QImage img(picture);
	img = img.smoothScale( 96, 96, QImage::ScaleMin );
	// crop image if not square
	if(img.width() < img.height())
	{
		img = img.copy((img.width()-img.height())/2, 0, 96, 96);
	}
	else if(img.width() > img.height())
	{
		img = img.copy(0, (img.height()-img.width())/2, 96, 96);
	}

	return img;
}
#include "msnprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

