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

#ifndef SRVDISCOIFACEIMPL_H
#define SRVDISCOIFACEIMPL_H

#include <srvdiscoiface.h>
#include <kopeteonlinestatus.h>
#include <qobject.h>

class SrvDiscoFilter;
class SrvDiscoIfaceImpl: public QObject, public SrvDiscoIface
{
	Q_OBJECT
public:	
	SrvDiscoIfaceImpl(const DCOPCString& obj);

	virtual QList<ContactDef> contacts();
	virtual int publish(ServiceDef d);
	virtual void stopPublishing(int);
	virtual QList<ServiceDef> getServices(const SrvDisco::ServiceType& type, const QString& id);
public slots:
	void newHandler(SrvDiscoFilter *f);
	
private slots:
	void onlineStatusChanged( Kopete::MetaContact *contact, Kopete::OnlineStatus::StatusType status );
	void displayNameChanged( const QString &oldName, const QString &newName );
	void gotNewService(Kopete::MetaContact *c,ServiceDef d);
	void gotDeleteService(Kopete::MetaContact *c, ServiceDef d);
	void contactAdded(Kopete::MetaContact* c);
private:
	bool sendMessage(bool announce, Kopete::MetaContact *m, ServiceDef d);
	
	unsigned int pcounter;
	QHash<int,ServiceDef> published;
	QHash<Kopete::MetaContact*, QHash<QString, ServiceDef> > discovered;
};

#endif


