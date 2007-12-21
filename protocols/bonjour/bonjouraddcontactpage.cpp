/*
    bonjouraddcontactpage.cpp - Kopete Bonjour Protocol

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

#include "bonjouraddcontactpage.h"

#include <qlayout.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

#include "bonjourcontact.h"

BonjourAddContactPage::BonjourAddContactPage( QWidget* parent )
		: AddContactPage(parent)
{
	kDebug(14210) ;
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget();
	m_bonjourAddUI.setupUi( w );
	l->addWidget( w );
}

BonjourAddContactPage::~BonjourAddContactPage()
{
}

bool BonjourAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
	if ( validateData() )
	{
		QString name = m_bonjourAddUI.m_uniqueName->text();

		if ( a->addContact(name, m, Kopete::Account::ChangeKABC ) )
		{
			BonjourContact * newContact = qobject_cast<BonjourContact*>( Kopete::ContactList::self()->findContact( a->protocol()->pluginId(), a->accountId(), name ) );
			if ( newContact )
			{
				newContact->setType( m_bonjourAddUI.m_rbEcho->isChecked() ? BonjourContact::Echo : BonjourContact::Group );
				return true;
			}
		}
		else
			return false;
	}
	return false;
}

bool BonjourAddContactPage::validateData()
{
    return true;
}


#include "bonjouraddcontactpage.moc"
