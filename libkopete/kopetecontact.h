/*
    kopetecontact.h - Kopete Contact

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

#ifndef KOPETECONTACT_H
#define KOPETECONTACT_H

#include <qobject.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <kurl.h>
//#include "kopetegroup.h"

class QString;
class QPixmap;

class KAction;
class KActionCollection;
class KListBox;
class KPopupMenu;

class KopeteEvent;
class KopeteGroup;
class KopeteHistoryDialog;
class KopeteMetaContact;
class KopeteProtocol;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
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
	KopeteContact( KopeteProtocol *protocol, const QString &id, KopeteMetaContact *parent );
	~KopeteContact();

	/**
	 * Contact's status
	 */
	enum ContactStatus { Online, Away, Offline, Unknown };

	/**
	 * Contact's idle status
	 */
	enum IdleState { Unspecified, Idle, Active };

	/**
	 * Return whether this contact is online or not.
	 * @return bool indicating whhether the contact is online
	 * FIXME: When all plugins support this, make this pure virtual!
	 */
	virtual bool isOnline() const { return status() != Offline && status() != Unknown; }

	/**
	 * Function used in determining if the contact is able to
	 * recieve messages even if offline, etc.  This function must
	 * be defined by child classes
	 *
	 * @return bool indicating whether or not the contact is reachable (can send a message to it)
	 */
	virtual bool isReachable() = 0;

	/**
	 * Accessor function for the contact's MetaContact
	 * @return The contact's metacontact
	 */
	KopeteMetaContact *metaContact() const { return m_metaContact; }

	/**
	 * Returns the identity this contact belongs to (i.e. for
	 * having multiple contacts of the same protocol in the metacontact)
	 * FIXME: This is a bad description, could someone clear it up?
	 *
	 * @return THe identity of the ID
	 */
	virtual QString identityId() const;

	/**
	 * The groups in which the user is physically located.
	 * The logical groups are stored in the Meta Contact. Physical groups
	 * can be different from the logical groups!
	 * Not the whole API supports multi-group membership yet, be careful
	 * relying on this for now!
	 */
//	virtual QStringList groups();

	/**
	 * Add a contact to a physical group. If the protocol doesn't support
	 * multi-group memberships this method can do nothing. The group name
	 * passed is the logical group. Protocols with server-side contact lists
	 * can use this to keep the local and remote lists in sync.
	 */
//	virtual void addToGroup( const QString &group );

	/**
	 * Remove a contact from a physical group.
	 * If the logical group passed is different from the physical group, or
	 * if this kind of changes is not supported this method may do nothing.
	 */
//	virtual void removeFromGroup( const QString &group );

	/**
	 * Move a contact from one group to another. Again, this method may do
	 * nothing if there's no support for this in the protocol.
	 */
//	virtual void moveToGroup( const QString &from, const QString &to );

	/**
	 * Sets the display name, or alias, for the contact.
	 * this is what is shown in the contact list.
	 * @param name Then new display name
	 */
	void setDisplayName( const QString &name );

	/**
	 * Get the current display name
	 * @return The display name
	 */
	QString displayName() const;

	/**
	 * Return the status of the contact
	 * @return the status of the contact
	 */
	virtual ContactStatus status() const;

	/**
	 * The text describing the contact's status
	 * The default implement does what you'd expect,
	 * but you might want to reimplement it for more
	 * fine-grained reporting of status
	 *
	 * @return Formatted status text
	 */
	virtual QString statusText() const;

	/**
	 * The name of the icon associated with the contact's status
	 * @return name of the icon associated with the contact's status
	 */
	virtual QString statusIcon() const;

	/**
	 * This method provides an scaled version of the status icon.
	 * useful for metacontacts, and it uses a primitive cache so
	 * we dont have to scale at every repaint, it asumes square.
	 */
	virtual QPixmap scaledStatusIcon(int size);

	/**
	 * The "importance" of this contact, used for sorting
	 * This is almost always related to the contact's status
	 * Here is how ICQ does it:
	 * 25 = Free For Chat
	 * 20 = Online
	 * 15 = Away (temporary away)
	 * 10 = Not Available (extended away)
	 * 5 = Invisible
	 * 0 = Offline
	 *
	 * The default implementation returns 20 for Online,
	 * 10 for away, and 0 for offline
	 * Please make your importance values between 0 and 25,
	 * and try to follow ICQ's scheme somewhat
	 */
	virtual int importance() const;

	/**
	 * Return the unique id that identifies a contact. Id is required
	 * to be unique per protocol and per identity. Across those boundaries
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
	 * Return the protocol id that identifies a contact.
	 *
	 * Note: Id is required to be unique per protocol and per identity.
	 * Across those boundaries ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 *
	 * @return the unique protocol id of the contact
	 */
	KopeteProtocol* protocol() const { return m_protocol; }

	/**
	 * Returns a set of custom menu items for the context menu
	 * which is displayed in showContextMenu (private).  Protocols
	 * should use this to add protocol-specific actions to the
	 * popup menu
	 *
	 * @return Collection of menu items to be show on the context menu
	 */
	 virtual KActionCollection *customContextMenuActions() {return 0L;};

	 /**
	  * Show a context menu of actions pertaining to this contact
	  *
	  * @param p The point at which to show the context menu
	  */
	 void showContextMenu( const QPoint& p );

	 /**
	  * Get the Context Menu for this contact
	  */
	KPopupMenu *createContextMenu();

	 /**
	  * Moves this contact to a new MetaContact
	  *
	  * @param m The new MetaContact to move this contact to
	  */
	 void moveToMetaContact(KopeteMetaContact *m);

	 /**
	  * Returns whether or not this contact is capable of file transfers or not
	  */
	 bool isFileCapable() const { return mFileCapable; }

	 /*
	  * Sets the capability of file transfers for this user. Once this is changed it will
	  * immediately add a new menu entry called "Send File...", and will call the
	  * virtual slotSendFile
	  */
	  void setFileCapable(bool filecap) { mFileCapable = filecap; }

	 /*
	  * Returns if this contact can accept file transfers
	  * @return True if this contact is online and is capable of sending files, False otherwise
	  */
	  bool canAcceptFiles() const { return isOnline() && mFileCapable; }

	 /**
	  * Accessor function for the idle state of the contact
	  * @return Idle state of the contact
	  */
	 IdleState idleState() const { return m_idleState; };

	 /**
	 	* Sets the idle state to newState
	  * If this function is not called by a plugin, the idle state is set to Unknown
	  */
	 void setIdleState( KopeteContact::IdleState newState );

public slots:
	/**
	 * This should typically pop up a KopeteChatWindow
	 */
	virtual void execute() = 0;

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
	 */
	virtual void slotDeleteContact() = 0;

	/**
	 * Method to retrieve user information.  Should be implemented by
	 * the protocols, and popup some sort of dialog box
	 */
	virtual void slotUserInfo() = 0;

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
	virtual void sendFile( const KURL &sourceURL = 0L, const QString &fileName = QString::null,
		const long unsigned int fileSize = 0L );

private slots:
	/**
	 * Function that is called when "Change Alias" is chosen from the
	 * context menu.  Presents dialog box to change alias, and calls
	 * setDisplayName with the returned value
	 */
	void slotChangeDisplayName();

	void slotMoveDialogOkClicked();
	void slotProtocolUnloading();

	/**
	 * View the chat history
	 */
	void slotViewHistory();

	/**
	 * Chat history widget got closed, remove the reference
	 */
	void slotHistoryDialogDestroyed();

signals:
	/**
	 * The contact's online status changed
	 */
	void statusChanged( KopeteContact *contact, KopeteContact::ContactStatus status );

	/**
	 * Deprecated, old signal! Use the above one instead
	 */
	//void statusChanged();

	/**
	 * Connect to this signal to know when the contact
	 * changed its name/nick
	 */
	void displayNameChanged(const QString &name);

	/**
	 * The contact is about to be destroyed.
	 * Called when entering the destructor. Useful for cleanup, since
	 * metaContact() is still accessible at this point.
	 */
	void contactDestroyed( KopeteContact *c );

	/**
	 * Called when the contact is moved to another MetaContact
	 */
	void moved(KopeteMetaContact *from , KopeteContact *)  ;

	/**
	 * The contact's idle state changed
	 */
	void idleStateChanged( KopeteContact *contact,
		KopeteContact::IdleState newState );

private:
	KopeteHistoryDialog *m_historyDialog;
	QString m_displayName;
	KopeteProtocol *m_protocol;
	bool mFileCapable;

	/**
	 * Scaled icon cache
	 */
	QPixmap m_cachedScaledIcon;
	int m_cachedSize;
	ContactStatus m_cachedOldStatus;

	//idle state
	IdleState m_idleState;

	KopeteMetaContact *m_metaContact;

	/* Context Menu Entries */
	KAction *actionSendMessage;
	KAction *actionDeleteContact;
	KAction *actionChangeMetaContact;
	KAction *actionViewHistory;
	KAction *actionChangeAlias;
	KAction *actionUserInfo;
	KAction *actionSendFile;

	KPopupMenu *contextMenu;

	/**
	 * Followed declarations are needed to move a contact into another metacontact
	 */
	KListBox *m_selectMetaContactListBox;
	class MetaContactListBoxItem : public QListBoxText
	{
		public:
			KopeteMetaContact *metaContact;
			MetaContactListBoxItem(KopeteMetaContact *m, QListBox *p);
	};

	QString m_contactId;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

