/*
    kopeteprotocol.h - Kopete Protocol

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPROTOCOL_H
#define KOPETEPROTOCOL_H

#include "kopeteplugin.h"
//FIXME - How can I avoid this?
#include "kopeteonlinestatus.h"

#include <qdict.h>

class KActionMenu;

class AddContactPage;
class KopeteContact;
class KopeteMetaContact;
class EditAccountWidget;
class KopeteAccount;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Martijn Klingens   <klingens@kde.org>
 * @author Olivier Goffart  <ogoffart@tiscalinet.be>
 */
class KopeteProtocol : public KopetePlugin
{
	Q_OBJECT

public:
	KopeteProtocol( QObject *parent = 0L, const char *name = 0L );
	virtual ~KopeteProtocol();

	/**
	 * return a new AddContact widget showed in the addContactWizzard.
	 * parent is the parent widget.
	 * the account is given
	 */
	virtual AddContactPage *createAddContactWidget(QWidget *parent, KopeteAccount* account) =0L;

	/**
	 * return a new EditAccount widget showed in the account part of the configurations
	 * account is the KopeteAccount to edit, if it is egal to 0l, then, we are creating a
	 * new account
	 */
	virtual EditAccountWidget *createEditAccountWidget( KopeteAccount * account, QWidget * parent ) =0L;


	/**
	 * create a new empty KopeteAccount with the id accountId
	 * this method is called during the loading from the xml file
	 */
	virtual KopeteAccount *createNewAccount( const QString &  accountId  ) =0L;

	/**
	 * Return the most significant status of the protocol's
	 * accounts.  Useful for aggregating status information.
	 */
	KopeteOnlineStatus status() const;

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
	 * Deserialize plugin data for a meta contact. This method splits up the
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
	 * A meta contact is about to save.
	 * Call serialize() for all contained contacts for this protocol.
	 */
	void slotMetaContactAboutToSave( KopeteMetaContact *metaContact );
	
	/**
	 * @internal
	 * kopeteAccount call this slot when accounts created or deleted
	 */
	void refreshAccounts();

signals:
	/**
	 * The status icon changed.
	 * This signal is only emitted if the new icon is different from
	 * the previous icon.
	 */
	void statusIconChanged( const KopeteOnlineStatus& );

private slots:
	/**
	 * Update the overall protocol status, called in
	 * response to account status changes
	 */
	void slotRefreshStatus();

protected:
	KopeteOnlineStatus m_status;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

