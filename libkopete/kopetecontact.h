/*
    kopetecontact.h - Kopete Contact

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

#ifndef __KOPETECONTACT_H__
#define __KOPETECONTACT_H__

#include <qobject.h>
#include <qstringlist.h>

#include <kurl.h>

struct KopeteContactPrivate;

class KActionCollection;
class KPopupMenu;
class KURL;

class KopeteGroup;
class KopeteMetaContact;
class KopeteMessageManager;
class KopeteOnlineStatus;
class KopetePlugin;
class KopeteProtocol;
class KopeteAccount;
typedef QPtrList<KopeteGroup> KopeteGroupList;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * This class abstracts a generic contact/buddie.
 * Use it for inserting contacts in the contact list for example.
 */
class KopeteContact : public QObject
{
	Q_OBJECT

public:
	/**
	 * Create new contact. Supply the parent meta contact!
	 */
	KopeteContact( KopeteAccount *account, const QString &id, KopeteMetaContact *parent, const QString &icon = QString::null );
	~KopeteContact();

	/**
	 * Contact's idle status
	 */
	enum IdleState { Unspecified, Idle, Active };

	/**
	 * Return whether this contact is online or not.
	 * @return bool indicating whhether the contact is online
	 */
	bool isOnline() const;

	/**
	 * Function used in determining if the contact is able to
	 * recieve messages even if offline, etc.  This function must
	 * be defined by child classes
	 *
	 * @return bool indicating whether or not the contact is reachable (can send a message to it)
	 */
	virtual bool isReachable();

	/**
	 * Accessor function for the contact's MetaContact
	 * @return The contact's metacontact
	 */
	KopeteMetaContact *metaContact() const;

	/**
	 * Serialize the contact for storage in the contact list.
	 *
	 * The provided serializedData contain the contact id in the field
	 * "contactId" and the display name in the field "displayName". If
	 * you don't like this, or don't want to store these fields at all,
	 * you are free to remove them from the list.
	 *
	 * Most plugins don't need more than these fields, so they only need
	 * to set the address book fields themselves. If you have nothing to
	 * save at all you can clear the QMap, an empty map is treated as
	 * 'nothing to save'.
	 *
	 * The provided addressBookFields QMap contains the index field as
	 * marked with @ref KopetePlugin::addAddressBookField() with the
	 * contact id as value. If no index field is available the QMap is
	 * simply passed as an empty map.
	 */
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	/**
	 * Get the current display name
	 * @return The display name
	 */
	QString displayName() const;

	/**
	 * Return the online status of the contact
	 * @return the online status of the contact
	 */
	const KopeteOnlineStatus& onlineStatus() const;

	/**
	 * Set the contact's online status
	 */
	void setOnlineStatus( const KopeteOnlineStatus &status );

	/**
	 * Return the unique id that identifies a contact. Id is required
	 * to be unique per protocol and per account. Across those boundaries
	 * ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return The unique id of the contact
	 */
	QString contactId() const;

	/**
	 * Return the protocol that the contact belongs to.
	 *
	 * Note: Id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return the contact's protocol
	 */
	KopeteProtocol* protocol() const;

	/**
	 * Return the account that the contact belongs to.
	 *
	 * Note: Id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return the contact's account
	 */
	KopeteAccount* account() const;

	/**
	 * Returns a set of custom menu items for the context menu
	 * which is displayed in showContextMenu (private).  Protocols
	 * should use this to add protocol-specific actions to the
	 * popup menu
	 *
	 * @return Collection of menu items to be show on the context menu
	 */
	virtual KActionCollection *customContextMenuActions();


	/**
	 * Get the Context Menu for this contact
	 */
	KPopupMenu *popupMenu();

	/**
	 * Moves this contact to a new MetaContact.
	 * This basically reparents the contact and updates the internal
	 * data structures.
	 *
	 * @param m The new MetaContact to move this contact to
	 */
	void setMetaContact(KopeteMetaContact *m);

	/**
	 * Returns whether or not this contact is capable of file transfers or not
	 */
	bool isFileCapable() const;

	/*
	 * Sets the capability of file transfers for this user. Once this is changed it will
	 * immediately add a new menu entry called "Send File...", and will call the
	 * virtual slotSendFile
	 */
	void setFileCapable( bool filecap );

	/*
	 * Returns if this contact can accept file transfers
	 * @return True if this contact is online and is capable of sending files, False otherwise
	 */
	bool canAcceptFiles() const;

	/**
	 * Accessor function for the idle state of the contact
	 * @return Idle state of the contact
	 */
	IdleState idleState() const;

	/**
	 * Sets the idle state to newState
	 * If this function is not called by a plugin, the idle state is set to Unknown
	 */
	void setIdleState( KopeteContact::IdleState newState );

	/**
	 * Rename a contact's display name.
	 * This method can be asynchronous, i.e. it starts the rename, but the
	 * result may not be instant. Whenever the rename is done the contact
	 * will call @ref setDisplayName() (which emits @ref displayNameChanged() )
	 * to confirm the change.
	 *
	 * The default implementation calls @ref setDisplayName() immediately.
	 */
	virtual void rename( const QString &newName );

	/**
	 * Returns the primary message manager affiliated with this contact
	 * Although a contact can have more than one active message manager
	 * (as is the case with MSN at least), only one message manager will
	 * ever be the contacts "primary" message manager.. aka the 1 on 1 chat.
	 * This function should always returnt that instance.
	 *
	 * @param canCreate If a new message manager can be created in addition
	 * to any existing managers. Current;y, this is only set to true when
	 * a chat is initiated by the user by clicking the contact list.
	 */
	virtual KopeteMessageManager * manager( bool canCreate = false ) =0;

/*  //this is useless, that caused crash when closing the kmm when the contact was deleted (ex: on exit)
	const int conversations() const;
	void setConversations( int ) const;*/

	/**
	 * Returns the name of the icon to use for this contact
	 */
	virtual QString& icon() const;


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
	 */
	virtual void slotDeleteContact();

	/**
	 * Method to retrieve user information.  Should be implemented by
	 * the protocols, and popup some sort of dialog box
	 */
	virtual void slotUserInfo();

	/**
	 * This is the KopeteContact level slot for sending files. It should be implemented by all
	 * contacts which have the setFileCapable() flag set to true. If the function is called through
	 * the GUI, no parameters are sent and they take on default values (the file is chosen with
	 * a file open dialog)
	 *
	 * @param sourceURL The actual KURL of the file you are sending
	 * @param fileName (Optional) An alternate name for the file - what the reciever will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending a nondeterminate
	 *                file size (such as over  asocket
	 */
	virtual void sendFile( const KURL &sourceURL = KURL(), const QString &fileName = QString::null, uint fileSize = 0L );

	/**
	 * Protocols with server-side contact lists can implement this to
	 * sync the server groups with the metaContact groups.
	 * This method is called everytime the metacontact has been moved
	 *
	 * default implementation do nothing
	 */
	virtual void syncGroups();

	/**
	 * Changet the icon to use for this contact
	 */
	void setIcon( const QString& icon );

protected:
	/**
	 * Sets the display name, or alias, for the contact.
	 * this is what is shown in the contact list.
	 * @param name Then new display name
	 */
	void setDisplayName( const QString &name );

private slots:
	/**
	 * Function that is called when "Change Alias" is chosen from the
	 * context menu.  Presents dialog box to change alias, and calls
	 * setDisplayName with the returned value
	 */
	void slotChangeDisplayName();

	/**
	 * This add the contact totaly in the list if it was a temporary contact
	 */
	void slotAddContact();

signals:
	/**
	 * The contact's online status changed
	 */
	void onlineStatusChanged( KopeteContact *contact, const KopeteOnlineStatus &status, const KopeteOnlineStatus &oldStatus );

	/**
	 * Connect to this signal to know when the contact
	 * changed its name/nick
	 */
	void displayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * The contact is about to be destroyed.
	 * Called when entering the destructor. Useful for cleanup, since
	 * metaContact() is still accessible at this point.
	 */
	void contactDestroyed( KopeteContact *contact );

	/**
	 * The contact's idle state changed
	 */
	void idleStateChanged( KopeteContact *contact, KopeteContact::IdleState newState );

private:
	KopeteContactPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

