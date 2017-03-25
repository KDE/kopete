/*
    irceditaccountwidget.h - IRC Account Widget

    Copyright (c) 2003      by Olivier Goffart  <ogoffart@kde.org>
    Copyright (c) 2003      by Jason Keirstead  <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCEDITACCOUNTWIDGET_H
#define IRCEDITACCOUNTWIDGET_H

#include "editaccountwidget.h"
#include "ui_irceditaccount.h"

class IRCAccount;
class K3ListView;
class Q3ListViewItem;

class IRCEditAccountWidget : public QWidget, public Ui::IRCEditAccountBase, public KopeteEditAccountWidget
{
    Q_OBJECT

public:
    explicit IRCEditAccountWidget(IRCAccount *account, QWidget *parent = 0);
    ~IRCEditAccountWidget();

    IRCAccount *account();
    virtual bool validateData();
    virtual Kopete::Account *apply();

private slots:
    void slotCommandContextMenu(K3ListView *, Q3ListViewItem *, const QPoint &);
    void slotCtcpContextMenu(K3ListView *, Q3ListViewItem *, const QPoint &);
    void slotAddCommand();
    void slotAddCtcp();
    void slotEditNetworks();
    void slotUpdateNetworks(const QString &);
    void slotUpdateNetworkDescription(const QString &);

private:
    void readNetworks();
    QString generateAccountId(const QString &network);
};

#endif // IRCEDITACCOUNTWIDGET_H
