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
class EditAccountWidget;
class KopeteAccount;

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

	/**
	 * return a new AddContact widget showed in the addContactWizzard.
	 * parent is the parent widget.
	 * the account is given 
	 * TODO: make pure virtual
	 */	
	virtual AddContactPage *createAddContactWidget(QWidget *parent, KopeteAccount* /*account*/)
	{
		return createAddContactWidget(parent);
	}
	/** obsolete one **/
	virtual AddContactPage *createAddContactWidget(QWidget */*parent*/)
	{
		return 0L;
	}

	/**
	 * return a new EditAccount widget showed in the account part of the configurations
	 * account is the KopeteAccount to edit, if it is egal to 0l, then, we are creating a 
	 * new account
	 */
	virtual EditAccountWidget *createEditAccountWidget( KopeteAccount * /*account*/, QWidget * /*parent*/ )
		{ return 0L; }  //TODO: make this pure virtual
		
	/**
	 * create a new empty KopeteAccount with the id accountId
	 * this method is called durring the loading from the xml file
	 */
	virtual KopeteAccount *createNewAccount( const QString & /* accountId */ )
		{ return 0L; }  //TODO: make this pure virtual

	/**
	 * The icon of the plugin as shown in the status bar. .mng animations
	 * are supported too, and tried first
	 */
	QString statusIcon() const;

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
	 */
	virtual KActionMenu* protocolActions();

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

	/** OBSOLETE **/
	virtual KopeteContact* myself() const;
	bool isConnected() const;
	virtual void setAway() {}
	virtual void setAvailable() {}
	bool isAway() const;
	/** OBSOLETE **/
	void setStatusIcon( const QString &icon );

	/**
	 * @internal
	 * Register a new KopeteContact with the protocol.
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
	 * (and by KopeteAccount::registerContact )
	 */
	void registerContact( KopeteContact *c );

public slots:
	/** OBSOLETE **/	
	virtual void connect() {};
	virtual void disconnect() {};
	bool addContact( const QString &contactId, const QString &displayName = QString::null,
		KopeteMetaContact *parentContact = 0L, const QString &groupName = QString::null, bool isTemporary = false);

	/**
	 * A meta contact is about to save.
	 * Call serialize() for all contained contacts for this protocol.
	 */
	void slotMetaContactAboutToSave( KopeteMetaContact *metaContact );

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

	void slotRefreshStatusIcon();
	void slotAccountAdded();

private:
	QString m_statusIcon;

	/**
	 * The list of all contacts for this protocol
	 */
	QDict<KopeteContact> m_contacts;
	
	KActionMenu *m_menu;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

