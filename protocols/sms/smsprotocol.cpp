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

	QString protocolId = this->id();

	mPrefs= new SMSPreferences( "sms_protocol", this );
	connect( mPrefs, SIGNAL(saved()) , this , SLOT ( slotPreferencesSaved() ));
	slotPreferencesSaved();

	m_mySelf = new SMSContact(protocol(), "", "", 0);

}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

bool SMSProtocol::unload()
{
	emit unloading();
	return true;
}

void SMSProtocol::Connect()
{
}

void SMSProtocol::Disconnect()
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

QString SMSProtocol::protocolIcon() const
{
	return "metacontact_online";
}

AddContactPage *SMSProtocol::createAddContactWidget(QWidget *parent)
{
	return (new SMSAddContactPage(this,parent));
}

void SMSProtocol::addContact( const QString nr , const QString name, KopeteMetaContact *m)
{
	m->addContact(new SMSContact(protocol(), nr, name, m));
}

SMSProtocol* SMSProtocol::s_protocol = 0L;

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

void SMSProtocol::slotPreferencesSaved()
{
}

bool SMSProtocol::serialize( KopeteMetaContact *metaContact,
	QStringList &stream ) const
{
	bool r=false;
	
	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if ( c->protocol()->id() != this->id() )
			continue;

		SMSContact *g = static_cast<SMSContact*>(c);

		if (g)
		{
			stream << g->id() << g->displayName();
			r=true;
		}
	}
	return r;
}

void SMSProtocol::deserialize( KopeteMetaContact *metaContact,
	const QStringList &strList )
{
	QString protocolId = this->id();

	uint idx = 0;
	while( idx < strList.size() )
	{
		QString nr = strList[ idx ];
		QString name = strList[ idx+1];

		addContact( nr, name, metaContact );
		idx += 2;
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

