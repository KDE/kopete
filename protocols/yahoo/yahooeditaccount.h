/*
    yahooeditaccount.h - UI Page to edit a Yahoo account

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
    Copyright (c) 2002 by Gav Wood <gav@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOEDITACCOUNT_H
#define YAHOOEDITACCOUNT_H

// KDE Includes

// QT Includes

// Kopete Includes
#include "editaccountwidget.h"
#include "kopetepasswordwidget.h"

// Local Includes
#include "yahooaccount.h"
#include "ui_yahooeditaccountbase.h"

namespace Kopete { class Account; }

class YahooEditAccount: public QWidget, private Ui::YahooEditAccountBase, public KopeteEditAccountWidget
{
	Q_OBJECT

private:
	YahooProtocol *theProtocol;
	QString m_photoPath;

public:
	YahooEditAccount(YahooProtocol *protocol, Kopete::Account *theAccount, QWidget *parent = 0);

	bool validateData() Q_DECL_OVERRIDE;

public slots:
	Kopete::Account *apply() Q_DECL_OVERRIDE;

private slots:
	void slotOpenRegister();
	void slotSelectPicture();
};

#endif // YAHOOEDITACCOUNT_H

