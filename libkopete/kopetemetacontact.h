/*
    kopetemetacontact.h - Kopete Meta Contact

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Will Stephenson        <will@stevello.free-online.co.uk>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef kopetemetacontact_h__
#define kopetemetacontact_h__

#include "kopetecontactlistelement.h"

#include <QtCore/QList>
#include <QtCore/QUuid>

#include <kdemacros.h>
#include <kurl.h>
#include "kopete_export.h"

#include "kopeteonlinestatus.h"
#include "kopetestatusmessage.h"

namespace Kopete {


class Plugin;
class Group;
class Picture;

/**
 * @author Will Stephenson <will@stevello.free-online.co.uk>
 * @author Martijn Klingens <klingens@kde.org>
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 * @author Olivier Goffart <ogoffart@kde.org>
 *
 * A metacontact represent a person. This is a kind of entry to
 * the contact list. All information of a contact is contained in
 * the metacontact. Plugins can store data in it with all
 * @ref ContactListElement methods
 */
class KOPETE_EXPORT MetaContact : public ContactListElement
{
	Q_OBJECT

	Q_PROPERTY( QString displayName READ displayName WRITE setDisplayName )
	Q_PROPERTY( QString statusString READ statusString )
	Q_PROPERTY( QString statusIcon READ statusIcon )
	Q_PROPERTY( bool isOnline READ isOnline )
	Q_PROPERTY( bool isReachable READ isReachable )
	Q_PROPERTY( bool isTemporary READ isTemporary )
	Q_PROPERTY( bool canAcceptFiles READ canAcceptFiles )
	//Q_PROPERTY( ulong idleTime READ idleTime )
	Q_PROPERTY( QUuid metaContactId READ metaContactId WRITE setMetaContactId )
	Q_PROPERTY( bool photoSyncedWithKABC READ isPhotoSyncedWithKABC WRITE setPhotoSyncedWithKABC )

public:
	typedef QList<MetaContact *> List;
	/** 
	 * Enumeration of possible sources for a property (which may be
	 * photos, see setPhotoSource() for instance).
	 */
	enum PropertySource { 
		SourceContact /**< Data comes from the contact itself. */, 
		SourceKABC /**< Data comes from KABC (addressbook). */, 
		SourceCustom /**< Data comes from somewhere else. */
	};

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
	 * Every metacontact has a unique id, set when creating the contact, 
	 * or reading the contactlist
	 */
	QUuid metaContactId() const;
  
	/**
	 * @internal
	 * Set the meta contact id for this meta contact to use. It should only
	 * be used by the contact list loading mechanism
	 */
	void setMetaContactId( const QUuid& newMetaContactId );

	/**
	 * @brief Get the KABC id for this metacontact
	 */
	QString kabcId() const;

	/**
	 * @brief Set the KABC id for this metacontact
	 * Use with care! You could create a one to many relationship
	 */
	void setKabcId( const QString& newKabcId );

	/**
	 * @brief Retrieve the list of contacts that are part of the meta contact
	 */
	QList<Contact *> contacts() const;

	/**
	 * @brief The groups the contact is stored in
	 */
	QList<Group *> groups() const;

	/**
	 * Find the Contact to a given contact. If contact
	 * is not found, a null pointer is returned.
	 * if @p protocolId or @p accountId are null, it is searched over all protocols/accounts
	 */
	Contact *findContact( const QString &protocolId, const QString &accountId, const QString &contactId );

	/**
	 * @brief Set the source of metacontact displayName
	 *
	 * This method selects the display name source for one
	 * of the sources defined in @ref PropertySource
	 *
	 * @see PropertySource
	 */
	void setDisplayNameSource(PropertySource source);

	void setDisplayNameSource( const QString &nameSourcePID, const QString &nameSourceAID, const QString &nameSourceCID );

	/**
	 * @brief get the source of metacontact display name
	 *
	 * This method obtains the current name source for one
	 * of the sources defined in @ref PropertySource
	 *
	 * @see PropertySource
	 */
	PropertySource displayNameSource() const;

	/**
	 * @brief Set the source of metacontact photo
	 *
	 * This method selects the photo source for one
	 * of the sources defined in @ref PropertySource
	 *
	 * @see PropertySource
	 */
	void setPhotoSource(PropertySource source);

	void setPhotoSource( const QString &photoSourcePID, const QString &photoSourceAID, const QString &photoSourceCID );

	/**
	 * @brief get the source of metacontact photo
	 *
	 * This method obtains the current photo source for one
	 * of the sources defined in @ref PropertySource
	 *
	 * @see PropertySource
	 */
	PropertySource photoSource() const;

	/**
	 * @brief the display name showed in the contact list window
	 *
	 * The displayname is the name which should be shown almost everywere to
	 * represent the metacontact.  (in the contact list, in the chatwindow, ...)
	 *
	 * This is a kind of alias, set by the kopete user, as opposed to a nickname
	 * set by the contact itself.
	 *
	 * If the protocol support alias serverside, the metacontact displayname
	 * should probably be synchronized with the alias on the server.
	 *
	 * This displayName is obtained from the source set with @ref setDisplayNameSource
	 */
	QString displayName() const;

	/**
	 * @brief the photo showed in the contact list window
	 *
	 * Returns a image for the metacontact. If the metacontact photo source is
	 * the KDE addressbook. it will return the picture stored in the addressbook
	 * It can also use a subcontact as the photo source.
	 *
	 * This photo is obtained from the source set with @ref setPhotoSource
	 * @deprecated Use picture().image() instead.
	 */
	KDE_DEPRECATED QImage photo() const;

	/**
	 * Return the correct Kopete::Picture object depending of the metacontact photo source.
	 *
	 * This photo is obtained from the source set with @ref setPhotoSource
	 *
	 * KDE4 TODO: Rename this to photo() and use the new object.
	 */
	Picture &picture() const;

	/**
	 * @brief Set the custom displayName.
	 *
	 * This display name is used when name source is Custom
	 * this metohd may emit @ref displayNameChanged signal.
	 * And will call @ref Kopete::Contact::sync
	 *
	 * @see displayName()
	 * @see displayNameSource()
	 */
	void setDisplayName( const QString &name );

	/**
	 * @brief Returns the custom display name
	 *
	 * @see displayName()
	 * @see displayNameSource()
	 */
	QString customDisplayName() const;

	/**
	 * @brief Returns the custom display photo
	 *
	 * @see photo()
	 * @see photoSource()
	 */
	KUrl customPhoto() const;


	/**
	 * @brief Set the custom photo.
	 *
	 * This photo is used when photo source is set toCustom
	 * this metohd may emit @ref photoChanged signal.
	 *
	 * @see photo()
	 * @see photoSource()
	 */
	void setPhoto( const KUrl &url );

	/**
	 * @brief get the subcontact being tracked for its displayname (null if not set)
	 *
	 * The MetaContact will adjust its displayName() every time the
	 * "nameSource" changes its nickname property.
	 */
	Contact *displayNameSourceContact() const;

	/**
	 * @brief set the subcontact whose name is to be tracked (set to null to disable tracking)
	 * @see nameSource
	 */
	void setDisplayNameSourceContact( Contact* contact );

	/**
	 * @brief get the subcontact being tracked for its photo
	 */
	Contact *photoSourceContact() const;

	/**
	 * @brief set the subcontact to use for SourceContact source
	 */
	void setPhotoSourceContact( Contact* contact );

	/**
	 * @return true if when a subcontact change his photo, the photo will be set to the kabc contact.
	 */
	bool isPhotoSyncedWithKABC() const;

	/**
	 * Set if the photo should be synced with the adressbook when the photosource change his photo
	 *
	 * If  \p b is true, the photo will be synced immediately if possible
	 */
	void setPhotoSyncedWithKABC(bool b);


	/**
	 * Temporary contacts will not be serialized.
	 * If they are added to the contact list, they appears in a special "Not in your contact list" group.
	 * (the @ref Group::temporary  group)
	 */
	bool isTemporary() const;

	/**
	 * @brief Add a contact which has just been deserialised to the meta contact
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

	/**
	 * @brief The name of the icon associated with the contact's status
	 * @todo improve with OnlineStatus
	 */
	QString statusIcon() const;

	/**
	 * @brief The status string of the contact
	 *
	 * @see @ref status()
	 * @todo improve with OnlineStatus
	 */
	QString statusString() const;

	/**
	 * Returns whether this contact can be reached online for at least one
	 * FIXME: Make that an enum, because status can be unknown for certain
	 *        protocols
	 */
	bool isOnline() const;

	/**
	 * Returns whether this contact is visible even if offline
	 */
	bool isAlwaysVisible() const;

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
	OnlineStatus::StatusType status() const;

	/**
	 * Like isOnline, but returns true even if the contact is not online, but
	 * can be reached trough offline-messages.
	 * it it return false, you are unable to open a chatwindow
	 * @todo : Here too, use preference order, not append order!
	 * @todo : Here too an enum.
	 */
	bool isReachable() const;

	/**
	 * return the time in second the contact is idle.
	 */
	quint32 idleTime() const;

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
	 * @param key The name of the address book field to get or set
	 *
	 * @todo: In the code the requirement that fields are registered first
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
	 * @param app The application ID in the KABC database
	 * @param key The name of the address book field to set
	 * @param value The value of the address book field to set
	 */
	void setAddressBookField( Plugin *p, const QString &app, const QString &key, const QString &value );

	/**
	 * @brief The status message of metacontact
	 */
	StatusMessage statusMessage() const;

public slots:

	/**
	 * @brief Send a file to this metacontact
	 *
	 * This is the MetaContact level slot for sending files. It may be called through the
	 * "Send File" entry in the GUI, or over DCOP. If the function is called through the GUI,
	 * no parameters are sent and they assume default values. This slot calls the slotSendFile
	 * with identical params of the highest ranked contact capable of sending files (if any)
	 *
	 * @param sourceURL The actual KUrl of the file you are sending
	 * @param altFileName (Optional) An alternate name for the file - what the receiver will see
	 * @param fileSize (Optional) Size of the file being sent. Used when sending a nondeterminate
	 *                file size (such as over a socket)
	 *
	 */
	void sendFile( const KUrl &sourceURL, const QString &altFileName = QString(),
		unsigned long fileSize = 0L );

	/**
	 * Serialize this metaContact
	 * This causes each Kopete::Protocol subclass to serialise its contacts' data into the metacontact's plugin data
	 */
	void serialize();

signals:
	/**
	 * One of the subcontacts' idle status has changed.  As with online status,
	 * this can occur without the metacontact changing idle state
	 */
	void contactIdleStateChanged( Kopete::Contact *contact );

	/**
	 * Status title or message has changed
	 */
	void statusMessageChanged( Kopete::MetaContact *metaContact );

public slots:

	/**
	 * @brief Move a contact from one group to another.
	 */
	void moveToGroup( Kopete::Group *from, Kopete::Group *to );

	/**
	 * @brief Remove a contact from one group
	 */
	void removeFromGroup( Kopete::Group *from );

	/**
	 * @brief Add a contact to another group.
	 */
	void addToGroup( Kopete::Group *to );

	/**
	 * @brief Set if this is a temporary contact. (see @ref isTemporary)
	 *
	 * @param b if the contact is or not temporary
	 * @param group if the contact was temporary and b is false, then the contact will be moved to this group.
	 *  if group is null, it will be moved to top-level
	 */
	void setTemporary( bool b = true, Kopete::Group *group = 0L );

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

	/**
	 * When all the plugins are loaded, set the Contact Source.
	 */
	void slotAllPluginsLoaded();

	/**
	 * If a plugin is loaded, maybe data about this plugin are already cached in the metacontact
	 */
	void slotPluginLoaded( Kopete::Plugin *plugin );

	/**
	 * If a protocol is loaded, deserialize cached data
	 */
	void slotProtocolLoaded( Kopete::Protocol *p );

signals:
	/**
	 *  @brief The MetaContact online status changed
	 */
	void onlineStatusChanged( Kopete::MetaContact *contact, Kopete::OnlineStatus::StatusType status );

	/**
	 * @brief A contact's online status changed
	 *
	 * this signal differs from @ref onlineStatusChanged because a contact can
	 * change his status without changing MetaContact status. It is mainly used to update the small icons
	 * in the contact list
	 */
	void contactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status );

	/**
	 * @brief The meta contact's display name changed
	 */
	void displayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * @brief The meta contact's photo changed
	 */
	void photoChanged();

	/**
	 * @brief  The contact was moved
	 */
	void movedToGroup( Kopete::MetaContact *contact, Kopete::Group *from, Kopete::Group *to );

	/**
	 * @brief The contact was removed from group
	 */
	void removedFromGroup( Kopete::MetaContact *contact, Kopete::Group *group );

	/**
	 * @brief The contact was added to another group
	 */
	void addedToGroup( Kopete::MetaContact *contact, Kopete::Group *to );

	/**
	 * @brief a contact has been added into this metacontact
	 *
	 * This signal is emitted when a contact is added to this metacontact
	 */
	void contactAdded( Kopete::Contact *c );

	/**
	 * @brief a contact has been removed from this metacontact
	 *
	 * This signal is emitted when a contact is removed from this metacontact
	 */
	void contactRemoved( Kopete::Contact *c );

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
	void slotContactStatusChanged( Kopete::Contact *c, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus );

	/**
	 * One of the child contact's property changed
	 */
	void slotPropertyChanged( Kopete::PropertyContainer *contact, const QString &key, const QVariant &oldValue, const QVariant &newValue  );

	void slotContactDisplayNameChanged( const QString &oldName, const QString &newName );

	/**
	 * A child contact was deleted, remove it from the list, if it's still
	 * there
	 */
	void slotContactDestroyed( Kopete::Contact* );

	/**
	 * Update the KABC Picture when the addressbook is changed.
	 */
	void slotUpdateAddressBookPicture();

protected:
	//QImage photoFromContact( Kopete::Contact *c) const;
	//QImage photoFromKABC( const QString &id ) const;
	QImage photoFromCustom() const;
	//QString nameFromContact( Kopete::Contact *c) const;
	//QString nameFromKABC( const QString &id ) const;

	void onlineStatusNotification( Kopete::Contact * c );

private:
	class Private;
	Private * const d;
};

// util functions shared with metacontact property dialog
KOPETE_EXPORT QImage photoFromContact( Kopete::Contact *c) /*const*/;
KOPETE_EXPORT QImage photoFromKABC( const QString &id ) /*const*/;
KOPETE_EXPORT QString nameFromContact( Kopete::Contact *c) /*const*/;
KOPETE_EXPORT QString nameFromKABC( const QString &id ) /*const*/;

} //END namespace Kopete


#endif

// vim: set noet ts=4 sts=4 sw=4:

