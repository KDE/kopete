/*
 * srvresolver.cpp - class to simplify SRV lookups
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"srvresolver.h"

#include<qcstring.h>
#include<qtimer.h>
#include<qdns.h>
#include"safedelete.h"
#include"ndns.h"

// CS_NAMESPACE_BEGIN

static void sortSRVList(QValueList<QDns::Server> &list)
{
	QValueList<QDns::Server> tmp = list;
	list.clear();

	while(!tmp.isEmpty()) {
		QValueList<QDns::Server>::Iterator p = tmp.end();
		for(QValueList<QDns::Server>::Iterator it = tmp.begin(); it != tmp.end(); ++it) {
			if(p == tmp.end())
				p = it;
			else {
				int a = (*it).priority;
				int b = (*p).priority;
				int j = (*it).weight;
				int k = (*p).weight;
				if(a < b || (a == b && j < k))
					p = it;
			}
		}
		list.append(*p);
		tmp.remove(p);
	}
}

class SrvResolver::Private
{
public:
	Private() {}

	QDns *qdns;
	NDns ndns;

	uint result;
	QString resultString;
	Q_UINT16 resultPort;

	bool srvonly;
	QString srv;
	QValueList<QDns::Server> servers;

	QTimer t;
	SafeDelete sd;
};

SrvResolver::SrvResolver(QObject *parent)
:QObject(parent)
{
	d = new Private;
	d->qdns = 0;

	connect(&d->ndns, SIGNAL(resultsReady()), SLOT(ndns_done()));
	connect(&d->t, SIGNAL(timeout()), SLOT(t_timeout()));
	stop();
}

SrvResolver::~SrvResolver()
{
	stop();
	delete d;
}

void SrvResolver::resolve(const QString &server, const QString &type, const QString &proto)
{
	stop();

	d->srvonly = false;
	d->srv = QString("_") + type + "._" + proto + '.' + server;
	d->t.start(15000, true);
	d->qdns = new QDns;
	connect(d->qdns, SIGNAL(resultsReady()), SLOT(qdns_done()));
	d->qdns->setRecordType(QDns::Srv);
	d->qdns->setLabel(d->srv);
}

void SrvResolver::resolveSrvOnly(const QString &server, const QString &type, const QString &proto)
{
	stop();

	d->srvonly = true;
	d->srv = QString("_") + type + "._" + proto + '.' + server;
	d->t.start(15000, true);
	d->qdns = new QDns;
	connect(d->qdns, SIGNAL(resultsReady()), SLOT(qdns_done()));
	d->qdns->setRecordType(QDns::Srv);
	d->qdns->setLabel(d->srv);
}

void SrvResolver::next()
{
	if(d->servers.isEmpty())
		return;

	tryNext();
}

void SrvResolver::stop()
{
	if(d->t.isActive())
		d->t.stop();
	if(d->qdns) {
		d->qdns->disconnect(this);
		d->sd.deleteLater(d->qdns);
		d->qdns = 0;
	}
	if(d->ndns.isBusy())
		d->ndns.stop();
	d->result = 0;
	d->resultString = "";
	d->resultPort = 0;
	d->servers.clear();
	d->srv = "";
}

bool SrvResolver::isBusy() const
{
	if(d->qdns || d->ndns.isBusy())
		return true;
	else
		return false;
}

QValueList<QDns::Server> SrvResolver::servers() const
{
	return d->servers;
}

uint SrvResolver::result() const
{
	return d->result;
}

QString SrvResolver::resultString() const
{
	return d->resultString;
}

Q_UINT16 SrvResolver::resultPort() const
{
	return d->resultPort;
}

void SrvResolver::tryNext()
{
	d->ndns.resolve(d->servers.first().name);
}

void SrvResolver::qdns_done()
{
	if(!d->qdns)
		return;

	// apparently we sometimes get this signal even though the results aren't ready
	if(d->qdns->isWorking())
		return;
	d->t.stop();

	SafeDeleteLock s(&d->sd);

	// grab the server list and destroy the qdns object
	QValueList<QDns::Server> list;
	if(d->qdns->recordType() == QDns::Srv)
		list = d->qdns->servers();
	d->qdns->disconnect(this);
	d->sd.deleteLater(d->qdns);
	d->qdns = 0;

	if(list.isEmpty()) {
		stop();
		resultsReady();
		return;
	}
	sortSRVList(list);
	d->servers = list;

	if(d->srvonly)
		resultsReady();
	else {
		// kick it off
		tryNext();
	}
}

void SrvResolver::ndns_done()
{
	SafeDeleteLock s(&d->sd);

	uint r = d->ndns.result();
	int port = d->servers.first().port;
	d->servers.remove(d->servers.begin());

	if(r) {
		d->result = r;
		d->resultString = d->ndns.resultString();
		d->resultPort = port;
		resultsReady();
	}
	else {
		// failed?  bail if last one
		if(d->servers.isEmpty()) {
			stop();
			resultsReady();
			return;
		}

		// otherwise try the next
		tryNext();
	}
}

void SrvResolver::t_timeout()
{
	SafeDeleteLock s(&d->sd);

	stop();
	resultsReady();
}

// CS_NAMESPACE_END
