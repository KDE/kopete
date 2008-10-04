/*
    main.cpp

    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "tests.h"

#include <QCoreApplication>
#include <QFile>
#include <stdio.h>

/*
* This is an example of a program/script that could be used with
* the pipes plugin. We read everything from stdin, then perform a
* series of transformations on it. The actual work is done in
* KopeteXmlParserTests::test*
* The final text is then printed to stdout, where it is picked up
* by the Pipes plugin.
* Please see the real documentation for details on how to write
* pipes for the Pipes plugin.
*/

int main (int argc, char ** argv)
{
	QCoreApplication app (argc, argv);
	
	// open standard input
	QFile input (&app);
	input.open (stdin, QIODevice::ReadOnly | QIODevice::Text);
	
	// do tests
	QString parsed = KopeteXmlParserTests::test1 ( input.readAll() );
	
	// write modified text to standard out
	QFile output (&app);
	output.open (stdout, QIODevice::WriteOnly | QIODevice::Text );
	output.write ( parsed.toLocal8Bit() );
}

