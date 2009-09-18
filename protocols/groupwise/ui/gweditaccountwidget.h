/*
    Kopete GroupWise Protocol
    gweditaccountwidget.h - widget for adding or editing GroupWise accounts

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
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

#ifndef GWEDITACCOUNTWIDGET_H
#define GWEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include <editaccountwidget.h>
#include "gwaccount.h"
#include "ui_gwaccountpreferences.h"

class QVBoxLayout;
namespace Kopete { class Account; }

/**
 * A widget for editing this protocol's accounts
 * @author Will Stephenson
*/
class GroupWiseEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT
public:
    GroupWiseEditAccountWidget( QWidget* parent, Kopete::Account* account);

    ~GroupWiseEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();
protected slots:
	void configChanged();
protected:
	bool settings_changed;
	GroupWiseAccount * account();
	void reOpen();
	void writeConfig();
	Kopete::Account *m_account;
	QVBoxLayout *m_layout;
	Ui::GroupWiseAccountPreferences m_ui;
};

#endif // GWEDITACCOUNTWIDGET_H
