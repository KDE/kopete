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

#include <kdemacros.h>

#include "kopete_export.h"

namespace KWallet { class Wallet; }

namespace Kopete
{

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 *
 * The Kopete::WalletManager class is a singleton, which looks after Kopete's
 * KWallet connection.
 */
class KOPETE_EXPORT WalletManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Retrieve the wallet manager instance
	 */
	static WalletManager *self();
	~WalletManager();

	/**
	 * @brief Attempt to open the KWallet asyncronously, then signal an
	 *        object to indicate the task is complete.
	 *
	 * @param object The object to call back to
	 * @param slot The slot on object to call; must have signature slot( KWallet::Wallet* )
	 *        The parameter to the slot will be the wallet that was opened if the call
	 *        succeeded, or NULL if the wallet failed to open or the Kopete folder was
	 *        inaccessible.
	 *
	 * For simplicity of client code, it is guaranteed that your slot
	 * will not be called during a call to this function.
	 */
	void openWallet( QObject *object, const char *slot );
	
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

private slots:
	/**
	 * Called by the stored wallet pointer when it is successfully opened or
	 * when it fails.
	 *
	 * Causes walletOpened to be emitted.
	 */
	void slotWalletChangedStatus();
	
	/**
	 * Called by a singleShot timer in the event that we are asked for a
	 * wallet when we already have one open and ready.
	 */
	void slotGiveExistingWallet();

private:
	void openWalletInner();
	void emitWalletOpened( KWallet::Wallet *wallet );

	class Private;
	Private * const d;

	WalletManager();
};

}

/**
 * @internal
 */
class KopeteWalletSignal : public QObject
{
	Q_OBJECT
	friend class Kopete::WalletManager;
signals:
	void walletOpened( KWallet::Wallet *wallet );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

