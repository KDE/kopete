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

#include <qtextstream.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "kopetewalletmanager.h"

static QTextStream _out( stdout, IO_WriteOnly );

/*
  A few simple tests for the wallet manager:
  1) Open the wallet. Should prompt for password.
  2) Ask for wallet again. Should have cached the wallet from before.
  3) Make sure the walletClosed signal works.
*/

typedef bool (*Test)();

void runTests( QString description, Test test )
{
	_out << endl;
	_out << "* " << description << endl;

	if ( !test() )
		_out << " FAILED!" << endl;
}

bool openWallet()
{
	return (KopeteWalletManager::self()->wallet() != 0);
}

bool closeWallet()
{
	KopeteWalletManager::self()->closeWallet();
	return true;
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopetewallettest", "kopetewallettest", "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app( "kopetewallettest" );
	
	runTests( "Trying to open wallet. Enter your passphrase.", openWallet );
	runTests( "Trying to open wallet (2). Passphrase should not be asked for.", openWallet );
	runTests( "Closing wallet.", closeWallet );
	runTests( "Trying to open wallet (3). Passphrase should be asked for.", openWallet );

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

