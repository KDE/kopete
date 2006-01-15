/**
 * Copyright (C)  2006  Jakub Stachowski <qbast@go2.pl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <kdebug.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetemessage.h>
#include <kopetecontact.h>
#include <kopetechatsession.h>
#include <kurl.h>

#include "srvdiscoifaceimpl.h"
#include "srvdiscofilter.h"
#include <srvdisco/servicetype.h>


using namespace Kopete;
using namespace SrvDisco;


SrvDiscoIfaceImpl::SrvDiscoIfaceImpl(const DCOPCString& obj) : DCOPObject(obj), pcounter(0)
{
	QList<ContactDef> ids;
	QList<MetaContact*> contacts = ContactList::self()->metaContacts();
	QList<MetaContact*>::const_iterator it=contacts.begin();
	while (it!=contacts.end()) {
		contactAdded(*it);
		++it;
	}	
	connect(ContactList::self(),SIGNAL(metaContactAdded(Kopete::MetaContact*)),this,SLOT(contactAdded(Kopete::MetaContact*)));
}

void SrvDiscoIfaceImpl::contactAdded(Kopete::MetaContact* c)
{
	kdDebug() << "We got new contact " << c->metaContactId() << endl;
		connect(c,SIGNAL(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)),
			this,SLOT(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)));
		connect(c,SIGNAL(displayNameChanged(const QString&,const QString&)),this,
			SLOT(displayNameChanged(const QString&,const QString&)));
}

QList<ContactDef> SrvDiscoIfaceImpl::contacts()
{
	QList<ContactDef> ids;
	QList<MetaContact*> contacts = ContactList::self()->metaContacts();
	QList<MetaContact*>::const_iterator it=contacts.begin();
	while (it!=contacts.end()) {
		if ((*it)->status()==OnlineStatus::Online) {
			ContactDef c;
			c.id = (*it)->metaContactId();
			kdDebug() << "ID: " << c.id << endl;
			c.display = (*it)->displayName();
			ids << c;
		}
		++it;
	}
	kdDebug() << "IDS is " << ids.count() << endl;
	return ids;
}


bool SrvDiscoIfaceImpl::sendMessage(bool announce, Kopete::MetaContact *m, ServiceDef d)
{
	if (!m->isOnline()) return false;
	QString message = QString("#SRVDISCO:") + (announce ? QString("ANNOUNCE:") : QString("RELEASE:"));
	message+=KURL::encode_string(d.name)+"/"+d.type.mainType()+"/"+d.type.auxType()+"/";
	message+=(d.type.protocol()==ServiceType::TCP ? QString("TCP") : QString("UDP"))+"/";
	// FIXME: TXT stuff
	message+=d.hostName+"/"+QString::number(d.port)+"//";
	Kopete::Contact * c = m->preferredContact();
	Kopete::ChatSession * manager = c->manager(Kopete::Contact::CanCreate);
	Kopete::Message msg = Kopete::Message( manager->myself(), manager->members(), message,
				Kopete::Message::Outbound, Kopete::Message::PlainText);
	manager->sendMessage( msg );
	// we assume that message has been sent correctly, maybe use messageSuccess signal?
	return true;
}
	

// FIXME: deal with our state change
void SrvDiscoIfaceImpl::onlineStatusChanged( Kopete::MetaContact *contact, Kopete::OnlineStatus::StatusType status )
{
	if (status==OnlineStatus::Online) {
		ContactDef c;
		c.id=contact->metaContactId();
		c.display=contact->displayName();
		kdDebug() << "Emitting newContact(" << c.display << ")" << endl;
		emit newContact(c);
	} else {
		kdDebug() << "Emitting deleteContact(" << contact->metaContactId() << ")" << endl;
		emit deleteContact(contact->metaContactId());
	}
}

void SrvDiscoIfaceImpl::displayNameChanged( const QString &oldName, const QString &newName )
{
	MetaContact *contact = static_cast<MetaContact*>(sender());
	emit deleteContact(contact->metaContactId());
	ContactDef c;
	c.id=contact->metaContactId();
	c.display=newName;
	emit newContact(c);
}

int SrvDiscoIfaceImpl::publish(ServiceDef d)
{
	Kopete::MetaContact *m = Kopete::ContactList::self()->metaContact( d.contact );
	if (!m) return 0;
	published[++pcounter]=d;
	sendMessage(true,m,d);
	return pcounter;
}

void SrvDiscoIfaceImpl::stopPublishing(int i)
{
	if (!published.contains(i)) return;
	ServiceDef d  = published.take(i);
	Kopete::MetaContact *m = Kopete::ContactList::self()->metaContact( d.contact );
	if (!m) return;
	sendMessage(false,m,d);
}

QList<ServiceDef> SrvDiscoIfaceImpl::getServices(const SrvDisco::ServiceType& type, const QString& id)
{
	Kopete::MetaContact *m = Kopete::ContactList::self()->metaContact( id );
	if (!m) return QList<ServiceDef>();
	return discovered[m].values();
}

void SrvDiscoIfaceImpl::newHandler(SrvDiscoFilter *f)
{
	kdDebug() << "Got new handler " << f << endl;
	connect(f,SIGNAL(newService(Kopete::MetaContact*,ServiceDef)),this,SLOT(gotNewService(Kopete::MetaContact*,ServiceDef)));
	connect(f,SIGNAL(deleteService(Kopete::MetaContact*,ServiceDef)),this,SLOT(gotDeleteService(Kopete::MetaContact*,ServiceDef)));
}

void SrvDiscoIfaceImpl::gotNewService(Kopete::MetaContact* c,ServiceDef d)
{
	if (discovered.contains(c) && discovered[c].contains(d.name)) return;
	discovered[c][d.name]=d;
	kdDebug() << "Added service " << d.name << " for MC: " << c->metaContactId() << endl;
	emit newService(d);
}

void SrvDiscoIfaceImpl::gotDeleteService(Kopete::MetaContact* c,ServiceDef d)
{
	if (!discovered.contains(c) || !discovered[c].contains(d.name)) return;
	discovered[c].remove(d.name);
	kdDebug() << "Removed service " << d.name << " for MC: " << c->metaContactId() << endl;
	emit deleteService(d);
}

#include "srvdiscoifaceimpl.moc"
