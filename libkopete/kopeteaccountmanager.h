/*
    kopeteaccountmanager.h - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

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
#include <kopeteonlinestatus.h>

class KopeteAccountManagerPrivate;

namespace Kopete
{

class Account;
class Plugin;
class Protocol;
class Contact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * Kopete::AccountManager manages all defined accounts in Kopete. You can
 * query them and globally set them all online or offline from here.
 */
class AccountManager : public QObject
{
	Q_OBJECT

	/**
	 * Kopete::Account needs to be able to call register/unregister,
	 * so make it a friend.
	 */
	friend class Account;

public:
	/**
	 * \brief Retrieve the instance of Kopete::AccountManager.
	 *
	 * The account manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 *
	 * \return the instance of the Kopete::AccountManager
	 */
	static AccountManager* manager();

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
	 * \param p is the Kopete::Protocol object you want accounts for
	 */
	QDict<Account> accounts( const Protocol *p );

	/**
	 * \brief Return the account asked
	 * \param protocolId is the ID for the protocol
	 * \param accountId is the ID for the account you want
	 * \return the Kopete::Account object found or NULL if no account was found
	 */
	Account* findAccount( const QString &protocolId, const QString &accountId );

	/**
	 * \brief Delete the account and clean the config data
	 */
	void removeAccount( Account *account );

	/**
	 * \brief Guess the color for a new account
	 *
	 * Guesses a color for the next account of a given protocol based on the already registered colors
	 * \return the color guessed for the account
	 */
	QColor guessColor( Protocol *protocol );

public slots:
	/**
	 * \brief Connect all accounts which have auto connect enabled
	 */
	 void autoConnect();

	/**
	 * \brief Connect all accounts at once.
	 *
	 * This is a slot, so you can connect directly to it from a KAction, for example.
	 */
	void connectAll();

	/**
	 * \brief Disconnect all accounts at once.
	 *
	 * This is a slot, so you can connect directly to it from a KAction, for example.
	 */
	void disconnectAll();

	/**
	 * \brief Set all accounts to away at once.
	 *
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAwayAll( const QString &awayReason = QString::null );

	/**
	 * \brief  Remove the away status from all accounts at once.
	 *
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAvailableAll();

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
	void accountReady( Kopete::Account *account );

	/**
	 * \brief Signals when an account has been unregistered
	 */
	void accountUnregistered( Kopete::Account *account );

	/**
	 * \brief An account has changed its onlinestatus
	 * Technically this monitors Kopete::Account::myself() onlinestatus changes
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

	/**
	 * \internal
	 * Register the account.
	 * To be called ONLY from Kopete::Account, not from any other class!
	 */
	bool registerAccount( Account *account );

	/**
	 * \internal
	 * Notify the Kopete::AccountManager that an account is ready for use
	 * Same rules apply as for registerAccount()
	 */
	void notifyAccountReady( Account *account );

	/**
	 * \internal
	 * Unregister the account.
	 * To be called ONLY from Kopete::Account, not from any other class!
	 */
	void unregisterAccount( Account *account );

private slots:
	void slotPluginLoaded( Kopete::Plugin *plugin );
	void slotAccountOnlineStatusChanged(Kopete::Contact *c,
		const Kopete::OnlineStatus &oldStatus, const Kopete::OnlineStatus &newStatus);

private:
	KopeteAccountManagerPrivate *d;

};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:
