/*
    Tests for Kopete::ContactList class.

    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlist_test.h"
#include <qfile.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <kunittest/module.h>

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopetecontactlist_test, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopeteContactList_Test );

void KopeteContactList_Test::allTests()
{
	testSomething();
}

void KopeteContactList_Test::testSomething()
{
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homePath() + "/.kopete-unittest" ), true );

	QString filename = KStandardDirs::locateLocal( "appdata", QString::fromLatin1( "contactlist.xml" ) );
	if( ! filename.isEmpty() )
	{
		// previous test run, delete the previous contact list
		bool removed = QFile::remove(filename);
		// if we cant remove the file, abort test
		if (!removed)
			return;
	}
	
	int result = 1;
	int expected = 1;
	// result should be the expected one
	CHECK(result, expected);
}


