/*
    kopeteidentitystatusbaricon.h  -  Kopete Identity StatusBar Dock Icon

	Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEIDENTITYSTATUSBARICON_H
#define KOPETEIDENTITYSTATUSBARICON_H

#include <QEvent>
#include <QLabel>
#include <QPoint>
#include <QMouseEvent>

namespace Kopete
{
class Identity;
class StatusRootAction;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteIdentityStatusBarIcon : public QLabel
{
	Q_OBJECT

public:
	/**
	 * Create a statusbar icon.
	 */
	KopeteIdentityStatusBarIcon( Kopete::Identity *identity, QWidget *parent );

signals:
	void leftClicked( Kopete::Identity *identity, const QPoint &p );

protected:
	virtual void mousePressEvent( QMouseEvent *me );

private slots:
	void updateMessage( Kopete::StatusRootAction *statusRootAction );

private:
	Kopete::Identity *m_identity;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

