/*
    kopetecontact.h - Kopete Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ tiscalinet.be>

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

#ifndef __KOPETECONTACT_H__
#define __KOPETECONTACT_H__

#include <qobject.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <kurl.h>
#include <qvaluelist.h>

#include "kopetecontactproperty.h"

struct KopeteContactPrivate;

class KPopupMenu;
class KURL;
class KAction;

namespace Kopete
{

class Group;
class MetaContact;
class MessageManager;
class OnlineStatus;
class Plugin;
class Protocol;
class Account;
typedef QPtrList<Group> GroupList;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * This class abstracts a generic contact
 * Use it for inserting contacts in the contact list for example.
 */
class Contact : public QObject
{
	Q_OBJECT

	//Q_PROPERTY( QString displayName READ displayName )
	Q_PROPERTY( QString formattedName READ formattedName )
	Q_PROPERTY( QString formattedIdleTime READ formattedIdleTime )
	Q_PROPERTY( bool isOnline READ isOnline )
	Q_PROPERTY( bool isFileCapable READ isFileCapable )
	Q_PROPERTY( bool canAcceptFiles READ canAcceptFiles )
	Q_PROPERTY( QString contactId READ contactId )
	Q_PROPERTY( QString icon READ icon )
	Q_PROPERTY( QString toolTip READ toolTip )

public:
	/**
	 * \brief Create new contact.
	 *
	 * <b>The parent Kopete::MetaContact must not be NULL</b>
	 *
	 * \note id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @param account is the parent account. this constructor automatically register the contact to the account
	 * @param id is the Kopete::Contact's unique Id (mostly the user's login)
	 * @param parent is the parent @ref Kopete::MetaContact this Kopete::Contact is part of
	 * @param icon is an optional icon
	 */
	Contact( Account *account, const QString &id, MetaContact *parent,
		const QString &icon = QString::null );

	~Contact();

	/**
	 * \brief Get whether this contact is online
	 * @return true if the contact is online
	 * @return false if the contact is not online
	 */
	bool isOnline() const;

	/**
	 * \brief Get whether this contact can receive messages
	 *
	 * Used in determining if the contact is able to
	 * receive messages.  This function must be defined by child classes
	 *
	 * @return true if the contact can be reached
	 * @return false if the contact can not be reached
	 */
	// FIXME: After KDE 3.2 we should split this into a public, NON-virtual
	//        isReachable() accessor that checks for account->isConnected()
	//        and then calls a new virtual method that does the
	//        protocol-specific work, like 'doIsUnreachable' or so - Martijn
	virtual bool isReachable();

	/**
	 * \brief Get the metacontact for this contact
	 * @return The Kopete::MetaContact object for this contact
	 */
	MetaContact *metaContact() const;

	/**
	 * @brief Serialize the contact for storage in the contact list.
	 *
	 * The provided serializedData contain the contact id in the field
	 * "contactId". If you don't like this, or don't want to
	 * store these fields at all,
	 * you are free to remove them from the list.
	 *
	 * Most plugins don't need more than these fields, so they only need
	 * to set the address book fields themselves. If you have nothing to
	 * save at all you can clear the QMap, an empty map is treated as
	 * 'nothing to save'.
	 *
	 * The provided addressBookFields QMap contains the index field as
	 * marked with @ref Kopete::Plugin::addAddressBookField() with the
	 * contact id as value. If no index field is available the QMap is
	 * simply passed as an empty map.
	 *
	 * @sa Kopete::Protocol::deserializeContact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	/**
	 * @brief Serialize the contacts persistent properties for storage in the contact list.
	 *
	 * Does the same as @ref serialize() does but for KopeteContactProperties
	 * set in this contact with their persistency flag turned on.
	 * In contrary to @ref serialize() this does not need to be reimplemented.
	 *
	 */
	void serializeProperties(QMap<QString, QString> &serializedData);

	/**
	 * @brief Deserialize the contacts persistent properties
	 */
	void deserializeProperties(QMap<QString, QString> &serializedData);

	/**
	 * \brief Get the current display name
	 * @return The display name
	 * @deprecated  Use the nickname property instead
	 */
	QString displayName() const KDE_DEPRECATED;

	/**
	 * @brief Get the online status of the contact
	 * @return the online status of the contact
	 */
	const OnlineStatus& onlineStatus() const;

	/**
	 * \brief Set the contact's online status
	 */
	void setOnlineStatus(const OnlineStatus &status);

	/**
	 * \brief Get the unique id that identifies a contact.
	 *
	 * \note Id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return The unique id of the contact
	 */
	QString contactId() const;

	/**
	 * \brief Get the protocol that the contact belongs to.
	 *
	 * \note Id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return the contact's protocol
	 */
	Protocol* protocol() const;

	/**
	 * \brief Get the account that this contact belongs to
	 *
	 * @return the Kopete::Account object for this contact
	 */
	Account* account() const;

	/**
	 * \brief Get the set of custom menu items for this contact
	 *
	 * Returns a set of custom menu items for the context menu
	 * which is displayed in showContextMenu (private).  Protocols
	 * should use this to add protocol-specific actions to the
	 * popup menu. Kopete take care of the deletion of the action collection.
	 * Actions should have the collection as parent.
	 *
	 * @return Collection of menu items to be show on the context menu
	 */
	virtual QPtrList<KAction> *customContextMenuActions();
	virtual QPtrList<KAction> *customContextMenuActions( MessageManager *manager );

	/**
	 * @brief Get the Context Menu for this contact
	 *
	 * This menu includes generic actions common to each protocol, and action defined in
	 * @ref customContextMenuActions()
	 */
	KPopupMenu *popupMenu( MessageManager *manager = 0L );

	/**
	 * \brief Move this contact to a new MetaContact.
	 * This basically reparents the contact and updates the internal
	 * data structures.
	 * If the old contact is going to be empty, a question may ask to the user if it wants to delete the old contact.
	 *
	 * @param m The new MetaContact to move this contact to
	 */
	void setMetaContact(MetaContact *m);

	/**
	 * \brief Get whether or not this contact is capable of file transfers
	 *
	 *
	 * \see setFileCapable()
	 * \return true if the protocol for this contact is capable of file transfers
	 * \return false if the protocol for this contact is not capable of file transfers
	 */
	bool isFileCapable() const;

	/**
	 * \brief Set the file transfer capability of this contact
	 *
	 * \param filecap The new file transfer capability setting
	 */
	void setFileCapable( bool filecap );

	/**
	 * \brief Get whether or not this contact can accept file transfers
	 *
	 * This function checks to make sure that the contact is online as well as
	 * capable of sending files.
	 * \see isReachable()
	 * @return true if this contact is online and is capable of receiving files
	 * \return false if this contact is not capable of receiving files
	 */
	bool canAcceptFiles() const;

    /**
	 * @brief Rename a contact's display name.
	 *
	 * The default implementation calls @ref setDisplayName() immediately.
	 *
	 * @deprecated if you want to rename the alias on the server, use syncGroups()
	 */
	virtual void rename( const QString &newName ) KDE_DEPRECATED;


	/**
	 * Returns the primary message manager affiliated with this contact
	 * Although a contact can have more than one active message manager
	 * (as is the case with MSN at least), only one message manager will
	 * ever be the contacts "primary" message manager.. aka the 1 on 1 chat.
	 * This function should always return that instance.
	 *
	 * @param canCreate If a new message manager can be created in addition
	 * to any existing managers. Currently, this is only set to true when
	 * a chat is initiated by the user by clicking the contact list.
	 */
	virtual MessageManager * manager( bool canCreate = false ) =0;

	/**
	 * Returns the name of the icon to use for this contact
	 */
	virtual QString& icon() const;

	/**
	 * \brief Get the time (in seconds) this contact has been idle
	 * It will return the time set in @ref setIdleTime() with an addition of the time
	 * since you set this last time
	 * @return time this contact has been idle for, in seconds
	 */
	virtual unsigned long int idleTime() const;

	/**
	 * \brief Set the current idle time in seconds.
	 * Kopete will automatically calculate the time in @ref idleTime
	 * except if you set 0.
	 */
	void setIdleTime(unsigned long int);

	/**
	 * @return A QStringList containing all property keys
	 **/
	QStringList properties() const;

	/**
	 * Check for existance of a certain property stored
	 * using "key".
	 * \param key the property to check for
	 **/
	bool hasProperty(const QString &key) const;

	/**
	 * \brief Get the value of a property with key "key".
	 *
	 * If you don't know the type of the returned QVariant, you will need
	 * to check for it.
	 * \return the value of the property
	 **/
	const Kopete::ContactProperty &property(const QString &key) const;
	const Kopete::ContactProperty &property(const Kopete::ContactPropertyTmpl &tmpl) const;

	/**
	 * \brief Add or Set a property for this contact.
	 *
	 * @param tmpl The template this property is based on, key, label etc. are
	 * taken from this one
	 * @param value The value to store
	 *
	 * \note Setting a NULL value removes the property if it already existed.
	 * <b>don't</b> abuse this for property-removal, instead use
	 * @ref removeProperty() if you want to remove on purpose.
	 * Removal on NULL is to clean up the list of properties and to purge them
	 * from UI
	 **/
	void setProperty(const Kopete::ContactPropertyTmpl &tmpl, const QVariant &value);

	/**
	 * \brief Remove a property if it exists
	 *
	 * @param tmpl the template this property is based on
	 **/
	void removeProperty(const Kopete::ContactPropertyTmpl &tmpl);

	/**
	 * \brief Get the tooltip for this contact
	 * Makes use of formattedName() and formattedIdleTime().
	 * \return an RTF tooltip depending on KopetePrefs settings
	 **/
	QString toolTip() const;

	/**
	 * Returns a formatted string of "firstName" and/or "lastName" properties
	 * if present.
	 * Suitable for GUI display
	 **/
	virtual QString formattedName() const;

	/**
	 * Returns a formatted string of idleTime().
	 * Suitable for GUI display
	 **/
	QString formattedIdleTime() const;

public slots:
	/**
	 * This should typically pop up a KopeteChatWindow
	 */
	void startChat();

	/**
	 * Pops up an email type window
	 */
	void sendMessage();

	/**
	 * The user clicked on the contact, do the default action
	 */
	void execute();

	/**
	 * Changes the MetaContact that this contact is a part of.  This function
	 * is called by the KAction changeMetaContact that is part of the context
	 * menu.
	 */
	void slotChangeMetaContact();

	/**
	 * Method to delete a contact from the contact list,
	 * should be implemented by protocol plugin to handle
	 * protocol-specific actions required to delete a contact
	 * (ie. messages to the server, etc)
	 * the default implementation simply call deleteLater()
	 *
	 * @todo rename that function to "deleteContact"
	 */
	virtual void slotDeleteContact();

	/**
	 * Method to retrieve user information.  Should be implemented by
	 * the protocols, and popup some sort of dialog box
	 */
	virtual void slotUserInfo();

	/**
	 * This is the Kopete::Contact level slot for sending files. It should be
	 * implemented by all contacts which have the setFileCapable() flag set to
	 * true. If the function is called through the GUI, no parameters are sent
	 * and they take on default values (the file is chosen with a file open dialog)
	 *
	 * @param sourceURL The actual KURL of the file you are sending
	 * @param fileName (Optional) An alternate name for the file - what the
	 * receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending
	 * a nondeterminate
	 *                file size (such as over  asocket
	 */
	virtual void sendFile( const KURL &sourceURL = KURL(),
		const QString &fileName = QString::null, uint fileSize = 0L );

	/**
	 * @brief Syncronise the server and the metacontact.
	 * Protocols with server-side contact lists can implement this to
	 * sync the server groups with the metaContact groups. Or the server alias if any.
	 *
	 * This method is called every time the metacontact has been moved or renamed.
	 *
	 * default implementation does nothing
	 *
	 * @todo rename it to syncronise since it's not anymore only for groups.
	 */
	virtual void syncGroups();

	/**
	 * Change the icon to use for this contact
	 */
	void setIcon( const QString& icon );

protected:
	/**
	 * Sets the display name, for the contact.
	 * this is what is shown in the contact list.
	 * @param name Then new display name
	 * @deprecated use the nickname property
	 */
	void setDisplayName( const QString &name ) KDE_DEPRECATED;

private slots:

	/**
	 * This add the contact totally in the list if it was a temporary contact
	 */
	void slotAddContact();
	
	/**
	 * slot called when the action "delete" is called.
	 */
	void slotDelete();
	
	/**
	 * slot called when the action "block" is called.
	 */
	void slotBlock();
	
	/**
	 * slot called when the action "unblock" is called.
	 */
	void slotUnblock();

signals:
	/**
	 * The contact's online status changed
	 */
	void onlineStatusChanged( Kopete::Contact *contact,
		const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );

	/**
	 * The contact is about to be destroyed.
	 * Called when entering the destructor. Useful for cleanup, since
	 * metaContact() is still accessible at this point.
	 *
	 * @warning this signal is emit in the Kopete::Contact destructor, so all
	 * virtual method are not available
	 */
	void contactDestroyed( Kopete::Contact *contact );

	/**
	 * The contact's idle state changed.
	 * You need to emit this signal to update the view.
	 * That mean when activity has been noticed
	 */
	void idleStateChanged( Kopete::Contact *contact );

	/**
	 * One of the contact's properties has changed.
	 * @param contact this contact, useful for listening to signals from more than one contact
	 * @param key the key whose value has changed
	 * @param oldValue the value before the change, or an invalid QVariant if the property is new
	 * @param newValue the value after the change, or an invalid QVariant if the property was removed
	 */
	void propertyChanged( Kopete::Contact *contact, const QString &key,
		const QVariant &oldValue, const QVariant &newValue );

private:
	KopeteContactPrivate *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

