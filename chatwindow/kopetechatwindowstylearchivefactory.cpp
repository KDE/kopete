 /*
    kopetechatwindowstylearchivefactory.h - Load Archive Depending on mimetype

    Copyright (c) 2009      by Pierre-Alexandre St-Jean     <pierrealexandre.stjean@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetechatwindowstylearchivefactory.h"

#include <kzip.h>
#include <ktar.h>
#include <kmimetype.h>

KArchive* KopeteChatWindowStyleArchiveFactory::getKArchive(const QString &path)
{
	QString currentBundleMimeType = KMimeType::findByPath(path, 0, false)->name();
	if(currentBundleMimeType == "application/zip")
	{
		return new KZip(path);
	}
	else if( currentBundleMimeType == "application/x-compressed-tar" || currentBundleMimeType == "application/x-bzip-compressed-tar" || currentBundleMimeType == "application/x-gzip" || currentBundleMimeType == "application/x-bzip" )
	{
		return new KTar(path);
	}
	else
	{
		return 0L;
	}
}

