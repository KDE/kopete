/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "smsprotocol.h"
#include "smspreferences.h"
#include "smscontact.h"
#include "smsaddcontactpage.h"
#include "kopetemetacontact.h"
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kdebug.h>

K_EXPORT_COMPONENT_FACTORY( kopete_sms, KGenericFactory<SMSProtocol> );

SMSProtocol::SMSProtocol( QObject *parent, const char *name, const QStringList& /*args*/)
: KopeteProtocol( parent, name ),
	SMSOnline(  KopeteOnlineStatus::Online,  25, this, 0,  "sms_online",  i18n( "Go O&nline" ),   i18n( "Online" ) ),
	SMSUnknown( KopeteOnlineStatus::Unknown, 25, this, 1,  "sms_unknown", "FIXME: Make optional", i18n( "Unknown" ) ),
	SMSOffline( KopeteOnlineStatus::Offline, 25, this, 2,  "sms_offline", i18n( "Go O&ffline" ),  i18n( "Offline" ) )
{
	if( s_protocol )
		kdWarning( 14160 ) << k_funcinfo << "s_protocol already defined!" << endl;
	else
		s_protocol = this;
	
	new SMSPreferences("sms_protocol", this);

	QString protocolId = pluginId();

	addAddressBookField( "messaging/sms", KopetePlugin::MakeIndexField );

	m_mySelf = new SMSContact(protocol(), "", "", 0);
}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

const QString SMSProtocol::protocolIcon()
{
	return QString::null;
}

void SMSProtocol::connect()
{
	m_mySelf->setOnlineStatus( SMSOnline );

	// FIXME: Set all contacts to SMSUnknown here
}

void SMSProtocol::disconnect()
{
	m_mySelf->setOnlineStatus( SMSOffline );

	// FIXME: Set all contacts to SMSOffline here
}

bool SMSProtocol::isConnected() const
{
	return m_mySelf->onlineStatus() == SMSOnline;
}

bool SMSProtocol::isAway() const
{
	return false;
}

KopeteContact* SMSProtocol::myself() const
{
	return m_mySelf;
}

AddContactPage *SMSProtocol::createAddContactWidget(QWidget *parent)
{
	return (new SMSAddContactPage(this,parent));
}

SMSContact* SMSProtocol::addContact( const QString& nr , const QString& name, KopeteMetaContact *m)
{
	SMSContact* c = new SMSContact(protocol(), nr, name, m);
	c->setOnlineStatus( SMSUnknown ); // FIXME: Set to offline when protocol is offline
	m->addContact(c);
	return c;
}

SMSProtocol* SMSProtocol::s_protocol = 0L;

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

void SMSProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	SMSContact* c = addContact( serializedData[ "contactId" ], serializedData[ "displayName" ], metaContact );

	QString serviceName = serializedData[ "serviceName" ];
	if( !serviceName.isNull() )
	{
		c->setServiceName( serviceName );
		c->setServicePrefs( QStringList::split( ',', serializedData[ "servicePrefs" ] ) );
	}
}

#include "smsprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

