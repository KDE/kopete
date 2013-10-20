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

#include <QListWidget>
#include <QWidget>
#include <QLabel>
#include <QMovie>
#include <QMouseEvent>
#include <QHideEvent>
#include <QShowEvent>
#include <QList>

class QGridLayout;
class QHideEvent;
class QShowEvent;

class EmoticonItem : public QListWidgetItem
{
public:
	EmoticonItem(const QString &emoticonText, const QString &pixmapPath, QListWidget *parent=0);
//	~EmoticonLabel();

	QString text() const;
	QString pixmapPath() const;

private:
	QString m_text;
	QString m_pixmapPath;
};

class EmoticonSelector : public QWidget
{
	Q_OBJECT

public:

	EmoticonSelector ( QWidget *parent = 0 );
//	~EmoticonSelector();

signals:
	/**
	* gets emitted when an emoticon has been selected from the list
	* the QString holds the emoticon as a string or is 0L if nothing was selected
	**/
	void itemSelected(const QString &);

public slots:
	void prepareList();

protected:
	virtual void hideEvent( QHideEvent* );
	virtual void showEvent( QShowEvent* );

protected slots:
	void emoticonClicked( QListWidgetItem* );
	void mouseOverItem( QListWidgetItem* );
	void currentChanged();

private:
	QListWidget *m_emoticonList;
	QLabel *m_currentEmoticon;
	QMovie *m_currentMovie;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
