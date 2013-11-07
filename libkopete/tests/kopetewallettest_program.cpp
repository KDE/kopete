/*
    Tests for the wallet manager

    Copyright (c) 2003      by Richard Smith          <kde@metafoo.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetewallettest_program.h"

#include <qtextstream.h>
#include <qtimer.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <kwallet.h>

#include "kopetewalletmanager.h"

static QTextStream _out( stdout, QIODevice::WriteOnly );

void closeWallet()
{
	Kopete::WalletManager::self()->closeWallet();
}

void delay()
{
	QTimer::singleShot( 3000, qApp, SLOT(quit()) );
	qApp->exec();
}

void openWalletAsync()
{
	WalletReciever *r = new WalletReciever;
	_out << "[ASYNC] About to open wallet, receiver: " << r << endl;
	Kopete::WalletManager::self()->openWallet( r, SLOT(gotWallet(KWallet::Wallet*)) );
}

void WalletReciever::gotWallet( KWallet::Wallet *w )
{
	_out << "[ASYNC] Received wallet pointer: " << w << " for receiver: " << this << endl;
}

void WalletReciever::timer()
{
	_out << "Timer..." << endl;
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopetewallettest", 0, ki18n("kopetewallettest"), "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );

	KCmdLineOptions opts;
	opts.add("+action");
	KCmdLineArgs::addCmdLineOptions( opts );
	KApplication app( "kopetewallettest" );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	// must register with DCOP or async callbacks will fail
	_out << "DCOP registration returned " << app.dcopClient()->registerAs(app.name()) << endl;

	for( int i = 0; i < args->count(); ++i )
	{
		QString arg = args->arg( i );
		_out << "Processing " << arg << endl;
		if( arg == QString::fromLatin1( "open" ) ) openWalletAsync();
		if( arg == QString::fromLatin1( "close" ) ) closeWallet();
		if( arg == QString::fromLatin1( "delay" ) ) delay();
		_out << "Done." << endl;
	}

	WalletReciever *r = new WalletReciever;

	QTimer timer;
	r->connect( &timer, SIGNAL(timeout()), SLOT(timer()) );
	timer.start( 1000 );

	_out << "About to start 30 second event loop" << endl;
	QTimer::singleShot( 30000, qApp, SLOT(quit()) );
	return qApp->exec();
}

#include "kopetewallettest_program.moc"

// vim: set noet ts=4 sts=4 sw=4:
