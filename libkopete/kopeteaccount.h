/*
    kopeteaccount.h - Kopete Account

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

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

QString cryptStr(const QString &aStr);

/**
 * @author Olivier Goffart  <ogoffart@tiscalinet.be>
 *
 * The KopeteAccount class handle one account.
 * Each protocol should subclass this class in their own customAccounts class.
 * there are few pure virtual method than the protocol must implement. like
 * @ref myslef()  @ref connect()  @ref disconnect() or @ref addContactToMetaContact()
 *
 * The accountId is an *constant* unique id. which represent the login.
 * The @ref myself() contact one of the most importent contact, which represent the user
 *
 * All account data are automaticaly saved in a accounts.xml file. that include the
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

public:
	/**
	 * constructor:
	 * The constructor register automaticaly the account to the @ref KopeteAccountManager
	 * @param parent it the protocol of this account. the accoun is a child object of the
	 * protocol, so it will be automaticaly deleted with the parent.
	 * @param accountID is the id of this protocol, it shouln't be changed after
	 * @param name is the QObject name. it can be 0L
	 */
	KopeteAccount(KopeteProtocol *parent, const QString &accountID, const char *name=0L);
	~KopeteAccount();

	/**
	 * return the protocol for this account
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
	 * Get the password for this account. Has the ability to open an input dialog
	 * if the password is not currently set
	 * @param error Set this value to true if you previously called getPassword and the
	 * result was incorrect (the password was wrong). It adds a label in the input
	 * dialog saying that the password was wrong
	 * @param ok is set to false if the user returned cancel
	 * @return The password or QString::null if the user has canceled
	 */
	QString getPassword( bool error = false, bool *ok =0L );
	/*
	 * Set the password for this account.
	 * @param pass set to Qtring::null means that
	 * the password is not remembered, otherwise sets the new password
	 * for this account
	 * Should be called only by EditAccountWidget
	 */
	void setPassword(const QString &pass = QString::null);

	/**
	 * say if the password is remember.
	 * if it return false, that mean a call to @ref getPassword() will popup a window
	 */
	bool rememberPassword();

	/*
	 * Set if the account should log in automatically or not
	 */
	void setAutoLogin(bool);
	/*
	 * Say if the account should log in automatically
	 */
	bool autoLogin() const;

	/*
	 * returns the user color for this account
	 */
	const QColor color() const;
	/*
	 * Set the color this account will use to differentiate from the other accounts
	 * normaly, this should be called by the Kopete's account config page. so you
	 * don't have to set yourself the color.
	 */
	void setColor( const QColor &color);

	/**
	 * this will be called if main-kopete wants
	 * the plugin to set the user's mode to away
	 */
	virtual void setAway( bool away, const QString &reason = QString::null ) = 0;

	/**
	 * Indicate whether the account is connected at all.
	 *
	 * This is a convenience method that queries @ref KopeteContact::onlineStatus()
	 * on @ref myself()
	 */
	bool isConnected() const;

	/**
	 * Indicate whether the account is away.
	 *
	 * This is a convenience method that queries @ref KopeteContact::onlineStatus()
	 * on @ref myself()
	 */
	bool isAway() const;

	/**
	 * This function has to be reimplemented in every single protocol
	 * and should return the KopeteContact associated with the 'home' user.
	 * the myself contact MUST be created in the account constructor!
	 * The myself contact Can't de deleted when kopete if this account still
	 * exist. The myself contact is used in each KMM,
	 * The myself contactId should be the accountID, his onlineStatus
	 * represent the current user's status. the statusbar icon is connected
	 * to myself.onlineStatusChanged to update the icon.
	 *
	 * @return contact associated with the currently logged in user
	 */
	virtual KopeteContact* myself() const = 0;

	/**
	 * return the menu for this account
	 */
	virtual KActionMenu* actionMenu();

	/**
	 * Retrieve the list of contacts for this protocol
	 *
	 * The list is guaranteed to contain only contacts for this account,
	 * so you can safely use static_cast to your own derived contact class
	 * if needed.
	 */
	const QDict<KopeteContact>& contacts();

	/**
	 * Save the account to an XML string. Only used internally
	 */
	const QDomElement toXML();

	/**
	 * Load account from XML
	 */
	bool fromXML(const QDomElement& cnode);

	/**
	 * @internal
	 * Register a new KopeteContact with the account
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
	 */
	void registerContact( KopeteContact *c );

protected:
	/**
	 * Create a new contact in the specified metacontact
	 * You shouldn't call yourself this method, for adding contact see @ref addContact()
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact ) =0;

public slots:
	/**
	 * Go online for this service.
	 * This is a slot, so it can be called directly from a KAction, for example.
	 */

	virtual void connect() = 0;

	/**
	 * Disconnect from this service.
	 * This is a slot, so it can be called directly from a KAction, for example.
	 */
	virtual void disconnect() = 0;

	/**
	 * Adds a contact to this protocol with the specified details
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 * @param groupName The name of the group to add the contact to
	 * @param isTemporary If this is a temporary contact
	 * @return Pointer to the KopeteContact object which was added
	 */
	bool addContact( const QString &contactId, const QString &displayName = QString::null,
		KopeteMetaContact *parentContact = 0L, const QString &groupName = QString::null, bool isTemporary = false) ;

signals:
	/**
	 * The accountId should be constant, see @ref KopeteAccount::setAccountId()
	 */
	void accountIdChanged();

	/**
	 * The password has changed. Practically, you shouldn't keep the password in memory, but you should
	 * call @ref getPassword() which takes care of asking for the password. Reserved only for
	 * expert usage.
	 */
	void passwordChanged();

	/**
	 * The myself status icon changed.
	 */
	void onlineStatusIconChanged( KopeteAccount *account );

protected slots:
	/*
	 * This method is called at the end of the  fromXML function
	 * since pluginData are not accessible yet in the constructor
	 */
	virtual void loaded();

private slots:
	/**
	 * Track the deletion of a KopeteContact and cleanup
	 */
	void slotKopeteContactDestroyed( KopeteContact * );

	/**
	 * mySelf contact is created in account child classes
	 * we use a single shot hack to connect myself signals
	 */
	void slotMyselfCreated();

	/**
	 * The myself status changed.
	 */
	void slotMyselfStatusChanged( );

private:
	KopeteAccountPrivate *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

