/*
    kopetewalletmanager.h - Kopete Wallet Manager

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

#ifndef KOPETEWALLETMANAGER_H
#define KOPETEWALLETMANAGER_H

#include <qobject.h>

namespace KWallet { class Wallet; }

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 *
 * The KopeteWalletManager class is a singleton, which looks after Kopete's
 * KWallet connection.
 */
class KopeteWalletManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the wallet manager instance
	 */
	static KopeteWalletManager *self();
	~KopeteWalletManager();

	enum ShouldCreate { CreateFolder, DoNotCreateFolder };

	/**
	 * Get a KWallet instance, possibly prompting the user for his
	 * passphrase if necessary. The returned wallet will already be set to
	 * the appropriate folder, and should not have setFolder called on it.
	 *
	 * You may store the wallet object returned from this function, but note
	 * that it will be deleted after the closeWallet signal is emitted.
	 *
	 * @param create If set to CreateFolder, the Kopete folder will be created
	 *        if it does not exist; if set to DoNotCreateFolder, the folder
	 *        will not be created. If the folder does not exist or could not
	 *        be created, this function will return NULL.
	 * @return The network KWallet to use for storing Kopete data, or NULL if
	 *        the wallet was inaccessible.
	 */
	KWallet::Wallet *wallet( ShouldCreate create = CreateFolder );

public slots:
	/**
	 * Close the connection to the wallet. Will cause walletLost() to be emitted.
	 */
	void closeWallet();

signals:
	/**
	 * Emitted when the connection to the wallet is lost.
	 */
	void walletLost();

private:
	struct KopeteWalletManagerPrivate;
	KopeteWalletManagerPrivate *d;

	KopeteWalletManager();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

