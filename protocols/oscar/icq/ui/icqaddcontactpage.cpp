 /*
    icqaddcontactpage.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz@gehn.net>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

#include "icqadd.h"
#include "icqprotocol.h"
#include "icqaccount.h"

#include <kopetecontactlist.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qlistview.h>
#include <qlabel.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

ICQAddContactPage::ICQAddContactPage(ICQAccount *owner, QWidget *parent, const char *name)
	: AddContactPage(parent,name)
{
	searchMode = 0;
	searchEvent = 0L;
	mAccount = owner;

	(new QVBoxLayout(this))->setAutoAdd(true);
	icqdata = new icqAddUI(this);

	icqdata->resultView->addColumn( i18n("Nick") );
	icqdata->resultView->addColumn( i18n("First Name") );
	icqdata->resultView->addColumn( i18n("Last Name") );
	icqdata->resultView->addColumn( i18n("UIN") );
	icqdata->resultView->addColumn( i18n("Email") );

	//TODO
/*	initCombo( icqdata->gender, 0, genders );
	initCombo( icqdata->age, 0, ages );
	initCombo( icqdata->country, 0, countries );
	initCombo( icqdata->language, 0, languages );*/

	icqdata->progressText->setText( "" );
	icqdata->progressPixmap->setPixmap( UserIcon("icq_offline") );

	connect( icqdata->startSearch, SIGNAL(clicked()), this, SLOT(slotStartSearch()) );
	connect( icqdata->stopSearch, SIGNAL(clicked()), this, SLOT(slotStopSearch()) );
	connect( icqdata->clearResults, SIGNAL(clicked()), this, SLOT(slotClearResults()) );
	connect( icqdata->searchTab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotSearchTabChanged(QWidget*)) );
	connect( icqdata->nickName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()) );
	connect( icqdata->firstName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()) );
	connect( icqdata->lastName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()) );
	connect( icqdata->uin, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()) );
	connect( icqdata->email, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()) );

	updateGui();

	if( !mAccount->isConnected() )
	{
		new QListViewItem( icqdata->resultView,
			i18n( "Adding Contacts is impossible while being offline" ), "", "", "", "" );
		new QListViewItem( icqdata->resultView,
			i18n( "Please go online before adding ICQ Contacts" ), "", "", "", "");

		icqdata->setDisabled(true);
	}
}

ICQAddContactPage::~ICQAddContactPage()
{
}

void ICQAddContactPage::slotSearchTabChanged( QWidget *tabWidget )
{
	int tab = icqdata->searchTab->indexOf( tabWidget );
	if ( tab != -1 )
	{
		searchMode = tab;
		updateGui();
	}
}

void ICQAddContactPage::slotTextChanged( void )
{
	updateGui();
}

void ICQAddContactPage::slotStartSearch( void )
{
//TODO
/*	kdDebug(14110) << "[ICQAddContactPage] slotStartSearch() with searchmode= " << searchMode << endl;
	switch ( searchMode )
	{
		case 0: // search by name
			searchEvent = mAccount->engine()->searchWP(
							icqdata->firstName->text().local8Bit(),
							icqdata->lastName->text().local8Bit(),
							icqdata->nickName->text().local8Bit(),
							icqdata->email->text().local8Bit(),
							icqdata->age->currentItem(),
							icqdata->gender->currentItem(),
							icqdata->language->currentItem(),
							icqdata->city->text().local8Bit(),
							"", // szState
							getComboValue( icqdata->country, countries ),
							"", // cCoName
							"", // szCoDept
							"", // szCoPos
							0,	// nOccupation
							0,	// nPast
							"",	// szPast
							0,	// nInterest
							"",	// szInterest
							0, 	// nAffilation
							"",	// szAffilation
							0,	// nHomepage
							"",	// szHomepage
							icqdata->onlyOnliners->isChecked() );
			break;

		case 1: // search by uin
			searchEvent = mAccount->engine()->searchByUin( icqdata->uin->text().toULong() );
			break;
	}

	if ( searchEvent )
	{
		icqdata->progressText->setText( i18n("Searching...") );
		icqdata->progressPixmap->setMovie(  QMovie( locate("data","kopete/pics/icq_connecting.mng") ) );
		connect(mAccount, SIGNAL(searchEvent(ICQEvent*)), this, SLOT(slotSearchResult(ICQEvent*)));
	}

	updateGui();*/
}

void ICQAddContactPage::slotSearchResult ( ICQEvent *e )
{
	//TODO
	/*
	if ( e != searchEvent ) // should never happen, just in case we have another search running.
		return;

	if ( e->state == ICQEvent::Fail )
	{
		removeSearch();
		icqdata->progressText->setText( i18n("No Users found") );
		updateGui();
		return;
	}

	SearchEvent *s = static_cast<SearchEvent *>(e);
	if (!s) return;

	// Do NOT allow searching for ourselves, it will break later and is stupid anyway [mETz, 26.01.2003]
	if (e->Uin() != mAccount->engine()->owner->Uin)
	{
		QListViewItem *item = new QListViewItem( icqdata->resultView,
			QString::fromLocal8Bit(s->nick.c_str()),
			QString::fromLocal8Bit(s->firstName.c_str()),
			QString::fromLocal8Bit(s->lastName.c_str()),
			QString::number(e->Uin()),
			QString::fromLocal8Bit(s->email.c_str()) );
		if ( !item )
			return;

		switch ( s->state )
		{
			case SEARCH_STATE_DISABLED:
			case SEARCH_STATE_OFFLINE:
				item->setPixmap ( 0, UserIcon("icq_offline") );
				break;
			case SEARCH_STATE_ONLINE:
				item->setPixmap ( 0, UserIcon("icq_online") );
				break;
		}
	}

	if ( s->lastResult )
	{
		removeSearch();
		icqdata->progressText->setText( i18n("Search finished") );

		if ( icqdata->resultView->childCount() == 1 )
		{
			icqdata->resultView->firstChild()->setSelected( true );
		}
	}

	updateGui();*/
}

void ICQAddContactPage::slotStopSearch(void)
{
	removeSearch();
	icqdata->progressText->setText( "" );
	updateGui();
}

void ICQAddContactPage::slotClearResults(void)
{
	icqdata->resultView->clear();
	icqdata->progressText->setText( "" );
	updateGui();
}


void ICQAddContactPage::removeSearch(void)
{
	searchEvent = 0L;
	disconnect(mAccount, SIGNAL(searchEvent(ICQEvent*)), this, SLOT(slotSearchResult(ICQEvent*)));
}

void ICQAddContactPage::updateGui(void)
{
	if (searchEvent)
	{
		icqdata->startSearch->setEnabled( false );
		icqdata->stopSearch->setEnabled( true );
		icqdata->clearResults->setEnabled( false );
		icqdata->searchTab->setEnabled( false );
	}
	else
	{
		icqdata->progressPixmap->setPixmap( UserIcon("icq_offline") );
		if( mAccount->isConnected() )
			icqdata->startSearch->setEnabled( true );
		icqdata->stopSearch->setEnabled( false );
		icqdata->searchTab->setEnabled( true );
		icqdata->clearResults->setEnabled( icqdata->resultView->childCount() );

		switch ( searchMode )
		{
			case 0:
				icqdata->startSearch->setEnabled( !icqdata->firstName->text().isEmpty()
					|| !icqdata->lastName->text().isEmpty()
					|| !icqdata->nickName->text().isEmpty()
					|| !icqdata->email->text().isEmpty() );
			break;

			case 1:
				icqdata->startSearch->setEnabled( !icqdata->uin->text().isEmpty() );
				break;
		}
	}
}

void ICQAddContactPage::slotFinish( KopeteMetaContact *parentContact )
{
	QListViewItem *item = icqdata->resultView->selectedItem();

	if ( !item )
		return;

	kdDebug(14110) << "[ICQAddContactPage] slotFinish() called; adding contact..." << endl;

	if( item->text(3).toULong() > 1000 )
	{
		QString contactId = item->text(3);
		QString displayName = item->text(0);
		kdDebug(14110) << "[ICQAddContactPage] uin: " << contactId << " displayName: " << displayName << endl;

		mAccount->addContact( contactId, displayName, parentContact);
	}
}

bool ICQAddContactPage::validateData()
{
	if( !mAccount->isConnected() )
		return false;

	if ( icqdata->resultView->selectedItem() )
		return true;
	else
		return false;
}

// vim: set noet ts=4 sts=4 sw=4:

#include "icqaddcontactpage.moc"
