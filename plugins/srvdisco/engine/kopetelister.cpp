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

#define DEBUG_PREFIX "KopeteListers"

#include <qvariant.h>
#include <qstringlist.h>
#include <qtimer.h>
#include "kopetelister.h"
#include "srvdiscoiface_stub.h"
#include <srvdisco/scopetree.h>
#include "datatypes.h"
#include <dcopclient.h>
#include "debug.h"

namespace SrvDisco {

KopeteLister::KopeteLister(Scope::Type type,Scope::Ptr parent)
 : ScopeLister(type,parent), DCOPObject()
{
	DEBUG_FUNC_INFO
}


KopeteLister::~KopeteLister()
{
//	DEBUG_FUNC_INFO
}

void KopeteLister::start()
{
//	QTimer::singleShot(0,this,SLOT(list()));
	list();
	connectDCOPSignal("kopete","srvdisco","newContact(ContactDef)","newContact(ContactDef)",false);
	connectDCOPSignal("kopete","srvdisco","deleteContact(QString)","deleteContact(QString)",false);
	debug() << "I am " << objId() << endl;
}

void KopeteLister::list()
{
	DEBUG_FUNC_INFO
	debug() << "DCOPCL: " << DCOPClient::mainClient() << endl;
	SrvDiscoIface_stub iface(DCOPClient::mainClient(),"kopete","srvdisco");
	QList<ContactDef> cs = iface.contacts();
	debug() << "Contact list is " << cs.count() << endl;
	Scope::Ptr root = ScopeTree::self(m_type).byId("contacts.core");
	foreach (ContactDef c, cs) {
		Scope* n = new Scope(c.id+".kopete",root,"kopete",c.display);
		m_scopes.append(n);
		emit scopeAdded(n);
	}
}

void KopeteLister::newContact(ContactDef c)
{
	DEBUG_FUNC_INFO
	debug() << "Got new contact " << c.display << endl;
	Scope* n = new Scope(c.id+".kopete",ScopeTree::self(m_type).byId("contacts.core"),"kopete",c.display);
	m_scopes.append(n);
	emit scopeAdded(n);
}

void KopeteLister::deleteContact(const QString& id)
{
	DEBUG_FUNC_INFO
	kdDebug() << "Got delete contact " << id << endl;
	QString fullID=id+".kopete";
	foreach (Scope::Ptr n, m_scopes) if (fullID==n->id()) {
		kdDebug() << "ok, removing " << n->prettyName() <<  endl;
		m_scopes.remove(n);
		emit scopeRemoved(n.data());
		return;
	}
}		

void KopeteLister::stop()
{
	DEBUG_FUNC_INFO
	disconnectDCOPSignal("kopete","srvdisco","newContact(ContactDef","newContact(ContactDef)");
	disconnectDCOPSignal("kopete","srvdisco","deleteContact(QString)","deleteContact(QString)");
	ScopeLister::stop();
}

}
#include "kopetelister.moc"
