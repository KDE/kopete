/*
    qqprotocol.cpp - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.u>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "qqprotocol.h"
#include <QList>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"
#include "qqaccount.h"
#include "qqcontact.h"
#include "qqaddcontactpage.h"
#include "ui/qqeditaccountwidget.h"

K_PLUGIN_FACTORY( QQProtocolFactory, registerPlugin<QQProtocol>(); )
K_EXPORT_PLUGIN( QQProtocolFactory( "kopete_qq" ) )

QQProtocol *QQProtocol::s_protocol = 0L;

QQProtocol::QQProtocol( QObject* parent, const QVariantList &/*args*/ )
	: Kopete::Protocol( QQProtocolFactory::componentData(), parent ),
	  qqOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QStringList(QString()),  
			  i18n( "Online" ),   i18n( "O&nline" ) ),
	  qqAway(  Kopete::OnlineStatus::Away, 25, this, 1, QStringList(QLatin1String("msn_away")),  
			  i18n( "Away" ),   i18n( "&Away" ) ),
	  qqOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QStringList(QString()), 
			  i18n( "Offline" ),   i18n( "O&ffline" ) ),
	  propNickName(Kopete::Global::Properties::self()->nickName()),
	  propFullName(Kopete::Global::Properties::self()->fullName()),
	  propCountry("QQVCardCountry", i18n("Country"), QString(), Kopete::PropertyTmpl::PersistentProperty ), 
	  propState("QQVCardState", i18n("State"), QString(), Kopete::PropertyTmpl::PersistentProperty ), 
	  propCity("QQVCardCity", i18n("State"), QString(), Kopete::PropertyTmpl::PersistentProperty ),
	  propStreet("QQVCardAddress", i18n("Home Address"), QString(), Kopete::PropertyTmpl::PersistentProperty ),
	  propZipcode("QQVCardZipcode", i18n("Zipcode"), QString(), Kopete::PropertyTmpl::PersistentProperty ), 
	  propAge("QQVCardAge", i18n("Age"), QString(), Kopete::PropertyTmpl::PersistentProperty ),
    propEmail(Kopete::Global::Properties::self()->emailAddress())
{
	kDebug( 14210 ) ;
	s_protocol = this;
}

QQProtocol::~QQProtocol()
{
}

Kopete::Contact *QQProtocol::deserializeContact(
	Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	QString type = serializedData[ "contactType" ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);
/*
	QQContact::QQContactType tbcType;
	if ( type == QString::fromLatin1( "echo" ) )
		tbcType = QQContact::Echo;
	if ( type == QString::fromLatin1( "null" ) )
		tbcType = QQContact::Null;
	else
		tbcType = QQContact::Null;
*/
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
	Kopete::Account* account = 0;
	foreach( Kopete::Account* acct, accounts )
	{
		if ( acct->accountId() == accountId )
			account = acct;
	}

	if ( !account )
	{
		kDebug(14210) << "Account doesn't exist, skipping";
		return 0;
	}

	QQContact *contact = new QQContact(account, contactId, metaContact);
	contact->setPreferredNameType(nameType);
	return contact;
}

AddContactPage * QQProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
	kDebug( 14210 ) << "Creating Add Contact Page";
	return new QQAddContactPage( parent );
}

KopeteEditAccountWidget * QQProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kDebug(14210) << "Creating Edit Account Page";
	return new QQEditAccountWidget( this, account, parent );
}

Kopete::Account *QQProtocol::createNewAccount( const QString &accountId )
{
	return new QQAccount( this, accountId );
}

bool QQProtocol::validContactId( const QString& userId )
{
	QRegExp re("[1-9][0-9]*");
	return re.exactMatch( userId );
}

QQProtocol *QQProtocol::protocol()
{
	return s_protocol;
}

#include "qqprotocol.moc"
