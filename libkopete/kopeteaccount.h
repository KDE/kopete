/*
    kopeteaccount.h - Kopete Account

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
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

#include "kopeteplugindataobject.h"
#include <qdict.h>

class QDomNode;
class KActionMenu;

class KopeteContact;
class KopetePlugin;
class KopeteProtocol;
class KopeteMetaContact;
class KopeteOnlineStatus;

struct KopeteAccountPrivate;

/**
 * @author Olivier Goffart  <ogoffart@tiscalinet.be>
 *
 * The KopeteAccount class handle one account.
 * Each protocol should subclass this class in their own customAccounts class.
 * there are few pure virtual method than the protocol must implement. like
 * @ref myself()  @ref connect()  @ref disconnect() or @ref addContactToMetaContact()
 *
 * The accountId is an *constant* unique id. which represent the login.
 * The @ref myself() contact one of the most importent contact, which represent the user
 *
 * All account data is automatically saved to @ref KConfig. This includes the
 * accountId, the password (in a encrypted format) , the autoconnect flag, the color,
 * and all PluginData. KopeteAccount is a @ref KopetePluginDataObject. in the account,
 * you can call setPluginData( protocol() , "key" , "value") and pluginData( protocol , "key")
 * see @ref KopetePluginDataObject::setPluginData() and KopetePluginDataObject::pluginData()
 * note than plugin data are not yet availiable in the account constructor. they are
 * only availiable after the xml file has been totaly parsed. You can reimplement
 * @ref KopeteAccount::loaded() to do what you have to do right after the xml file is
 * loaded. in the same way, you can't set pluginData in the destructor, because the
 * xml file has already been writed, and new changes will not be updated on the disk.
 */
class KopeteAccount : public KopetePluginDataObject
{
	Q_OBJECT

	Q_PROPERTY( QString accountId READ accountId )
	Q_PROPERTY( QString password READ password WRITE setPassword )
	Q_PROPERTY( bool rememberPassword READ rememberPassword )
	Q_PROPERTY( bool autoLogin READ autoLogin WRITE setAutoLogin )
	Q_PROPERTY( QColor color READ color WRITE setColor )
	Q_PROPERTY( QPixmap accountIcon READ accountIcon )
	Q_PROPERTY( bool isConnected READ isConnected )
	Q_PROPERTY( bool isAway READ isAway )
	Q_PROPERTY( bool suppressStatusNotification READ suppressStatusNotification )
	Q_PROPERTY( uint priority READ priority WRITE setPriority )

public:
	/**
	 * \brief Describes what should be done when the contact is added to a metacontact
	 */
	enum AddMode { ChangeKABC=0, DontChangeKABC=1 };
	
	/**
	 * constructor:
	 * The constructor register automatically the account to the @ref KopeteAccountManager
	 * @param parent it the protocol of this account. the accoun is a child object of the
	 * protocol, so it will be automatically deleted with the parent.
	 * @param accountID is the id of this protocol, it shouln't be changed after
	 * @param name is the QObject name. it can be 0L
	 */
	KopeteAccount(KopeteProtocol *parent, const QString &accountID, const char *name=0L);
	~KopeteAccount();

	/**
	 * return the KopeteProtocol for this account
	 */
	KopeteProtocol *protocol() const ;

	/**
	 * return the unique id of this account used as the login
	 */
	QString accountId() const;

	/**
	 * The account ID should be constant, don't use this method.
	 * Reserved for expert usage, use it with care of risk
	 * Changing the accountId on the fly, can result in bug and crash.
	 * the myself contact id will never be changed (it is impossible to
	 * change the contactId on the fly, and deleting myself is not possible
	 * because it is connected to slot, pointer to this contact are stored
	 * everywere, for examples, in the KMM.
	 */
	void setAccountId( const QString &accountId );

 	/**
	 * Returns the priority of this account. Used for sorting and determining
	 * the preferred account to message a contact
	 */
	const uint priority() const;

	/**
	 * Sets the account priority. This method is called by the UI, should not
	 * be called elsewhere.
	 */
	void setPriority( uint priority );

	/**
	 * Get the password for this account. Has the ability to open an input dialog
	 * if the password is not currently set
	 * @param error Set this value to true if you previously called password and the
	 * result was incorrect (the password was wrong). It adds a label in the input
	 * dialog saying that the password was wrong
	 * @param ok is set to false if the user returned cancel
	 * @param maxLength maximum length for a password, restricts the password
	 * lineedit accordingly, the default is no limit at all
	 * @return The password or QString::null if the user has canceled
	 */
	QString password( bool error = false, bool *ok =0L, unsigned int maxLength=0 ) const;

	/**
	 * Set the password for this account.
	 * @param pass set to Qtring::null means that
	 * the password is not remembered, otherwise sets the new password
	 * for this account
	 * Cas be called only by EditAccountWidget
	 */
	void setPassword(const QString &pass = QString::null);

	/**
	 * @brief say if the password is remembered.
	 *
	 * if it return false, that mean a call to @ref password() will popup a window.
	 *
	 * This function should be used by the editAccountPage (and only by the editAccountPage)
	 */
	bool rememberPassword() const;

	/**
	 * Set if the account should log in automatically or not
	 *
	 * This function can be used by the editaccountPage (and only by it)
	 * Remember that kopete handles autoconnection itself
	 */
	void setAutoLogin(bool);
	/**
	 * Say if the account should log in automatically
	 *
 	 * This function can be used by the editaccountPage (and only by it)
	 * Remember than kopete handle autoconnection itself
	 */
	bool autoLogin() const;

	/**
	 * returns the user color for this account
	 */
	const QColor color() const;

	/**
	 * Set the color this account will use to differentiate from the other accounts
	 * normaly, this should be called by the Kopete's account config page. so you
	 * don't have to set yourself the color.
	 */
	void setColor( const QColor &color);

	/**
	 * return the icon for this account, colored if needed
	 * Note: there is no cache in this funciton
	 * @param size is the size of the icon.  If the size is 0, the default size is used
	 *
	 */
	QPixmap accountIcon(const int size=0) const;

	/**
	 * @brief Indicate whether the account is connected at all.
	 *
	 * This is a convenience method that queries @ref KopeteContact::onlineStatus()
	 * on @ref myself()
	 */
	bool isConnected() const;

	/**
	 * @brief Indicate whether the account is away.
	 *
	 * This is a convenience method that queries @ref KopeteContact::onlineStatus()
	 * on @ref myself()
	 */
	bool isAway() const;

	/**
	 * Retrieve the 'myself' contact.
	 *
	 * Returns 0L if myself is not initialized yet.
	 *
	 * See also @ref setMyself().
	 */
	KopeteContact * myself() const;

	/**
	 * @brief Return the menu for this account
	 *
	 * You have to reimplement this method to return the custom action menu which will
	 * be shown in the statusbar. Kopete takes care of the deletion of the menu. Actions
	 * should have the menu as parent.
	 */
	virtual KActionMenu* actionMenu() ;

	/**
	 * @brief Retrieve the list of contacts for this protocol
	 *
	 * The list is guaranteed to contain only contacts for this account,
	 * so you can safely use static_cast to your own derived contact class
	 * if needed.
	 */
	const QDict<KopeteContact>& contacts();

	/**
	 * Read the account's config (KConfig)
	 *
	 * @ref configGroup is the group in the config file to use.
	 */
	void readConfig( const QString &configGroup );

	/**
	 * Write the config back
	 *
	 * @ref configGroup is the group in the config file to use.
	 */
	void writeConfig( const QString &configGroup );

	/**
	 * @internal
	 * Register a new KopeteContact with the account
	 * To be called <b>only</b> from @ref KopeteContact constructor
	 * not from any other class! (Not even a derived class).
	 */
	void registerContact( KopeteContact *c );

	/**
	 * @return the name of the config group to be used
	 */
	QString configGroup() const;

	/**
	 * @return whether we should suppress status notifications for contacts
	 * belonging to this account. When true, no passive popups and other
	 * notifications should be used
	 */
	bool suppressStatusNotification() const;

protected:
	/**
	 * Set the 'myself' contact. This contact HAS to be defined for every
	 * account, because it holds the online status of an account!
	 *
	 * The myself contact can't be deleted as long as the account still
	 * exists. The myself contact is used in each KMM, the myself contactId
	 * should be the accountID, the onlineStatus should represent the
	 * current user's status. The statusbar icon is connected to
	 * myself-> @ref onlineStatusChanged() to update the icon.
	 *
	 * @return contact associated with the currently logged in user
	 */
	void setMyself( KopeteContact *myself );

	/**
	 * Create a new contact in the specified metacontact
	 *
	 * You shouldn't ever call this method yourself, For adding contacts see @ref addContact()
	 *
	 * This method is called by @ref KopeteAccount::addContact() in this method, you should
	 * simply create the new custom @ref KopeteContact in the given metacontact. And you can
	 * add the contact to the server if the protocol supports it
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols)
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact ) =0;

public slots:
	/**
	 * @brief Go online for this service.
	 *
	 */
	virtual void connect() = 0;
	
	/**
	 * @brief Go online for this service using a different status
	 */
	virtual void connect( const KopeteOnlineStatus& initialStatus );

	/**
	 * @brief Disconnect from this service.
	 *
	 */
	virtual void disconnect() = 0;

	/**
	 * Adds a contact to this protocol with the specified details
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 * @param mode If the KDE addressbook should be changed to include the new contact.  Don't change if you are using this method to deserialise.
	 * @param groupName The name of the group to add the contact to
	 * @param isTemporary If this is a temporary contact
	 * @return Pointer to the KopeteContact object which was added
	 */
	bool addContact( const QString &contactId, const QString &displayName = QString::null,
		KopeteMetaContact *parentContact = 0L, const AddMode mode = DontChangeKABC,
		const QString &groupName = QString::null, bool isTemporary = false ) ;

	/**
	 * this will be called if main-kopete wants
	 * the plugin to set the user's mode to away
	 */
	virtual void setAway( bool away, const QString &reason = QString::null ) = 0;

signals:
	/**
	 * The accountId should be constant, see @ref KopeteAccount::setAccountId()
	 */
	void accountIdChanged();

protected slots:
	/**
	 * This method is called at the end of the fromXML function
	 * since pluginData are not accessible yet in the constructor
	 */
	virtual void loaded();

private slots:
	/**
	 * Track the deletion of a KopeteContact and cleanup
	 */
	void slotKopeteContactDestroyed( KopeteContact * );

	/**
	 * Called by a single shot hack to make the account tell its @ref KopeteAccountManager
	 * to signal that the account is fully created and ready to use.
	 */
	void slotAccountReady();

	/**
	 * Our online status changed.
	 *
	 * Currently this slot is used to set a timer that allows suppressing online status
	 * notifications just after connecting.
	 */
	void slotOnlineStatusChanged( KopeteContact *contact, const KopeteOnlineStatus &newStatus, const KopeteOnlineStatus &oldStatus );

	/**
	 * Stop the suppression of status notification
	 */
	void slotStopSuppression();

private:

	KopeteAccountPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

