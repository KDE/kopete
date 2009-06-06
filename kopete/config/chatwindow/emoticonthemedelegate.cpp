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

#include "emoticonthemedelegate.h"
#include "emoticonthemeitem.h" // for the enum
#include <QModelIndex>
#include <QPainter>
#include <QApplication>

EmoticonThemeDelegate::EmoticonThemeDelegate(QObject *parent)
: QStyledItemDelegate(parent)
{
}

void EmoticonThemeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

	QString theme = index.data().toString();

	QVariant v = index.data(EmoticonThemeItem::EmoticonPixmaps);
	QList<QVariant> pixmapList = qvariant_cast<QList<QVariant> >(v);

	painter->save();
		painter->translate(option.rect.topLeft());
		if (option.state & QStyle::State_Selected)
			painter->setPen(option.palette.color(QPalette::Normal, QPalette::HighlightedText));
		else
			painter->setPen(option.palette.color(QPalette::Normal, QPalette::Text));
		QFont f = painter->font();
		f.setBold(true);
		painter->setFont(f);
		painter->drawText(10,20, theme);

		QSize s = sizeHint(option, index);

		// draw the emoticons themselves
		QPoint top(10, 22);
		int maxHeight = s.height() - top.y() - 2;
		int middle = (maxHeight / 2) + top.y();
		QStringList emotes = qvariant_cast<QStringList>(index.data(Qt::UserRole));
		int count = 0;
		foreach(const QString &emote, emotes)
		{
			QPixmap pix;

			// check if we have already loaded the requested pixmap
			if (count < pixmapList.count())
			{
				pix = pixmapList.at(count++).value<QPixmap>();
			}
			else
			{
				pix.load(emote);
				pixmapList.append(pix);
			}

			if (pix.isNull())
				continue;

			if (top.x() + pix.width() > option.rect.width() - 10)
				break;

			// check if the emoticon height is bigger than the maximum allowed
			if (pix.height() > maxHeight)
				pix = pix.scaledToHeight(maxHeight);
			
			top.setY(middle - pix.height()/2);
			painter->drawPixmap(top, pix);
			top.setX(top.x() + pix.width() + 2);
		}
	painter->restore();

	// set the pixmapList as data of the index
	QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
	model->setData(index, pixmapList, EmoticonThemeItem::EmoticonPixmaps);
}

QSize EmoticonThemeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);

    return QSize(100,100);
}

