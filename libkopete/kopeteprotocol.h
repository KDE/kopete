/*
    kopeteprotocol.h - Kopete Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
class KopeteEditAccountWidget;
class KopeteMetaContact;
class KopeteAccount;

class KopeteProtocolPrivate;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Martijn Klingens       <klingens@kde.org>
 * @author Olivier Goffart        <ogoffart@tiscalinet.be>
 */
class KopeteProtocol : public KopetePlugin
{
	Q_OBJECT

public:
	/**
	 * @brief Available capabilities for the rich text widget.
	 *
	 * richTextCapabilities() returns an ORed list of these, which
	 * the edit widget interperts to determine what buttons to show
	 */
	enum RichTextCapabilities
	{
		BaseFgColor = 1,     // Setting the bg color of the whole edit widget / message
		BaseBgColor = 2,     // Setting the fg color of the whole edit widget / message
		RichFgColor = 4,       // Setting the fg/bg color of text portions individually
		RichBgColor = 8,       // Setting the fg/bg color of text portions individually

		BaseFont = 16,        // Setting the font of the whole edit widget / message
		RichFont = 32,       // Setting the font of text portions individually

		// Setting the formatting of the whole edit widget / message
		BaseUFormatting = 64,
		BaseIFormatting = 128,
		BaseBFormatting = 256,

		// Setting the formatting of text portions individually
		RichUFormatting = 512,
		RichIFormatting = 1024,
		RichBFormatting = 2048,

		Alignment = 4096,     // Setting the alignment of text portions

		// Setting the formatting of the whole edit widget / message
		BaseFormatting = BaseIFormatting | BaseUFormatting | BaseBFormatting,

		// Setting the formatting of text portions individually
		RichFormatting = RichIFormatting | RichUFormatting | RichBFormatting,

		RichColor = RichBgColor | RichFgColor,
		BaseColor = BaseBgColor | BaseFgColor,

		//Shortcut for All of the above - full HTML
		FullRTF =  65535
	};

	/**
	 * @brief Constructor for KopeteProtocol
	 *
	 * @param instance The protocol's instance, every plugin needs to have a @ref KInstance of its own
	 * @param parent The protocol's parent object
	 * @param name The protocol's name
	 */
	KopeteProtocol( KInstance *instance, QObject *parent, const char *name );
	virtual ~KopeteProtocol();

	/**
	 * @brief Create a new AddContactPage widget to be shown in the Add Contact Wizard.
	 *
	 * @return A new AddContactPage to be shown in the Add Contact Wizard
	 */
	virtual AddContactPage *createAddContactWidget( QWidget *parent, KopeteAccount *account ) = 0;

	/**
	 * @brief Create a new KopeteEditAccountWidget
	 *
	 * @return A new KopeteEditAccountWidget to be shown in the account part of the configurations.
	 *
	 * @param account is the KopeteAccount to edit. If it's 0L, then we create a new account
	 * @param parent The parent of the 'to be returned' widget
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( KopeteAccount *account, QWidget *parent ) = 0;

	/**
	 * @brief Create an empty KopeteAccount
	 *
	 * This method is called during the loading from the xml file
	 * @param accountId - the account ID to create the account with. This is usually
	 * the login name of the account
	 *
	 * @return The new @ref KopeteAccount object created by this function
	 */
	virtual KopeteAccount *createNewAccount( const QString &accountId ) = 0;

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
	virtual KActionMenu * protocolActions();


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
	 * meta contact yourself. @p serializedData and @p addressBookData will be
	 * the data the contact provided in KopeteContact::serialize.
	 *
	 * The default implementation does nothing.
	 *
	 * @return The contact created from the data
	 * @sa KopeteContact::serialize
	 */
	virtual KopeteContact *deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData,
		const QMap<QString, QString> &addressBookData );

	/**
	 * @brief Return if this protocol supports advanced rich text (HTML returned from chat widget)
	 */
	int richTextCapabilities() const;

	/**
	 * Reimplemented from KopetePlugin.
	 *
	 * This method disconnects all accounts and deletes them, after which it
	 * will emit readyForUnload.
	 *
	 * Note that this is an asynchronous operation that may take some time
	 * with active chats. It's no longer immediate as it used to be in
	 * Kopete 0.7.x and before. This also means that you can do a clean
	 * shutdown.
	 *
	 * WARNING: The method is not private to allow subclasses to reimplement
	 *          it even more, but if you need to do this please explain why
	 *          on the list first. It might make more sense to add another
	 *          virtual for protocols that's called instead, but for now I
	 *          actually think protocols don't need their own implementation
	 *          at all, so I left out the necessary hooks on purpose.
	 *          - Martijn
	 */
	virtual void aboutToUnload();

public slots:
	/**
	 * A meta contact is about to save.
	 * Call serialize() for all contained contacts for this protocol.
	 */
	void slotMetaContactAboutToSave( KopeteMetaContact *metaContact );

protected:
	/**
	 * @brief Sets the RTF capabilities of this protcol.
	 *
	 * Should be called from subclass constructor.
	 */
	void setRichTextCapabilities( int );

private slots:
	/**
	 * The account changed online status. Used while unloading the protocol.
	 */
	void slotAccountOnlineStatusChanged( KopeteContact *self, const KopeteOnlineStatus &newStatus, const KopeteOnlineStatus &oldStatus );

	/**
	 * The account is destroyed. When it's the last account we emit the
	 * readyForUnload signal. Used while unloading the protocol.
	 */
	void slotAccountDestroyed( QObject *account );

private:
	KopeteProtocolPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

