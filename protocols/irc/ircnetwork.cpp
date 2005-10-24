/*
    ircaccount.cpp - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2004 by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircnetwork.h"

#include <kdebug.h>
#include <kstandarddirs.h> // for locate

#include <qdom.h>
#include <qfile.h>

class IRC::Networks::Private {
public:
	IRC::NetworkList networks;
};

using namespace IRC;

Networks::Networks()
	: d(0)
{
	slotReadNetworks();
}

Networks::~Networks()
{
//	delete d;
}

Networks *Networks::self()
{
	static Networks *self = new IRC::Networks;
	return self;
}

const NetworkList &Networks::networks() const
{
	return d->networks;
}

bool Networks::slotReadNetworks()
{
	d->networks.clear();

	QFile xmlFile( locate( "appdata", "ircnetworks.xml" ) );
	xmlFile.open( IO_ReadOnly );

	QDomDocument doc;
	doc.setContent( &xmlFile );
	QDomElement networkNode = doc.documentElement().firstChild().toElement();
	while( !networkNode.isNull () )
	{
		Network network;

		QDomElement networkChild = networkNode.firstChild().toElement();
		while( !networkChild.isNull() )
		{
			if( networkChild.tagName() == "name" )
				network.name = networkChild.text();
			else if( networkChild.tagName() == "description" )
				network.description = networkChild.text();
			else if( networkChild.tagName() == "servers" )
			{
				QDomElement server = networkChild.firstChild().toElement();
				while( !server.isNull() )
				{
					Host host;

					QDomElement serverChild = server.firstChild().toElement();
					while( !serverChild.isNull() )
					{
						if( serverChild.tagName() == "host" )
							host.host = serverChild.text();
						else if( serverChild.tagName() == "port" )
							host.port = serverChild.text().toInt();
						else if( serverChild.tagName() == "useSSL" )
							host.ssl = ( serverChild.text() == "true" );

						serverChild = serverChild.nextSibling().toElement();
					}

					network.hosts.append( host );
//					d->hosts.insert( host->host, host );
					server = server.nextSibling().toElement();
				}
			}
			networkChild = networkChild.nextSibling().toElement();
		}

		d->networks.append(network);
		networkNode = networkNode.nextSibling().toElement();
	}

	xmlFile.close();
}

bool Networks::slotSaveNetworkConfig() const
{
/*
	// store any changes in the UI
	storeCurrentNetwork();
	kdDebug( 14120 ) <<  k_funcinfo << m_uiCurrentHostSelection << endl;
	storeCurrentHost();

	QDomDocument doc("irc-networks");
	QDomNode root = doc.appendChild( doc.createElement("networks") );

	foreach(const Network &network, d->networks)
	{
		QDomNode networkNode = root.appendChild( doc.createElement("network") );
		QDomNode nameNode = networkNode.appendChild( doc.createElement("name") );
		nameNode.appendChild( doc.createTextNode( net->name ) );

		QDomNode descNode = networkNode.appendChild( doc.createElement("description") );
		descNode.appendChild( doc.createTextNode( net->description ) );

		QDomNode serversNode = networkNode.appendChild( doc.createElement("servers") );

		foreach(const Host &host, network.host)
		{
			QDomNode serverNode = serversNode.appendChild( doc.createElement("server") );

			QDomNode hostNode = serverNode.appendChild( doc.createElement("host") );
			hostNode.appendChild( doc.createTextNode( (*it2)->host ) );

			QDomNode portNode = serverNode.appendChild( doc.createElement("port" ) );
			portNode.appendChild( doc.createTextNode( QString::number( (*it2)->port ) ) );

			QDomNode sslNode = serverNode.appendChild( doc.createElement("useSSL") );
			sslNode.appendChild( doc.createTextNode( (*it2)->ssl ? "true" : "false" ) );
		}
	}

//	kdDebug(14121) << k_funcinfo << doc.toString(4) << endl;
	QFile xmlFile( locateLocal( "appdata", "ircnetworks.xml" ) );

	if (xmlFile.open(IO_WriteOnly))
	{
		QTextStream stream( &xmlFile );
		stream << doc.toString(4);
		xmlFile.close();
		return true;
	}
*/
	kdDebug(14121) << k_funcinfo << "Failed to save the Networks definition file" << endl;
	return false;
}
/*
void Networks::addNetwork( IRCNetwork *network )
{
	m_networks.insert( network->name, network );
//	slotSaveNetworkConfig();
}
*/

#include "ircnetwork.moc"
