/*
    kopeteaccounrstatusbaricon.h  -  Kopete Account StatusBar Dock Icon

    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEACCOUNTSTATUSBARICON_H
#define KOPETEACCOUNTSTATUSBARICON_H

#include <qevent.h>
#include <qlabel.h>
#include <qpoint.h>

class KopeteAccount;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteAccountStatusBarIcon : public QLabel
{
	Q_OBJECT

public:
	/**
	 * Create a statusbar icon.
	 */
	KopeteAccountStatusBarIcon( KopeteAccount *acc, QWidget *parent,
		const char *name = 0 );

	~KopeteAccountStatusBarIcon();

signals:
	void rightClicked( KopeteAccount *acc, const QPoint &p );
	void leftClicked( KopeteAccount *acc, const QPoint &p );

protected:
	virtual void mousePressEvent( QMouseEvent *me );

private:
	KopeteAccount *m_account;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

