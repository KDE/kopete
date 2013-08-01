/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
  Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>

  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "oscarprotocol.h"

#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>

#include "kopeteaccountmanager.h"

#include "oscaraccount.h"
#include "connection.h"

OscarProtocol::OscarProtocol( const KComponentData &instance, QObject *parent, bool canAddMyself )
	: Kopete::Protocol( instance, parent, canAddMyself ),
	statusTitle(Kopete::Global::Properties::self()->statusTitle()),
	statusMessage(Kopete::Global::Properties::self()->statusMessage()),
	clientFeatures("clientFeatures", i18n("Client Features"), 0),
	buddyIconHash("iconHash", i18n("Buddy Icon MD5 Hash"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	contactEncoding("contactEncoding", i18n("Contact Encoding"), QString(), Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty),
	memberSince("memberSince", i18n("Member Since"), QString(), 0),
	client("client", i18n("Client"), QString(), 0),
	protocolVersion("protocolVersion", i18n("Protocol Version"), QString(), 0)
{
	KConfigGroup config( KGlobal::config(), "OscarProtocol" );
	if ( config.hasKey( "StartFlapSequences" ) )
	{
		kWarning(OSCAR_GEN_DEBUG) << "Overriding default start flap sequence algorithm!";

		QList<Oscar::WORD> startFlapSequenceList;
		QList<int> flapSequenceList = config.readEntry( "StartFlapSequences", QList<int>() );
		foreach ( int flapSeq, flapSequenceList )
			startFlapSequenceList << flapSeq;

		Connection::setStartFlapSequenceList( startFlapSequenceList );
	}
}

OscarProtocol::~OscarProtocol()
{
}

Kopete::Contact *OscarProtocol::deserializeContact(Kopete::MetaContact *metaContact,
    const QMap<QString, QString> &serializedData,
    const QMap<QString, QString> &/*addressBookData*/)
{
	QString contactId = serializedData["contactId"];
	QString accountId = serializedData["accountId"];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

	// Get the account it belongs to
	Kopete::Account* account = Kopete::AccountManager::self()->findAccount( this->pluginId(), accountId );

	if ( !account ) //no account
		return 0;

	uint ssiGid = 0, ssiBid = 0, ssiType = 0xFFFF;
	QString ssiName;
	bool ssiWaitingAuth = false;
	QByteArray ssiMetaInfoId;
	QString ssiAlias;

	if ( serializedData.contains( "ssi_name" ) )
		ssiName = serializedData["ssi_name"];

	if ( serializedData.contains( "ssi_alias" ) )
		ssiAlias = serializedData["ssi_alias"];

	if ( serializedData.contains( "ssi_metaInfoId" ) )
		ssiMetaInfoId = QByteArray::fromHex( serializedData["ssi_metaInfoId"].toUtf8() );

	if ( serializedData.contains( "ssi_waitingAuth" ) )
	{
		QString authStatus = serializedData["ssi_waitingAuth"];
		if ( authStatus == "true" )
			ssiWaitingAuth = true;
	}

	if ( serializedData.contains( "ssi_gid" ) )
		ssiGid = serializedData["ssi_gid"].toUInt();
	if ( serializedData.contains( "ssi_bid" ) )
		ssiBid = serializedData["ssi_bid"].toUInt();
	if ( serializedData.contains( "ssi_type" ) )
		ssiType = serializedData["ssi_type"].toUInt();

	OContact item( ssiName, ssiGid, ssiBid, ssiType, QList<TLV>(), 0 );
	item.setWaitingAuth( ssiWaitingAuth );
	item.setAlias( ssiAlias );
	item.setMetaInfoId( ssiMetaInfoId );

	OscarAccount* oaccount = static_cast<OscarAccount*>(account);
	OscarContact* ocontact = oaccount->createNewContact( contactId, metaContact, item );

	if ( !ocontact )
		return 0;

	ocontact->setPreferredNameType( nameType );
	return ocontact;
}

#include "oscarprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
