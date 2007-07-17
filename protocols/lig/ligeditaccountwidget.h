/*
    ligeditaccountwidget.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIGEDITACCOUNTWIDGET_H
#define LIGEDITACCOUNTWIDGET_H

#include <qwidget.h>

#include "editaccountwidget.h"

namespace Kopete { class Account; }

class LigProtocol;

class LigEditAccountWidgetPrivate;

/**
 * @author Cláudio da Silveira Pinheiro
*/
class LigEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
	Q_OBJECT

public:
	LigEditAccountWidget( LigProtocol *proto, Kopete::Account *account, QWidget *parent = 0, const char *name = 0 );
//	LigEditAccountWidget( QWidget* parent, Kopete::Account* account);
	~LigEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();
protected:
	Kopete::Account *m_account;
//	LigAccountPreferences *m_preferencesWidget;

private slots:
/*	void slotAllow();
	void slotBlock();
	void slotShowReverseList();
	void slotSelectImage();*/
	void slotOpenRegister();

private:
	LigEditAccountWidgetPrivate *d;
};

#endif
