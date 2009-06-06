/***************************************************************************
                          wpeditaccount.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPEDITACCOUNT_H
#define WPEDITACCOUNT_H

// KDE Includes

// QT Includes

// Kopete Includes
#include "editaccountwidget.h"

// Local Includes
#include "wpprotocol.h"
#include "wpaccount.h"
#include "ui_wpeditaccountbase.h"

namespace Kopete { class Account; }

class WPEditAccount: public QWidget, private Ui::WPEditAccountBase, public KopeteEditAccountWidget
{
	Q_OBJECT

private:
	WPProtocol *mProtocol;
	WPAccount *mAccount;

public:
	WPEditAccount(QWidget *parent, Kopete::Account *theAccount);

	virtual bool validateData();
	void writeConfig();

public slots:
	virtual Kopete::Account *apply();
	virtual void installSamba();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
