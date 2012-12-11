 /*
    jabbercapabilitiesmanager.h - Manage entity capabilities(JEP-0115) pool.

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
#ifndef JABBERCAPABILITIESMANAGER_H
#define JABBERCAPABILITIESMANAGER_H

#include <QPair>
#include <QList>
#include <QDate>

#include <QStringList>
#include <QDomElement>

#include <im.h>
#include <xmpp.h>

using namespace XMPP;

class JabberAccount;

/**
 * @brief Manage Jabber entity capabilities (JEP-0115)
 * @author Michaël Larouche <larouche@kde.org>
 * @author Remko Troncon
 */
class JabberCapabilitiesManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * Construct
	 */
	JabberCapabilitiesManager();
	~JabberCapabilitiesManager();

	/**
	 * Load cached information from local file.
	 */
	void loadCachedInformation();

	/**
	 * Check if the jid support Entity capabitilies.
	 * @param jid JID to check.
	 * @return true if the jid support entity capabitilies.
	 */
	bool capabilitiesEnabled(const Jid& jid) const;

	/**
	 * Remove account from manager.
	 */
	void removeAccount(JabberAccount *account);

	/**
	 * Return the features supported for the JID.
	 */
	XMPP::Features features(const Jid& jid) const;
	/**
	 * Return the client name for the current JID.
	 */
	QString clientName(const Jid& jid) const;
	/**
	 * Return the client version for the current JID.
	 */
	QString clientVersion(const Jid& jid) const;

signals:
	void capabilitiesChanged(const XMPP::Jid &jid);

public slots:
	/**
	 * Update if necessary the capabities for the JID passed in args.
	 * Caps are received in Presence messages so that's why we are
	 * passing a XMPP::Status object.
	 *
	 * @param jid JID that capabilities was updated.
	 * @param status The XMPP::Status that contain the caps.
	 */
	void updateCapabilities(JabberAccount *account, const XMPP::Jid &jid, const XMPP::Status &status);

private slots:
	/**
	 * @brief Called when a reply to disco#info request was received.
	 * If the result was successful, the resulting features are recorded in the
	 * features database for the requested node, and all the affected jids are
	 * put in the queue for update notification.
	 * If the result was unsuccessful, another jid with the same capabilities is
	 * selected and sent a disco#info query.
	 */
	void discoRequestFinished();

private:
	/**
	 * @brief Sends a disco#info request to a given node of a jid through an account.
	 * When the request is finished, the discoRequestFinished() slot is called.
	 *
	 * @param account The account through which to send the disco request.
	 * @param jid The target entity's JID 
	 * @param node The target disco#info node
	 */
	void requestDiscoInfo(JabberAccount *account, const Jid& jid, const QString& node);

	/**
	 * Save capabilities information to disk.
	 */
	void saveInformation();

	class Capabilities;
	typedef QList<Capabilities> CapabilitiesList;
	/**
	 * @brief A class representing an entity capability specification.
	 * An entity capability is a combination of a node, a version, and a set of
	 * extensions.
	 */
	class Capabilities
	{
		public:
			/**
			 * Default constructor.
			 */
			Capabilities();
			/**
			 * Define capabilities.
			 * @param node the node
			 * @param version the version
			 * @param extensions the list of extensions (separated by spaces)
			 * @param hash the hash
			 */
			Capabilities(const QString &node, const QString &version, const QString &extensions, const QString &hash);
			/**
			 * Returns the node of the capabilities specification.
			 */
			const QString& node() const;
			/**
			 * @brief Returns the version of the capabilities specification.
			 */
			const QString& version() const;
			/**
			 * @brief Returns the extensions of the capabilities specification.
			 */
			const QString& extensions() const; 
			/**
			 * @brief Returns the hash of the capabilities specification.
			 */
			const QString& hash() const;
			/**
			 * \brief Flattens the caps specification into the set of 'simple' specifications.
			 * A 'simple' specification is a specification with exactly one extension,
			 * or with the version number as the extension.
			 *
			 * Example: A caps specification with node=http://psi-im.org, version=0.10,
			 * and ext='achat vchat' would be expanded into the following list of specs:
			 *	node=http://psi-im.org, ver=0.10, ext=0.10
			 *	node=http://psi-im.org, ver=0.10, ext=achat
			 *	node=http://psi-im.org, ver=0.10, ext=vchat
			 */
			CapabilitiesList flatten() const;
	
			bool operator==(const Capabilities&) const;
			bool operator!=(const Capabilities&) const;
			bool operator<(const Capabilities&) const;
				
		private:
			QString m_node, m_version, m_extensions, m_hash;
	};

	class CapabilitiesInformation
	{
		public:
			CapabilitiesInformation();
			const QStringList& features() const;
			const DiscoItem::Identities& identities() const;
			QStringList jids() const;
			bool discovered() const;
			int pendingRequests() const;

			void reset();
			void removeAccount(JabberAccount* acc);
			void removeJid(const Jid&);
			void addJid(const Jid&, JabberAccount*);
			QPair<Jid,JabberAccount*> nextJid(const Jid&, const Task*);
			
			void setDiscovered(bool);
			void setPendingRequests(int);
			void setIdentities(const DiscoItem::Identities&);
			void setFeatures(const QStringList&);
			
			QDomElement toXml(QDomDocument *) const;
			void fromXml(const QDomElement&);

		protected:
			void updateLastSeen();
			
		private:
			bool m_discovered;
			int m_pendingRequests;
			QStringList m_features;
			DiscoItem::Identities m_identities;

			typedef QList<QPair<QString, JabberAccount*> > JidList;
			JidList m_jids;

			QDate m_lastSeen;
	};

	class Private;
	Private * const d;
};

#endif
