/*
    kopeteaccount.h - Kopete Account

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2003-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEACCOUNT_H
#define KOPETEACCOUNT_H

#include "kopeteonlinestatus.h"
#include "kopetestatusmessage.h"
#include "kopete_export.h"

#include <QtCore/QObject>
#include <QtGui/QPixmap>
#include <QtCore/QHash>

#include <kconfiggroup.h>
#include <kiconloader.h>
#include <solid/networking.h>

class KActionMenu;
class KConfigGroup;

namespace Kopete
{
class Contact;
class Protocol;
class MetaContact;
class Group;
class OnlineStatus;
class BlackLister;
class StatusMessage;
class Identity;
class PropertyContainer;

/**
 * The Kopete::Account class handles one account.
 * Each protocol implementation should subclass this class in its own custom account class.
 * There are a few pure virtual methods that must be implemented. Examples are:
 * \li \ref connect()
 * \li \ref disconnect()
 * \li \ref createContact()
 *
 * The accountId is an @em constant unique id, which represents the login.
 * The @ref myself() contact is one of the most important contacts, which represents
 * the user tied to this account. You must create this contact in the contructor of your
 * account and pass it to @ref setMyself().
 *
 * All account data is saved to @ref KConfig. This includes the accountId, the autoconnect flag and
 * the color. You can save more data using @ref configGroup()
 *
 * When you create a new account, you have to register it with the account manager by calling
 * @ref AccountManager::registerAccount.
 *
 * @author Olivier Goffart  <ogoffart\@kde.org>
 */
class KOPETE_EXPORT Account : public QObject
{
	Q_OBJECT

	Q_ENUMS( AddMode )
	Q_PROPERTY( QString accountId READ accountId )
	Q_PROPERTY( bool excludeConnect READ excludeConnect WRITE setExcludeConnect )
	Q_PROPERTY( QColor color READ color WRITE setColor )
	Q_PROPERTY( QPixmap accountIcon READ accountIcon )
	Q_PROPERTY( bool isConnected READ isConnected )
	Q_PROPERTY( bool isAway READ isAway )
	Q_PROPERTY( bool suppressStatusNotification READ suppressStatusNotification )
	Q_PROPERTY( uint priority READ priority WRITE setPriority )

public:
	/**
	 * \brief Describes how the account was disconnected
	 *
	 * Manual means that the disconnection was done by the user and no reconnection
	 * will take place. Any other value will reconnect the account on disconnection.
	 * The case where the password is wrong will be handled differently.
	 * @see @ref disconnected 
	 */
	enum DisconnectReason {
		OtherClient = -4,    ///< connection went down because another client connected the same account
		BadPassword = -3,    ///< connection failed because password was incorrect
		BadUserName = -2,    ///< connection failed because user name was invalid / unknown
		InvalidHost = -1,    ///< connection failed because host is unreachable
		Manual = 0,          ///< the user disconnected normally
		ConnectionReset = 1, ///< the connection was lost
		Unknown = 99         ///< the reason for disconnection is unknown
	};

	/**
	 * \brief Describes what should be done when the contact is added to a metacontact
	 * @sa @ref addContact()
	 */
	enum AddMode {
		ChangeKABC = 0,     ///< The KDE Address book may be updated
		DontChangeKABC = 1, ///< The KDE Address book will not be changed
		Temporary = 2       ///< The contact will not be added on the contact list
	};

	/**
	 * \brief Describes what should be done when we set new OnlineStatus
	 * @sa @ref setOnlineStatus()
	 */
	enum OnlineStatusOption {
		None = 0x00,               ///< Use the online status
		KeepSpecialFlags = 0x01    ///< Use the online status but keep special flags, e.g. Invisible
	};
	Q_DECLARE_FLAGS(OnlineStatusOptions, OnlineStatusOption)

	/**
	 * Constructor for the Account object.
	 * 
	 * @param parent the protocol for this account. The account is a child object of the
	 * protocol, so it will be automatically deleted when the protocol is.
	 * @param accountID the unique ID of this account.
	 * @param name the name of this QObject.
	 */
	Account(Protocol *parent, const QString &accountID);

	/**
	 * Destroy the Account object.
	 */
	~Account();

	/**
	 * \return the Protocol for this account
	 */
	Protocol *protocol() const ;

	/**
	 * \return the unique ID of this account used as the login
	 */
	QString accountId() const;

	/**
	 * The label for this account.
	 *
	 * This is used in the GUI.
	 * \return The label of this account
	 */
	QString accountLabel() const;

	/**
	 * \brief Get the priority of this account.
	 *
	 * Used for sorting and determining the preferred account to message a contact.
	 */
	uint priority() const;

	/**
	 * \brief Set the priority of this account.
	 *
	 * @note This method is called by the UI, and should not be called elsewhere.
	 */
	void setPriority( uint priority );

	/**
	 * \brief Set if the account should not log in automatically.
	 *
	 * This function can be used by the EditAccountPage. Kopete handles connection automatically.
	 * @sa @ref excludeConnect
	 */
	void setExcludeConnect(bool);

	/**
	 * \brief Get if the account should not log in.
	 *
 	 * @return @c true if the account should not be connected when connectAll at startup, @c false otherwise.
	 */
	bool excludeConnect() const;

	/**
	 * \brief Get the color for this account.
	 *
 	 * The color will be used to visually differentiate this account from other accounts on the
	 * same protocol.
	 *
	 * \return the user color for this account
	 */
	const QColor color() const;

	/**
	 * \brief Set the color for this account.
	 *
	 * This is called by Kopete's account config page; you don't have to set the color yourself.
	 *
	 * @sa @ref color()
	 */
	void setColor( const QColor &color);

	/**
	 * \brief Get the icon for this account.
	 *
	 * Generates an image of size @p size representing this account. The result is not cached.
	 *
	 * @param size the size of the icon. If the size is 0, the default size is used.
	 * @return the icon for this account, colored if needed
	 */
	QPixmap accountIcon( const int size = 0 ) const;

	/*
	 * \brief Get the icon path for this account.
	 *
	 * This is the account icon path.
	 *
	 * @return the icon path for this account.
	 */
	QString accountIconPath(const KIconLoader::Group size) const;

	/**
	 * \brief change the account icon.
	 * by default the icon of an account is the protocol one, but it may be overide it.
	 * Set QString() to go back to the default  (the protocol icon)
	 * 
	 * this call will emit colorChanged()
	 */
	void setCustomIcon( const QString& );
	
	/**
	 * \brief return the icon base
	 * This is the custom account icon set with setIcon.   if this icon is null, then the protocol icon is used
	 * don't use this function to get the icon that need to be displayed, use accountIcon
	 */
	QString customIcon() const;
	
	/**
	 * \brief Retrieve the identity this account belongs to
	 *
	 * \return a pointer to the Identity object this account belongs to.
	 *
	 * \see setIdentity().
	 */
	Identity * identity() const;

	/**
	 * \brief Sets the identity this account belongs to
	 *
	 * Setting the account to a new identity implies it to be removed from the 
	 * identity it was previously associated.
	 *
	 * @param ident The identity this account should be associated to
	 * \return @c true if the identity was changed, @c false otherwise
	 *
	 * @note You should call the default implementation from your reimplementation
	 */
	virtual bool setIdentity( Kopete::Identity *ident );

	/**
	 * \brief Retrieve the 'myself' contact.
	 *
	 * \return a pointer to the Contact object for this account
	 *
	 * \see setMyself().
	 */
	Contact * myself() const;

	/**
	 * @brief Fill the menu with actions for this account
	 *
	 * You have to reimplement this method to add custom actions to the @p actionMenu which will
	 * be shown in the statusbar. It is the caller's responsibility to ensure the menu is deleted.
	 *
	 * The default implementation provides a generic menu, with actions generated from the protocol's
	 * registered statuses, and an action to show the account's settings dialog.
	 *
	 * You should call the default implementation from your reimplementation, and add more actions
	 * you require to the resulting action menu.
	 *
	 * @see OnlineStatusManager::registerOnlineStatus
	 */
	virtual void fillActionMenu( KActionMenu *actionMenu );

	/**
	 * @brief Return true if account has custom status menu.
	 *
	 * You have to reimplement this method and return true if you don't want to have status menu in menu
	 * which will be shown in the statusbar
	 *
	 * The default implementation returns false.
	 */
	virtual bool hasCustomStatusMenu() const;

	/**
	 * @brief Retrieve the list of contacts for this account (except myself contact)
	 *
	 * The list is guaranteed to contain only contacts for this account,
	 * so you can safely use static_cast to your own derived contact class
	 * if needed.
	 */
	const QHash<QString,Contact*>& contacts();

	/**
	 * Indicates whether or not we should suppress status notifications
	 * for contacts belonging to this account.
	 *
	 * This is used when we just connected or disconnected, and every contact has their initial
	 * status set.
	 *
	 * @return @c true if notifications should not be used, @c false otherwise
	 */
	bool suppressStatusNotification() const;

	/**
	 * \brief Create a contact (creating a new metacontact if necessary)
	 *
	 * If a contact for this account with ID @p contactId is not already on the contact list,
	 * a new contact with that ID is created, and added to a new metacontact.
	 *
	 * If @p mode is @c ChangeKABC, MetaContact::updateKABC will be called on the resulting metacontact.
	 * If @p mode is @c Temporary, MetaContact::setTemporary will be called on the resulting metacontact,
	 * and the metacontact will not be added to @p group.
	 * If @p mode is @c DontChangeKABC, no additional action is carried out.
	 *
	 * @param contactId the @ref Contact::contactId of the contact to create
	 * @param displayName the displayname (alias) of the new metacontact. Leave as QString() if
	 *                    no alias is known, then by default, the nick will be taken as alias and tracked if changed.
	 * @param group the group to add the created metacontact to, or 0 for the top-level group.
	 * @param mode the mode used to add the contact. Use DontChangeKABC when deserializing.
	 * @return the new created metacontact or 0L if the operation failed
	 */
	MetaContact* addContact( const QString &contactId, const QString &displayName = QString(), Group *group = 0, AddMode mode = DontChangeKABC );

	/**
	 * @brief Create a new contact, adding it to an existing metacontact
	 *
	 * If a contact for this account with ID @p contactId is not already on the contact list,
	 * a new contact with that ID is created, and added to the metacontact @p parent.
	 *
	 * @param contactId the @ref Contact::contactId of the contact to create
	 * @param parent the parent metacontact (must not be 0)
	 * @param mode the mode used to add the contact. See addContact(const QString&,const QString&,Group*,AddMode) for details.
	 *
	 * @return @c true if creation of the contact succeeded or the contact was already in the list,
	 *         @c false otherwise.
	 */
	bool addContact( const QString &contactId, MetaContact *parent, AddMode mode = DontChangeKABC );

	/**
	 * @brief Indicate whether the account is connected at all.
	 *
	 * This is a convenience method that calls @ref Contact::isOnline() on @ref myself().
	 * This function is safe to call if @ref setMyself() has not been called yet.
	 *
	 * @see @ref isConnectedChanged()
	 */
	bool isConnected() const;

	/**
	 * @brief Indicate whether the account is away.
	 *
	 * This is a convenience method that queries @ref Contact::onlineStatus() on @ref myself().
	 * This function is safe to call if @ref setMyself() has not been called yet.
	 *
	 * @see @ref isBusy()
	 */
	bool isAway() const;

	/**
	 * @brief Indicate whether the account is busy.
	 *
	 * In busy mode all visible and sound events should be disabled.
	 *
	 * This is a convenience method that queries @ref Contact::onlineStatus() on @ref myself().
	 * This function is safe to call if @ref setMyself() has not been called yet.
	 *
	 * @see @ref isAway()
	 */
	bool isBusy() const;

	/**
	 * Return the @ref KConfigGroup used to write and read special properties
	 *
	 * "Protocol", "AccountId" , "Color", "AutoConnect", "Priority", "Enabled" , "Icon" are reserved keyword
	 * already in use in that group.
	 * 
	 * for compatibility, try to not use key that start with a uppercase
	 */
	KConfigGroup *configGroup() const;

	/**
	 * @brief Remove the account from the server.
	 *
	 * Reimplement this if your protocol supports removing the accounts from the server.
	 * This function is called by @ref AccountManager::removeAccount typically when you remove the
	 * account on the account config page.
	 *
	 * You should add a confirmation message box before removing the account. The default
	 * implementation does nothing.
	 *
	 * @return @c false only if the user requested for the account to be deleted, and deleting the
	 * account failed. Returns @c true in all other cases.
	 */
	virtual bool removeAccount();

	/**
	 * \return a pointer to the blacklist of the account
	 * @todo remove or implement correctly (BlackLister)
	 */
	BlackLister* blackLister();

	/**
	 * \return @c true if the contact with ID @p contactId is in the blacklist, @c false otherwise.
	 * @todo remove or implement correctly (BlackLister)
	 */
	virtual bool isBlocked( const QString &contactId );

protected:
	/**
	 * \brief Set the 'myself' contact.
	 *
	 * This contact must be defined for every account, because it holds the online status
	 * of the account. You must call this function in the constructor of your account.
	 *
	 * The myself contact can't be deleted as long as the account still exists. The myself
	 * contact is used as a member of every ChatSession involving this account. myself's
	 * contactId should be the accountID. The online status of the myself contact represents
	 * the account's status.
	 * 
	 * The myself should have the @ref ContactList::myself() as parent metacontact
	 * 
	 */
	void setMyself( Contact *myself );

	/**
	 * \brief Create a new contact in the specified metacontact
	 *
	 * You shouldn't ever call this method yourself. To add contacts, use @ref addContact().
	 *
	 * This method is called by @ref addContact(). In this method, you should create the
	 * new custom @ref Contact, using @p parentContact as the parent.
	 *
	 * If the metacontact is not temporary and the protocol supports it, you can add the
	 * contact to the server.
	 *
	 * @param contactId the ID of the contact to create
	 * @param parentContact the metacontact to add this contact to
	 * @return @c true if creating the contact succeeded, @c false on failure.
	 */
	virtual bool createContact( const QString &contactId, MetaContact *parentContact ) =0;


	/**
	 * \brief Sets the account label
	 *
	 * @param label The label to set
	 */
	void setAccountLabel( const QString &label );

protected slots:
	/**
	 * \brief The service has been disconnected
	 *
	 * You have to call this method when you are disconnected. Depending on the value of
	 * @p reason, this function may attempt to reconnect to the server.
	 *
	 * - BadPassword will ask again for the password
	 * - OtherClient will show a message box
	 *
	 * @param reason the reason for the disconnection.
	 */
	virtual void disconnected( Kopete::Account::DisconnectReason reason );

	/**
	 * @brief Sets the online status of all contacts in this account to the same value
	 *
	 * Some protocols do not provide status-changed events for all contacts when an account
	 * becomes connected or disconnected. For such protocols, this function may be useful
	 * to set all contacts offline.
	 *
	 * Calls @ref Kopete::Contact::setOnlineStatus on all contacts of this account (except the
	 * @ref myself() contact), passing @p status as the status.
	 *
	 * @param status the status to set all contacts of this account except @ref myself() to.
	 */
	void setAllContactsStatus( const Kopete::OnlineStatus &status );

        /**
	 * @brief React to network status changes
	 *
	 * @param status the new network status.
	 */
	void networkingStatusChanged( const Solid::Networking::Status status );

signals:
	/**
	 * The color of the account has been changed
	 * 
	 * also emitted when the icon change
	 * @todo  probably rename to accountIconChanged
	 */
	void colorChanged( const QColor & );

	/**
	 * Emitted when the account is deleted.
	 * @warning emitted in the Account destructor. It is not safe to call any functions on @p account.
	 */
	void accountDestroyed( const Kopete::Account *account );

	/**
	 * Emitted whenever @ref isConnected() changes.
	 */
	void isConnectedChanged();

private:
	/**
	 * @internal
	 * Reads the configuration information of the account from KConfig.
	 */
	void readConfig();

public:
	/**
	 * @internal
	 * Register a new Contact with the account. This should be called @em only from the
	 * @ref Contact constructor, not from anywhere else (not even a derived class).
	 */
	bool registerContact( Contact *c );

public slots:
	/**
	 * @brief Go online for this service.
	 *
	 * @param initialStatus is the status to connect with. If it is an invalid status for this
	 * account, the default online for the account should be used.
	 * @todo probably deprecate in favor of setOnlineStatus
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = OnlineStatus() ) = 0;

	/**
	 * @brief Go offline for this service.
	 *
	 * If the service is connecting, you should abort the connection.
	 *
	 * You should call the @ref disconnected function from this function.
	 * @todo  probably deprecate in favor of setOnlineStatus
	 */
	virtual void disconnect( ) = 0 ;

public slots:
	/**
	 * Reimplement this function to set the online status
	 * @param status is the new status
	 * @param reason is the status message to set.
	 * @param options specify how the status should be set
	 * @note If needed, you need to connect.  if the offline status is given, you should disconnect
	 */
	virtual void setOnlineStatus( const Kopete::OnlineStatus& status,
	                              const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                              const OnlineStatusOptions& options = None ) = 0;
	/**
	 * Reimplement this function to set the status message(with metadata).
	 * You should use this method to set the status message instead of using setOnlineStatus.
	 * @param statusMessage is the status message to set. (Use Kopete::StatusMessage).
	 */
	virtual void setStatusMessage( const Kopete::StatusMessage &statusMessage ) = 0;

	/**
	 * Disconnects account, required before resume()
	 * Returns false if account is already suspended.
	 */
	bool suspend( const Kopete::StatusMessage &reason = Kopete::StatusMessage() );

	/**
	 * Sets account to the online status that was active when suspend() was called.
	 * Returns false if account has not been suspended or status has changed to something other than Offline in the meantime.
	 */
	bool resume();

	/**
	 * Display the edit account widget for the account
	 */
	void editAccount( QWidget* parent = 0L );

	/**
	 * Add a user to the blacklist. The default implementation calls
	 * blackList()->addContact( contactId )
	 *
	 * @param contactId the contact to be added to the blacklist
	 * @todo remove or implement correctly (BlackLister)
	 */
	virtual void block( const QString &contactId );

	/**
	 * Remove a user from the blacklist. The default implementation calls
	 * blackList()->removeContact( contactId )
	 *
	 * @param contactId the contact to be removed from the blacklist
	 * @todo remove or implement correctly (BlackLister)
	 */
	virtual void unblock( const QString &contactId );

private slots:
	/**
	 * Restore online status and status message on reconnect.
	 */
	void reconnect();

	/**
	 * Track the deletion of a Contact and clean up
	 */
	void contactDestroyed( Kopete::Contact * );

	/**
	 * The @ref myself() contact's online status changed.
	 */
	void slotOnlineStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus );

	/**
	 * The @ref myself() contact's property changed.
	 */
	void slotContactPropertyChanged( Kopete::PropertyContainer *, const QString &, const QVariant &, const QVariant & );

	/**
	 * Stop the suppression of status notification (connected to a timer)
	 */
	void slotStopSuppression();

private:
	class Private;
	Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Account::OnlineStatusOptions)

} //END namespace Kopete

#endif

