/*
    statusbaricon.h  -  Kopete StatusBar Dock Icon

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPROTOCOLSTATUSBARICON_H
#define KOPETEPROTOCOLSTATUSBARICON_H

#include <qevent.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpoint.h>

class KopeteProtocol;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteProtocolStatusBarIcon : public QLabel
{
	Q_OBJECT

public:
	/**
	 * Create a statusbar icon.
	 */
	KopeteProtocolStatusBarIcon( KopeteProtocol *proto, QWidget *parent,
		const char *name = 0 );

	~KopeteProtocolStatusBarIcon();

signals:
	void rightClicked( KopeteProtocol *proto, const QPoint &p );

protected:
	virtual void mousePressEvent( QMouseEvent *me );

private:
	KopeteProtocol *m_proto;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

