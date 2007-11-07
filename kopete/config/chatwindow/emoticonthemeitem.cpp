/*
    emoticonthemeitem  -  Kopete Emoticon Theme Item

    Copyright (c) 2007      by Gustavo Pichorim Boiko  <gustavo.boiko@kdemail.net>

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

#include "emoticonthemeitem.h"

#include <kopeteemoticons.h>

EmoticonThemeItem::EmoticonThemeItem(const QString &theme)
: QListWidgetItem(theme, 0, UserType)
{
	// list the theme
	Kopete::Emoticons emoticons( theme );
	setData(EmoticonList, QStringList(emoticons.emoticonAndPicList().keys()));

	// set the emoticon pixmap list as an empty list: pixmaps will be added to 
	// the list as they are rendered by the delegator
	setData(EmoticonPixmaps, QList<QVariant>());
}
