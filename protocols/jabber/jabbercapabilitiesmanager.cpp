 /*
    jabbercapabilitiesmanager.cpp - Manage entity capabilities(JEP-0115).

    Copyright (c) 2006      by Michaël Larouche     <larouche@kde.org>
    Copyright     2006      by Tommi Rantala <tommi.rantala@cs.helsinki.fi>

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
#include <qfile.h>
#include <qtextcodec.h>

#include <kstandarddirs.h>
#include <kdebug.h>

#include <xmpp_tasks.h>

#include "jabberaccount.h"
#include "jabberprotocol.h"

using namespace XMPP;

//BEGIN Capabilities
JabberCapabilitiesManager::Capabilities::Capabilities()
{}

JabberCapabilitiesManager::Capabilities::Capabilities(const QString& node, const QString& version, const QString& extensions, const QString& hash)
	: m_node(node), m_version(version), m_extensions(extensions), m_hash(hash)
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

const QString& JabberCapabilitiesManager::Capabilities::hash() const
{
	return m_hash;
}

JabberCapabilitiesManager::CapabilitiesList JabberCapabilitiesManager::Capabilities::flatten() const 
{
	CapabilitiesList capsList;
	capsList.append( Capabilities(node(), version(), version(), hash()) );

	QStringList extensionList = extensions().split(' ');

	foreach(QStringList::const_reference str, extensionList)
	{
		capsList.append( Capabilities(node(), version(), str, hash()) );
	}

	return capsList;
}

bool JabberCapabilitiesManager::Capabilities::operator==(const Capabilities &other) const 
{
	return (node() == other.node() && version() == other.version() && extensions() == other.extensions() && hash() == other.hash());
}

bool JabberCapabilitiesManager::Capabilities::operator!=(const Capabilities &other) const 
{
	return !((*this) == other);
}

bool JabberCapabilitiesManager::Capabilities::operator<(const Capabilities &other) const 
{
	return (node() != other.node() ? node() < other.node() :
			(version() != other.version() ? version() < other.version() : 
			 (extensions() != other.extensions() ? extensions() < other.extensions() :
			  hash() < other.hash())));
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

	foreach(JidList::const_reference pair, m_jids)
	{
		QString jid = pair.first;

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
	JidList::Iterator it = m_jids.begin();
	while( it != m_jids.end() ) 
	{
		if( (*it).second == account) 
		{
			it = m_jids.erase(it);
		}
		else 
		{
			it++;
		}
	}
}

void JabberCapabilitiesManager::CapabilitiesInformation::addJid(const Jid& jid, JabberAccount* account)
{
	QPair<QString, JabberAccount*> jidAccountPair(jid.full(),account);

	if( !m_jids.contains(jidAccountPair) ) 
	{
		m_jids.push_back(jidAccountPair);
		updateLastSeen();
	}
}

void JabberCapabilitiesManager::CapabilitiesInformation::removeJid(const Jid& jid)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Unregistering " << QString(jid.full()).replace('%',"%%");

	JidList::Iterator it = m_jids.begin();
	while( it != m_jids.end() ) 
	{
		if( (*it).first == jid.full() ) 
		{
			it = m_jids.erase(it);
		}
		else 
		{
			it++;
		}
	}
}

QPair<Jid,JabberAccount*> JabberCapabilitiesManager::CapabilitiesInformation::nextJid(const Jid& jid, const Task* t)
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Looking for next JID";

	JidList::ConstIterator it = m_jids.constBegin(), itEnd = m_jids.constEnd();
	for( ; it != itEnd; ++it) 
	{
		if( (*it).first == jid.full() && (*it).second->client()->rootTask() == t) 
		{
			it++;
			if (it == itEnd) 
			{
				kDebug(JABBER_DEBUG_GLOBAL) << "No more JIDs";

				return QPair<Jid,JabberAccount*>(Jid(),0L);
			}
			else if( (*it).second->isConnected() ) 
			{
				//qDebug("caps.cpp: Account isn't active");
				kDebug(JABBER_DEBUG_GLOBAL) << "Account isn't connected.";

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
	foreach(DiscoItem::Identities::const_reference ident, m_identities)
	{
		QDomElement identity = doc->createElement("identity");
		identity.setAttribute("category", ident.category);
		identity.setAttribute("name",     ident.name);
		identity.setAttribute("type",     ident.type);
		info.appendChild(identity);
	}

	// Features
	foreach(QStringList::const_reference feat, m_features)
	{
		QDomElement feature = doc->createElement("feature");
		feature.setAttribute("node", feat);
		info.appendChild(feature);
	}

	return info;
}

void JabberCapabilitiesManager::CapabilitiesInformation::fromXml(const QDomElement &element)
{
	if( element.tagName() != "info") 
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Invalid info element";
		return;
	}
	
	//if (!e.attribute("last-seen").isEmpty())
	//	lastSeen_ = QDate::fromString(e.attribute("last-seen"),Qt::ISODate);

	for(QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) 
	{
		QDomElement infoElement = node.toElement();
		if( infoElement.isNull() ) 
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Null element";
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
			kDebug(JABBER_DEBUG_GLOBAL) << "Unknown element";
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
	kDebug(JABBER_DEBUG_GLOBAL) << "Removing account " << account->accountId();

	QList<CapabilitiesInformation> info = d->capabilitiesInformationMap.values();

	foreach(CapabilitiesInformation cap, info)
	{
		cap.removeAccount(account);
	}
}

void JabberCapabilitiesManager::updateCapabilities(JabberAccount *account, const XMPP::Jid &jid, const XMPP::Status &status )
{
	if( !account->client() || !account->client()->rootTask() )
		return;

	const QString &node = status.caps().node();
	const QString &version = status.caps().version();
	const QString &extensions = status.caps().ext().join(" ");
	const QString &hash = XMPP::CapsSpec::cryptoMap().key(status.caps().hashAlgorithm());

	Capabilities capabilities( node, version, extensions, hash );
	
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
		if( !node.isEmpty() && !version.isEmpty() )
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
					kDebug(JABBER_DEBUG_GLOBAL) << QString("Sending disco request to %1, node=%2").arg(QString(jid.full()).replace('%',"%%")).arg(node + '#' + (*newCapsIt).extensions());

					d->capabilitiesInformationMap[*newCapsIt].setPendingRequests(1);
					requestDiscoInfo(account, jid, node + '#' + (*newCapsIt).extensions());
				}
			}
		}
		else 
		{
			// Remove all caps specifications
			kDebug(JABBER_DEBUG_GLOBAL) << QString("Illegal caps info from %1: node=%2, ver=%3").arg(QString(jid.full()).replace('%',"%%")).arg(node).arg(version);

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
	kDebug(JABBER_DEBUG_GLOBAL) << QString("Disco response from %1, node=%2, success=%3").arg(QString(jid.full()).replace('%',"%%")).arg(discoInfo->node()).arg(discoInfo->success());

	const QString &tokens = discoInfo->node();
	int idx = tokens.lastIndexOf('#');

	if (idx < 0)
		return;

	// Update features
	QString node = tokens.left(idx);
	QString extensions = tokens.mid(idx + 1);

	Capabilities jidCapabilities = d->jidCapabilitiesMap[jid.full()];
	if( jidCapabilities.node() == node )
	{
		Capabilities capabilities(node, jidCapabilities.version(), extensions, jidCapabilities.hash());

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
			kDebug(JABBER_DEBUG_GLOBAL) << "notify affected jids";
			foreach( const QString &jid, jids )
			{
				emit capabilitiesChanged(jid);
			}
		}
		else 
		{
			QPair<Jid,JabberAccount*> jidAccountPair = d->capabilitiesInformationMap[capabilities].nextJid(jid,discoInfo->parent());
			if( jidAccountPair.second ) 
			{
				kDebug(JABBER_DEBUG_GLOBAL) << QString("Falling back on %1.").arg(QString(jidAccountPair.first.full()).replace('%',"%%"));
				requestDiscoInfo( jidAccountPair.second, jidAccountPair.first, discoInfo->node() );
			}
			else 
			{
				kDebug(JABBER_DEBUG_GLOBAL) << "No valid disco request avalable.";
				d->capabilitiesInformationMap[capabilities].setPendingRequests(0);
			}
		}
	}
	else 
		kDebug(JABBER_DEBUG_GLOBAL) << QString("Current client node '%1' does not match response '%2'").arg(jidCapabilities.node()).arg(node);

	//for (unsigned int i = 0; i < item.features().list().count(); i++) 
	//	printf("    Feature: %s\n",item.features().list()[i].toLatin1());

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
	capsFileName = KStandardDirs::locateLocal("appdata", QString::fromUtf8("jabber-capabilities-cache.xml"));

	// Load settings
	QDomDocument doc;
	QFile cacheFile(capsFileName);
	if( !cacheFile.open(QIODevice::ReadOnly) )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Could not open the Capabilities cache from disk.";
		return;
	}
	if( !doc.setContent(&cacheFile) )
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Could not set the Capabilities cache from file.";
		return;
	}
	cacheFile.close();

	QDomElement caps = doc.documentElement();
	if( caps.tagName() != "capabilities" ) 
	{
		kDebug(JABBER_DEBUG_GLOBAL) << "Invalid capabilities element.";
		return;
	}
	
	QDomNode node;	
	for(node = caps.firstChild(); !node.isNull(); node = node.nextSibling()) 
	{
		QDomElement element = node.toElement();
		if( element.isNull() ) 
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Found a null element.";
			continue;
		}

		if( element.tagName() == "info" ) 
		{
			CapabilitiesInformation info;
			info.fromXml(element);
			Capabilities entityCaps( element.attribute("node"),element.attribute("ver"),element.attribute("ext"),element.attribute("hash") );
			d->capabilitiesInformationMap[entityCaps] = info;
		}
		else 
		{
			kDebug(JABBER_DEBUG_GLOBAL) << "Unknow element";
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

		foreach(CapabilitiesList::const_reference cap, capabilitiesList)
		{
			featuresList += d->capabilitiesInformationMap[cap].features();
		}
	}

	return Features(featuresList);
}

QString JabberCapabilitiesManager::clientName(const Jid& jid) const
{
	if( capabilitiesEnabled(jid) ) 
	{
		Capabilities caps = d->jidCapabilitiesMap[jid.full()];
		const XMPP::DiscoItem::Identities &identities = d->capabilitiesInformationMap[Capabilities(caps.node(),caps.version(),caps.version(),caps.hash())].identities();
		QString name;

		for ( int i = 0; i < identities.size(); ++i )
		{
			if ( identities[i].category == "client" && ! identities[i].name.isEmpty() )
			{
				name = identities[i].name;
				break;
			}
		}
		
		// Try to be intelligent about the name
		/*if (name.isEmpty()) {
			name = cs.node();
			if (name.startsWith("http://"))
				name = name.right(name.length() - 7);
				
			if (name.startsWith("www."))
				name = name.right(name.length() - 4);
			
			int cut_pos = name.find('.');
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
	if (capabilitiesEnabled(jid) && d->jidCapabilitiesMap[jid.full()].hash().isEmpty())
		return d->jidCapabilitiesMap[jid.full()].version();
	return QString();
}

void JabberCapabilitiesManager::saveInformation()
{
	QString capsFileName;
	capsFileName = KStandardDirs::locateLocal("appdata", QString::fromUtf8("jabber-capabilities-cache.xml"));

	// Generate XML
	QDomDocument doc;
	QDomElement capabilities = doc.createElement("capabilities");
	doc.appendChild(capabilities);

	QMap<Capabilities,CapabilitiesInformation>::ConstIterator it = d->capabilitiesInformationMap.constBegin(), itEnd = d->capabilitiesInformationMap.constEnd();
	for( ; it != itEnd; ++it ) 
	{
		QDomElement info = it.value().toXml(&doc);
		info.setAttribute("node",it.key().node());
		info.setAttribute("ver",it.key().version());
		info.setAttribute("ext",it.key().extensions());
		info.setAttribute("hash",it.key().hash());
		capabilities.appendChild(info);
	}

	// Save
	QFile capsFile(capsFileName);
	if( !capsFile.open(QIODevice::WriteOnly) ) 
	{
		kDebug(JABBER_DEBUG_GLOBAL	) << "Error while opening Capabilities cache file.";
		return;
	}

	QTextStream textStream;
	textStream.setDevice(&capsFile);
	textStream.setCodec(QTextCodec::codecForName("UTF-8"));
	textStream << doc.toString();
	textStream.setDevice(0);
	capsFile.close();
}

#include "jabbercapabilitiesmanager.moc"
