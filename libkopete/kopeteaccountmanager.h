/*
    kopeteaccountmanager.h - Kopete Account Manager

    Copyright (c) 2002      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

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

#ifndef __kopeteaccountmanager_h__
#define __kopeteaccountmanager_h__

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qdom.h>

class KopeteAccount;
class KopeteProtocol;
class KopetePlugin;

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
	 * Retrieve the instance of KopeteAccountManager.
	 * The account manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 */
	static KopeteAccountManager* manager();

	~KopeteAccountManager();

	/**
	 * Retrieve the list of accounts
	 */
	const QPtrList<KopeteAccount> & accounts() const;

	/**
	 * Retrieve a QDict of accounts for the given protocol
	 *
	 * The list is guaranteed to contain only accounts for the specified
	 * protocol
	 */
	QDict<KopeteAccount> accounts( const KopeteProtocol *p );

	/**
	 * Return the account asked
	 */
	KopeteAccount* findAccount( const QString &protocolId, const QString &accountId );

	/**
	 * Tries to guess a color for the next account in the given protocol based on the already registered ones
	 */
	QColor guessColor( KopeteProtocol* protocol );

public slots:
	/**
	 * Connect all accounts which have auto connect enabled
	 */
	 void autoConnect();

	/**
	 * Connect all accounts at once.
	 * This is a slot, so you can connect directly to it from a KAction, for example.
	 */
	void connectAll();

	/**
	 * Disconnect all accounts at once.
	 * This is a slot, so you can connect directly to it from a KAction, for example.
	 */
	void disconnectAll();

	/**
	 * Set all accounts to away at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAwayAll( const QString &awayReason = QString::null );

	/**
	 * Remove the away status from all accounts at once.
	 * This is a slot, so you can connect directly to it from e.g. a KAction.
	 */
	void setAvailableAll();

	/**
	 * @internal
	 * save to accounts.xml
	 */
	void save();

	/**
	 * @internal
	 * load accounts.xml
	 */
	void load();

signals:
	/**
	 * An account is ready for use
	 */
	void accountReady( KopeteAccount *account );

	/**
	 * An account has been unregistered
	 */
	void accountUnregistered(KopeteAccount *a);

private:
	/**
	 * Private constructor, because we're a singleton
	 */
	KopeteAccountManager();

	/**
	 * @internal
	 * Register the account.
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
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
	 * To be called ONLY from KopeteContact, not from any other class!
	 * (Not even a derived class).
	 */
	void unregisterAccount( KopeteAccount *account );

	static KopeteAccountManager *s_manager;

	QPtrList<KopeteAccount> m_accounts;
	QDomDocument m_accountList;

private slots:
	void loadProtocol( KopetePlugin * );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

