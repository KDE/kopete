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

	bool validateData() Q_DECL_OVERRIDE;

public slots:
	bool apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact) Q_DECL_OVERRIDE;

	void slotSelected(const QString &Group);
	void slotUpdateGroups();
};

#endif

