/*
    kopeteballoon.h  -  Nice Balloon

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code based on KSim Applet code
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

class QLabel;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteBalloon : public QWidget
{
	Q_OBJECT

public:
	KopeteBalloon( const QString &text, const QString &pic );
	KopeteBalloon();
	~KopeteBalloon();

	void setAnchor( const QPoint &anchor );

	//virtual bool eventFilter( QObject *o, QEvent *ev );

signals:
	void signalButtonClicked();
	void signalIgnoreButtonClicked();
	void signalBalloonClicked();

protected:
	virtual void updateMask();

private:
	QPoint m_anchor;
	QLabel* m_image;
	QLabel* m_caption;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

