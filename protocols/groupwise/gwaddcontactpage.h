/*
    gwaddcontactpage.h - Kopete GroupWise Protocol

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

#ifndef TESTBEDADDCONTACTPAGE_H
#define TESTBEDADDCONTACTPAGE_H

#include <addcontactpage.h>

class KopeteAccount;
class KopeteMetaContact;
class GroupWiseAddUI;

/**
 * A page in the Add Contact Wizard
 * @author Will Stephenson
*/
class GroupWiseAddContactPage : public AddContactPage
{
	Q_OBJECT
public:
    GroupWiseAddContactPage( QWidget* parent = 0, const char* name = 0 );
    ~GroupWiseAddContactPage();
	
    /**
	 * Make a contact out of the entered data
	 */
	virtual bool apply(KopeteAccount* a, KopeteMetaContact* m);
	/**
	 * Is the data correct?
	 */
    virtual bool validateData();

protected:
	GroupWiseAddUI *m_gwAddUI;
};

#endif
