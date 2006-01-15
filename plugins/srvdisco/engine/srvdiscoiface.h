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

#ifndef SRVDISCOIFACE_H
#define SRVDISCOIFACE_H

#include <qstring.h>
#include <srvdisco/servicetype.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qlist.h>
#include <qdatastream.h>
#include <dcopobject.h>
#include "datatypes.h"


class SrvDiscoIface: virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual QList<ContactDef> contacts()=0;
	virtual int publish(ServiceDef s)=0;
	virtual ASYNC stopPublishing(int)=0;
	virtual QList<ServiceDef> getServices(const SrvDisco::ServiceType& type, const QString& id)=0;
	
k_dcop_signals: 
	void published(int,bool);
	void newContact(ContactDef c);
	void deleteContact(const QString& id);
	void newService(ServiceDef d);
	void deleteService(ServiceDef d);
};

#endif


