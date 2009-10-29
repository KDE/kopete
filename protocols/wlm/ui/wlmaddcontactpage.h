/*
    wlmaddcontactpage.h - Kopete Wlm Protocol

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

#ifndef WLMADDCONTACTPAGE_H
#define WLMADDCONTACTPAGE_H

#include <addcontactpage.h>

namespace Kopete
{
    class Account;
}
namespace Kopete
{
    class MetaContact;
}
namespace Ui
{
    class WlmAddUI;
}

/**
 * A page in the Add Contact Wizard
 * @author Will Stephenson
*/
class WlmAddContactPage:public AddContactPage
{
	Q_OBJECT
public:
	WlmAddContactPage (Kopete::Account * account, QWidget * parent = 0);
	~WlmAddContactPage ();

	/**
	 * Make a contact out of the entered data
	 */
	virtual bool apply (Kopete::Account * a, Kopete::MetaContact * m);
	/**
	 * Is the data correct?
	 */
	virtual bool validateData ();

protected:
	Ui::WlmAddUI * m_wlmAddUI;
	Kopete::Account * m_account;
};

#endif
