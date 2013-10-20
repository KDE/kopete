/*
    qqaddcontactpage.cpp - Kopete QQ Protocol

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

#include "qqaddcontactpage.h"

#include <qlayout.h>
#include <qradiobutton.h>
#include <QVBoxLayout>
#include <qlineedit.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "ui_qqaddui.h"

QQAddContactPage::QQAddContactPage( QWidget* parent )
		: AddContactPage(parent)
{
	kDebug(14210) ;
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget();
	m_qqAddUI = new Ui::QQAddUI;
	m_qqAddUI->setupUi( w );
	l->addWidget( w );
}

QQAddContactPage::~QQAddContactPage()
{
	delete m_qqAddUI;
}

bool QQAddContactPage::apply( Kopete::Account* a, Kopete::MetaContact* m )
{
    if ( validateData() )
	{
		bool ok = false;
		QString type;
		QString name;
		if ( m_qqAddUI->m_rbEcho->isChecked() )
		{
			type = m_qqAddUI->m_uniqueName->text();
			name = QString::fromLatin1( "Echo Contact" );
			ok = true;
		}
		if ( ok )
			return a->addContact(type, /* FIXME: ? name, */ m, Kopete::Account::ChangeKABC );
		else
			return false;
	}
	return false;
}

bool QQAddContactPage::validateData()
{
    return true;
}


#include "qqaddcontactpage.moc"
