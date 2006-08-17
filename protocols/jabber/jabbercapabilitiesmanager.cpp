 /*
    jabbercapabilitiesmanager.cpp - Manage entity capabilities(JEP-0115).

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    Imported from caps.cpp from Psi:
    Copyright (C) 2005  Remko Troncon

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "jabbercapabilitiesmanager.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qpair.h>
#include <qdom.h>
#include <qtextstream.h>

#include <kstandarddirs.h>
#include <kdebug.h>

#include <xmpp_tasks.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"

using namespace XMPP;

//BEGIN Capabilities
JabberCapabilitiesManager::Capabilities::Capabilities()
{}

JabberCapabilitiesManager::Capabilities::Capabilities(const QString& node, const QString& version, const QString& extensions) 
	: m_node(node), m_version(version), m_extensions(extensions) 
{}

const QString& JabberCapabilitiesManager::Capabilities::node() const 
{ 
	return m_node; 
}

const QString& JabberCapabilitiesManager::Capabilities::version() const 
{ 
	return m_version; 
}

const QString& JabberCapabilitiesManager::Capabilities::extensions() const 
{ 
	return m_extensions; 
}

JabberCapabilitiesManager::CapabilitiesList JabberCapabilitiesManager::Capabilities::flatten() const 
{
	CapabilitiesList capsList;
	capsList.append( Capabilities(node(), version(), version()) );

	QStringList extensionList = QStringList::split(" ",extensions());
	QStringList::ConstIterator it, itEnd = extensionList.constEnd();
	for(it = extensionList.constBegin(); it != itEnd; ++it)
	{
		capsList.append( Capabilities(node(),version(),*it) );
	}

	return capsList;
}

bool JabberCapabilitiesManager::Capabilities::operator==(const Capabilities &other) const 
{
	return (node() == other.node() && version() == other.version() && extensions() == other.extensions());
}

bool JabberCapabilitiesManager::Capabilities::operator!=(const Capabilities &other) const 
{
	return !((*this) == other);
}

bool JabberCapabilitiesManager::Capabilities::operator<(const Capabilities &other) const 
{
	return (node() != other.node() ? node() < other.node() :
			(version() != other.version() ? version() < other.version() : 
			 extensions() < other.extensions()));
}
//END Capabilities

//BEGIN CapabilitiesInformation
JabberCapabilitiesManager::CapabilitiesInformation::CapabilitiesInformation() 
	: m_discovered(false), m_pendingRequests(0)
{
	updateLastSeen();
}

const QStringList& JabberCapabilitiesManager::CapabilitiesInformation::features() const
{
	return m_features;
}

const DiscoItem::Identities& JabberCapabilitiesManager::CapabilitiesInformation::identities() const
{
	return m_identities;
}

QStringList JabberCapabilitiesManager::CapabilitiesInformation::jids() const
{
	QStringList jids;
	
	QValueList<QPair<QString,JabberAccount*> >::ConstIterator it = m_jids.constBegin(), itEnd = m_jids.constEnd();
	for( ; it != itEnd; ++it) 
	{
		QString jid( (*it).first );
		if( !jids.contains(jid) )
			jids.push_back(jid);
	}

	return jids;
}

bool JabberCapabilitiesManager::CapabilitiesInformation::discovered() const
{
	return m_discovered;
}

int JabberCapabilitiesManager::CapabilitiesInformation::pendingRequests() const
{
	return m_pendingRequests;
}

void JabberCapabilitiesManager::CapabilitiesInformation::reset()
{
	m_features.clear();
	m_identities.clear();
	m_discovered = false;
}

void JabberCapabilitiesManager::CapabilitiesInformation::removeAccount(JabberAccount *account)
{
	QValueList<QPair<QString,JabberAccount*> >::Iterator it = m_jids.begin();
	while( it != m_jids.end() ) 
	{
		if( (*it).second == account) 
		{
			QValueList<QPair<QString,JabberAccount*> >::Iterator otherIt = it;
			it++;
			m_jids.remove(otherIt);
		}
		else 
		{
			it++;
		}
	}
}

void JabberCapabilitiesManager::CapabilitiesInformation::addJid(const Jid& jid, JabberAccount* account)
{
	QPair<QString,JabberAccount*> jidAccountPair(jid.full(),account);

	if( !m_jids.contains(jidAccountPair) ) 
	{
		m_jids.push_back(jidAccountPair);
		updateLastSeen();
	}
}

void JabberCapabilitiesManager::CapabilitiesInformation::removeJid(const Jid& jid)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unregistering " << QString(jid.full()).replace('%',"%%") << endl;

	QValueList<QPair<QString,JabberAccount*> >::Iterator it = m_jids.begin();
	while( it != m_jids.end() ) 
	{
		if( (*it).first == jid.full() ) 
		{
			QValueList<QPair<QString,JabberAccount*> >::Iterator otherIt = it;
			it++;
			m_jids.remove(otherIt);
		}
		else 
		{
			it++;
		}
	}
}

QPair<Jid,JabberAccount*> JabberCapabilitiesManager::CapabilitiesInformation::nextJid(const Jid& jid, const Task* t)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Looking for next JID" << endl;

	QValueList<QPair<QString,JabberAccount*> >::ConstIterator it = m_jids.constBegin(), itEnd = m_jids.constEnd();
	for( ; it != itEnd; ++it) 
	{
		if( (*it).first == jid.full() && (*it).second->client()->rootTask() == t) 
		{
			it++;
			if (it == itEnd) 
			{
				kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "No more JIDs" << endl;

				return QPair<Jid,JabberAccount*>(Jid(),0L);
			}
			else if( (*it).second->isConnected() ) 
			{
				//qDebug("caps.cpp: Account isn't active");
				kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Account isn't connected." << endl;

				return QPair<Jid,JabberAccount*>( (*it).first,(*it).second );
			}
		}
	}
	return QPair<Jid,JabberAccount*>(Jid(),0L);
}

void JabberCapabilitiesManager::CapabilitiesInformation::setDiscovered(bool value)
{
	m_discovered = value;
}

void JabberCapabilitiesManager::CapabilitiesInformation::setPendingRequests(int pendingRequests)
{
	m_pendingRequests = pendingRequests;
}

void JabberCapabilitiesManager::CapabilitiesInformation::setIdentities(const DiscoItem::Identities& identities)
{
	m_identities = identities;
}

void JabberCapabilitiesManager::CapabilitiesInformation::setFeatures(const QStringList& featureList)
{
	m_features = featureList;
}
	
void JabberCapabilitiesManager::CapabilitiesInformation::updateLastSeen()
{
	m_lastSeen = QDate::currentDate();
}

QDomElement JabberCapabilitiesManager::CapabilitiesInformation::toXml(QDomDocument *doc) const
{
	QDomElement info = doc->createElement("info");
	//info.setAttribute("last-seen",lastSeen_.toString(Qt::ISODate));

	// Identities
	DiscoItem::Identities::ConstIterator discoIt = m_identities.constBegin(), discoItEnd = m_identities.constEnd();
	for( ; discoIt != discoItEnd; ++discoIt ) 
	{
		QDomElement identity = doc->createElement("identity");
		identity.setAttribute("category",(*discoIt).category);
		identity.setAttribute("name",(*discoIt).name);
		identity.setAttribute("type",(*discoIt).type);
		info.appendChild(identity);
	}

	// Features
	QStringList::ConstIterator featuresIt = m_features.constBegin(), featuresItEnd = m_features.constEnd();
	for( ; featuresIt != featuresItEnd; ++featuresIt )
	{
		QDomElement feature = doc->createElement("feature");
		feature.setAttribute("node",*featuresIt);
		info.appendChild(feature);
	}

	return info;
}

void JabberCapabilitiesManager::CapabilitiesInformation::fromXml(const QDomElement &element)
{
	if( element.tagName() != "info") 
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Invalid info element" << endl;
		return;
	}
	
	//if (!e.attribute("last-seen").isEmpty())
	//	lastSeen_ = QDate::fromString(e.attribute("last-seen"),Qt::ISODate);

	for(QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) 
	{
		QDomElement infoElement = node.toElement();
		if( infoElement.isNull() ) 
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Null element" << endl;
			continue;
		}

		if( infoElement.tagName() == "identity") 
		{
			DiscoItem::Identity id;
			id.category = infoElement.attribute("category");
			id.name = infoElement.attribute("name");
			id.type = infoElement.attribute("type");
			m_identities += id;
		}
		else if( infoElement.tagName() == "feature" ) 
		{
			m_features += infoElement.attribute("node");
		}
		else 
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unknown element" << endl;
		}

		m_discovered = true;
	}
}
//END CapabilitiesInformation

//BEGIN Private(d-ptr)
class JabberCapabilitiesManager::Private
{
public:
	Private()
	{}

	// Map a full jid to a capabilities
	QMap<QString,Capabilities> jidCapabilitiesMap;
	// Map a capabilities to its detail information
	QMap<Capabilities,CapabilitiesInformation> capabilitiesInformationMap;
};
//END Private(d-ptr)

JabberCapabilitiesManager::JabberCapabilitiesManager()
	: d(new Private)
{
}

JabberCapabilitiesManager::~JabberCapabilitiesManager()
{
	saveInformation();
	delete d;
}

void JabberCapabilitiesManager::removeAccount(JabberAccount *account)
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Removing account " << account->accountId() << endl;

	QValueList<CapabilitiesInformation> info = d->capabilitiesInformationMap.values();

	QValueList<CapabilitiesInformation>::Iterator it, itEnd = info.end();
	for(it = info.begin(); it != info.end(); ++it) 
	{
		(*it).removeAccount(account);
	}
}

void JabberCapabilitiesManager::updateCapabilities(JabberAccount *account, const XMPP::Jid &jid, const XMPP::Status &status )
{
	if( !account->client() || !account->client()->rootTask() )
		return;
	
	
	// Do don't anything if the jid correspond to the account's JabberClient jid.
	// false means that we don't check for resources.
	if( jid.compare(account->client()->jid(), false) )
		return;

	QString node = status.capsNode(), version = status.capsVersion(), extensions = status.capsExt();
	Capabilities capabilities( node, version, extensions );
	
	// Check if the capabilities was really updated(i.e the content is different)
	if( d->jidCapabilitiesMap[jid.full()] != capabilities) 
	{
		// Unregister from all old caps nodes
		// FIXME: We should only unregister & register from changed nodes
		CapabilitiesList oldCaps = d->jidCapabilitiesMap[jid.full()].flatten();
		CapabilitiesList::Iterator oldCapsIt = oldCaps.begin(), oldCapsItEnd = oldCaps.end();
		for( ; oldCapsIt != oldCapsItEnd; ++oldCapsIt) 
		{
			if( (*oldCapsIt) != Capabilities() ) 
			{
				d->capabilitiesInformationMap[*oldCapsIt].removeJid(jid);
			}
		}

		// Check if the jid has caps in his presence message.
		if( !status.capsNode().isEmpty() && !status.capsVersion().isEmpty() ) 
		{
			// Register with all new caps nodes
			d->jidCapabilitiesMap[jid.full()] = capabilities;
			CapabilitiesList caps = capabilities.flatten();
			CapabilitiesList::Iterator newCapsIt = caps.begin(), newCapsItEnd = caps.end();
			for( ; newCapsIt != newCapsItEnd; ++newCapsIt ) 
			{
				d->capabilitiesInformationMap[*newCapsIt].addJid(jid,account);
			}
			
			emit capabilitiesChanged(jid); 

			// Register new caps and check if we need to discover features
			newCapsIt = caps.begin();
			for( ; newCapsIt != newCapsItEnd; ++newCapsIt ) 
			{
				if( !d->capabilitiesInformationMap[*newCapsIt].discovered() && d->capabilitiesInformationMap[*newCapsIt].pendingRequests() == 0 ) 
				{
					kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << QString("Sending disco request to %1, node=%2").arg(QString(jid.full()).replace('%',"%%")).arg(node + "#" + (*newCapsIt).extensions()) << endl;

					d->capabilitiesInformationMap[*newCapsIt].setPendingRequests(1);
					requestDiscoInfo(account, jid, node + "#" + (*newCapsIt).extensions());
				}
			}
		}
		else 
		{
			// Remove all caps specifications
			kdDebug(JABBER_DEBUG_GLOBAL) << QString("Illegal caps info from %1: node=%2, ver=%3").arg(QString(jid.full()).replace('%',"%%")).arg(node).arg(version) << endl;

			d->jidCapabilitiesMap.remove( jid.full() );
		}
	}
	else
	{
		// Add to the list of jids
		CapabilitiesList caps = capabilities.flatten();
		CapabilitiesList::Iterator capsIt = caps.begin(), capsItEnd = caps.end();
		for( ; capsIt != capsItEnd; ++capsIt) 
		{
			d->capabilitiesInformationMap[*capsIt].addJid(jid,account);
		}
	}
}

void JabberCapabilitiesManager::requestDiscoInfo(JabberAccount *account, const Jid& jid, const QString& node) 
{
	if( !account->client()->rootTask() )
		return;
 
	JT_DiscoInfo *discoInfo = new JT_DiscoInfo(account->client()->rootTask());
	connect(discoInfo, SIGNAL(finished()), SLOT(discoRequestFinished()));
	discoInfo->get(jid, node);
	//pending_++;
	//timer_.start(REQUEST_TIMEOUT,true);
	discoInfo->go(true);
}

void JabberCapabilitiesManager::discoRequestFinished()
{
	JT_DiscoInfo *discoInfo = (JT_DiscoInfo*)sender();
	if (!discoInfo)
		return;

	DiscoItem item = discoInfo->item();
	Jid jid = discoInfo->jid();
	kdDebug(JABBER_DEBUG_GLOBAL) << QString("Disco response from %1, node=%2, success=%3").arg(QString(jid.full()).replace('%',"%%")).arg(discoInfo->node()).arg(discoInfo->success()) << endl;

	QStringList tokens = QStringList::split("#",discoInfo->node());

	// Update features
	Q_ASSERT(tokens.count() == 2);
	QString node = tokens[0];
	QString extensions = tokens[1];

	Capabilities jidCapabilities = d->jidCapabilitiesMap[jid.full()];
	if( jidCapabilities.node() == node )
	{
		Capabilities capabilities(node, jidCapabilities.version(), extensions);

		if( discoInfo->success() )
		{
			// Save identities & features
			d->capabilitiesInformationMap[capabilities].setIdentities(item.identities());
			d->capabilitiesInformationMap[capabilities].setFeatures(item.features().list());
			d->capabilitiesInformationMap[capabilities].setPendingRequests(0);
			d->capabilitiesInformationMap[capabilities].setDiscovered(true);

			// Save(Cache) information
			saveInformation();
			
			// Notify affected jids.
			QStringList jids = d->capabilitiesInformationMap[capabilities].jids();
			QStringList::ConstIterator jidsIt = jids.constBegin(), jidsItEnd = jids.constEnd();
			for( ; jidsIt != jidsItEnd; ++jidsItEnd ) 
			{
				emit capabilitiesChanged(*jidsIt);
			}
		}
		else 
		{
			QPair<Jid,JabberAccount*> jidAccountPair = d->capabilitiesInformationMap[capabilities].nextJid(jid,discoInfo->parent());
			if( jidAccountPair.second ) 
			{
				kdDebug(JABBER_DEBUG_GLOBAL) << QString("Falling back on %1.").arg(QString(jidAccountPair.first.full()).replace('%',"%%")) << endl;
				requestDiscoInfo( jidAccountPair.second, jidAccountPair.first, discoInfo->node() );
			}
			else 
			{
				kdDebug(JABBER_DEBUG_GLOBAL) << "No valid disco request avalable." << endl;
				d->capabilitiesInformationMap[capabilities].setPendingRequests(0);
			}
		}
	}
	else 
		kdDebug(JABBER_DEBUG_GLOBAL) << QString("Current client node '%1' does not match response '%2'").arg(jidCapabilities.node()).arg(node) << endl;

	//for (unsigned int i = 0; i < item.features().list().count(); i++) 
	//	printf("    Feature: %s\n",item.features().list()[i].latin1());

	// Check pending requests
//	pending_ = (pending_ > 0 ? pending_-1 : 0);
//	if (!pending_) {
//		timer_.stop();
//		updatePendingJIDs();
//	}
}

void JabberCapabilitiesManager::loadCachedInformation()
{
	QString capsFileName;
	capsFileName = locateLocal("appdata", QString::fromUtf8("jabber-capabilities-cache.xml"));

	// Load settings
	QDomDocument doc;
	QFile cacheFile(capsFileName);
	if( !cacheFile.open(IO_ReadOnly) )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Could not open the Capabilities cache from disk." << endl;
		return;
	}
	if( !doc.setContent(&cacheFile) )
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Could not set the Capabilities cache from file." << endl;
		return;
	}
	cacheFile.close();

	QDomElement caps = doc.documentElement();
	if( caps.tagName() != "capabilities" ) 
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Invalid capabilities element." << endl;
		return;
	}
	
	QDomNode node;	
	for(node = caps.firstChild(); !node.isNull(); node = node.nextSibling()) 
	{
		QDomElement element = node.toElement();
		if( element.isNull() ) 
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Found a null element." << endl;
			continue;
		}

		if( element.tagName() == "info" ) 
		{
			CapabilitiesInformation info;
			info.fromXml(element);
			Capabilities entityCaps( element.attribute("node"),element.attribute("ver"),element.attribute("ext") );
			d->capabilitiesInformationMap[entityCaps] = info;
		}
		else 
		{
			kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unknow element" << endl;
		}
	}
}

bool JabberCapabilitiesManager::capabilitiesEnabled(const Jid &jid) const
{
	return d->jidCapabilitiesMap.contains( jid.full() );	
}

XMPP::Features JabberCapabilitiesManager::features(const Jid& jid) const
{
	QStringList featuresList;

	if( capabilitiesEnabled(jid) ) 
	{
		CapabilitiesList capabilitiesList = d->jidCapabilitiesMap[jid.full()].flatten();
		CapabilitiesList::ConstIterator capsIt = capabilitiesList.constBegin(), capsItEnd = capabilitiesList.constEnd();
		for( ; capsIt != capsItEnd; ++capsIt) 
		{
			featuresList += d->capabilitiesInformationMap[*capsIt].features();
		}
	}

	return Features(featuresList);
}

QString JabberCapabilitiesManager::clientName(const Jid& jid) const
{
	if( capabilitiesEnabled(jid) ) 
	{
		Capabilities caps = d->jidCapabilitiesMap[jid.full()];
		QString name = d->capabilitiesInformationMap[Capabilities(caps.node(),caps.version(),caps.version())].identities().first().name;
		
		// Try to be intelligent about the name
		/*if (name.isEmpty()) {
			name = cs.node();
			if (name.startsWith("http://"))
				name = name.right(name.length() - 7);
				
			if (name.startsWith("www."))
				name = name.right(name.length() - 4);
			
			int cut_pos = name.find(".");
			if (cut_pos != -1) {
				name = name.left(cut_pos);
			}
		}*/

		return name;
	}
	else 
	{
		return QString();
	}
}

QString JabberCapabilitiesManager::clientVersion(const Jid& jid) const
{
	return (capabilitiesEnabled(jid) ? d->jidCapabilitiesMap[jid.full()].version() : QString());
}

void JabberCapabilitiesManager::saveInformation()
{
	QString capsFileName;
	capsFileName = locateLocal("appdata", QString::fromUtf8("jabber-capabilities-cache.xml"));

	// Generate XML
	QDomDocument doc;
	QDomElement capabilities = doc.createElement("capabilities");
	doc.appendChild(capabilities);
	QMap<Capabilities,CapabilitiesInformation>::ConstIterator it = d->capabilitiesInformationMap.constBegin(), itEnd = d->capabilitiesInformationMap.constEnd();
	for( ; it != itEnd; ++it ) 
	{
		QDomElement info = it.data().toXml(&doc);
		info.setAttribute("node",it.key().node());
		info.setAttribute("ver",it.key().version());
		info.setAttribute("ext",it.key().extensions());
		capabilities.appendChild(info);
	}

	// Save
	QFile capsFile(capsFileName);
	if( !capsFile.open(IO_WriteOnly) ) 
	{
		kdDebug(JABBER_DEBUG_GLOBAL	) << k_funcinfo << "Error while opening Capabilities cache file." << endl;
		return;
	}

	QTextStream textStream;
	textStream.setDevice(&capsFile);
	textStream.setEncoding(QTextStream::UnicodeUTF8);
	textStream << doc.toString();
	textStream.unsetDevice();
	capsFile.close();
}

#include "jabbercapabilitiesmanager.moc"
