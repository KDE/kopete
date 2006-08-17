/*
    kopeteballoon.h  -  Nice Balloon

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2005      by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code based on Kim Applet code
    Copyright (c) 2000-2002 by Malte Starostik        <malte@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEBALLOON_H
#define KOPETEBALLOON_H

#include <qwidget.h>
#include <kactivelabel.h>

/**
 * A class derived from KActiveLabel so we can handle how
 * links are opened.
 */
class KopeteActiveLabel : public KActiveLabel
{
	Q_OBJECT

public:
	KopeteActiveLabel( QWidget *parent = 0, const char* name = 0 );
	KopeteActiveLabel( const QString& text, QWidget *parent = 0, const char* name = 0 );

public slots:
	virtual void openLink( const QString &link );
};



/**
 * A little balloon for notifications
 *
 * @author Malte Starostik <malte@kde.org>
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 */
class KopeteBalloon : public QWidget
{
	Q_OBJECT

public:
	KopeteBalloon(const QString &text, const QString &pic);
//	KopeteBalloon();

	void setAnchor(const QPoint &anchor);

signals:
	void signalButtonClicked();
	void signalIgnoreButtonClicked();
	void signalBalloonClicked();
	void signalTimeout();

protected:
	virtual void updateMask();

private:
	QPoint mAnchor;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

