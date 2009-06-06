/*
    yahooaddcontact.h - UI Page for Adding a Yahoo Contact

    Copyright (c) 2003 by Gav Wood               <gav@kde.org>
    Copyright (c) 2003 by Matt Rogers            <mattrogers@sbcglobal.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __YAHOOADDCONTACT_H
#define __YAHOOADDCONTACT_H

// Local Includes

// Kopete Includes
#include <addcontactpage.h>

// QT Includes

// KDE Includes

class YahooProtocol;
namespace Ui { class YahooAddContactBase; }
namespace Kopete { class MetaContact; }

class YahooAddContact: public AddContactPage
{
	Q_OBJECT

private:
	YahooProtocol *theProtocol;
	Ui::YahooAddContactBase *theDialog;

public:
	explicit YahooAddContact(YahooProtocol *owner, QWidget *parent = 0);
	~YahooAddContact();

	virtual bool validateData();

public slots:
	virtual bool apply(Kopete::Account *theAccount, Kopete::MetaContact *theMetaContact);
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

