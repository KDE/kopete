
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "javascriptfile.h"

#include <kstandarddirs.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

JavaScriptFile::JavaScriptFile(QObject *parent)
	: QObject(parent)
{
}

QString JavaScriptFile::script( bool reload )
{
	if( reload || m_script.isEmpty() )
	{
		m_script = QString::null;
		QString localScriptsDir( KStandardDirs::locateLocal("data", QString::fromLatin1("kopete/scripts")) );
		QFile f( localScriptsDir + "/" +  id + "/" + fileName );

		if ( f.open( IO_ReadOnly ) )
		{
			QTextStream stream( &f );
			m_script = stream.read();
			f.close();
		}
	}
	return m_script;
}

#include "javascriptfile.moc"

