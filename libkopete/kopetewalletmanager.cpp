/*
    kopetewalletmanager.cpp - Kopete Wallet Manager

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

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )

#include "kopetewalletmanager.h"

#include <kstaticdeleter.h>
#include <kwallet.h>

struct KopeteWalletManager::KopeteWalletManagerPrivate
{
	KopeteWalletManagerPrivate() : wallet(0) {}
	~KopeteWalletManagerPrivate() { delete wallet; }

	KWallet::Wallet *wallet;
};

KopeteWalletManager::KopeteWalletManager()
 : d( new KopeteWalletManagerPrivate )
{
}

KopeteWalletManager::~KopeteWalletManager()
{
	closeWallet();
	delete d;
}

namespace
{
	KStaticDeleter<KopeteWalletManager> s_deleter;
	KopeteWalletManager *s_self = 0;
}

KopeteWalletManager *KopeteWalletManager::self()
{
	if ( !s_self )
		s_deleter.setObject( s_self, new KopeteWalletManager() );
	return s_self;
}

KWallet::Wallet *KopeteWalletManager::wallet( KopeteWalletManager::ShouldCreate create )
{
	if ( !KWallet::Wallet::isEnabled() ) return 0;

	if ( d->wallet ) return d->wallet;

	d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
		/* FIXME: put a real wId here */ 0, KWallet::Wallet::Synchronous );

	if ( d->wallet )
	{
		QObject::connect( d->wallet, SIGNAL( walletClosed() ), this, SLOT( closeWallet() ) );

		if ( !d->wallet->hasFolder( QString::fromLatin1( "Kopete" ) ) && create == CreateFolder )
			d->wallet->createFolder( QString::fromLatin1( "Kopete" ) );

		if ( !d->wallet->setFolder( QString::fromLatin1( "Kopete" ) ) )
		{
			delete d->wallet;
			d->wallet = 0;
		}
	}

	return d->wallet;
}

void KopeteWalletManager::closeWallet()
{
	if ( !d->wallet ) return;

	emit walletLost();

	delete d->wallet;
	d->wallet = 0L;
}

#include "kopetewalletmanager.moc"

#endif // KDE_IS_VERSION

// vim: set noet ts=4 sts=4 sw=4:

