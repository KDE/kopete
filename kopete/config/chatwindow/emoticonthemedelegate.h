#ifndef EMOTICONTHEMEDELEGATE_H
#define EMOTICONTHEMEDELEGATE_H

/*
    emoticonthemedelegate  -  Kopete Emoticon Theme Delegate

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

#include <QStyledItemDelegate>

class EmoticonThemeDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	EmoticonThemeDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
