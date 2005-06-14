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

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "channellistdialog.h"

#include "kircengine.h"

#include "kopeteaccountmanager.h"
#include "kopeteaway.h"
#include "kopeteawayaction.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcompletionbox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpopupmenu.h>

#include <qlayout.h>
#include <qtimer.h>

using namespace Kopete;

void IRCNetworkConfigWidget::slotReadNetworks()
{
	m_networks.clear();
	m_hosts.clear();

	QFile xmlFile( locate( "appdata", "ircnetworks.xml" ) );
	xmlFile.open( IO_ReadOnly );

	QDomDocument doc;
	doc.setContent( &xmlFile );
	QDomElement networkNode = doc.documentElement().firstChild().toElement();
	while( !networkNode.isNull () )
	{
		IRCNetwork *net = new IRCNetwork;

		QDomElement networkChild = networkNode.firstChild().toElement();
		while( !networkChild.isNull() )
		{
			if( networkChild.tagName() == "name" )
				net->name = networkChild.text();
			else if( networkChild.tagName() == "description" )
				net->description = networkChild.text();
			else if( networkChild.tagName() == "servers" )
			{
				QDomElement server = networkChild.firstChild().toElement();
				while( !server.isNull() )
				{
					IRCHost *host = new IRCHost;

					QDomElement serverChild = server.firstChild().toElement();
					while( !serverChild.isNull() )
					{
						if( serverChild.tagName() == "host" )
							host->host = serverChild.text();
						else if( serverChild.tagName() == "port" )
							host->port = serverChild.text().toInt();
						else if( serverChild.tagName() == "useSSL" )
							host->ssl = ( serverChild.text() == "true" );

						serverChild = serverChild.nextSibling().toElement();
					}

					net->hosts.append( host );
					m_hosts.insert( host->host, host );
					server = server.nextSibling().toElement();
				}
			}
			networkChild = networkChild.nextSibling().toElement();
		}

		m_networks.insert( net->name, net );
		networkNode = networkNode.nextSibling().toElement();
	}

	xmlFile.close();
}

void IRCNetworkConfigWidget::slotSaveNetworkConfig()
{
	// store any changes in the UI
	storeCurrentNetwork();
	kdDebug( 14120 ) <<  k_funcinfo << m_uiCurrentHostSelection << endl;
	storeCurrentHost();

	QDomDocument doc("irc-networks");
	QDomNode root = doc.appendChild( doc.createElement("networks") );

	for( QDictIterator<IRCNetwork> it( m_networks ); it.current(); ++it )
	{
		IRCNetwork *net = it.current();

		QDomNode networkNode = root.appendChild( doc.createElement("network") );
		QDomNode nameNode = networkNode.appendChild( doc.createElement("name") );
		nameNode.appendChild( doc.createTextNode( net->name ) );

		QDomNode descNode = networkNode.appendChild( doc.createElement("description") );
		descNode.appendChild( doc.createTextNode( net->description ) );

		QDomNode serversNode = networkNode.appendChild( doc.createElement("servers") );

		for( QValueList<IRCHost*>::iterator it2 = net->hosts.begin(); it2 != net->hosts.end(); ++it2 )
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

	kdDebug(14121) << k_funcinfo << doc.toString(4) << endl;
	QFile xmlFile( locateLocal( "appdata", "ircnetworks.xml" ) );
	QTextStream stream( &xmlFile );

	xmlFile.open( IO_WriteOnly );
	stream << doc.toString(4);
	xmlFile.close();

	if (netConf)
		emit networkConfigUpdated( netConf->networkList->currentText() );
}

#include "ircnetwork.moc"
