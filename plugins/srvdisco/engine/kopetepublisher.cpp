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

#define DEBUG_PREFIX "KopetePublisher"

#include <qvariant.h>
#include <qtimer.h>
#include "kopetepublisher.h"
#include "debug.h"
#include "srvdiscoiface_stub.h"
#include <srvdisco/publicservice.h>
#include "datatypes.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ksocketaddress.h>

namespace SrvDisco {

KopetePublisher::KopetePublisher(PublicService* service, Scope::Ptr scope)
 : Publisher(service,scope), DCOPObject(), sid(0)
{
	debug() << "KopetePublisher created for " << service << " at " << scope->id() <<	 endl;
	iface = new SrvDiscoIface_stub("kopete","srvdisco");
}


KopetePublisher::~KopetePublisher()
{
	debug() << "KopetePublisher for " <<  m_scope->id()  << " destructor" << endl;
	delete iface;
}

QString KopetePublisher::getInternetIP()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock == -1) return QString();
	addr.sin_family = AF_INET;
	addr.sin_port = 1;	// Not important, any port and public address will do
	addr.sin_addr.s_addr = 0x11111111;
	if ((::connect(sock,(const struct sockaddr*)&addr,sizeof(addr))) == -1) { close(sock); return QString(); }
	if ((getsockname(sock,(struct sockaddr*)&addr, &len)) == -1) { close(sock); return QString(); }
	::close(sock);
	KNetwork::KIpAddress s_addr(addr.sin_addr.s_addr);
	return s_addr.toString();
}


void KopetePublisher::start()
{
	ServiceDef d;
	d.name = m_service->serviceName();
	d.type = m_service->serviceType();
	if (m_service->hostName().isEmpty()) { // guess internet IP 
		QString ip = getInternetIP();
		if (ip.isEmpty()) {
			emit finished(false);
			return;
		}
		// FIXME: set m_service->hostName as well?
		d.hostName = ip;
	} else 	d.hostName=m_service->hostName();
	d.port = m_service->port();
	d.txt = m_service->properties();
	d.contact = m_scope->id().section('.',0,-2);
	sid=iface->publish(d);
	debug() << "Got SID " << sid << endl;
	if (!sid) emit finished(false);
	connectDCOPSignal("kopete","srvdisco","published(int,bool)","published(int,bool)",true);
}

void KopetePublisher::published(int id, bool result)
{
	if (sid!=id) return;
	emit finished(result); 
}

void KopetePublisher::stop()
{
	debug() << "doing stop" << endl;
	disconnectDCOPSignal("kopete","srvdisco","published(int,bool)","published(int,bool)");
	if (sid) {
		debug()<< "Sending stopPublishing(" << sid << ")" << endl;
		iface->stopPublishing(sid);
	}
	debug() << "KopetePublisher:Stopped" << endl;
	Publisher::stop();
}

}

#include "kopetepublisher.moc"
