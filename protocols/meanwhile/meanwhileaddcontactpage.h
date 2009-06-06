/*
    meanwhileaddcontactpage.h - add a contact

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MEANWHILEADDCONTACTPAGE_H
#define MEANWHILEADDCONTACTPAGE_H

#include <addcontactpage.h>
#include "ui_meanwhileaddcontact.h"

namespace Kopete { class Account; }
namespace Kopete { class MetaContact; }

class MeanwhileAddContactPage : public AddContactPage
{
    Q_OBJECT
public:
    MeanwhileAddContactPage( QWidget* parent = 0,
                             Kopete::Account *account=0);
    ~MeanwhileAddContactPage();

    virtual bool apply(Kopete::Account* a, Kopete::MetaContact* m);
    virtual bool validateData();

protected:
    Ui::MeanwhileAddUI ui;
    Kopete::Account *theAccount;
    QWidget *theParent;

public slots:
    void slotFindUser();
};

#endif
