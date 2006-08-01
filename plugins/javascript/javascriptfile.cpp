
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

const QString JavaScriptFile::script( bool reload = false )
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

