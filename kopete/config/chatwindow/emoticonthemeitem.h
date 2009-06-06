#ifndef EMOTICONTHEMEITEM_H
#define EMOTICONTHEMEITEM_H

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

#include <QListWidgetItem>

class QLabel;

class EmoticonThemeItem : public QListWidgetItem
{
public:
	enum DataRole
	{
		EmoticonList = 32,
		EmoticonPixmaps = 33
	};
	EmoticonThemeItem(const QString &theme);
};

#endif
