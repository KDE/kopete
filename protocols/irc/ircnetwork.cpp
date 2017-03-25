/*
    ircaccount.cpp - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2004 by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

struct IRC::Networks::Private {
    IRC::NetworkList networks;
};

using namespace IRC;

Networks::Networks()
    : d(new Private)
{
    slotReadNetworks();
}

Networks::~Networks()
{
    delete d;
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

void Networks::setNetworks(const IRC::NetworkList &networks)
{
    d->networks = networks;
}

const IRC::Network &Networks::network(const QString &name)
{
    foreach (const Network &net, d->networks) {
        if (net.name == name) {
            return net;
        }
    }
    return Network();
}

void Networks::slotReadNetworks()
{
    d->networks.clear();

    QFile xmlFile(KStandardDirs::locate("appdata", "ircnetworks.xml"));
    xmlFile.open(QIODevice::ReadOnly);

    // FIXME
    QDomDocument doc;
    doc.setContent(&xmlFile);
    QDomElement networkNode = doc.documentElement().firstChild().toElement();
    while (!networkNode.isNull())
    {
        Network network;

        QDomElement networkChild = networkNode.firstChild().toElement();
        while (!networkChild.isNull())
        {
            if (networkChild.tagName() == "name") {
                network.name = networkChild.text();
            } else if (networkChild.tagName() == "description") {
                network.description = networkChild.text();
            } else if (networkChild.tagName() == "servers") {
                QDomElement server = networkChild.firstChild().toElement();
                while (!server.isNull())
                {
                    Host host;

                    QDomElement serverChild = server.firstChild().toElement();
                    while (!serverChild.isNull())
                    {
                        if (serverChild.tagName() == "host") {
                            host.host = serverChild.text();
                        } else if (serverChild.tagName() == "port") {
                            host.port = serverChild.text().toInt();
                        } else if (serverChild.tagName() == "useSSL") {
                            host.ssl = (serverChild.text() == "true");
                        }

                        serverChild = serverChild.nextSibling().toElement();
                    }

                    network.hosts.append(host);
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
    // store any changes in the UI
    //storeCurrentNetwork();
    //kDebug( 14120 ) << m_uiCurrentHostSelection;
    //storeCurrentHost();

    QDomDocument doc("irc-networks");
    QDomNode root = doc.appendChild(doc.createElement("networks"));

    foreach (const Network &network, d->networks) {
        QDomNode networkNode = root.appendChild(doc.createElement("network"));
        QDomNode nameNode = networkNode.appendChild(doc.createElement("name"));
        nameNode.appendChild(doc.createTextNode(network.name));

        QDomNode descNode = networkNode.appendChild(doc.createElement("description"));
        descNode.appendChild(doc.createTextNode(network.description));

        QDomNode serversNode = networkNode.appendChild(doc.createElement("servers"));

        foreach (const Host &host, network.hosts) {
            QDomNode serverNode = serversNode.appendChild(doc.createElement("server"));

            QDomNode hostNode = serverNode.appendChild(doc.createElement("host"));
            hostNode.appendChild(doc.createTextNode(host.host));

            QDomNode portNode = serverNode.appendChild(doc.createElement("port"));
            portNode.appendChild(doc.createTextNode(QString::number(host.port)));

            QDomNode sslNode = serverNode.appendChild(doc.createElement("useSSL"));
            sslNode.appendChild(doc.createTextNode(host.ssl ? "true" : "false"));
        }
    }

//	kDebug(14121) << doc.toString(4);
    QFile xmlFile(KStandardDirs::locateLocal("appdata", "ircnetworks.xml"));

    if (xmlFile.open(QIODevice::WriteOnly)) {
        QTextStream stream(&xmlFile);
        stream << doc.toString(4);
        xmlFile.close();
        return true;
    }

    kDebug(14121) << "Failed to save the Networks definition file";
    return false;
}

/*
void Networks::addNetwork( IRCNetwork *network )
{
    m_networks.insert( network->name, network );
//	slotSaveNetworkConfig();
}
*/
