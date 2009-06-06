/*
    contactselectorwidget.h

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef CONTACT_SELECTOR_WIDGET_H
#define CONTACT_SELECTOR_WIDGET_H

#include <QWidget>
#include "privacyaccountlistmodel.h"

class Ui_ContactSelectorWidget_Base;

class ContactSelectorWidget : public QWidget
{
public:
	ContactSelectorWidget( QWidget *parent = 0);
	~ContactSelectorWidget();

	QList<AccountListEntry> contacts();
private:
	Ui_ContactSelectorWidget_Base *mUi;
};

#endif
