/*
    ContactList Layout Widget

    Copyright (c) 2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2009      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CONTACTLISTLAYOUTWIDGET_H
#define CONTACTLISTLAYOUTWIDGET_H

#include <QWidget>

#include "ui_contactlistlayoutwidget.h"

class ContactListLayoutWidget : public QWidget, private Ui::ContactListLayoutWidget
{
	Q_OBJECT
public:
	ContactListLayoutWidget( QWidget *parent = 0 );

	void load();
	bool save();

signals:
	void changed();

private slots:
	void emitChanged();
	void setLayout( const QString &layoutName );
	void reloadLayoutList();
	void preview();
	void remove();

private:
	bool saveLayoutData( QString& layoutName, bool showPrompt = false );

private:
	QString mCurrentLayoutName;
	bool mChanged;
	bool mLoading;
};

#endif
