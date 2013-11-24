/*
    meanwhileeditaccountwidget.h - edit an account

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
#ifndef MEANWHILEEDITACCOUNTWIDGET_H
#define MEANWHILEEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include <editaccountwidget.h>
#include "ui_meanwhileeditaccount.h"

namespace Kopete { class Account; }

class MeanwhileProtocol;

class MeanwhileEditAccountWidget :
          public QWidget,
          public KopeteEditAccountWidget
{
    Q_OBJECT
public:
    MeanwhileEditAccountWidget(QWidget* parent,
                               Kopete::Account* account,
                               MeanwhileProtocol *protocol);

    virtual ~MeanwhileEditAccountWidget();

    virtual Kopete::Account* apply();

    virtual bool validateData();

protected slots:
    void slotSetServer2Default();

protected:
    MeanwhileProtocol *protocol;

private:
    Ui::MeanwhileEditAccount ui;
    void setupClientList();
    void selectClientListItem(int selectedID);
};

#endif
