/*
    emoticonselector.cpp

    a button that pops up a list of all emoticons and returns
    the emoticon-string if one is selected in the list

    Copyright (c) 2002      by Stefan Gehn            <metz@gehn.net>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "emoticonselector.h"
#include "kopeteemoticons.h"

#include <math.h>

#include <QPixmap>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QObject>
#include <QHideEvent>
#include <QShowEvent>

#include <kdebug.h>
#include <kemoticons.h>

EmoticonItem::EmoticonItem(const QString &emoticonText, const QString &pixmapPath, QListWidget *parent)
	: QListWidgetItem(parent)
{
	m_text = emoticonText;
	m_pixmapPath = pixmapPath;
	QPixmap p(m_pixmapPath);
    //
    // Some of the custom icons are rather large
    // so lets limit them to a maximum size for this display panel
    //
    if (p.width() > 32 || p.height() > 32)
		p = p.scaled(QSize(32,32), Qt::KeepAspectRatio);

	setIcon(p);
}

QString EmoticonItem::text() const
{
	return m_text;
}

QString EmoticonItem::pixmapPath() const
{
	return m_pixmapPath;
}

EmoticonSelector::EmoticonSelector(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setSpacing( 0 );
	lay->setContentsMargins( 0, 0, 0, 0 );
	m_emoticonList = new QListWidget(this);
	lay->addWidget(m_emoticonList);
	m_emoticonList->setViewMode(QListView::IconMode);
	m_emoticonList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_emoticonList->setMouseTracking(true);
	m_emoticonList->setDragEnabled(false);

	m_currentEmoticon = new QLabel( this );
	m_currentEmoticon->setFrameShape( QFrame::Box );
	m_currentEmoticon->setMinimumSize(QSize(128,128));
	m_currentEmoticon->setAlignment( Qt::AlignCenter );
	lay->addWidget(m_currentEmoticon);

	m_currentMovie = new QMovie(this);
	m_currentEmoticon->setMovie(m_currentMovie);

	connect(m_emoticonList, SIGNAL(itemEntered(QListWidgetItem*)),
			this, SLOT(mouseOverItem(QListWidgetItem*)));
	connect(m_emoticonList, SIGNAL(itemSelectionChanged()),
			this, SLOT(currentChanged()));
  connect(m_emoticonList, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(emoticonClicked(QListWidgetItem*)));

}

void EmoticonSelector::prepareList(void)
{
	m_emoticonList->clear();
//	kDebug(14000) << "called.";
	QHash<QString, QStringList> list = Kopete::Emoticons::self()->theme().emoticonsMap();

	for (QHash<QString, QStringList>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
		(void) new EmoticonItem(it.value().first(), it.key(), m_emoticonList);

	m_emoticonList->setIconSize(QSize(32,32));
}

void EmoticonSelector::emoticonClicked(QListWidgetItem *i)
{
	EmoticonItem *item = dynamic_cast<EmoticonItem*>(i);
	if (!item)
		return;

	// KDE4/Qt TODO: use qobject_cast instead.
	emit itemSelected ( item->text() );
	if ( isVisible() && parentWidget() &&
		parentWidget()->inherits("QMenu") )
	{
		parentWidget()->close();
	}
}

void EmoticonSelector::mouseOverItem(QListWidgetItem *item)
{
	item->setSelected(true);	
	if (!m_emoticonList->hasFocus())
		m_emoticonList->setFocus();
}

void EmoticonSelector::currentChanged()
{

	if (!m_emoticonList->selectedItems().count())
		return;

	EmoticonItem *item = dynamic_cast<EmoticonItem*>(m_emoticonList->selectedItems().first());
	if (!item)
		return;

	m_currentMovie->stop();
	m_currentMovie->setFileName(item->pixmapPath());
	m_currentMovie->start();
	// schedule a full update of the label, so there are no glitches of the previous emoticon
	// (Qt bug?)
	m_currentEmoticon->update();
}

void EmoticonSelector::hideEvent( QHideEvent* )
{
	m_currentMovie->stop();
}

void EmoticonSelector::showEvent( QShowEvent* )
{
	m_currentMovie->start();
}

#include "emoticonselector.moc"

// vim: set noet ts=4 sts=4 sw=4:

