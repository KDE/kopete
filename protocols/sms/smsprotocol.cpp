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
: KopeteProtocol( parent, name )
{
	if (s_protocol)
		kdDebug() << "SMSProtocol::SMSProtocol: WARNING s_protocol already defined!" << endl;
	else
		s_protocol = this;
	
	new SMSPreferences("sms_protocol", this);

	QString protocolId = this->pluginId();

	m_mySelf = new SMSContact(protocol(), "", "", 0);
}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

bool SMSProtocol::unload()
{
	return KopeteProtocol::unload();
}

void SMSProtocol::connect()
{
}

void SMSProtocol::disconnect()
{
}

bool SMSProtocol::isConnected() const
{
	return true;
}


void SMSProtocol::setAway(void)
{
}

void SMSProtocol::setAvailable(void)
{
}

bool SMSProtocol::isAway(void) const
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
	m->addContact(c);
	return c;
}

SMSProtocol* SMSProtocol::s_protocol = 0L;

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

void SMSProtocol::serialize( KopeteMetaContact *metaContact)
{
	QStringList stream;
	
	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if ( c->protocol()->pluginId() != this->pluginId() )
			continue;

		SMSContact *g = static_cast<SMSContact*>(c);

		if (g)
		{
			stream << g->contactId() << g->displayName();
			if (g->serviceName() != QString::null)
			{
				stream << g->serviceName();
				stream += g->servicePrefs();
			}
		}
		stream << ".";
	}
	metaContact->setPluginData(this, stream);
}

void SMSProtocol::deserialize( KopeteMetaContact *metaContact,
	const QStringList &strList )
{
	QString protocolId = this->pluginId();

	unsigned idx=0;

	while (idx < strList.size())
	{
		QString nr = strList[ idx+0 ];
		QString name = strList[ idx+1 ];
		SMSContact* c = addContact(nr, name, metaContact);

		if (strList.size() > (idx+2) && strList[ idx+2 ] != ".")
		{
			c->setServiceName(strList[ 2 ]);
		}
		else
		{
			idx += 3;
			break;
		}

		QStringList prefs;
		for (idx+=3; idx < strList.size() && strList[idx] != "."; idx++)
			prefs << strList[idx];
		idx++;

		c->setServicePrefs(prefs);

	}
}


#include "smsprotocol.moc"






/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

