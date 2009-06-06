/*
    kopeteaccountstatusbaricon.h  -  Kopete Account StatusBar Dock Icon

    Copyright (c) 2008      by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers  <kopete-devel@kde.org>

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

#include <QtGui/QLabel>

namespace Kopete
{
	class Account;
}

class QMouseEvent;
class QMovie;

/**
 * @author Roman Jarosz <kedgedev@centrum.cz>
 */
class KopeteAccountStatusBarIcon : public QLabel
{
	Q_OBJECT

public:
	/**
	 * Create a statusbar icon.
	 */
	KopeteAccountStatusBarIcon( Kopete::Account *account, QWidget *parent );
	virtual ~KopeteAccountStatusBarIcon();

protected:
	virtual void mousePressEvent( QMouseEvent *me );
	virtual bool event( QEvent *event );

private slots:
	void statusIconChanged();

private:
	Kopete::Account *mAccount;
	QMovie* mMovie;
};

#endif
