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

namespace Kopete
{
class Account;
}

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
	KopeteAccountStatusBarIcon( Kopete::Account *acc, QWidget *parent,
		const char *name = 0 );

	~KopeteAccountStatusBarIcon();

signals:
	void rightClicked( Kopete::Account *acc, const QPoint &p );
	void leftClicked( Kopete::Account *acc, const QPoint &p );

protected:
	virtual void mousePressEvent( QMouseEvent *me );

private:
	Kopete::Account *m_account;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

