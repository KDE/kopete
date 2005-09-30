/*
    kopeteaccountmanager.h - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@ tiscalinet.be>

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

#ifndef __kopeteaccountmanager_h__
#define __kopeteaccountmanager_h__

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>

#include "kopete_export.h"


namespace Kopete {

class Account;
class Plugin;
class Protocol;
class Contact;
class OnlineStatus;

/**
 * AccountManager manages all defined accounts in Kopete. You can
 * query them and globally set them all online or offline from here.
 *
 * AccountManager is a singleton, you may uses it with @ref AccountManager::self()
 *
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@ tiscalinet.be>
 */
class KOPETE_EXPORT AccountManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Retrieve the instance of AccountManager.
	 *
	 * The account manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 *
	 * \return the instance of the AccountManager
	 */
	static AccountManager* self();

	~AccountManager();

	/**
	 * \brief Retrieve the list of accounts
	 * \return a list of all the accounts
	 */
	const QPtrList<Account> & accounts() const;

	/**
	 * \brief Retrieve a QDict of accounts for the given protocol
	 *
	 * The list is guaranteed to contain only accounts for the specified
	 * protocol
	 * \param p is the Protocol object you want accounts for
	 */
	QDict<Account> accounts( const Protocol *p ) const;

	/**
	 * \brief Return the account asked
	 * \param protocolId is the ID for the protocol
	 * \param accountId is the ID for the account you want
	 * \return the Account object found or NULL if no account was found
	 */
	Account* findAccount( const QString &protocolId, const QString &accountId );

	/**
	 * \brief Delete the account and clean the config data
	 *
	 * This is praticaly called by the account config page when you remove the account.
	 */
	void removeAccount( Account *account );

	/**
	 * \brief Guess the color for a new account
	 *
	 * Guesses a color for the next account of a given protocol based on the already registered colors
	 * \return the color guessed for the account
	 */
	QColor guessColor( Protocol *protocol ) const ;

	/**
	 * @brief Register the account.
	 *
	 * This adds the account in the manager's account list.
	 * It will check no accounts already exist with the same ID, if any, the account is deleted. and not added
	 *
	 * @return @p account, or 0L if the account was deleted because id collision
	 */
	Account *registerAccount( Account *account );


	/**
	 * Flag to be used in setOnlineStatus
	 *
	 * @c ConnectIfOffline : if set, this will connect offlines account with the status.
	 */
	enum SetOnlineStatusFlag { ConnectIfOffline=0x01 };


public slots:
	/**
	 * \brief Connect all accounts at once.
	 *
	 * Connect every account if the flag excludeConnect is false
	 * @see @ref Account::excludeConnect()
	 */
	void connectAll();

	/**
	 * \brief Disconnect all accounts at once.
	 */
	void disconnectAll();

	/**
	 * @brief Set all accounts a status in the specified category
	 *
	 * Account that are offline will not be connected, unless the ConnectIfOffline flag is set.
	 *
	 * @param category is one of the Kopete::OnlineStatusManager::Categories
	 * @param awayMessage is the new away message
	 * @param flags is a bitmask of SetOnlineStatusFlag
	 */
	void setOnlineStatus( /*Kopete::OnlineStatusManager::Categories*/ uint category,
						  const QString& awayMessage = QString::null, uint flags=0);

	/**
	 * \brief Set all accounts to away at once.
	 *
	 * All account that are connected, but not invisible will be set to away
	 * @see Account::setAway
	 * @param awayReason is the away message that will be set.
	 * @param away decides whether the message is away/non-away
	 */
	void setAwayAll( const QString &awayReason = QString::null, bool away=true );

	/**
	 * \brief Connect or make available every account.
	 * Make all accounts Available, by setting status, and connecting if necessary.
	 * Accounts are connected based on their excludeConnect() setting.
	 * Accounts which are already connected are controlled regardless of their excludeConnect() setting.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 * @param awayReason is the away(status) message that will be set.
	 */
	void setAvailableAll( const QString &awayReason = QString::null );

	/**
	 * \internal
	 * Save the account data to KConfig
	 */
	void save();

	/**
	 * \internal
	 * Load the account data from KConfig
	 */
	void load();



signals:
	/**
	 * \brief Signals when an account is ready for use
	 */
	void accountRegistered( Kopete::Account *account );

	/**
	 * \brief Signals when an account has been unregistered
	 *
	 * At this state, we are already in the Account destructor.
	 */
	void accountUnregistered( const Kopete::Account *account );

	/**
	 * \brief An account has changed its onlinestatus
	 * Technically this monitors Account::myself() onlinestatus changes
	 * \param account Account which changed its onlinestatus
	 * \param oldStatus The online status before the change
	 * \param newStatus The new online status
	 */
	void accountOnlineStatusChanged(Kopete::Account *account,
		const Kopete::OnlineStatus &oldStatus, const Kopete::OnlineStatus &newStatus);

private:
	/**
	 * Private constructor, because we're a singleton
	 */
	AccountManager();

private slots:
	void slotPluginLoaded( Kopete::Plugin *plugin );
	void slotAccountOnlineStatusChanged(Kopete::Contact *c,
		const Kopete::OnlineStatus &oldStatus, const Kopete::OnlineStatus &newStatus);

	/**
	 * \internal
	 * Unregister the account.
	 */
	void unregisterAccount( const Kopete::Account *account );

private:
	bool isAnyAccountConnected();
	static AccountManager *s_self;
	class Private;
	Private *d;
};

} //END namespace Kopete


#endif


