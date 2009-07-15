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

#ifndef KOPETECHATWINDOWSTYLEARCHIVEFACTORY_H
#define KOPETECHATWINDOWSTYLEARCHIVEFACTORY_H

#include <karchive.h>

class KopeteChatWindowStyleArchiveFactory
{
	public:
	  static KArchive* getKArchive(const QString &path);
};

#endif
