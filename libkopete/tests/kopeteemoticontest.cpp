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

#include <stdlib.h>

#include <qstring.h>
#include <qdir.h>
#include <qfile.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <kunittest/module.h>
#include "kopeteemoticontest.h"
#include "kopetemessage.h"
#include "kopeteemoticons.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kopeteemoticontest, "KopeteSuite");
KUNITTEST_MODULE_REGISTER_TESTER( KopeteEmoticonTest );

/*
  There are three sets of tests, the Kopete 0.7 baseline with tests that were
  working properly in Kopete 0.7.x. When these fail it's a real regression.

  The second set are those known to work in the current codebase.
  The last set is the set with tests that are known to fail right now.

   the name convention is working|broken-number.input|output
*/


void KopeteEmoticonTest::allTests()
{
	// change user data dir to avoid messing with user's .kde dir
	setenv( "KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kopete-unittest" ), true );

	//KApplication::disableAutoDcopRegistration();
	//KApplication app;

	testEmoticonParser();
}

void KopeteEmoticonTest::testEmoticonParser()
{
	Kopete::Emoticons emo("Default");
	QString basePath = QString::fromLatin1( SRCDIR ) + QString::fromLatin1("/emoticon-parser-testcases");
	QDir testCasesDir(basePath);
	
	QStringList inputFileNames = testCasesDir.entryList("*.input");
	for ( QStringList::ConstIterator it = inputFileNames.begin(); it != inputFileNames.end(); ++it)
	{
		QString fileName = *it;
		kdDebug() << "testcase: " << fileName << endl;
		QString outputFileName = fileName;
		outputFileName.replace("input","output");
		// open the input file
		QFile inputFile(basePath + QString::fromLatin1("/") + fileName);
		QFile expectedFile(basePath + QString::fromLatin1("/") + outputFileName);
		// check if the expected output file exists
		// if it doesn't, skip the testcase
		if ( ! expectedFile.exists() )
		{
			SKIP("Warning! expected output for testcase "+ *it + " not found. Skiping testcase");
			continue;
		}
		if ( inputFile.open( IO_ReadOnly ) && expectedFile.open( IO_ReadOnly ))
		{
			QTextStream inputStream(&inputFile);
			QTextStream expectedStream(&expectedFile);
			QString inputData;
			QString expectedData;
			inputData = inputStream.read();
			expectedData = expectedStream.read();

			inputFile.close();
			expectedFile.close();

			QString path = KGlobal::dirs()->findResource( "emoticons", "Default/smile.png" ).replace( "smile.png", QString::null );
				
			Kopete::Emoticons::self();
			QString result = emo.parse( inputData ).replace( path, QString::null );	
			
			// HACK to know the test case we applied, concatenate testcase name to both
			// input and expected string. WIll remove when I can add some sort of metadata
			// to a CHECK so debug its origin testcase
			result = fileName + QString::fromLatin1(": ") + result;
			expectedData = fileName + QString::fromLatin1(": ") + expectedData;
			// if the test case begins with broken, we expect it to fail, then use XFAIL
			// otherwise use CHECK
			if ( fileName.section("-", 0, 0) == QString::fromLatin1("broken") )
			{
				kdDebug() << "checking known-broken testcase: " << fileName << endl;
				XFAIL(result, expectedData);
			}
			else
			{
				kdDebug() << "checking known-working testcase: " << fileName << endl;
				CHECK(result, expectedData);
			}
		}
		else
		{
			SKIP("Warning! can't open testcase files for "+ *it + ". Skiping testcase");
			continue;
		}
	}
	
}




 