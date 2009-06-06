/*
    kconf_update app for migrating the list of loaded plugins in
    kopete 0.7.x to the new KPluginSelector format.

    Copyright (c) 2003      by Martijn Klingens <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qtextstream.h>
#include <qregexp.h>
#include <QStringList>

static QTextStream qcin ( stdin,  QIODevice::ReadOnly );
static QTextStream qcout( stdout, QIODevice::WriteOnly );
static QTextStream qcerr( stderr, QIODevice::WriteOnly );

void parseKey( const QString &group, const QString &key, const QString &value, const QString &rawLine )
{
	//qcerr << "*** group='" << group << "'" << endl;
	if ( group.isEmpty() && key == "Plugins" )
	{
		QStringList plugins = QStringList::split( ',', value );
		if ( !plugins.isEmpty() )
		{
			qcout << "[Plugins]" << endl;
			for ( QStringList::Iterator it = plugins.begin(); it != plugins.end(); ++it )
				qcout << "kopete_" << ( *it ).remove( ".desktop" ) << "Enabled=true" << endl;
		}
		qcout << "# DELETE []Plugins" << endl;
	}
	else
	{
		// groups we don't convert. output the raw line instead.
		qcout << rawLine << endl;
	}
}

int main()
{
	qcin.setCodec(QTextCodec::codecForName("UTF-8"));
	qcout.setCodec(QTextCodec::codecForName("UTF-8"));

	QString curGroup;

	QRegExp groupRegExp( "^\\[(.*)\\]" );
	QRegExp keyRegExp( "^([a-zA-Z0-9:, _-]*)\\s*=\\s*(.*)\\s*" );
	QRegExp commentRegExp( "^(#.*)?$" );

	while ( !qcin.atEnd() )
	{
		QString line = qcin.readLine();

		if ( commentRegExp.exactMatch( line ) )
		{
			// We found a comment, leave unchanged
			qcout << line << endl;
		}
		else if ( groupRegExp.exactMatch( line ) )
		{
			// We found the start of a group, leave unchanged
			qcout << line << endl;

			curGroup = groupRegExp.capturedTexts()[ 1 ];
		}
		else if ( keyRegExp.exactMatch( line ) )
		{
			// We found the a key line
			parseKey( curGroup, keyRegExp.capturedTexts()[ 1 ], keyRegExp.capturedTexts()[ 2 ], line );
		}
		else
		{
			qcerr << "** Unknown input line: " << line << endl;
		}
	}

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

