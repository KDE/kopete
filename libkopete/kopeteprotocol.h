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
	KopeteProtocol( KInstance *instance, QObject *parent, const char *name );
	virtual ~KopeteProtocol();

	/**
	 * @brief Create a new AddContactPage widget to be shown in the Add Contact Wizard.
	 *
	 * @return A new AddContactPage to be shown in the Add Contact Wizard
	 */
	virtual AddContactPage *createAddContactWidget(QWidget *parent, KopeteAccount* account) =0L;

	/**
	 * @brief Create a new EditAccountWidget
	 *
	 * @return A new EditAccountWidget to be shown in the account part of the configurations.
	 *
	 * @param account is the KopeteAccount to edit. If it's 0L, then we create a new account
	 */
	virtual EditAccountWidget *createEditAccountWidget( KopeteAccount * account, QWidget * parent ) =0L;


	/**
	 * @brief Create an empty KopeteAccount
	 *
	 * This method is called during the loading from the xml file
	 * @param accountId - the account ID to create the account with. This is usually
	 * the login name of the account
	 *
	 * @return The new @ref KopeteAccount object created by this function
	 */
	virtual KopeteAccount *createNewAccount( const QString &  accountId  ) =0L;

	/**
	 * @brief Get the most significant status of the protocol's accounts.
	 *
	 * Useful for aggregating status information.
	 */
	KopeteOnlineStatus status() const;

	/**
	 * @brief Determine whether the protocol supports offline messages.
	 *
	 * @todo Make pure virtual, or define protected method
	 * setOfflineCapable(), instead of default implementation always
	 * returning false.
	 */
	bool canSendOffline() const { return false; }

	/**
	 * @brief Get the KActionMenu for the custom protocol actions
	 *
	 * Use this function for creating a custom menu to plug in to the system tray
	 * icon or the protocol icon in the status bar.
	 *
	 * @return The KActionMenu object created by this function
	 */
	virtual KActionMenu* protocolActions();


	/**
	 * @brief Deserialize the plugin data for a meta contact.
	 *
	 * This method splits up the data into the independent KopeteContact objects
	 * and calls @ref deserializeContact() for each contact.
	 *
	 * Note that you can still reimplement this method if you prefer, but you are
	 * strongly recommended to use this version of the method instead, unless you
	 * want to do _VERY_ special things with the data...
	 */
	virtual void deserialize( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData );

	/**
	 * @brief Deserialize a single contact.
	 *
	 * This method is called by @ref deserialize() for each separate contact,
	 * so you don't need to add your own hooks for multiple contacts in a single
	 * meta contact yourself.
	 * The default implementation does nothing.
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
		const QMap<QString, QString> &addressBookData );
		
	/**
	 * Return if this protocol supports advanced rich text (HTML returned from chat widget)
	 */
	 virtual bool supportsRichText() const;

public slots:
	/**
	 * A meta contact is about to save.
	 * Call serialize() for all contained contacts for this protocol.
	 */
	void slotMetaContactAboutToSave( KopeteMetaContact *metaContact );

	/**
	 * @internal
	 * KopeteAccount will call this slot when accounts are created or deleted
	 */
	void refreshAccounts();

signals:
	/**
	 * @brief Signal the status icon changed.
	 *
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

