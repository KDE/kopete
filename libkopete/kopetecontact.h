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

class QString;
class QPixmap;

class KopeteEvent;
class KopeteMetaContact;
class KAction;
class KActionCollection;
class KListBox;
class KPopupMenu;

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
	KopeteContact( const QString &protocolId, KopeteMetaContact *parent );
	~KopeteContact();
	
	/**
	 * Contact's status
	 */
	enum ContactStatus { Online, Away, Offline };

	/**
	 * Return whether this contact is online or not.
	 * FIXME: Make the return value an enum, because this value might
	 *        be Unknown or NotApplicable!
	 * FIXME: When all plugins support this, make this pure virtual!
	 */
	bool isOnline() const { return status() != Offline; }

	/**
	 * Return whether or not this contact is REACHABLE.
	 * Useful in determining if the contact is able to
	 * recieve messages even if offline, etc.
	 */
	virtual bool isReachable() = 0;
	
	/**
	 * The meta contact this contact is contained in
	 */
	KopeteMetaContact *metaContact() const { return m_metaContact; }

	/**
	 * Return the ID of the identity this contact belongs to
	 */
	virtual QString identityId() const;
	
	/**
	 * The groups in which the user is physically located.
	 * The logical groups are stored in the Meta Contact. Physical groups
	 * can be different from the logical groups!
	 * Not the whole API supports multi-group membership yet, be careful
	 * relying on this for now!
	 * FIXME: When all protocols support the group API, make these methods
	 *        pure virtual or have different defaults!
	 */
	virtual QStringList groups();

	/**
	 * Add a contact to a physical group. If the protocol doesn't support
	 * multi-group memberships this method can do nothing. The group name
	 * passed is the logical group. Protocols with server-side contact lists
	 * can use this to keep the local and remote lists in sync.
	 */
	virtual void addToGroup( const QString &group );

	/**
	 * Remove a contact from a physical group.
	 * If the logical group passed is different from the physical group, or
	 * if this kind of changes is not supported this method may do nothing.
	 */
	virtual void removeFromGroup( const QString &group );

	/**
	 * Move a contact from one group to another. Again, this method may do
	 * nothing if there's no support for this in the protocol.
	 */
	virtual void moveToGroup( const QString &from, const QString &to );

	/**
	 * Sets the display name, or alias, for the contact.
	 * this is what is shown in the contact list.
	 * @param name Then new display name
	 */
	void setDisplayName( const QString &name );
	
	/**
	 * Get the current display name
	 */
	QString displayName() const;

	/**
	 * Return the status of the contact
	 */
	virtual ContactStatus status() const;

	/**
	 * The text describing the contact's status
	 * The default implement does what you'd expect,
	 * but you might want to reimplement it for more
	 * fine-grained reporting of status
	 */
	virtual QString statusText() const;
	
	/**
	 * The name of the icon associated with the contact's status
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
	 */
	virtual QString id() const = 0;

	/**
	 * Return the protocol id that identifies a contact. Id is required
	 * to be unique per protocol and per identity. Across those boundaries
	 * ids may occur multiple times.
	 * The id is solely for comparing items safely (using pointers is
	 * more crash-prone). DO NOT assume anything regarding the id's
	 * value! Even if it may look like an ICQ UIN or an MSN passport,
	 * this is undefined and may change at any time!
	 */
	QString protocol() const { return m_protocolId; }
	
	/**
	 * Returns a set of custom menu items for the context menu
	 * which is displayed in showContextMenu (private).  Protocols
	 * should use this to add protocol-specific actions to the 
	 * popup menu
	 */
	 virtual KActionCollection *customContextMenuActions() = 0;
	 
	 /**
	 * Show a context menu of actions pertaining to this contact
	 * I hate having the group parameter, but its used for when
	 * a contact can be in multiple groups and you have to move
	 * a specific instance from one group to another.
	 */
	void showContextMenu( const QPoint& p, const QString& group );


	void moveToMetaContact(KopeteMetaContact *m);


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
	 * Method to view the history, should be implemented by the protocol plugin
	 */
	virtual void slotViewHistory() = 0;
	
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
	
private slots:
	/**
	 * Function that is called when "Change Alias" is chosen from the
	 * context menu.  Presents dialog box to change alias, and calls
	 * setDisplayName with the returned value
	 */
	void slotChangeDisplayName();

	void slotMoveDialogOkClicked();

signals:
	/**
	 * The contact's online status changed
	 */
	void statusChanged( KopeteContact *contact, KopeteContact::ContactStatus status );

	/**
	 * Deprecated, old signal! Use the above one instead
	 */
	void statusChanged();

	/**
	 * Connect to this signal to know when the contact
	 * changed its name/nick
	 */
	void displayNameChanged(const QString &name);

	/**
	 * This gets emitted usually when you receive a message from
	 * this contact.
	 */
	void incomingEvent(KopeteEvent *);

	/**
	 * The contact is about to be destroyed.
	 * Called when entering the destructor. Useful for cleanup, since
	 * metaContact() is still accessible at this point.
	 */
	void contactDestroyed( KopeteContact *c );

	/**
	 * Called when the contact is moved to another MetaContact
	 */
	void moved(KopeteContact *)  ;

private:
	
	QString m_displayName;
	QString m_protocolId;

	/**
	 * Scaled icon cache
	 */
	QPixmap m_cachedScaledIcon;
	int m_cachedSize;
	ContactStatus m_cachedOldStatus;

	KopeteMetaContact *m_metaContact;

	/* Function to build the contextMenu */
	void initActions();
	/* Context Menu Entries */
	KAction *actionSendMessage;
	KAction *actionDeleteContact;
	KAction *actionChangeMetaContact;
	KAction *actionViewHistory;
	KAction *actionChangeAlias;
	KAction *actionUserInfo;

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
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

