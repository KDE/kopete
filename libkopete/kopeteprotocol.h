/*
    kopeteprotocol.h - Kopete Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QtCore/QFlags>

#include "kopeteplugin.h"
#include "kopeteonlinestatus.h"

class KopeteEditAccountWidget;
class AddContactPage;
class KJob;

#include "kopete_export.h"

namespace Kopete
{

class Contact;
class MetaContact;
class Account;

/*namespace UI
{
	class EditAccountWidget;
	class AddContactPage;
}*/


/**
 * @brief base class of every protocol.
 *
 * A protocol is just a particular case of Plugin
 *
 * Protocol is an abstract class, you need to reimplement createNewAccount,
 * createAddContactPage, createEditAccountWidget
 *
 *
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Martijn Klingens       <klingens@kde.org>
 * @author Olivier Goffart        <ogoffart\@kde.org>
 */
class KOPETE_EXPORT Protocol : public Plugin
{
	Q_OBJECT

public:

	/**
	 * @todo  Ideally, the destructor should be protected. but we need it public to allow QPtrList<Protocol>
	 */
	virtual ~Protocol();

	/**
	 * @brief Create an empty Account
	 *
	 * This method is called during the loading of the config file.
	 * @param accountId - the account ID to create the account with. This is usually
	 * the login name of the account
	 *
	 * you don't need to register the account to the AccountManager in this function.
	 * But if you want to use this function don't forget to call  @ref AccountManager::registerAccount
	 *
	 * @return The new @ref Account object created by this function
	 */
	virtual Account *createNewAccount( const QString &accountId ) = 0;

	/**
	 * @brief Create a new AddContactPage widget to be shown in the Add Contact Wizard.
	 *
	 * @return A new AddContactPage to be shown in the Add Contact Wizard
	 */
	virtual AddContactPage *createAddContactWidget( QWidget *parent, Account *account ) = 0;

	/**
	 * @brief Create a new KopeteEditAccountWidget
	 *
	 * @return A new KopeteEditAccountWidget to be shown in the account part of the configurations.
	 *
	 * @param account is the KopeteAccount to edit. If it's 0L, then we create a new account
	 * @param parent The parent of the 'to be returned' widget
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Account *account, QWidget *parent ) = 0;

	/**
	 * @brief Available capabilities
	 *
	 * @ref capabilities() returns an ORed list of these, which
	 * the edit widget interperts to determine what buttons to show
	 */
	enum Capability
	{
		BaseFgColor = 0x1,     ///< Setting the bg color of the whole edit widget / message
		BaseBgColor = 0x2,     ///< Setting the fg color of the whole edit widget / message
		RichFgColor = 0x4,       ///< Setting the fg/bg color of text portions individually
		RichBgColor = 0x8,       ///< Setting the fg/bg color of text portions individually

		BaseFont = 0x10,        ///< Setting the font of the whole edit widget / message
		RichFont = 0x20,       ///< Setting the font of text portions individually

		/// Setting the formatting of the whole edit widget / message
		BaseUFormatting = 0x40,
		BaseIFormatting = 0x80,
		BaseBFormatting = 0x100,

		/// Setting the formatting of text portions individually
		RichUFormatting = 0x200,
		RichIFormatting = 0x400,
		RichBFormatting = 0x800,

		Alignment = 0x1000,     ///< Setting the alignment of text portions

		/// Setting the formatting of the whole edit widget / message
		BaseFormatting = BaseIFormatting | BaseUFormatting | BaseBFormatting,

		/// Setting the formatting of text portions individually
		RichFormatting = RichIFormatting | RichUFormatting | RichBFormatting,

		RichColor = RichBgColor | RichFgColor,
		BaseColor = BaseBgColor | BaseFgColor,

		//Shortcut for All of the above - full HTML
		FullRTF =  RichFormatting | Alignment | RichFont | RichFgColor | RichBgColor ,


		CanSendOffline = 0x10000 ///< If it's possible to send  offline messages
	};
	Q_DECLARE_FLAGS(Capabilities, Capability)

	/**
	 * @brief a bitmask of the capabilities of this protocol
	 * @sa @ref setCapabilities
	 */
	Capabilities capabilities() const;

	/**
	 * @brief true if account can add own contact into a contact list
	 */
	bool canAddMyself() const;
	
	/**
	 * @brief Returns the status used for contacts when accounts of this protocol are offline
	 */
	Kopete::OnlineStatus accountOfflineStatus() const;


protected:
	/**
	 * @brief Constructor for Protocol
	 *
	 * @param instance The protocol's instance, every plugin needs to have a KComponentData of its own
	 * @param parent The protocol's parent object
	 * @param name The protocol's name
	 */
	Protocol( const KComponentData &instance, QObject *parent, bool canAddMyself = false );

	/**
	 * @brief Sets the capabilities of this protcol.
	 *
	 * The subclass contructor is a good place for calling it.
	 * @sa @ref capabilities()
	 */
	void setCapabilities( Capabilities );

public:

	/**
	 * Reimplemented from Kopete::Plugin.
	 *
	 * This method disconnects all accounts and deletes them, after which it
	 * will emit readyForUnload.
	 *
	 * Note that this is an asynchronous operation that may take some time
	 * with active chats. It's no longer immediate as it used to be in
	 * Kopete 0.7.x and before. This also means that you can do a clean
	 * shutdown.
 	 * @note    The method is not private to allow subclasses to reimplement
	 *          it even more, but if you need to do this please explain why
	 *          on the list first. It might make more sense to add another
	 *          virtual for protocols that's called instead, but for now I
	 *          actually think protocols don't need their own implementation
	 *          at all, so I left out the necessary hooks on purpose.
	 *          - Martijn
	 */
	virtual void aboutToUnload();

private slots:
	/**
	 * @internal
	 * The account changed online status. Used while unloading the protocol.
	 */
	void slotAccountOnlineStatusChanged( Kopete::Contact *self );

	/**
	 * @internal
	 * The account is destroyed. When it's the last account we emit the
	 * readyForUnload signal. Used while unloading the protocol.
	 */
	void slotAccountDestroyed(  );


public:

	/**
	 * @brief Deserialize the plugin data for a meta contact.
	 *
	 * This method splits up the data into the independent Kopete::Contact objects
	 * and calls @ref deserializeContact() for each contact.
	 *
	 * Note that you can still reimplement this method if you prefer, but you are
	 * strongly recommended to use this version of the method instead, unless you
	 * want to do _VERY_ special things with the data...
	 *
	 * @todo we probably should think to another way to save the contacltist.
	 */
	virtual void deserialize( MetaContact *metaContact, const QMap<QString, QString> &serializedData );

	/**
	 * @brief Deserialize the plugin data for a meta contact's contacts.
	 */
	virtual void deserializeContactList( MetaContact *metaContact, const QList< QMap<QString, QString> > &dataList );

	/**
	 * @brief Deserialize a single contact.
	 *
	 * This method is called by @ref deserialize() for each separate contact,
	 * so you don't need to add your own hooks for multiple contacts in a single
	 * meta contact yourself. @p serializedData and @p addressBookData will be
	 * the data the contact provided in Kopete::Contact::serialize.
	 *
	 * The default implementation does nothing.
	 *
	 * @return The contact created from the data
	 * @sa Contact::serialize
	 *
	 * @todo we probably should think to another way to save the contacltist.
	 */
	virtual Contact *deserializeContact( MetaContact *metaContact,
		const QMap<QString, QString> &serializedData,
		const QMap<QString, QString> &addressBookData );

	/**
	 * @brief Factory method to create a protocol Task
	 *
	 * Protocols Task are tasks needed for executing specific
	 * commands for the given protocol. This method creates the
	 * required task for the given task type.
	 *
	 * If a task type is not available in the protocol, just return
	 * a null pointer, like the default implementation.
	 *
	 * Example of a implementation of createProtocolTask()
	 * @code
KJob* JabberProtocol::createProtocolTask(const QString &taskType)
{
	if( taskType == QLatin1String("DeleteContactTask") )
	{
		return new JabberDeleteContactTask();
	}
	if( taskType == QLatin1String("AddContactTask") )
	{
		return new JabberAddContactTask();
	}

	return 0;
}
	 * @endcode
	 *
	 * @param taskType a task type as a string. Check each task for name.
	 */
	virtual KJob *createProtocolTask(const QString &taskType);

	/**
	 * Check whether a password is valid for this protocol.  The default implementation
	 * validates every password
	 * @param password The password to check
	 */
	 virtual bool validatePassword( const QString & password ) const;

public:
	/**
	 * Serialize meta contact into the metacontact's plugin data
	 * Call serialize() for all contained contacts for this protocol.
	 * @internal
	 *   it's public because for example, Contact::setMetaContact uses it.
	 * @todo we probably should think to another way to save the contacltist.
	 */
	void serialize( Kopete::MetaContact *metaContact );


private:
	class Private;
	Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Protocol::Capabilities)

} //END namespace kopete

#endif

