/*
    kopetecontact.h - Kopete Contact

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>

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

#ifndef KOPETECONTACT_H
#define KOPETECONTACT_H

#include "kopetecontactlistelement.h"
#include "kopetepropertycontainer.h"

#include <kurl.h>
#include <kdemacros.h>
#include <ktoggleaction.h>
#include "kopeteglobal.h"

#include "kopete_export.h"

class KMenu;
class KAction;

namespace Kopete
{

class Group;
class MetaContact;
class ChatSession;
class OnlineStatus;
class Plugin;
class Protocol;
class Account;
class StatusMessage;

typedef QList<Group *> GroupList;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 *
 * This class abstracts a generic contact
 * Use it for inserting contacts in the contact list for example.
 */
class KOPETE_EXPORT Contact
	: public ContactListElement
{
	Q_OBJECT

	Q_ENUMS( CanCreateFlags )
	Q_PROPERTY( QString formattedName READ formattedName )
	Q_PROPERTY( QString formattedIdleTime READ formattedIdleTime )
	Q_PROPERTY( bool isOnline READ isOnline )
	Q_PROPERTY( bool fileCapable READ isFileCapable WRITE setFileCapable )
	Q_PROPERTY( bool canAcceptFiles READ canAcceptFiles )
	//Q_PROPERTY( bool isReachable READ isReachable )
	Q_PROPERTY( QString contactId READ contactId )
	Q_PROPERTY( QString icon READ icon WRITE setIcon )
	Q_PROPERTY( QString toolTip READ toolTip )
	Q_PROPERTY( QString nickName READ nickName WRITE setNickName )
	//Q_PROPERTY( unsigned long idleTime READ idleTime WRITE setIdleTime )

public:	
	/**
	 * used in @ref sync()
	 */
	enum Changed{ MovedBetweenGroup = 0x01, ///< the contact has been moved between groups
		      DisplayNameChanged = 0x02 ///< the displayname of the contact changed
	};

	enum NameType {
		NickName,	///< Nick name, comes from contact
		CustomName,	///< Custom name set by user and stored on server contact list, can be changed
		FormattedName,	///< Formatted name (first and/or last name)
		ContactId,	///< Contact id, will never change
	};

	/**
	 * These functions do conversion between enum NameType and QString
	 * Usefull for protocol serialize/deserialize funcions
	 */
	static NameType nameTypeFromString(const QString &nameType);
	static const QString nameTypeToString(NameType nameType);

	/**
	 * \brief Create new contact.
	 *
	 * <b>The parent MetaContact must not be NULL</b>
	 *
	 * \note id is required to be unique per protocol and per account.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @param account is the parent account. this constructor automatically register the contact to the account
	 * @param id is the Contact's unique Id (mostly the user's login)
	 * @param parent is the parent @ref MetaContact this Contact is part of
	 * @param icon is an optional icon
	 */
	Contact( Account *account, const QString &id, MetaContact *parent,
		const QString &icon = QString() );

	~Contact();

	/**
	 * \brief Get the metacontact for this contact
	 * @return The MetaContact object for this contact
	 */
	MetaContact *metaContact() const;


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
	 * simply return account()->protocol()
	 *
	 * @return the contact's protocol
	 */
	Protocol* protocol() const;

	/**
	 * \brief Get the account that this contact belongs to
	 *
	 * @return the Account object for this contact
	 */
	Account* account() const;

	/**
	 * \brief Move this contact to a new MetaContact.
	 * This basically reparents the contact and updates the internal
	 * data structures.
	 * If the old contact is going to be empty, the old contact will be removed.
	 *
	 * @param m The new MetaContact to move this contact to
	 */
	void setMetaContact(MetaContact *m);

	/**
	 * @brief Get whether this contact is online.
	 * @return @c true if the contact is online, @c false otherwise.
	 */
	bool isOnline() const;

	/**
	 * \brief Get whether this contact can receive messages
	 *
	 * Used in determining if the contact is able to
	 * receive messages.  This function must be defined by child classes
	 *
	 * @return true if the contact can be reached
	 * @return false if the contact cannot be reached
	 */
	// FIXME: After KDE 3.2 we should split this into a public, NON-virtual
	//        isReachable() accessor that checks for account->isConnected()
	//        and then calls a new virtual method that does the
	//        protocol-specific work, like 'doIsUnreachable' or so - Martijn
	//
	//FIXME: Can this be made const please? - JK
	virtual bool isReachable();

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
	 * marked with @ref Plugin::addAddressBookField() with the
	 * contact id as value. If no index field is available the QMap is
	 * simply passed as an empty map.
	 *
	 * @sa Protocol::deserializeContact
	 */
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	/**
	 * @brief Get the online status of the contact
	 * @return the online status of the contact
	 */
	OnlineStatus onlineStatus() const;

	/**
	 * \brief Set the contact's online status
	 */
	void setOnlineStatus(const OnlineStatus &status);

	/**
	 * @brief Get the current status message of the contact.
	 * @return the status in a Kopete::StatusMessage.
	 */
	Kopete::StatusMessage statusMessage() const;
	/**
	 * @brief Set the contact's status message.
	 * It sets also the "awayMessage" property so you don't need to do it.
	 */
	void setStatusMessage(const Kopete::StatusMessage &statusMessage);
	 
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
	 * @todo if possible, try to use KXMLGUI
	 */
	virtual QList<KAction *> *customContextMenuActions();

	/**
	 * @deprecated
	 */
	virtual KDE_DEPRECATED QList<KAction *> *customContextMenuActions( ChatSession *manager );

	/**
	 * @brief Get the Context Menu for this contact
	 *
	 * This menu includes generic actions common to each protocol, and action defined in
	 * @ref customContextMenuActions()
	 */
	KMenu *popupMenu();

	/**
	 * @deprecated
	 */
	KDE_DEPRECATED KMenu *popupMenu( ChatSession *manager );

	/**
	 * \brief Get whether or not this contact is capable of file transfers
	 *
	 *
	 * \see setFileCapable()
	 * \return true if the protocol for this contact is capable of file transfers
	 * \return false if the protocol for this contact is not capable of file transfers
	 *
	 * @todo have a capabilioties. or move to protocol capabilities
	 */
	bool isFileCapable() const;

	/**
	 * \brief Set the file transfer capability of this contact
	 *
	 * \param filecap The new file transfer capability setting
	 * @todo have a capabilioties. or move to protocol capabilities
	 */
	void setFileCapable( bool filecap );

	/**
	 * \brief Get whether or not this contact can accept file transfers
	 *
	 * This function checks to make sure that the contact is online as well as
	 * capable of sending files.
	 * \see isReachable()
	 * @return true if this contact is online and is capable of receiving files
	 * @todo have a capabilioties. or move to protocol capabilities
	 */
	bool canAcceptFiles() const;

	enum CanCreateFlags {  CannotCreate=false , CanCreate=true  };

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
	virtual ChatSession * manager( CanCreateFlags canCreate = CannotCreate ) =0;

	/**
	 * Returns the name of the icon to use for this contact
	 *  If null, the protocol icon need to be used.
	 * The icon is not colored, nor has the status icon overloaded
	 */
	QString& icon() const;

	/**
	 * @brief Change the icon to use for this contact
	 * If you don't want to have the protocol icon as icon for this contact, you may set
	 * another icon.  The icon doesn't need to be colored with the account icon as this operation
	 * will be performed later.
	 *
	 * if you want to go back to the protocol icon, set a null string.
	 */
	void setIcon( const QString& icon );

	/**
	 * \brief Get the time (in seconds) this contact has been idle
	 * It will return the time set in @ref setIdleTime() with an addition of the time
	 * since you set this last time
	 * @return time this contact has been idle for, in seconds
	 //
         // FIXME: Can we make this just 'unsigned long' ? QT Properties can't handle
         // 'unsigned long int'
	 */
	virtual unsigned long int idleTime() const;

	/**
	 * \brief Set the current idle time in seconds.
	 * Kopete will automatically calculate the time in @ref idleTime
	 * except if you set 0.
	 //
	 // FIXME: Can we make this just 'unsigned long' ? QT Properties can't handle
	 // 'unsigned long int'
	 */
	void setIdleTime(unsigned long int);

        /**
	 * \brief Convenience method to set the nickName property to the specified value
	 * @param name The nickname to set
	 */
	void setNickName( const QString &name );

	/**
	 * \brief Convenience method to retrieve the nickName property.
	 * Property nickName comes from contact and cannot be changed by user custom name
	 * This method will return the contactId if there has been no nickName property set
	 */
	QString nickName() const;

	/**
	 * \brief Convenience method to set the customkName property to the specified value
	 * @param name The name to set
	 */
	void setCustomName( const QString &name );

	/**
	 * \brief Convenience method to retrieve the customName property.
	 * Property customName is name set by user and stored on server contact list
	 * This method will return nickName if there has been no customName property set
	 */
	QString customName() const;

	/**
	 * Set preferred name type, used by displayName function
	 */
	void setPreferredNameType(NameType type);

	/**
	 * Returns prefered name type, used by displayName function
	 * Default is CustomName
	 */
	NameType preferredNameType() const;

	/**
	 * Returns display name of contact. Suitable for GUI display.
	 * Call formattedName or nickName or customName or contactId depending on preferredNameType.
	 * This method will return contactId if preferredNameType property is empty string.
	 */
	QString displayName() const;

	/**
	 * \brief Get the tooltip for this contact
	 * Makes use of formattedName() and formattedIdleTime().
	 * \return an RTF tooltip depending on Kopete::AppearanceSettings settings
	 **/
	QString toolTip() const;

	/**
	 * Returns a formatted string of "firstName" and/or "lastName" properties if present.
	 **/
	QString formattedName() const;

	/**
	 * Returns a formatted string of idleTime().
	 * Suitable for GUI display
	 **/
	QString formattedIdleTime() const;

	/**
	 * @brief Convience method to set the photo property
	 * @param photoPath Local path to the photo
	 */
	void setPhoto(const QString &photoPath);

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
	 * Toggle the visibility of this contact even if offline.  This function
	 * is called by the KAction toggleAlwaysVisible that is part of the context
	 * menu.
	 */
	void toggleAlwaysVisible();

	/**
	 * Changes the MetaContact that this contact is a part of.  This function
	 * is called by the KAction changeMetaContact that is part of the context
	 * menu.
	 */
	void changeMetaContact();
	/**
	 * Method to retrieve user information.  Should be implemented by
	 * the protocols, and popup some sort of dialog box
	 *
	 * reimplement it to show the informlation
	 * @todo rename and make it pure virtual
 	 */
	virtual void slotUserInfo() {}

	/**
	 * @brief Syncronise the server and the metacontact.
	 * Protocols with server-side contact lists can implement this to
	 * sync the server groups with the metaContact groups. Or the server alias if any.
	 *
	 * This method is called every time the metacontact has been moved or renamed.
	 *
	 * default implementation does nothing
	 *
	 * @param changed is a bitmask of the @ref Changed enum which say why the call to this function is done.
	 */
	virtual void sync(unsigned int changed = 0xFF);

	/**
	 * @deprecated Use DeleteContactTask instead.
	 * Method to delete a contact from the contact list,
	 * should be implemented by protocol plugin to handle
	 * protocol-specific actions required to delete a contact
	 * (ie. messages to the server, etc)
	 * the default implementation simply call deleteLater()
	 */
	virtual KDE_DEPRECATED void deleteContact();

	/**
	 * This is the Contact level slot for sending files. It should be
	 * implemented by all contacts which have the setFileCapable() flag set to
	 * true. If the function is called through the GUI, no parameters are sent
	 * and they take on default values (the file is chosen with a file open dialog)
	 *
	 * @param sourceURL The actual KUrl of the file you are sending
	 * @param fileName (Optional) An alternate name for the file - what the
	 * receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending
	 * a nondeterminate
	 *                file size (such as over  asocket
	 */
	virtual void sendFile( const KUrl &sourceURL = KUrl(),
			       const QString &fileName = QString(), uint fileSize = 0L );
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

	/**
	 * The account's isConnected has changed.
	 */
	void slotAccountIsConnectedChanged();

	/**
	 * slot called when the metaContact was deleted.
	 */
	void slotMetaContactDestroyed( QObject* mc );

	/**
	 * slot to handle changing display name
	 */
	void slotPropertyChanged(Kopete::PropertyContainer *, const QString &key, const QVariant &oldValue, const QVariant &newValue);
signals:
	/**
	 * The contact's online status changed
	 */
	void onlineStatusChanged( Kopete::Contact *contact,
		const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );

	/**
	 * The contact's status message changed
	 */
	void statusMessageChanged( Kopete::Contact *contact );

	/**
	 * The contact is about to be destroyed.
	 * Called when entering the destructor. Useful for cleanup, since
	 * metaContact() is still accessible at this point.
	 *
	 * @warning this signal is emit in the Contact destructor, so all
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
	 * \brief Emitted when the file transfer capability of this contact has changed
	 */
	void canAcceptFilesChanged();

	/**
	 * Emitted when displayName has changed
	 */
	void displayNameChanged(const QString &oldName, const QString &newName);

private:
	class Private;
	Private * const d;


};


} //END namespace Kopete

#endif

