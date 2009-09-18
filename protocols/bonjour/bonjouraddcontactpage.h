/*
    bonjouraddcontactpage.h - Kopete Bonjour Protocol

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

#ifndef BONJOURADDCONTACTPAGE_H
#define BONJOURADDCONTACTPAGE_H

#include <ui/addcontactpage.h>
#include "ui_bonjouraddui.h"

namespace Kopete { class Account; }
namespace Kopete { class MetaContact; }
namespace Ui { class BonjourAddUI; }

/**
 * @brief A Dummy Widget that Shows a Message
 * A page in the Add Contact Wizard
 * This just shows a messaging saying `sorry, you cannot add anyone`
 * @todo Remove this All Together
 * @author Tejas Dinkar <tejas\@gja.in>
*/
class BonjourAddContactPage : public AddContactPage
{
	Q_OBJECT
public:
	BonjourAddContactPage( QWidget* parent = 0 );
	~BonjourAddContactPage();

    	/**
	 * Make a contact out of the entered data
	 */
	virtual bool apply(Kopete::Account* a, Kopete::MetaContact* m);

	/**
	 * Is the data correct?
	 */
	virtual bool validateData();

protected:
	Ui::BonjourAddUI m_bonjourAddUI;
};

#endif
