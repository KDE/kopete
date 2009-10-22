/*
    migrator.cpp

    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>
    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historyplugin.h"

#include <kfiledialog.h>
#include <klocalizedstring.h>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>

/*
void HistoryPlugin::migrateKopeteLogsToAkonadi()
{
    kDebug() << " ";
    const QString oldPath = "/home/roide/.kde4/share/apps/kopete";
    KUrl url;
    if ( !oldPath.isEmpty() )
        url = KUrl::fromPath( oldPath );
    else
        url = KUrl::fromPath( QDir::homePath() );

    const QString title = i18nc( "@title:window", "Select a folder or file" );
    const QString newPath = KFileDialog::getExistingDirectory( url, 0, title );
    kDebug() << newPath;
    QDirIterator it( newPath );
    while ( it.hasNext() )
    {
        it.next();
        kDebug() << it.fileName();
	QDir dir( it.filePath() );
	dir.setFilter( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable );
	const QFileInfoList entries = dir.entryInfoList();
	
    }

}
*/