/*
    Tests for KopeteMessage::parseLinks

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#define private public
#include "kopetemessage.h"
#undef private

static QTextStream _out( stdout, IO_WriteOnly );

/*
  There are four sets of tests: for each of plain text and html, we have those
  known to work in the current codebase, and those known to fail right now.

  Each entry in a set consists of two strings. The first string is the input,
  the second string is the EXPECTED output. If the two differ the test is
  considered failed and the actual result is output for comparison.

  A set should end with an entry containing at least one null pointer.
*/

typedef const char * TestSet[][ 2 ];


static TestSet knownGoodPlain =
{
	{ "http://www.kde.org", "<a href=\"http://www.kde.org\" title=\"http://www.kde.org\">http://www.kde.org</a>" },
	{ NULL, NULL }
};

static TestSet knownBrokenPlain =
{
	{ NULL, NULL }
};

static TestSet knownGoodHTML =
{
	{ "http://www.kde.org", "<a href=\"http://www.kde.org\" title=\"http://www.kde.org\">http://www.kde.org</a>" },
	{ "<a href=\"http://www.kde.org\">KDE</a>", "<a href=\"http://www.kde.org\">KDE</a>" },
	{ NULL, NULL }
};

static TestSet knownBrokenHTML =
{
	{ "<a href=\"http://www.kde.org\" title=\"http://www.kde.org\">http://www.kde.org</a>", "<a href=\"http://www.kde.org\" title=\"http://www.kde.org\">http://www.kde.org</a>" },
	{ NULL, NULL }
};

void runTests( QString description, TestSet tests, KopeteMessage::MessageFormat format )
{
	_out << endl;
	_out << "* Running test set '" << description << "'" << endl;

	uint i = 0;
	while ( tests[ i ][ 0 ] && tests[ i ][ 1 ] )
	{
		QString result = KopeteMessage::parseLinks( tests[ i ][ 0 ], format );

		if ( result == tests[ i ][ 1 ] )
		{
			_out << "  - Succeeded test for '" << tests[ i ][ 0 ] << "'" << endl;
		}
		else
		{
			_out << "  - FAILED test for '" << tests[ i ][ 0 ] << "'" << endl;
			_out << "    Expected output: '" << tests[ i ][ 1 ] << "'" << endl;
			_out << "    Real output:     '" << result << "'" << endl;
		}

		i++;
	}
}

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopetelinktest", "kopetelinktest", "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app( "kopetelinktest" );

	runTests( "Known working plaintext tests", knownGoodPlain, KopeteMessage::PlainText );
	runTests( "Known broken plaintext tests", knownBrokenPlain, KopeteMessage::PlainText );
	runTests( "Known working HTML tests", knownGoodHTML, KopeteMessage::RichText );
	runTests( "Known broken HTML tests", knownBrokenHTML, KopeteMessage::RichText );

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

