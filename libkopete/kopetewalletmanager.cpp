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

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kwallet.h>

#include <qtimer.h>
#include <qwidget.h>
#include <qapplication.h>

struct KopeteWalletManager::KopeteWalletManagerPrivate
{
	KopeteWalletManagerPrivate() : wallet(0), signal(0) {}
	~KopeteWalletManagerPrivate() { delete wallet; delete signal; }

	KWallet::Wallet *wallet;

	// we can't just connect every slot that wants the wallet to the
	// walletOpened signal - since we disconnect all the slots immediately
	// after emitting the signal, this would result in everyone who asked
	// for the wallet again in response to a walletOpened signal to fail
	// to receive it.
	// instead, we store a KopeteWalletSignal which we connect to, and create
	// a new one for each set of requests.
	KopeteWalletSignal *signal;
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
	
	WId mainWindowID()
	{
		if ( QWidget *w = qApp->mainWidget() )
			return w->winId();
		return 0;
	}
}

KopeteWalletManager *KopeteWalletManager::self()
{
	if ( !s_self )
		s_deleter.setObject( s_self, new KopeteWalletManager() );
	return s_self;
}

void KopeteWalletManager::openWallet( QObject *object, const char *slot )
{
	if ( !d->signal )
		d->signal = new KopeteWalletSignal;
	// allow connecting to protected slots  by calling object->connect
	connect( d->signal, SIGNAL( walletOpened( KWallet::Wallet* ) ), object, slot );
	//object->connect( d->signal, SIGNAL( walletOpened( KWallet::Wallet* ) ), slot );
	openWalletInner();
}

void KopeteWalletManager::openWalletInner()
{
	// do we already have a wallet?
	if ( d->wallet )
	{
		// if the wallet isn't open yet, we're pending a slotWalletChangedStatus
		// anyway, so we don't set up a single shot.
		if ( d->wallet->isOpen() )
		{
			kdDebug(14010) << k_funcinfo << " wallet already open" << endl;
			QTimer::singleShot( 0, this, SLOT( slotGiveExistingWallet() ) );
		}
		else
		{
			kdDebug(14010) << k_funcinfo << " still waiting for earlier request" << endl;
		}
		return;
	}
	
	kdDebug(14010) << k_funcinfo << " about to open wallet async" << endl;

	// we have no wallet: ask for one.
	d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
	            mainWindowID(), KWallet::Wallet::Asynchronous );

	connect( d->wallet, SIGNAL( walletOpened(bool) ), SLOT( slotWalletChangedStatus() ) );
}

void KopeteWalletManager::slotWalletChangedStatus()
{
	kdDebug(14010) << k_funcinfo << " isOpen: " << d->wallet->isOpen() << endl;

	if( d->wallet->isOpen() )
	{
		if ( !d->wallet->hasFolder( QString::fromLatin1( "Kopete" ) ) )
			d->wallet->createFolder( QString::fromLatin1( "Kopete" ) );

		if ( d->wallet->setFolder( QString::fromLatin1( "Kopete" ) ) )
		{
			// success!
			QObject::connect( d->wallet, SIGNAL( walletClosed() ), this, SLOT( closeWallet() ) );
		}
		else
		{
			// opened OK, but we can't use it
			delete d->wallet;
			d->wallet = 0;
		}
	}
	else
	{
		// failed to open
		delete d->wallet;
		d->wallet = 0;
	}

	emitWalletOpened( d->wallet );
}

void KopeteWalletManager::slotGiveExistingWallet()
{
	kdDebug(14010) << k_funcinfo << " with d->wallet " << d->wallet << endl;

	if ( d->wallet )
	{
		// the wallet was already open
		if ( d->wallet->isOpen() )
			emitWalletOpened( d->wallet );
		// if the wallet was not open, but d->wallet is not 0,
		// then we're waiting for it to open, and will be told
		// when it's done: do nothing.
		else
			kdDebug(14010) << k_funcinfo << " wallet gone, waiting for another wallet" << endl;
	}
	else
	{
		// the wallet was lost between us trying to open it and
		// getting called back. try to reopen it.
		openWalletInner();
	}
}

KWallet::Wallet *KopeteWalletManager::wallet()
{
	if ( !KWallet::Wallet::isEnabled() )
		return 0;

	if ( d->wallet && d->wallet->isOpen() )
		return d->wallet;

	// if we have a wallet here, we're really unfortunate: we've had a sync wallet open
	// call while the wallet dialog was open async. we can't wait for it to close, since
	// we're not allowed to processEvents(), so the best we can do is:
	delete d->wallet;

	d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
	            mainWindowID(), KWallet::Wallet::Synchronous );

	// if we got a wallet, prepare it, and tell everyone who cares either way
	if ( d->wallet )
		slotWalletChangedStatus();
	else
		emitWalletOpened( 0 );

	return d->wallet;
}

void KopeteWalletManager::closeWallet()
{
	if ( !d->wallet ) return;

	delete d->wallet;
	d->wallet = 0L;

	emit walletLost();
}

void KopeteWalletManager::emitWalletOpened( KWallet::Wallet *wallet )
{
	KopeteWalletSignal *signal = d->signal;
	d->signal = 0;
	if ( signal )
		emit signal->walletOpened( wallet );
	delete signal;
}


#include "kopetewalletmanager.moc"

#endif // KDE_IS_VERSION

// vim: set noet ts=4 sts=4 sw=4:

