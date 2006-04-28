/***************************************************************************
                          wpaddcontact.h  -  description
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

#ifndef WPADDCONTACT_H
#define WPADDCONTACT_H

// Kopete Includes
#include <addcontactpage.h>

// QT Includes

// KDE Includes

// Local Includes

class WPAccount;
namespace Ui { class WPAddContactBase; }
namespace Kopete { class MetaContact; }

class WPAddContact: public AddContactPage
{
	Q_OBJECT

private:
	WPAccount *theAccount;
	Ui::WPAddContactBase *theDialog;

public:
	WPAddContact(QWidget *parent, WPAccount *newAccount);
	~WPAddContact();

	virtual bool validateData();

public slots:
	virtual bool apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact);

	void slotSelected(const QString &Group);
	void slotUpdateGroups();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
