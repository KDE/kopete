/*
    Tests for Kopete::Message::parseLinks

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
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

#include <qstring.h>

#include <kunittest/module.h>
#include "kopetelinktest.h"

#define private public
#include "kopetemessage.h"
#undef private

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopetelinktest, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopeteLinkTest );

/*
  There are four sets of tests: for each of plain text and html, we have those
  known to work in the current codebase, and those known to fail right now.

  Each entry in a set consists of two strings. The first string is the input,
  the second string is the EXPECTED output. If the two differ the test is
  considered failed and the actual result is output for comparison.

  A set should end with an entry containing at least one null pointer.
*/

typedef const char * LinkTestSet[][ 2 ];

static LinkTestSet knownGoodPlain =
{
	{ "$URL", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ "$URL/", "<a href=\"$URL/\" title=\"$URL/\">$URL/</a>" },
	{ "www.kde.org/", "<a href=\"$URL/\" title=\"$URL/\">www.kde.org/</a>" },
	{ NULL, NULL }
};

static LinkTestSet knownBrokenPlain =
{
	{ NULL, NULL }
};

static LinkTestSet knownGoodHTML =
{
	{ "$URL", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ "<a href=\"$URL\">KDE</a>", "<a href=\"$URL\">KDE</a>" },
	{ NULL, NULL }
};

static LinkTestSet knownBrokenHTML =
{
	{ "<a href=\"$URL\" title=\"$URL\">$URL</a>", "<a href=\"$URL\" title=\"$URL\">$URL</a>" },
	{ NULL, NULL }
};

void KopeteLinkTest::allTests()
{
	testKnownGoodHTML();
	testKnownBrokenHTML();
	testKnownGoodPlain();
	testKnownBrokenPlain();
}

void KopeteLinkTest::testKnownGoodHTML()
{
	uint i = 0;
	while ( knownGoodHTML[ i ][ 0 ] && knownGoodHTML[ i ][ 1 ] )
	{
		QString input = knownGoodHTML[ i ][ 0 ], expected = knownGoodHTML[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::RichText );
	
		CHECK(result, expected);
		i++;
	}
}

void KopeteLinkTest::testKnownBrokenHTML()
{
	uint i = 0;
	while ( knownBrokenHTML[ i ][ 0 ] && knownBrokenHTML[ i ][ 1 ] )
	{
		QString input = knownBrokenHTML[ i ][ 0 ], expected = knownBrokenHTML[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::RichText );
	
		XFAIL(result, expected);
		i++;
	}
}
void KopeteLinkTest::testKnownGoodPlain()
{
	uint i = 0;
	while ( knownGoodPlain[ i ][ 0 ] && knownGoodPlain[ i ][ 1 ] )
	{
		QString input = knownGoodPlain[ i ][ 0 ], expected = knownGoodPlain[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::PlainText );
	
		CHECK(result, expected);
		i++;
	}
}

void KopeteLinkTest::testKnownBrokenPlain()
{
	uint i = 0;
	while ( knownBrokenPlain[ i ][ 0 ] && knownBrokenPlain[ i ][ 1 ] )
	{
		QString input = knownBrokenPlain[ i ][ 0 ], expected = knownBrokenPlain[ i ][ 1 ];
		input.replace( "$URL","http://www.kde.org" );
		expected.replace( "$URL","http://www.kde.org" );
		QString result = Kopete::Message::parseLinks( input, Kopete::Message::PlainText );
	
		XFAIL(result, expected);
		i++;
	}
}


 