/*
    kopetemetacontact.h - Kopete Meta Contact

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
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

#ifndef __kopetemetacontact_h__
#define __kopetemetacontact_h__

#include <qobject.h>
#include <qptrlist.h>
#include <qdom.h>

#include "kopetecontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteplugindataobject.h"

class QDomNode;

class KURL;

class KopeteGroup;

struct KopeteMetaContactPrivate;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * A metacontact represent a person. This is a kind of entry to
 * the contactlist. All information of a contact is contained in
 * the metacontact. Plugins can store data in it with all
 * @ref KopetePluginData methods
 */
class KopeteMetaContact : public KopetePluginDataObject
{
	Q_OBJECT

public:
	KopeteMetaContact();
	~KopeteMetaContact();

	/**
	 * @brief Retrieve the list of contacts that are part of the meta contact
	 */
	QPtrList<KopeteContact> contacts() const;

	/**
	 * @brief Add a contact to the meta contact
	 */
	void addContact( KopeteContact *c );

	/**
	 * Find the KopeteContact to a given contact. If contact
	 * is not found, a null pointer is returned.
	 */
	KopeteContact *findContact( const QString &protocolId, const QString &accountId, const QString &contactId );
	/**
	 * @brief The name of the icon associated with the contact's status
	 */
	virtual QString statusIcon() const;

	/**
	 * @brief The status string of the contact
	 *
	 * @see @ref status()
	 */
	QString statusString() const;

	/**
	 * Returns whether this contact can be reached online for at least one
	 * protocol. Protocols are processed in loading order.
	 * FIXME: Make that user preference order!
	 * FIXME: Make that an enum, because status can be unknown for certain
	 *        protocols
	 */
	bool isOnline() const;

	/**
	 * Returns whether this contact can accept files
	 * @return True if the user is online with a file capable protocol, false otherwise
	 */
	bool canAcceptFiles() const;

		/**
	 * Return a more fine-grained status.
	 * Online means at least one sub-contact is online, away means at least
	 * one is away, but nobody is online and offline speaks for itself
	 */
	KopeteOnlineStatus::OnlineStatus status() const;

	/**
	 * Like isOnline, but returns true even if the contact is not online, but
	 * can be reached trough offline-messages.
	 * it it return false, you are unable to open a chatwindow
	 * FIXME: Here too, use preference order, not append order!
	 * FIXME: Here too an enum.
	 */
	bool isReachable() const;

	/**
	 * return the time in second the contact is idle.
	 */
	unsigned long int idleTime() const;

	/**
	 * @return the display name showed in the contactlist window, or in the chatwindow
	 */
	QString displayName() const;
	/**
	 * @brief Set the displayName.
	 *
	 * this metohd may emit @ref displayNameChanged signal.
	 * If @ref trackChildNameChanges was true, this will automaticaly set it to false
	 */
	void setDisplayName( const QString &name );

	/**
	 * @brief get the tracking of contact names
	 *
	 * The MetaContact will adjust its displayName() every time the contact
	 * inside changes its name.
	 * This should only work for MCs with exactly ONE contact inside in order
	 * to not confuse users (think about 4 subcontacts and what happens if one
	 * changes nickname...)
	 */
	bool trackChildNameChanges() const;
	/**
	 * @brief set if the metacontact displayname follow subcontacts displayname
	 *
	 * When setting it to true, it will refresh the dysplayname to the subcontactone instentaneous,
	 * and each time the contact change.
	 *
	 * Note that this method has an effect only if there is exactly one subcontact.
	 *
	 * @see @ref trackChildNameChanges , @ref setDisplayName
	 */
	void setTrackChildNameChanges( bool track );

	/**
	 * @brief The groups the contact is stored in
	 */
	KopeteGroupList groups() const;

	/**
	 * Return a XML representation of the metacontact
	 * @internal
	 */
	const QDomElement toXML();

	/**
	 * Creates a metacontact from XML
	 * Return value of false indicated that
	 * creation failed and this contact should be
	 * discarded.
	 * @internal
	 */
	bool fromXML( const QDomElement& cnode );

	/**
	 * @brief Move a contact from one group to another.
	 */
	void moveToGroup( KopeteGroup *from, KopeteGroup *to );

	/**
	 * @brief Remove a contact from one group
	 */
	void removeFromGroup( KopeteGroup *from );

	/**
	 * @brief Add a contact to another group.
	 */
	void addToGroup( KopeteGroup *to );

	/**
	 * Temporary contacts will not be serialized.
	 * If they are added to the contactlist, they appears in a special "Not in your contactlist" group.
	 * (the @ref KopeteGroup::temporary  group)
	 */
	bool isTemporary() const;
	/**
	 * Set if this is a temporary contact. (see @ref isTemporary)
	 *
	 * @param b if the contact is or not temporary
	 * @param group if the contact was temporary and b is true, then the contact will be moved to this group.
	 *  if group is null, it will be moved to top-level
	 */
	void setTemporary( bool b = true ,KopeteGroup *group = 0L );

	/**
	 * @brief Return true if the contact is shown at toplevel.
	 * You may now check if @ref groups() contains @ref KopeteGroup::toplevel
	 */
	bool isTopLevel();

	/**
	 * add or remove from top-level
	 * @obsolete
	 * use @ref addToGroup() with @ref KopeteGroup::toplevel
	 */
	void setTopLevel( bool b = true );

	/**
	 * @brief remove the contact from this metacontact
	 *
	 * set 'deleted' to true if the KopeteContact is already deleted
	 *
	 * @param c is the contact to remove
	 * @param deleted : if it is false, it will disconnect the old contact, and call some method.
	 */
	void removeContact( KopeteContact *c , bool deleted = false );

	/**
	 * @brief Returns this metacontact's ID.
	 *
	 * Every metacontact has a unique id, set by kopete when creating the contact, or reading the contactlist
	 */
	QString metaContactId() const;

	/**
	 * Get or set a field for the KDE address book backend. Fields not
	 * registered during the call to KopetePlugin::addressBookFields()
	 * cannot be altered!
	 *
	 * @param app refers to the application id in the libkabc database.
	 * This should be a standardized format to make sense in the address
	 * book in the first place - if you could use "kopete" as application
	 * then probably you should use the plugin data API instead of the
	 * address book fields.
	 *
	 * FIXME: In the code the requirement that fields are registered first
	 *        is already lifted, but the API needs some review before we
	 *        can remove it here too.
	 *        Probably it requires once more some rewrites to get it working
	 *        properly :( - Martijn
	 */
	QString addressBookField( KopetePlugin *p, const QString &app, const QString &key ) const;
	/**
	 * @bief set an address book field
	 *
	 * @see also @ref addressBookField()
	 */
	void setAddressBookField( KopetePlugin *p, const QString &app, const QString &key, const QString &value );

public slots:
	/**
	 * @brief Contact another user.
	 *
	 * Depending on the config settings, call sendMessage() or
	 * startChat()
	 *
	 * returns the KopeteContact that was chosen as the preferred
	 */
	KopeteContact *execute();

	/**
	 * @brief Send a single message, classic ICQ style.
	 *
	 * The actual sending is done by the KopeteContact, but the meta contact
	 * does the GUI side of things.
	 * This is a slot to allow being called easily from e.g. a GUI.
	 *
	 * returns the KopeteContact that was chosen as the preferred
	 */
	KopeteContact *sendMessage();

	/**
	 * @brief start a chat in a persistent chat window
	 *
	 * Like sendMessage, but this time a full-blown chat will be opened.
	 * Most protocols can't distinguish between the two and are either
	 * completely session based like MSN or completely message based like
	 * ICQ the only true difference is the GUI shown to the user.
	 *
	 * returns the KopeteContact that was chosen as the preferred
	 */
	KopeteContact *startChat();

	/**
	 * This is the KopeteMetaContact level slot for sending files. It may be called through the
	 * "Send File" entry in the GUI, or over DCOP. If the function is called through the GUI,
	 * no parameters are sent and they assume default values. This slot calls the slotSendFile
	 * with identical params of the highest ranked contact capable of sending files (if any)
	 *
	 * @param sourceURL The actual KURL of the file you are sending
	 * @param altFileName (Optional) An alternate name for the file - what the receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending a nondeterminate
	 *                file size (such as over a socket)
	 *
	 */
	void sendFile( const KURL &sourceURL, const QString &altFileName = QString::null,
		unsigned long fileSize = 0L );


signals:
	/**
	 *  @brief The MetaContact online status changed
	 *
	 * Do *NOT* emit this signal directly, unless you also update the
	 * cache m_onlineStatus value! In all other cases, just call
	 * updateOnlineStatus() instead.
	 */
	void onlineStatusChanged( KopeteMetaContact *contact, KopeteOnlineStatus::OnlineStatus status );

	/**
	 * @brief A contact's online status changed
	 *
	 * this signal differs from @ref onlineStatusChanged because a contact can
	 * change his status without changing MetaContact status. It is mainly used to update the small icons
	 * in the contactlist
	 */
	void contactStatusChanged( KopeteContact *contact, const KopeteOnlineStatus &status );

	/**
	 * @brief The meta contact's display name changed
	 */
	void displayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * @brief  The contact was moved
	 */
	void movedToGroup( KopeteMetaContact *contact, KopeteGroup *from, KopeteGroup *to );

	/**
	 * @brief The contact was removed from group
	 */
	void removedFromGroup( KopeteMetaContact *contact, KopeteGroup *group );

	/**
	 * @brief The contact was added to another group
	 */
	void addedToGroup( KopeteMetaContact *contact, KopeteGroup *to );

	/**
	 * @brief a contact has been added into this metacontact
	 *
	 * This signal is emitted when a contact is added to this metacontact
	 */
	void contactAdded( KopeteContact *c );

	/**
	 * @brief a contact has been removed from this metacontactµ
	 *
	 * This signal is emitted when a contact is removed from this metacontact
	 */
	void contactRemoved( KopeteContact *c );

	/**
	 * This metaContact is going to be saved to the contactlist. Plugin should
	 * connect to this signal to update data with setPluginData()
	 */
	void aboutToSave( KopeteMetaContact *metaContact );

	/**
	 * One of the subcontacts' idle status has changed.  As with online status,
	 * this can occur without the metacontact changing idle state
	 */
	void contactIdleStateChanged( KopeteContact *contact );

private slots:
	/**
	 * Update the contact's online status and emit onlineStatusChanged
	 * when appropriate
	 */
	void updateOnlineStatus();

	/**
	 * One of the child contact's online status changed
	 */
	void slotContactStatusChanged( KopeteContact *c, const KopeteOnlineStatus &status, const KopeteOnlineStatus &oldStatus );

	/**
	 * One of the child contact's display names changed
	 */
	void slotContactNameChanged( const QString &oldName, const QString &newName );

	/**
	 * A child contact was deleted, remove it from the list, if it's still
	 * there
	 */
	void slotContactDestroyed( KopeteContact* );

	/**
	 * If a plugin is loaded, maybe data about this plugin are already cached in the metacontact
	 */
	void slotPluginLoaded( KopetePlugin *plugin );


private:
	KopeteContact *preferredContact();

	KopeteMetaContactPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

