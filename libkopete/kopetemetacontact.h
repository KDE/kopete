/*
    kopetemetacontact.h - Kopete Meta Contact

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetemetacontact_h__
#define __kopetemetacontact_h__

#include <qobject.h>
#include <qptrlist.h>
//#include <qstringlist.h>
#include <qmap.h>

#include "kopetecontact.h"
#include "kopetegroup.h"

class QDomNode;
class QStringList;

class KopetePlugin;
class KopetePlugin;


/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopeteMetaContact : public QObject
{
	Q_OBJECT

public:
	// <PluginID, Value   >
	typedef QMap< QString, QString > AddressBookFields;

	KopeteMetaContact();
	~KopeteMetaContact();

	/**
	 * Retrieve the list of contacts that are part of the meta contact
	 */
	QPtrList<KopeteContact> contacts() const { return m_contacts; }

	/**
	 * Add contact to the meta contact
	 */
	void addContact( KopeteContact *c);
//	void addContact( KopeteContact *c, const QStringList &groups );
	
	/**
	 * Find the KopeteContact to a given contact. If contact
	 * is not found, a null pointer is returned.
	 */
	KopeteContact *findContact( const QString &protocolId, const QString &identityId, const QString &contactId );
	
	/**
	 * The name of the icon associated with the contact's status
	 */
	virtual QString statusIcon() const;

	/**
	 * The status-string of the contact
	 */
	QString statusString() const;

	/**
	 * Returns whether this contact can be reached online for at least one
	 * protocol. Protocols are processed in loading order.
	 * FIXME: Make that user preference order!
	 * FIXME: Make that an enum, because status can be unknown for certain
	 *        protocols
	 */
	bool isOnline() const;

	/**
	 * Contact's status
	 */
	enum OnlineStatus { Online, Away, Offline, Unknown };

	/**
	 * Return more fine-grained status.
	 * Online means at least one sub-contact is online, away means at least
	 * one is away, but nobody is online and offline speaks for itself
	 */
	OnlineStatus status() const;

	/**
	 * Like isOnline, but returns true even if the contact is not online, but
	 * can be reached trough offline-messages.
	 * FIXME: Here too, use preference order, not append order!
	 * FIXME: Here too an enum.
	 */
	bool isReachable() const;

	/**
	 * Get/set the display name
	 */
	QString displayName() const;
	void setDisplayName( const QString &name);
	
	/**
	 * The groups the contact is stored in
	 */
	KopeteGroupList groups() const;

	/**
	 * Return a XML representation of the metacontact
	 */
	QString toXML();

	/**
	 * Creates a metacontact from XML
	 * Return value of false indicated that
	 * creation failed and this contact should be
	 * discarded.
	 */
	bool fromXML( const QDomNode& cnode );

	/**
	 * Move a contact from one group to another.
	 */
	void moveToGroup( KopeteGroup *from, KopeteGroup *to );
    /**
	 * Remove a contact from one group
	 */
	void removeFromGroup( KopeteGroup *from);

	/**
	 * Add a contact to another group.
	 */
	void addToGroup( KopeteGroup *to );

	/**
	 * Temporary contacts will not be sarialized
	 */
	bool isTemporary() const;
	void setTemporary( bool b = true ,KopeteGroup *group=0l );

	/**
	 * When true, the meta-contact needs to be serialized
	 * and the previous serialize can't be used anymore
	 */
	bool isDirty() const;
	/**
	 * Plugins should set the metacontact to dirty
	 * as a save-me request
	 */
	void setDirty( bool b = true );

	/**
	 * Return true if the contact is shown at toplevel
	 */
	bool isTopLevel();

	/**
	 * add or remove from top-level
	 */
	void setTopLevel( bool b = true );

	/**
	 * remove the contact from this metacontact
	 * set 'deleted' to true if the KopeteContact is already deleted
	 */
	void removeContact(KopeteContact *c , bool deleted=false);

public slots:
	/**
	 * Contact another user.
	 * Depending on the config settings, call sendMessage() or
	 * startChat()
	 */
	void execute();

	/**
	 * Send a single message, classic ICQ style.
	 * The actual sending is done by the KopeteContact, but the meta contact
	 * does the GUI side of things.
	 * This is a slot to allow being called easily from e.g. a GUI.
	 */
	void sendMessage();

	/**
	 * Like sendMessage, but this time a full-blown chat will be opened.
	 * Most protocols can't distinguish between the two and are either
	 * completely session based like MSN or completely message based like
	 * ICQ the only true difference is the GUI shown to the user.
	 */
	void startChat();

	/**
	 * Get or set a field for the KDE address book backend. Fields not
	 * registered during the call to KopetePlugin::addressBookFields()
	 * cannot be altered!
	 */
	QString addressBookField( KopetePlugin *p, const QString &key ) const;
	void setAddressBookField( KopetePlugin *p, const QString &key,
		const QString &value );

	/**
	 * Return a copy of all address book fields exported by this
	 * meta contact
	 */
	AddressBookFields addressBookFields() const;

signals:
	/**
	 * The contact's online status changed
	 * Do *NOT* emit this signal directly, unless you also update the
	 * cache m_onlineStatus value! In all other case, just call
	 * updateOnlineStatus() instead.
	 */
	void onlineStatusChanged( KopeteMetaContact *contact,
		KopeteMetaContact::OnlineStatus status );

	/**
	 * The meta contact's display name changed
	 */
	void displayNameChanged( KopeteMetaContact *c, const QString &name );

	/**
	 * The contact was moved
	 */
	void movedToGroup( KopeteGroup *from, KopeteGroup *to, KopeteMetaContact *contact);

	/**
	 * The contact was removed from group
	 */
	void removedFromGroup(KopeteGroup *from, KopeteMetaContact *contact);

	/**
	 * The contact was added to another group
	 */
	void addedToGroup( KopeteGroup * to, KopeteMetaContact *contact);

	void contactAdded( KopeteContact *c );
	void contactRemoved( KopeteContact *c );

private slots:
	/**
	 * Update the contact's online status and emit onlineStatusChanged
	 * when appropriate
	 */
	void updateOnlineStatus();

	/**
	 * One of the child contact's online status changed
	 */
	void slotContactStatusChanged( KopeteContact *c,
		KopeteContact::ContactStatus s );

	/**
	 * One of the child contact's display names changed
	 * FIXME: Add a KopeteContact * to this method and the associated signal
	 *        in KopeteContact!
	 */
	void slotContactNameChanged( const QString &name );

	/**
	 * A child contact was deleted, remove it from the list, if it's still
	 * there
	 */
	void slotContactDestroyed( QObject *obj );

	/**
	 * If a plugin is loaded, maybe dada about this plugins are already cached in the metacontact
	 */
	void slotPluginLoaded(KopetePlugin *p);

private:
	QPtrList<KopeteContact> m_contacts;

	/**
	 * Display name as shown
	 */
	QString m_displayName;

	/**
	 * When true, track changes in child contact's display name and update
	 * the meta contact's display name too
	 */
	bool m_trackChildNameChanges;

	KopeteGroupList m_groups;
//	bool m_isTopLevel;

	/**
	 * Data to store in the XML file
	 */
	QMap<QString, QString> m_pluginData;
	AddressBookFields m_addressBook;

	bool m_temporary;

	bool m_dirty;

	OnlineStatus m_onlineStatus;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

