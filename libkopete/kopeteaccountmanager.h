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

class KopeteAccount;
class KopetePlugin;
class KopeteProtocol;
class KopeteContact;

class KopeteAccountManagerPrivate;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * KopeteAccountManager manages all defined accounts in Kopete. You can
 * query them and globally set them all online or offline from here.
 */
class KopeteAccountManager : public QObject
{
	Q_OBJECT

	/**
	 * KopeteAccount needs to be able to call register/unregister,
	 * so make it a friend.
	 */
	friend class KopeteAccount;

public:
	/**
	 * \brief Retrieve the instance of KopeteAccountManager.
	 *
	 * The account manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 *
	 * \return the instance of the KopeteAccountManager
	 */
	static KopeteAccountManager* manager();

	~KopeteAccountManager();

	/**
	 * \brief Retrieve the list of accounts
	 */
	const QPtrList<KopeteAccount> & accounts() const;

	/**
	 * \brief Retrieve a QDict of accounts for the given protocol
	 *
	 * The list is guaranteed to contain only accounts for the specified
	 * protocol
	 */
	QDict<KopeteAccount> accounts( const KopeteProtocol *p );

	/**
	 * \brief Return the account asked
	 */
	KopeteAccount* findAccount( const QString &protocolId, const QString &accountId );

	/**
	 * \brief Delete the account and clean the config data
	 */
	void removeAccount( KopeteAccount *account );

	/**
	 * \brief Guess the color for a new account
	 *
	 * Guesses a color for the next account of a given protocol based on the already registered colors
	 */
	QColor guessColor( KopeteProtocol *protocol );

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
	 * @internal
	 * Save the account data to KConfig
	 */
	void save();

	/**
	 * @internal
	 * Load the account data from KConfig
	 */
	void load();

signals:
	/**
	 * \brief An account is ready for use
	 */
	void accountReady( KopeteAccount *account );

	/**
	 * \brief An account has been unregistered
	 */
	void accountUnregistered( KopeteAccount *account );

	/**
	 * \brief An account has changed its onlinestatus
	 * Technically this monitors KopeteAccount::myself() onlinestatus changes
	 * @p account Account which changed its onlinestatus
	 * @p oldStatus Status before the change
	 * @p newStatus New status
	 */
	void accountOnlineStatusChanged(KopeteAccount *account,
		const KopeteOnlineStatus &oldStatus, const KopeteOnlineStatus &newStatus);

private:
	/**
	 * Private constructor, because we're a singleton
	 */
	KopeteAccountManager();

	/**
	 * @internal
	 * Register the account.
	 * To be called ONLY from KopeteAccount, not from any other class!
	 */
	void registerAccount( KopeteAccount *account );

	/**
	 * @internal
	 * Notify the KopeteAccountManager that an account is ready for use
	 * Same rules apply as for registerAccount()
	 */
	void notifyAccountReady( KopeteAccount *account );

	/**
	 * @internal
	 * Unregister the account.
	 * To be called ONLY from KopeteAccount, not from any other class!
	 */
	void unregisterAccount( KopeteAccount *account );

private slots:
	void slotPluginLoaded( KopetePlugin *plugin );
	void slotAccountOnlineStatusChanged(KopeteContact *c,
		const KopeteOnlineStatus &oldStatus, const KopeteOnlineStatus &newStatus);

private:
	KopeteAccountManagerPrivate *d;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:
