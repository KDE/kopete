/***************************************************************************
                            description
                             -------------------
    begin                :
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "aimaddcontactpage.h"
#include "ui_aimaddcontactui.h"

#include "kopeteaccount.h"

#include <kmessagebox.h>

#include "oscarutils.h"

AIMAddContactPage::AIMAddContactPage(bool connected, QWidget *parent)
	: AddContactPage(parent)
{
    m_gui = 0;
	if(connected)
	{
		m_gui = new Ui::aimAddContactUI();
		m_gui->setupUi(this);
		connect( m_gui->icqRadioButton, SIGNAL(toggled(bool)), m_gui->icqEdit, SLOT(setEnabled(bool)) );
		connect( m_gui->aimRadioButton, SIGNAL(toggled(bool)), m_gui->aimEdit, SLOT(setEnabled(bool)) );
		m_gui->aimEdit->setFocus();
		canadd = true;
	}
	else
	{
		QVBoxLayout *layout = new QVBoxLayout( this );
		layout->setContentsMargins( 0, 0, 0, 0 );

		layout->addWidget( new QLabel(i18n("You need to be connected to be able to add contacts.\nConnect to the AIM network and try again."), this) );
		canadd = false;
	}
}


AIMAddContactPage::~AIMAddContactPage()
{
	delete m_gui;
}

bool AIMAddContactPage::validateData()
{
    if ( !canadd )
        return false;

    if ( !m_gui )
        return false;

	if ( m_gui->icqRadioButton->isChecked() )
	{
		ulong uin = m_gui->icqEdit->text().toULong();
		if ( uin < 1000 )
		{
			KMessageBox::sorry( this, i18n("You must enter a valid ICQ number."), i18n("ICQ Plugin") );
			return false;
		}
		return true;
	}
	else if ( m_gui->aimRadioButton->isChecked() )
	{
		QRegExp rx("^[0-9]*$");
		if ( rx.exactMatch( m_gui->aimEdit->text() ) )
		{
			KMessageBox::sorry( this, i18n("You must enter a valid AOL screen name."), i18n("No Screen Name") );
			return false;
		}
		return true;
	}

	return false;
}

bool AIMAddContactPage::apply(Kopete::Account *account,
	Kopete::MetaContact *metaContact)
{
	if ( m_gui->icqRadioButton->isChecked() )
	{
		QString contactId = Oscar::normalize( m_gui->icqEdit->text() );
		return account->addContact( contactId, metaContact, Kopete::Account::ChangeKABC );
	}
	else if ( m_gui->aimRadioButton->isChecked() )
	{
		QString contactId = Oscar::normalize( m_gui->aimEdit->text() );
		return account->addContact( contactId, metaContact, Kopete::Account::ChangeKABC );
	}
	return false;
}
//kate: tab-width 4; indent-mode csands;

#include "aimaddcontactpage.moc"
