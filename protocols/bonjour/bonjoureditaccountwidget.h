/*
    bonjoureditaccountwidget.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
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

#ifndef BONJOUREDITACCOUNTWIDGET_H
#define BONJOUREDITACCOUNTWIDGET_H

#include <qwidget.h>
#include <ui/editaccountwidget.h>
#include <KConfigGroup>

namespace Kopete { class Account; }
namespace Ui { class BonjourAccountPreferences; }

/**
 * @brief A widget for editing this protocol's accounts
 *
 * @todo This entire Widget needs to be made more beautiful
 *
 * @author Tejas Dinkar
*/
class BonjourEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT
public:
    BonjourEditAccountWidget( QWidget* parent, Kopete::Account* account);

    ~BonjourEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();
protected:
	Ui::BonjourAccountPreferences *m_preferencesWidget;
	KConfigGroup *group;
};

#endif
