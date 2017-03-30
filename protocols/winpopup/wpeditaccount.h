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

namespace Kopete {
class Account;
}

class WPEditAccount : public QWidget, private Ui::WPEditAccountBase, public KopeteEditAccountWidget
{
    Q_OBJECT

private:
    WPProtocol *mProtocol;
    WPAccount *mAccount;

public:
    WPEditAccount(QWidget *parent, Kopete::Account *theAccount);

    bool validateData() Q_DECL_OVERRIDE;
    void writeConfig();

public Q_SLOTS:
    Kopete::Account *apply() Q_DECL_OVERRIDE;
    virtual void installSamba();
};

#endif
