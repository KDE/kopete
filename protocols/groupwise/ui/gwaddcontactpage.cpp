/*
    gwaddcontactpage.cpp - Kopete GroupWise Protocol

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

#include "gwaddcontactpage.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "gwerror.h"

#include "gwaddui.h"

GroupWiseAddContactPage::GroupWiseAddContactPage( KopeteAccount * owner, QWidget* parent, const char* name )
		: AddContactPage(parent, name)
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	if (owner->isConnected ())
	{
		m_gwAddUI = new GroupWiseAddUI( this );
		connect( m_gwAddUI->rb_userId, SIGNAL( toggled( bool ) ), SLOT( slotAddMethodChanged() ) );
		connect( m_gwAddUI->rb_userName, SIGNAL( toggled( bool ) ), SLOT( slotAddMethodChanged() ) );
		m_gwAddUI->show ();

		m_canadd = true;
	}
	else
	{
		m_noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		m_noaddMsg2 = new QLabel (i18n ("Connect to the GroupWise server and try again."), this);
		m_canadd = false;
	}
}

GroupWiseAddContactPage::~GroupWiseAddContactPage()
{
}

void GroupWiseAddContactPage::slotAddMethodChanged()
{
	if ( m_gwAddUI->rb_userId->isChecked() )
		m_gwAddUI->m_userId->setFocus();
	else
		m_gwAddUI->m_userName->setFocus();
}

bool GroupWiseAddContactPage::apply( KopeteAccount* account, KopeteMetaContact* parentContact )
{
	if ( m_canadd && validateData() )
	{
		QString contactId = m_gwAddUI->m_userId->text();
		QString displayName = parentContact->displayName();
		
		if ( displayName.isEmpty() )
			displayName = contactId;

		return ( account->addContact ( contactId, displayName, parentContact, KopeteAccount::ChangeKABC ) );
	}
	else
		return false;
}

bool GroupWiseAddContactPage::validateData()
{
    return true;
}


#include "gwaddcontactpage.moc"
