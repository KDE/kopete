 /*
    icqaddcontactpage.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz@gehn.net>

	Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "icqaddcontactpage.h"

#include <ctype.h>

#include <kdebug.h>
#include <kmessagebox.h>

#include "ui_icqadd.h"
#include "icqaccount.h"
#include "icqsearchdialog.h"

#include "oscarutils.h"


ICQAddContactPage::ICQAddContactPage(ICQAccount *owner, QWidget *parent)
	: AddContactPage(parent)
{
	kDebug(14153) << "called";
	mAccount = owner;
	m_searchDialog = 0L;

	addUI = new Ui::icqAddUI();
	addUI->setupUi(this);
	connect( addUI->searchButton, SIGNAL(clicked()), this, SLOT(showSearchDialog()) );
	connect( addUI->icqRadioButton, SIGNAL(toggled(bool)), addUI->icqEdit, SLOT(setEnabled(bool)) );
	connect( addUI->icqRadioButton, SIGNAL(toggled(bool)), addUI->searchButton, SLOT(setEnabled(bool)) );
	connect( addUI->aimRadioButton, SIGNAL(toggled(bool)), addUI->aimEdit, SLOT(setEnabled(bool)) );
	addUI->icqEdit->setFocus();
}

ICQAddContactPage::~ICQAddContactPage()
{
	delete addUI;
}

void ICQAddContactPage::setUINFromSearch( const QString& uin )
{
	addUI->icqEdit->setText( uin );
}

void ICQAddContactPage::showEvent(QShowEvent *e)
{
//	slotSelectionChanged();
	AddContactPage::showEvent(e);
}

bool ICQAddContactPage::apply(Kopete::Account* , Kopete::MetaContact *parentContact  )
{
	kDebug(14153) << "called; adding contact...";

	if ( addUI->icqRadioButton->isChecked() )
	{
		QString contactId = Oscar::normalize( addUI->icqEdit->text() );
		return mAccount->addContact( contactId, parentContact, Kopete::Account::ChangeKABC );
	}
	else if ( addUI->aimRadioButton->isChecked() )
	{
		QString contactId = Oscar::normalize( addUI->aimEdit->text() );
		return mAccount->addContact( contactId, parentContact, Kopete::Account::ChangeKABC );
	}
	return false;
}

bool ICQAddContactPage::validateData()
{
	if(!mAccount->isConnected())
	{
		// Account currently offline
		KMessageBox::sorry( this, i18n("You must be online to add a contact."), i18n("ICQ Plugin") );
		return false;
	}

	if ( addUI->icqRadioButton->isChecked() )
	{
		ulong uin = addUI->icqEdit->text().toULong();
		if ( uin < 1000 )
		{
			KMessageBox::sorry( this, i18n("You must enter a valid ICQ number."), i18n("ICQ Plugin") );
			return false;
		}
		return true;
	}
	else if ( addUI->aimRadioButton->isChecked() )
	{
		QRegExp rx("^[0-9]*$");
		if ( rx.exactMatch( addUI->aimEdit->text() ) )
		{
			KMessageBox::sorry( this, i18n("You must enter a valid AOL screen name."), i18n("ICQ Plugin") );
			return false;
		}
		return true;
	}
	return false;
}

void ICQAddContactPage::showSearchDialog()
{
	if ( m_searchDialog )
		m_searchDialog->raise();
	else
	{
		m_searchDialog = new ICQSearchDialog( mAccount, this );
		m_searchDialog->show();
		connect( m_searchDialog, SIGNAL(finished()), this, SLOT(searchDialogDestroyed()) );
	}
}

void ICQAddContactPage::searchDialogDestroyed()
{
	QObject::disconnect( this, 0, m_searchDialog, 0 );
	m_searchDialog->deleteLater();
	m_searchDialog = NULL;
}


#include "icqaddcontactpage.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; space-indent off; replace-tabs off;
