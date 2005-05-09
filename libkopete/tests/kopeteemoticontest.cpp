/*
    Tests for Kopete::Message::parseEmoticons

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
#include <kglobal.h>
#include <kstandarddirs.h>

#include <kunittest/module.h>
#include "kopeteemoticontest.h"
#include "kopeteemoticons.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopeteemoticontest, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopeteEmoticonTest );

/*
  There are three sets of tests, the Kopete 0.7 baseline with tests that were
  working properly in Kopete 0.7.x. When these fail it's a real regression.

  The second set are those known to work in the current codebase.

  The last set is the set with tests that are known to fail right now.

  Each entry in a set consists of two strings. The first string is the input,
  the second string is the EXPECTED output. If the two differ the test is
  considered failed and the actual result is output for comparison.

  A set should end with an entry containing at least one null pointer.
*/

typedef const char * EmoticonTestSet[][ 2 ];


static EmoticonTestSet kopete07Baseline =
{
	{ NULL, NULL }
};

static EmoticonTestSet knownGood =
{
	{ ":):)", "<img align=\"center\" width=\"20\" height=\"20\" src=\"smile.png\" title=\":)\"/>"
			  "<img align=\"center\" width=\"20\" height=\"20\" src=\"smile.png\" title=\":)\"/>" },
	{ "<img src=\"...\" title=\":-)\" />", "<img src=\"...\" title=\":-)\" />" },
	{ "End of sentence:p", "End of sentence<img align=\"center\" width=\"20\" height=\"20\" src=\"tongue.png\" title=\":p\"/>" },
	{ "http://www.kde.org", "http://www.kde.org" },
	{ "&gt;:-)", "<img align=\"center\" width=\"20\" height=\"20\" src=\"devil.png\" title=\"&gt;:-)\"/>" },
	{ NULL, NULL }
};

static EmoticonTestSet knownBroken =
{
	{ ":))", ":))" },
	{ "In a sentence:practical example", "In a sentence:practical example" },
	{ "Bla (&nbsp;)", "Bla (&nbsp;)" },
	{ ":D and :-D are not the same as :d and :-d", "<img align=\"center\" width=\"20\" height=\"20\" src=\"teeth.png\" title=\":D\"/> and <img align=\"center\" width=\"20\" height=\"20\" src=\"teeth.png\" title=\":-D\"/> are not the same as :d and :-d" },
	// A future emoticon theme may very well have an emoticon for :d/:-d (an
	// upward tongue expression - Casey
	{ "4d:D>:)F:/&gt;:-(:Pu:d9", "4d:D>:)F:/&gt;:-(:Pu:d9" },
	{ "&lt;::pvar:: test=1&gt;", "&lt;::pvar:: test=1&gt;" },
	{ "a non-breaking space (&nbsp;) character", "a non-breaking space (&nbsp;) character" },
	{ "-+-[-:-(-:-)-:-]-+-", "-+-[-:-(-:-)-:-]-+-" },
	{ "::shrugs::", "::shrugs::" },
	{ ":Ptesting:P", ":Ptesting:P" },
	{ NULL, NULL }
};

void KopeteEmoticonTest::allTests()
{
	testKnownGood();
	testKnownBroken();
}

void KopeteEmoticonTest::testKnownBroken()
{
	QString path = KGlobal::dirs()->findResource( "emoticons", "KMess-Cartoon/smile.png" ).replace( "smile.png", QString::null );
	uint i = 0;
	while ( knownBroken[ i ][ 0 ] && knownBroken[ i ][ 1 ] )
	{
		QString result = Kopete::Emoticons::parseEmoticons( knownBroken[ i ][ 0 ] ).replace( path, QString::null );	
		XFAIL(result, QString::fromLatin1(knownBroken[ i ][ 1 ]));
		i++;
	}
}
void KopeteEmoticonTest::testKnownGood()
{
	QString path = KGlobal::dirs()->findResource( "emoticons", "KMess-Cartoon/smile.png" ).replace( "smile.png", QString::null );
	uint i = 0;
	while ( knownGood[ i ][ 0 ] && knownGood[ i ][ 1 ] )
	{
		QString result = Kopete::Emoticons::parseEmoticons( knownGood[ i ][ 0 ] ).replace( path, QString::null );	
		CHECK(result, QString::fromLatin1(knownGood[ i ][ 1 ]));
		i++;
	}
}




 