/*
    kopetemetacontact.h - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @tiscalinet.be>
    Copyright (c) 2003      by Will Stephenson        <will@stevello.free-online.co.uk>

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

#ifndef __kopetemetacontact_h__
#define __kopetemetacontact_h__

#include "kopetecontactlistelement.h"
#include <qptrlist.h>

#include <kabc/addressbook.h>
#include <kdemacros.h>


#include "kopeteonlinestatus.h"  


class QDomNode;

class KURL;

namespace Kopete {


class Plugin;
class Group;

/**
 * @author Will Stephenson <will@stevello.free-online.co.uk>
 * @author Martijn Klingens <klingens@kde.org>
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Olivier Goffart <ogoffart @tiscalinet.be>
 *
 * A metacontact represent a person. This is a kind of entry to
 * the contactlist. All information of a contact is contained in
 * the metacontact. Plugins can store data in it with all
 * @ref ContactListElement methods
 */
class MetaContact : public ContactListElement
{
	Q_OBJECT

	Q_PROPERTY( QString displayName READ displayName WRITE setDisplayName )
//	Q_PROPERTY( QString statusString READ statusString )
//	Q_PROPERTY( QString statusIcon READ statusIcon )
//	Q_PROPERTY( bool isOnline READ isOnline )
//	Q_PROPERTY( bool isReachable READ isReachable )
//	Q_PROPERTY( bool isTopLevel READ isTopLevel )
//	Q_PROPERTY( bool canAcceptFiles READ canAcceptFiles )
	Q_PROPERTY( QString metaContactId READ metaContactId WRITE setMetaContactId )
	Q_PROPERTY( bool trackChildNameChanges READ trackChildNameChanges WRITE setTrackChildNameChanges )

public:

	/** 
	 * constructor
	 */
	MetaContact();
	/**
	 * destructor
	 */
	~MetaContact();
	
	/**
	 * @brief Returns this metacontact's ID.
	 *
	 * Every metacontact has a unique id, set by  when creating the contact, or reading the contactlist
	 * TODO: make it real
	 */
	QString metaContactId() const;

	/**
	 * @brief Add or change the link to a KDE addressbook (KABC) Addressee.
	 * FIXME: Use with care.  You could create 1 to many relationships with the current implementation
	 */
	void setMetaContactId( const QString& newMetaContactId );


	/**
	 * @brief Retrieve the list of contacts that are part of the meta contact
	 */
	QPtrList<Contact> contacts() const;
	
	/**
	 * @brief The groups the contact is stored in
	 */
	QPtrList<Group> groups() const;


	/**
	 * Find the Contact to a given contact. If contact
	 * is not found, a null pointer is returned.
	 * if @p protocolId or @p accountId are null, it is searched over all protocols/accounts
	 */
	Contact *findContact( const QString &protocolId, const QString &accountId, const QString &contactId );
	
	
	/**
	 * @return the display name showed in the contactlist window, or in the chatwindow
	 */
	QString displayName() const;
	/**
	 * @brief Set the displayName.
	 *
	 * this metohd may emit @ref displayNameChanged signal.
	 * If @ref trackChildNameChanges was true, this will automatically set it to false
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
	 * Temporary contacts will not be serialized.
	 * If they are added to the contactlist, they appears in a special "Not in your contactlist" group.
	 * (the @ref Group::temporary  group)
	 */
	bool isTemporary() const;


	/**
	 * @brief Add a brand new contact to the meta contact. 
	 *  Updates KABC
	 * @param c The Contact being added
	 */
	void addContact( Contact *c );
	
	/**
	 * @brief remove the contact from this metacontact
	 *
	 * set 'deleted' to true if the Contact is already deleted
	 *
	 * @param c is the contact to remove
	 * @param deleted : if it is false, it will disconnect the old contact, and call some method.
	 */
	void removeContact( Contact *c , bool deleted = false );
	
	
	
	
	/**
	 * @return the preferred child Contact for communication, or 0 if none is suitable (all unreachable).
	 */
	Contact *preferredContact();

	

#if 0 //TODO	
		
	
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
	OnlineStatus::OnlineStatus status() const;

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
	 * Get or set a field for the KDE address book backend. Fields not
	 * registered during the call to Plugin::addressBookFields()
	 * cannot be altered!
	 *
	 * @param p The Plugin by which uses this field
	 * @param app refers to the application id in the libkabc database.
	 * This should be a standardized format to make sense in the address
	 * book in the first place - if you could use "" as application
	 * then probably you should use the plugin data API instead of the
	 * address book fields.
	 *
	 * FIXME: In the code the requirement that fields are registered first
	 *        is already lifted, but the API needs some review before we
	 *        can remove it here too.
	 *        Probably it requires once more some rewrites to get it working
	 *        properly :( - Martijn
	 */
	QString addressBookField( Plugin *p, const QString &app, const QString &key ) const;
	
	/**
	 * @brief set an address book field
	 *
	 * @see also @ref addressBookField()
	 * @param p The Plugin by which uses this field
	 */
	void setAddressBookField( Plugin *p, const QString &app, const QString &key, const QString &value );
	
 //slots
	
	/**
	 * @brief Send a file to this metacontact
	 *
	 * This is the MetaContact level slot for sending files. It may be called through the
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

	/**
	 * @brief Change the KABC data associated with this metacontact
	 *
	 * The KABC exposed data changed, so change it in KABC
	 */
	void updateKABC();

	/**
	 * @brief Remove any KABC data for this meta contact
	 */
	void removeKABC();
	
	/**
	 * Check for any new addresses added to this contact's KABC entry
	 * and prompt if they should be added in Kopete too.
	 * @return whether any contacts were added from KABC.
	 */
	bool syncWithKABC();

	
//signals	
	/**
	 * This metaContact is going to be saved to the contactlist. Plugins should
	 * connect to this signal to update data with setPluginData()
	 */
	void aboutToSave( MetaContact *metaContact );

	/**
	 * One of the subcontacts' idle status has changed.  As with online status,
	 * this can occur without the metacontact changing idle state
	 */
	void contactIdleStateChanged( Contact *contact );


#endif


public slots:

	/**
	 * @brief Move a contact from one group to another.
	 */
	void moveToGroup( Group *from, Group *to );

	/**
	 * @brief Remove a contact from one group
	 */
	void removeFromGroup( Group *from );

	/**
	 * @brief Add a contact to another group.
	 */
	void addToGroup( Group *to );


	/**
	 * @brief Set if this is a temporary contact. (see @ref isTemporary)
	 *
	 * @param b if the contact is or not temporary
	 * @param group if the contact was temporary and b is true, then the contact will be moved to this group.
	 *  if group is null, it will be moved to top-level
	 */
	void setTemporary( bool b = true ,Group *group = 0L );


	/**
	 * @brief Contact another user.
	 *
	 * Depending on the config settings, call sendMessage() or
	 * startChat()
	 *
	 * returns the Contact that was chosen as the preferred
	 */
	Contact *execute();

	/**
	 * @brief Send a single message, classic ICQ style.
	 *
	 * The actual sending is done by the Contact, but the meta contact
	 * does the GUI side of things.
	 * This is a slot to allow being called easily from e.g. a GUI.
	 *
	 * returns the Contact that was chosen as the preferred
	 */
	Contact *sendMessage();

	/**
	 * @brief Start a chat in a persistent chat window
	 *
	 * Like sendMessage, but this time a full-blown chat will be opened.
	 * Most protocols can't distinguish between the two and are either
	 * completely session based like MSN or completely message based like
	 * ICQ the only true difference is the GUI shown to the user.
	 *
	 * returns the Contact that was chosen as the preferred
	 */
	Contact *startChat();

signals:
	/**
	 *  @brief The MetaContact online status changed
	 */
	void onlineStatusChanged( MetaContact *contact, OnlineStatus::OnlineStatus status );

	/**
	 * @brief A contact's online status changed
	 *
	 * this signal differs from @ref onlineStatusChanged because a contact can
	 * change his status without changing MetaContact status. It is mainly used to update the small icons
	 * in the contactlist
	 */
	void contactStatusChanged( Contact *contact, const OnlineStatus &status );

	/**
	 * @brief The meta contact's display name changed
	 */
	void displayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * @brief  The contact was moved
	 */
	void movedToGroup( MetaContact *contact, Group *from, Group *to );

	/**
	 * @brief The contact was removed from group
	 */
	void removedFromGroup( MetaContact *contact, Group *group );

	/**
	 * @brief The contact was added to another group
	 */
	void addedToGroup( MetaContact *contact, Group *to );

	/**
	 * @brief a contact has been added into this metacontact
	 *
	 * This signal is emitted when a contact is added to this metacontact
	 */
	void contactAdded( Contact *c );

	/**
	 * @brief a contact has been removed from this metacontact
	 *
	 * This signal is emitted when a contact is removed from this metacontact
	 */
	void contactRemoved( Contact *c );
	
	/**
	 * Some part of this object's persistent data (as returned by toXML) has changed.
	 */
	void persistentDataChanged(  );
	

private slots:
	/**
	 * Update the contact's online status and emit onlineStatusChanged
	 * when appropriate
	 */
	void updateOnlineStatus();

	/**
	 * One of the child contact's online status changed
	 */
	void slotContactStatusChanged( Contact *c, const OnlineStatus &status, const OnlineStatus &oldStatus );

	/**
	 * One of the child contact's property changed
	 */
	void slotPropertyChanged( Contact *contact, const QString &key, const QVariant &oldValue, const QVariant &newValue  );

	/**
	 * A child contact was deleted, remove it from the list, if it's still
	 * there
	 */
	void slotContactDestroyed( Contact* );

	/**
	 * Perform a delayed address book write
	 */
	void slotWriteAddressBook();

private:

#if 0 //TODO
	/**
	 * If a plugin is loaded, maybe data about this plugin are already cached in the metacontact
	 */
	void slotPluginLoaded( Plugin *plugin );
#endif



	class Private;
	Private *d;

	/**
	 * Request an address book write, will be delayed to bundle any others happening around the same time
	 */
	void writeAddressBook();

	static KABC::AddressBook* addressBook();
    
	static void splitField( const QString &str, QString &app, QString &name, QString &value );
	
	static KABC::AddressBook* m_addressBook;
};

} //END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:

