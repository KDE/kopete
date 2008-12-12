/*
    Kopete GroupWise Protocol
    gwaddcontactpage.h- widget for adding GroupWise contacts

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

#ifndef GWADDCONTACTPAGE_H
#define GWADDCONTACTPAGE_H

#include "gwerror.h"

#include <addcontactpage.h>
#include <QtGui/QLabel>

class QLabel;
namespace Kopete { class Account; }
namespace Kopete { class MetaContact; }
class GroupWiseAccount;
class GroupWiseAddUI;
//TODO: change this to a wrapper around Contact Search and Chatroom Search
class GroupWiseContactSearch;

/**
 * A page in the Add Contact Wizard
 * @author Will Stephenson
*/
class GroupWiseAddContactPage : public AddContactPage
{
	Q_OBJECT
public:
    explicit GroupWiseAddContactPage( Kopete::Account * owner, QWidget* parent = 0 );
    ~GroupWiseAddContactPage();
	
    /**
	 * Make a contact out of the entered data
	 */
	virtual bool apply(Kopete::Account* a, Kopete::MetaContact* m);
	/**
	 * Is the data correct?
	 */
    virtual bool validateData();
private slots:
	void searchResult(bool valid);
protected:
	unsigned char searchOperation( int comboIndex );
	GroupWiseAccount * m_account;
	GroupWiseAddUI * m_gwAddUI;
	//TODO: make wrapper
	GroupWiseContactSearch * m_searchUI;
	QLabel *m_noaddMsg1;
	QLabel *m_noaddMsg2;
	bool m_canadd;
};

#endif // GWADDCONTACTPAGE_H
