/*
    emoticonselector.cpp

    a button that pops up a list of all emoticons and returns
    the emoticon-string if one is selected in the list

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
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

#include <qmovie.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qtooltip.h>
#include <qobjectlist.h>

#include <kdebug.h>

EmoticonLabel::EmoticonLabel(const QString &emoticonText, const QString &pixmapPath, QWidget *parent, const char *name)
	: QLabel(parent,name)
{
	mText = emoticonText;
	setMovie( QMovie(pixmapPath) );
	setAlignment(Qt::AlignCenter);
	QToolTip::add(this,emoticonText);
	// Somehow QLabel doesn't tell a reasonable size when you use setMovie
	// although it does it correctly for setPixmap. Therefore here is a little workaround
	// to tell our minimum size.
	QPixmap p(pixmapPath);
    //
    // Some of the custom icons are rather large
    // so lets limit them to a maximum size for this display panel
    //
    if (p.width() > 32 || p.height() > 32)
        p.resize(32, 32);
	setMinimumSize(p.size());
}

void EmoticonLabel::mouseReleaseEvent(QMouseEvent*)
{
	emit clicked(mText);
}

EmoticonSelector::EmoticonSelector(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;
	lay = 0L;
}

void EmoticonSelector::prepareList(void)
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;
	int row = 0;
	int col = 0;
	QMap<QString, QStringList> list = Kopete::Emoticons::self()->emoticonAndPicList();
	int emoticonsPerRow = static_cast<int>(sqrt(list.count()));
	//kdDebug(14000) << "emoticonsPerRow=" << emoticonsPerRow << endl;

	if ( lay )
	{
		QObjectList *objList = queryList( "EmoticonLabel" );
		//kdDebug(14000) << k_funcinfo << "There are " << objList->count() << " EmoticonLabels to delete." << endl;
		objList->setAutoDelete(true);
		objList->clear();
		delete objList;
		delete lay;
	}

	lay = new QGridLayout(this, 0, 0, 4, 4, "emoticonLayout");
	movieList.clear();
	for (QMap<QString, QStringList>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
	{
		QWidget *w = new EmoticonLabel(it.data().first(), it.key(), this);
		movieList.push_back( ((QLabel*)w)->movie() );
		connect(w, SIGNAL(clicked(const QString&)), this, SLOT(emoticonClicked(const QString&)));
//		kdDebug(14000) << "adding Emoticon to row=" << row << ", col=" << col << "." << endl;
		lay->addWidget(w, row, col);
		if ( col == emoticonsPerRow )
		{
			col = 0;
			row++;
		}
		else
			col++;
	}
	resize(minimumSizeHint());
}

void EmoticonSelector::emoticonClicked(const QString &str)
{
//	kdDebug(14000) << "selected emoticon '" << str << "'" << endl;
	// KDE4/Qt TODO: use qobject_cast instead.
	emit ItemSelected ( str );
	if ( isVisible() && parentWidget() &&
		parentWidget()->inherits("QPopupMenu") )
	{
		parentWidget()->close();
	}
}

void EmoticonSelector::hideEvent( QHideEvent* )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	MovieList::iterator it;
	for( it = movieList.begin(); it != movieList.end(); ++it )
	{
		(*it)->pause();
	}
}

void EmoticonSelector::showEvent( QShowEvent* )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	MovieList::iterator it;
	for( it = movieList.begin(); it != movieList.end(); ++it )
	{
		(*it)->unpause();
	}
}

#include "emoticonselector.moc"

// vim: set noet ts=4 sts=4 sw=4:

