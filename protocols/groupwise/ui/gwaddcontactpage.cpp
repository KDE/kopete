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

#include <qlayout.h>
#include <qradiobutton.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "gwerror.h"

#include "gwaddui.h"

GroupWiseAddContactPage::GroupWiseAddContactPage( QWidget* parent, const char* name )
		: AddContactPage(parent, name)
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_gwAddUI = new GroupWiseAddUI( this );
}

GroupWiseAddContactPage::~GroupWiseAddContactPage()
{
}

bool GroupWiseAddContactPage::apply( KopeteAccount* a, KopeteMetaContact* m )
{
    if ( validateData() )
	{
		bool ok = false;
		QString type;
		QString name;
		if ( m_gwAddUI->m_rbEcho->isOn() )
		{
			type = QString::fromLatin1( "echo" );
			name = QString::fromLatin1( "Echo Contact" );
			ok = true;
		}
		if ( ok )
			return a->addContact(type, name, m, KopeteAccount::ChangeKABC );
		else
			return false;
	}
	return false;
}

bool GroupWiseAddContactPage::validateData()
{
    return true;
}


#include "gwaddcontactpage.moc"
