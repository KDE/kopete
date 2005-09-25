/*
    emoticonselector.h

    a button that pops up a list of all emoticons and returns
    the emoticon-string if one is selected in the list

    Copyright (c) 2002 by Stefan Gehn            <metz AT gehn.net>
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

#ifndef __emoticonselector_h__
#define __emoticonselector_h__

#include <qlabel.h>
#include <qwidget.h>
class QGridLayout;
class QHideEvent;
class QShowEvent;

class EmoticonLabel : public QLabel
{
	Q_OBJECT

public:
	EmoticonLabel(const QString &emoticonText, const QString &pixmapPath, QWidget *parent=0, const char *name=0);
//	~EmoticonLabel();

signals:
	void clicked(const QString &text);

protected:
	void mouseReleaseEvent(QMouseEvent*);
	QString mText;
};

class EmoticonSelector : public QWidget
{
	Q_OBJECT

public:

	EmoticonSelector ( QWidget *parent = 0, const char *name = 0 );
//	~EmoticonSelector();

	typedef QValueList<QMovie*> MovieList;
signals:
	/**
	* gets emitted when an emoticon has been selected from the list
	* the QString holds the emoticon as a string or is 0L if nothing was selected
	**/
	void ItemSelected(const QString &);

public slots:
	void prepareList();

protected:
	virtual void hideEvent( QHideEvent* );
	virtual void showEvent( QShowEvent* );
	MovieList movieList;
	QGridLayout *lay;

protected slots:
	void emoticonClicked(const QString &);
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
