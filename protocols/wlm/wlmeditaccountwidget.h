/*
    wlmeditaccountwidget.h - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
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

#ifndef WLMEDITACCOUNTWIDGET_H
#define WLMEDITACCOUNTWIDGET_H

#include <QWidget>
#include <editaccountwidget.h>

namespace Kopete
{
    class Account;
}
namespace Ui
{
    class WlmAccountPreferences;
}

/**
 * A widget for editing this protocol's accounts
 * @author Will Stephenson
*/
class WlmEditAccountWidget:public QWidget,
  public KopeteEditAccountWidget
{
  Q_OBJECT public:
    WlmEditAccountWidget (QWidget * parent, Kopete::Account * account);

    ~WlmEditAccountWidget ();

        /**
	 * Make an account out of the entered data
	 */
    virtual Kopete::Account * apply ();
        /**
	 * Is the data correct?
	 */
    virtual bool validateData ();

private slots:
    void slotOpenRegister();

private:
      Kopete::Account * m_account;
      Ui::WlmAccountPreferences * m_preferencesWidget;
};

#endif
