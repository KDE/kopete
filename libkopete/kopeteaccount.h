/*
    kopeteaccount.h - Kopete Account

    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@ tiscalinet.be>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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

#include <qobject.h>
#include <kdemacros.h>
#include <qdict.h>

#include "kopeteonlinestatus.h"


#include <kconfig.h> //TODO: remove
#include <kinputdialog.h> //TODO: remove

class QDomNode;
class KActionMenu;
class KConfigGroup;


namespace Kopete
{
class Contact;
class Plugin;
class Protocol;
class MetaContact;
class Group;
class OnlineStatus;
class BlackLister;

/**
 * @author Olivier Goffart  <ogoffart @tiscalinet.be>
 *
 * The Kopete::Account class handles one account.
 * Each protocol should subclass this class in their own custom accounts class.
 * There are few pure virtual method that the protocol must implement. Examples are:
 * \li \ref connect()
 * \li \ref disconnect()
 * \li \ref createContact()
 *
 * But if your account may require a password, reimplement @ref PasswordedAccount which handle passwords
 *
 * The accountId is an <b>constant</b> unique id, which represents the login.
 * The @ref myself() contact is one of the most important contacts, which represents
 * the user tied to this account. You must create this contact in the contructor of your
 * account and use @ref setMyself()
 *
 * All account data is saved to @ref KConfig. This includes the accountId, the autoconnect flag, the color.
 * You can save more data using @ref configGroup()
 *
 * When you create a new account, you have to register it in the account manager with @ref AccountManager::registerAccount
 *
 */
class KOPETE_EXPORT Account : public QObject
{
	Q_OBJECT

	Q_PROPERTY( QString accountId READ accountId )
	Q_PROPERTY( bool autoConnect READ autoConnect WRITE setAutoConnect )
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
		BadUserName = -2,	/** impossible to connect because some informations (Login, Password, ...) were incorrect*/
		InvalidHost = -1,	/** impossible to connect because host is unreachable */
		Manual = 0,			/** the user disconnected normaly */
		ConnectionReset = 1, /** the connection has been lost */
		Unknown = 99 };



	/**
	 * constructor:
	 * The constructor register automatically the account to the @ref AccountManager
	 * @param parent it the protocol of this account. the accoun is a child object of the
	 * protocol, so it will be automatically deleted with the parent.
	 * @param accountID is the id of this protocol, it shouln't be changed after
	 * @param name is the QObject name. it can be 0L
	 */
	Account(Protocol *parent, const QString &accountID, const char *name=0L);
	~Account();

	/**
	 * \return the Protocol for this account
	 */
	Protocol *protocol() const ;

	/**
	 * \return the unique id of this account used as the login
	 */
	QString accountId() const;


	/**
	 * \brief Get the priority of this account.
	 *
	 * Used for sorting and determining the preferred account to message a contact
	 */
	uint priority() const;

	/**
	 * \brief Set the priority of this account.
	 *
	 * This method is called by the UI, and should not be called elsewhere.
	 */
	void setPriority( uint priority );

	/**
	 * \brief Set if the account should log in automatically.
	 *
	 * This function can be used by the EditAccountPage
	 * Kopete handles the autoconnection automatically
	 * @sa @ref autoConnect
	 */
	void setAutoConnect(bool);

	/**
	 * \brief Get if the account should log in automatically.
	 *
 	 * Say if yes or no the account should be connected with the connect all button.
	 */
	bool autoConnect() const;

	/**
	 * \brief Get the color for this account.
 	 * The color will be used to differentiate this account from the other accounts
	 * \return the user color for this account
	 */
	const QColor color() const;

	/**
	 * \brief Set the color for this account.
	 * Normally, this should be called by Kopete's account config page so you
	 * don't have to set the color yourself
	 * @sa @ref color()
	 */
	void setColor( const QColor &color);

	/**
	 * \brief Get the icon for this account.
	 *
	 * The icon is not cached.
	 * \return the icon for this account, colored if needed
	 * @param size is the size of the icon.  If the size is 0, the default size is used
	 *
	 */
	QPixmap accountIcon(const int size=0) const;


	/**
	 * \brief Retrieve the 'myself' contact.
	 *
	 * \return the pointer to the Contact object for this account
	 *
	 * \see setMyself().
	 */
	Contact * myself() const;

	/**
	 * @brief Return the menu for this account
	 *
	 * You have to reimplement this method to return the custom action menu which will
	 * be shown in the statusbar. Kopete takes care of the deletion of the menu.
	 */
	virtual KActionMenu* actionMenu() ;

	/**
	 * @brief Retrieve the list of contacts for this account
	 *
	 * The list is guaranteed to contain only contacts for this account,
	 * so you can safely use static_cast to your own derived contact class
	 * if needed.
	 */
	const QDict<Contact>& contacts();


	/**
	 * Indicates whether or not we should suppress status notifications
	 * for contacts belonging to this account.
	 *
	 * This is when we just connected or disconnected, and every contact get their initial status.
	 *
	 * \return true if notifications should not be used, false if
	 * notifications should be used
	 */
	bool suppressStatusNotification() const;


	/**
	 * \brief Describes what should be done when the contact is added to a metacontact
	 * @sa @ref addContact()
	 * @sa @ref addMetaContact()
	 */
	enum AddMode { ChangeKABC=0, 	/** The KDE Address book may be updated */
				   DontChangeKABC=1 /** The KDE Address book will not be changed */,
				   Temporary=2  	/** The contact will not be added on the contactlist */ };

	/**
	 * \brief add a contact to a new metacontact
	 *
	 * This method will add a new metacontact containing only the contact.
	 * It will take care the contact isn't already on the contactlist.
	 * if it is already, the contact is not created, but the metacontact containing the contact is returned
	 *
	 * @param contactId The @ref Contact::contactId
	 * @param displayName The displayname (alias) of the new metacontact. Let empty if not applicable.
	 * @param group the group to add the contact. if NULL, it will be added to toplevel
	 * @param mode If the KDE addressbook should be changed to include the new contact. Don't change if you are using this method to deserialise.
	 *   if Temporary, @p groups is not used
	 * @return the new metacontact or 0L if no contact was created because of error.
	 */
	MetaContact *addMetaContact( const QString &contactId, const QString &displayName = QString::null, Group *group=0L, AddMode mode = DontChangeKABC ) ;

	/**
	 * @brief add a contact to an existing metacontact
	 *
	 * This method will check if the contact is not already in the contactlist, and if not, will add a contact
	 * to the given metacontact.
	 *
	 * @param contactId The @ref Contact::contactId of the contact
	 * @param parent The metaContact parent (must exists)
	 * @param mode If the KDE address book should be updated. see @ref AddMode.
	 *
	 */
	bool addContact(const QString &contactId , MetaContact *parent, AddMode mode = DontChangeKABC );

	/**
	 * @brief Indicate whether the account is connected at all.
	 *
	 * This is a convenience method that queries @ref Contact::onlineStatus()
	 * on @ref myself()
	 */
	bool isConnected() const;


	/**
	 * @brief Indicate whether the account is away.
	 *
	 * This is a convenience method that queries @ref Contact::onlineStatus()
	 * on @ref myself()
	 */
	bool isAway() const;


	/**
	  * Return the @ref KConfigGroup used to write and read special properties
	  *
	  * "Protocol", "AccountId" , "Color", "AutoConnect", "Priority", "Enabled" are reserved keyword already in use in that group
	  */
	KConfigGroup *configGroup() const;

	/**
	 * @brief remove the account on the server.
	 * reimplement it if your protocol support removing the removing of the account on the server.
	 * that function is called by @ref AccountManager::removeAccount tipicaly when you remove the
	 * account on the account config page.
	 *
	 * You probably should add a confirmation message box before removing the account.
	 *
	 * the default implementation do nothing
	 *
	 * @return false only if the account deletion need to be canceled,  return true in other cases
	 */
	virtual bool removeAccount();

	/**
	 * \return a pointer to the blacklist of the account
	 */
	BlackLister* blackLister();

	/**
	 * \return @c true if the contact with ID @p contactId is in the blacklist, @c false otherwise
	 */
	virtual bool isBlocked( const QString &contactId );

protected:
	/**
	 * \brief Set the 'myself' contact.
	 *
	 * This contact HAS to be defined for every  account, because it
	 * holds the online status of an account!
	 * You must call this function in the constructor of your account
	 *
	 * The myself contact can't be deleted as long as the account still
	 * exists. The myself contact is used in each ChatSession,
	 * the myself contactId can be the accountID, the onlineStatus
	 * should represent the current user's status. The statusbar icon
	 * is connected to @p myself's @ref Kopete::Contact::onlineStatusChanged()
	 * to update the icon.
	 */
	void setMyself( Contact *myself );

	/**
	 * \brief Create a new contact in the specified metacontact
	 *
	 * You shouldn't ever call this method yourself, For adding contacts see @ref addContact()
	 *
	 * This method is called by @ref Account::addContact(). In this method, you should
	 * simply create the new custom @ref Contact in the given metacontact.
	 * If the metacontact is not temporary, You can add the contact to the server if the protocol supports it
	 *
	 * @param contactId The unique ID for this protocol
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool createContact( const QString &contactId, MetaContact *parentContact ) =0;

protected slots:

	/**
	 * \brief the service has been disconnected
	 *
	 * You have to call this method when you are disconnected.
	 * @param reason if the reason is positive, Kopete will try to reconnect the account
	 */
	virtual void disconnected( DisconnectReason reason );


signals:

	/**
	 * The color of the account has been changed
	 */
	void colorChanged( const QColor & );

	/**
	 * The account is going to be deleted
	 * @warning emitted in the Account destructor, any virtual fucntion may not anymore be called
	 */
	void accountDestroyed( const Kopete::Account* );


private:


	/**
	 * \brief Read the account's configuration
	 *
	 * Uses KConfig to read the configuration for this account
	 *
	 * @internal
	 */
	void readConfig(  );

public:
	/**
	 * \internal
	 * Register a new Contact with the account
	 * To be called <b>only</b> from @ref Contact constructor
	 * not from any other class! (Not even a derived class).
	 */
	void registerContact( Contact *c );



public slots:
	/**
	 * @brief Go online for this service.
	 *
	 * @param initialStatus is the status to use initialy.  If it is an invalid status, the default online for that protocol should be used
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = OnlineStatus() ) =0;

	/**
	 * @brief Go offline for this service.
	 *
	 * If the service is connecting, you should abort the connection
	 *
	 * you should call the @ref diconnected function from this function.
	 */
	virtual void disconnect( ) = 0 ;



public slots:
	/**
	 * this will be called if main-kopete wants
	 * the plugin to set the user's mode to away
	 * @todo change
	 */
	virtual void setAway( bool away, const QString &reason = QString::null ) = 0;

	/**
	 * Add a user to the blacklist. The default implementation calls
	 * blackList()->addContact( contactId )
	 *
	 * @param contactId the contact to be added to the blacklist
	 */
	virtual void block( const QString &contactId );

	/**
	 * Remove a user from the blacklist. The default implementation calls
	 * blackList()->removeContact( contactId )
	 *
	 * @param contactId the contact to be removed from the blacklist
	 */
	virtual void unblock( const QString &contactId );


	/**
	 * @deprecated   place everithing in the constructor
	 * @todo remove
	 */
	virtual void loaded() {};


private slots:
	/**
	 * Track the deletion of a Contact and cleanup
	 */
	void slotContactDestroyed( Kopete::Contact * );

	/**
	 * Our online status changed.
	 *
	 * Currently this slot is used to set a timer that allows suppressing online status
	 * notifications just after connecting.
	 */
	void slotOnlineStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus );

	/**
	 * Stop the suppression of status notification
	 * (connected on the timer)
	 */
	void slotStopSuppression();

private:
	class Private;
	Private *d;

protected:
	virtual void virtual_hook( uint id, void* data);


public:
//MOC_SKIP_BEGIN//
//(moc doesn't like the KDE_DEPRECATED macro)

	/**
	 * @todo remove
	 * @deprecated  use configGroup
	 */
	KDE_DEPRECATED void setPluginData( Plugin* /*plugin*/, const QString &key, const QString &value )
		{  configGroup()->writeEntry(key,value);  }


	/**
	 * @todo remove
	 * @deprecated  use configGroup
	 */
	KDE_DEPRECATED QString pluginData( Plugin* /*plugin*/, const QString &key ) const
		{ return configGroup()->readEntry(key); }
//MOC_SKIP_END//
};

} //END namespace Kopete

#endif

