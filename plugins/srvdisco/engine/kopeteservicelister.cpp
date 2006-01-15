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

#define DEBUG_PREFIX "KopeteServiceLister"

#include <qvariant.h>
#include <qstringlist.h>
#include <qtimer.h>
#include "kopeteservicelister.h"
#include "srvdiscoiface_stub.h"
#include <srvdisco/scopetree.h>
#include "datatypes.h"
#include <dcopclient.h>
#include "debug.h"

namespace SrvDisco {


KopeteServiceLister::KopeteServiceLister(Scope::Ptr scope, ServiceType type): 
	ServiceLister(scope,type)
{
	DEBUG_FUNC_INFO
}

KopeteServiceLister::~KopeteServiceLister()
{
	DEBUG_FUNC_INFO
}

void KopeteServiceLister::start()
{
	SrvDiscoIface_stub iface("kopete","srvdisco");
	QList<ServiceDef> dlist = iface.getServices(m_type,m_scope->id().section('.',0,-2));
	foreach (ServiceDef d, dlist) newService(d);
	connectDCOPSignal("kopete","srvdisco","newService(ServiceDef)","newService(ServiceDef)",false);
	connectDCOPSignal("kopete","srvdisco","deleteService(ServiceDef)","removeService(ServiceDef)",false);
	debug() << "I am " << objId() << endl;
}

void KopeteServiceLister::stop()
{
	disconnectDCOPSignal("kopete","srvdisco","newService(ServiceDef)","newService(ServiceDef)");
	disconnectDCOPSignal("kopete","srvdisco","deleteService(ServiceDef)","removeService(ServiceDef)");
	ServiceLister::stop();
}

void KopeteServiceLister::newService(ServiceDef d)
{
	if (d.contact!=m_scope->id().section('.',0,-2)) return;
	ServiceBase* b = new ServiceBase(d.name, d.type, Scope::List() << m_scope, d.hostName, d.port);
	m_services.append(b);
	emit serviceAdded(b);
}

void KopeteServiceLister::removeService(ServiceDef d)
{
	if (d.contact!=m_scope->id().section('.',0,-2)) return;
	foreach (ServiceBase *b, m_services) if (d.name == b->serviceName()) {
		emit serviceRemoved(b);
		m_services.remove(b);
		delete b;
		return;
	}
}


}
#include "kopeteservicelister.moc"
