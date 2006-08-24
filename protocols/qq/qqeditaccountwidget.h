/*
    qqeditaccountwidget.h - Kopete QQ Protocol
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef QQEDITACCOUNTWIDGET_H
#define QQEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include <editaccountwidget.h>

namespace Kopete { class Account; }
class QQEditAccountWidgetPrivate;
class QQProtocol;

/**
 * A widget for editing this protocol's accounts
 * @author Hui Jin based on Oliver Goffart's MSNEditAccountWidget
*/
class QQEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT
public:
	QQEditAccountWidget( QQProtocol *proto, Kopete::Account *account, QWidget *parent );

    ~QQEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();

private slots:
	void slotOpenRegister();

private:
	QQEditAccountWidgetPrivate* d;
};

#endif
