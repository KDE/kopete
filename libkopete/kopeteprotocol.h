/*
    kopeteprotocol.h - Kopete Protocol

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

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

#ifndef KOPETEPROTOCOL_H
#define KOPETEPROTOCOL_H

#include "kopeteplugin.h"

#include <qdict.h>

class KActionMenu;

class AddContactPage;
class KopeteContact;
class KopeteMetaContact;
class EditIdentityWidget;
class KopeteIdentity;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteProtocol : public KopetePlugin
{
	Q_OBJECT

public:
	KopeteProtocol( QObject *parent = 0L, const char *name = 0L );
	virtual ~KopeteProtocol();

	/**
	 * Protocol API. Must be reimplemented
	 */
	virtual bool unload();
	virtual bool isConnected() const = 0;

	// this will be called if main-kopete wants
	// the plugin to set the user's mode to away or something similar
	virtual void setAway() = 0;
	// this will be called if main-kopete wants
	// the plugin to set the user's mode to online or something similar
	virtual void setAvailable() = 0;
	// plugin has to return wether it is away or not
	// plugins should also return TRUE for modes like occupied not-vailable etc.
	virtual bool isAway() const = 0;

	virtual const QString protocolIcon() = 0;
	
	virtual AddContactPage *createAddContactWidget(QWidget *parent)=0;
	
	/**
	 * return a new EditIdentity widget showed in the identity part of the configurations
	 * identity is the KopeteIdentity to edit, if it is egal to 0l, then, we are creating a 
	 * new identity
	 */
	virtual EditIdentityWidget *createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent)
		{ return 0L; }  //TODO: make this pure virtual

	/**
	 * The icon of the plugin as shown in the status bar. .mng animations
	 * are supported too, and tried first
	 */
	QString statusIcon() const;
	void setStatusIcon( const QString &icon );

	/**
	 * Return whether the protocol supports offline messages.
	 * FIXME: Make pure virtual, or define protected method
	 *        setOfflineCapable(), instead of default implementation always
	 *        returning false.
	 */
	bool canSendOffline() const { return false; }

	/**
	 * Return a KActionMenu using a custom menu to plug into e.g. the system
	 * tray icon and the protocol icon in the status bar, but maybe elsewhere
	 * too.
	 * The default implementation returns a null pointer, to disable any menu.
	 *
	 * Note that you are responsible for allocating and deleting the
	 * KActionMenu yourself (as far as Qt's API doesn't do it for you ).
	 */
	virtual KActionMenu* protocolActions();

	/**
	 * Function has to be reimplemented in every single protocol
	 * and return the KopeteContact associated with the 'home' user.
	 *
	 * @return contact associated with the currently logged in user
	 */
	virtual KopeteContact* myself() const = 0;

	/**
	 * Retrieve the list of contacts for this protocol
	 *
	 * The list is guaranteed to contain only contacts for this protocol,
	 * so you can safely use static_cast to your own derived contact class
	 * if needed.
	 */
	const QDict<KopeteContact>& contacts();

	/**
	 * Retrieve the list of contacts for this protocol and the given meta
	 * contact.
	 *
	 * The list is guaranteed to contain only contacts for this protocol,
	 * and only for the specified meta contact, so you can safely use
	 * static_cast to your own derived contact class if needed.
	 *
	 * Note that unlike the void method @ref contacts() this method doesn't
	 * returns a reference to the dictionary, because the protocol has no
	 * internal data structure to reference. This makes the method slower
	 * than @ref contacts(). Generally you don't need to use this method
	 * very often, so it shouldn't really matter in practice.
	 */
	QDict<KopeteContact> contacts( KopeteMetaContact *mc );

	/**
	 * Adds a contact to an existing MetaContact. Also performs any server-related
	 * functions. *MUST* be implemented in each protocol
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact );

	/**
	 * Deserialize plugin-data for a meta contact. This method splits up the
	 * data into the independent KopeteContacts and calls @ref deserializeContact()
	 * for each contact.
	 *
	 * Note that you can still reimplement this method if you prefer, but you are
	 * strongly recommended to use this version of the method instead, unless you
	 * want to do _VERY_ special things with the data...
	 */
	virtual void deserialize( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData );

	/**
	 * Deserialize a single contact.
	 * This method is called by @ref deserialize() for each separate contact,
	 * so you don't need to add your own hooks for multiple contacts in a single
	 * meta contact yourself.
	 * The default implementation does nothing.
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
		const QMap<QString, QString> &addressBookData );

public slots:
	/**
	 * Go online for this service.
	 * This is a slot, so it can be called directly from e.g. a KAction.
	 */
	virtual void connect() = 0;

	/**
	 * Disconnect from this service.
	 * This is a slot, so it can be called directly from e.g. a KAction.
	 */
	virtual void disconnect() = 0;
	
	/**
	 * Adds a contact to this protocol with the specified details
	 * 
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 * @param groupName The name of the group to add the contact to
	 * @param isTemporary If this is a temporary contact
	 * @return Pointer to the KopeteContact object which was added
	 */
	bool addContact( const QString &contactId, const QString &displayName = QString::null,
		KopeteMetaContact *parentContact = 0L, const QString &groupName = QString::null, bool isTemporary = false);

signals:
	/**
	 * The status icon changed. See also @ref setStatusIcon().
	 * This signal is only emitted if the new icon is different from
	 * the previous icon.
	 */
	void statusIconChanged( KopeteProtocol *protocol, const QString &icon );

private slots:
	/**
	 * Track the deletion of a KopeteContact and cleanup
	 */
	void slotKopeteContactDestroyed( KopeteContact * );

	/**
	 * A meta contact is about to save.
	 * Call serialize() for all contained contacts for this protocol.
	 */
	void slotMetaContactAboutToSave( KopeteMetaContact *metaContact );

private:
	/**
	 * KopeteContact needs to access @ref registerContact(), so it is a
	 * friend of KopeteProtocol. Please do _NOT_ use this friendship to
	 * access other members without documenting them here!
	 */
	friend class KopeteContact;

	/**
	 * @internal
	 * Register a new KopeteContact with the protocol.
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
	 */
	void registerContact( KopeteContact *c );

	QString m_statusIcon;

	/**
	 * The list of all contacts for this protocol
	 */
	QDict<KopeteContact> m_contacts;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

