 /*
    icqaddcontactpage.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2002 by Stefan Gehn <metz AT gehn.net>
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

#include "icqadd.h"
#include "icqaccount.h"
#include "icqprotocol.h"
#include <klistview.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

ICQAddContactPage::ICQAddContactPage(ICQAccount *owner, QWidget *parent, const char *name)
	: AddContactPage(parent,name)
{
	kdDebug(14153) << k_funcinfo << "called" << endl;
	searchMode = 0;
	searching = false;
	mAccount = owner;

	(new QVBoxLayout(this))->setAutoAdd(true);
	icqdata = new icqAddUI(this);


	icqdata->resultView->addColumn(i18n("Nick"));
	icqdata->resultView->addColumn(i18n("First Name"));
	icqdata->resultView->addColumn(i18n("Last Name"));
	icqdata->resultView->addColumn(i18n("UIN"));
	icqdata->resultView->addColumn(i18n("Email"));

	ICQProtocol *p = ICQProtocol::protocol();
	p->fillComboFromTable(icqdata->gender, p->genders());
	//p->fillComboFromTable(icqdata->age, p->genders());
	icqdata->age->setEnabled(false); // TODO
	p->fillComboFromTable(icqdata->country, p->countries());
	p->fillComboFromTable(icqdata->language, p->languages());

	icqdata->progressText->setText("");
	icqdata->progressPixmap->setPixmap(SmallIcon("icq_offline"));

	connect(icqdata->startSearch, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
	connect(icqdata->stopSearch, SIGNAL(clicked()), this, SLOT(slotStopSearch()));
	connect(icqdata->clearResults, SIGNAL(clicked()), this, SLOT(slotClearResults()));
	connect(icqdata->searchTab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotSearchTabChanged(QWidget*)));
	connect(icqdata->nickName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()));
	connect(icqdata->firstName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()));
	connect(icqdata->lastName, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()));
	connect(icqdata->uin, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()));
	connect(icqdata->email, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged()));
	connect(icqdata->resultView, SIGNAL(selectionChanged()),
		this, SLOT(slotSelectionChanged()));

	updateGui();

	if(!mAccount->isConnected())
	{
		new QListViewItem(icqdata->resultView,
			i18n("Adding contacts is impossible while offline"), "", "", "", "");
		new QListViewItem(icqdata->resultView,
			i18n("Please go online before adding ICQ contacts"), "", "", "", "");

		icqdata->progressText->setText(i18n("Adding contacts is impossible while offline"));

		icqdata->startSearch->setDisabled(true);
		icqdata->stopSearch->setDisabled(true);
		icqdata->clearResults->setDisabled(true);
		icqdata->searchTab->setTabEnabled(icqdata->tab, false);
		icqdata->searchTab->setTabEnabled(icqdata->tab_2, false);
	}
}

ICQAddContactPage::~ICQAddContactPage()
{
}

void ICQAddContactPage::showEvent(QShowEvent *e)
{
	slotSelectionChanged();
	AddContactPage::showEvent(e);
}

void ICQAddContactPage::slotSearchTabChanged(QWidget *tabWidget)
{
	int tab = icqdata->searchTab->indexOf(tabWidget);
	if(tab != -1)
	{
		searchMode = tab;
		kdDebug(14153) << k_funcinfo << "searchMode=" << searchMode << endl;
		updateGui();
	}
}

void ICQAddContactPage::slotTextChanged()
{
	updateGui();
}

void ICQAddContactPage::slotStartSearch()
{
	ICQProtocol *p = ICQProtocol::protocol();

	kdDebug(14153) << k_funcinfo << "Called; searchmode= " << searchMode << endl;
	switch(searchMode)
	{
		case 0: // search by name
		{
			mAccount->engine()->sendCLI_SEARCHWP(
				icqdata->firstName->text(),
				icqdata->lastName->text(),
				icqdata->nickName->text(),
				icqdata->email->text(),
				0/*icqdata->age->currentItem()*/, 0, // min and max age
				icqdata->gender->currentItem(), // Gender
				p->getCodeForCombo(icqdata->language, p->languages()), // Lang
				icqdata->city->text(), // City
				QString::null, // State
				p->getCodeForCombo(icqdata->country, p->countries()), // country code
				QString::null, // company
				QString::null, // department
				QString::null, // position
				0, // Occupation
				icqdata->onlyOnliners->isChecked());
			searching = true;
			break;
		}

		case 1: // search by uin
			mAccount->engine()->sendCLI_SEARCHBYUIN(icqdata->uin->text().toULong());
			searching = true;
			break;
	}

	if (searching)
	{
		icqdata->progressText->setText(i18n("Searching..."));
//		icqdata->progressPixmap->setMovie(QMovie(locate("data","kopete/pics/icq_connecting.mng") ));
		icqdata->progressPixmap->setPixmap(SmallIcon("icq_online"));
		connect(
			mAccount->engine(), SIGNAL(gotSearchResult(ICQSearchResult &, const int)),
			this, SLOT(slotSearchResult(ICQSearchResult &, const int)));
	}

	updateGui();
}


void ICQAddContactPage::slotSearchResult (ICQSearchResult &res, const int missed)
{
	/*kdDebug(14153) << k_funcinfo << "searching=" << searching <<
		", res.uin=" << res.uin << ", missed=" << missed << endl;*/

	if(!searching)
		return;

	if(res.uin == 1 && missed == 0)
	{
		removeSearch();
		icqdata->progressText->setText(i18n("No Users found"));
		updateGui();
		return;
	}

	// Do NOT allow searching for ourselves, it will break later
	// and is stupid anyway [mETz, 26.01.2003]
	if (res.uin != mAccount->accountId().toULong())
	{
		QListViewItem *item = new QListViewItem(icqdata->resultView,
			res.nickName, res.firstName, res.lastName, QString::number(res.uin),
			res.eMail);

		if (!item)
			return;

		switch(res.status)
		{
			case ICQ_SEARCHSTATE_DISABLED:
			case ICQ_SEARCHSTATE_OFFLINE:
				item->setPixmap(0, SmallIcon("icq_offline"));
				break;
			case ICQ_SEARCHSTATE_ONLINE:
				item->setPixmap(0, SmallIcon("icq_online"));
				break;
		}
	}

	if(missed != -1)
	{
		removeSearch();
		if(missed == 0)
		{
			icqdata->progressText->setText(i18n("Search finished"));
		}
		else
		{
			icqdata->progressText->setText(
				i18n("Search finished. %n search result not shown",
				"Search finished. %n search results not shown", missed));
		}

		if(icqdata->resultView->childCount() == 1)
			icqdata->resultView->firstChild()->setSelected(true);
	}

	updateGui();
}


void ICQAddContactPage::slotStopSearch()
{
	removeSearch();
	icqdata->progressText->setText("");
	updateGui();
}

void ICQAddContactPage::slotClearResults()
{
	icqdata->resultView->clear();
	icqdata->progressText->setText("");
	updateGui();
}


void ICQAddContactPage::removeSearch()
{
	searching=false;
	disconnect(
		mAccount->engine(), SIGNAL(gotSearchResult(ICQSearchResult &, const int)),
		this, SLOT(slotSearchResult(ICQSearchResult &, const int)));
}

void ICQAddContactPage::updateGui()
{
	if(searching)
	{
		icqdata->startSearch->setEnabled(false);
		icqdata->stopSearch->setEnabled(true);
		icqdata->clearResults->setEnabled(false);
		icqdata->searchTab->setEnabled(false);
	}
	else
	{
		icqdata->progressPixmap->setPixmap(SmallIcon("icq_offline"));
		icqdata->searchTab->setEnabled(mAccount->isConnected());
		icqdata->startSearch->setEnabled(mAccount->isConnected());
		icqdata->stopSearch->setEnabled(false);
		icqdata->clearResults->setEnabled(icqdata->resultView->childCount()>0 && mAccount->isConnected());
		slotSelectionChanged();

		switch(searchMode)
		{
			case 0:
				/*icqdata->startSearch->setEnabled(!icqdata->firstName->text().isEmpty()
					|| !icqdata->lastName->text().isEmpty()
					|| !icqdata->nickName->text().isEmpty()
					|| !icqdata->email->text().isEmpty());*/
			break;

			case 1:
				icqdata->startSearch->setEnabled((icqdata->uin->text().length() > 4));
				break;
		}
	}
}

bool ICQAddContactPage::apply(Kopete::Account* , Kopete::MetaContact *parentContact  )
{
	QListViewItem *item = icqdata->resultView->selectedItem();

	if (!item )
		return false;

	kdDebug(14153) << k_funcinfo << "called; adding contact..." << endl;

	if(item->text(3).toULong() > 1000)
	{
		QString contactId = item->text(3);
		QString displayName = item->text(0);
		kdDebug(14153) << k_funcinfo << "uin=" << contactId << ", displayName=" << displayName << endl;

		return mAccount->addContact(contactId, displayName, parentContact, Kopete::Account::ChangeKABC );
	}
	return false;
}

bool ICQAddContactPage::validateData()
{
	if(!mAccount->isConnected())
		return false;

	return (icqdata->resultView->selectedItem() != 0L);
}

void ICQAddContactPage::slotSelectionChanged()
{
	bool valid;
	kdDebug(14153) << k_funcinfo << endl;
	valid = icqdata->resultView->selectedItem() != 0L;
	if (!mAccount->isConnected())
	{
		valid = false;
	}
	emit dataValid( this, valid);
}

#include "icqaddcontactpage.moc"
// vim: set noet ts=4 sts=4 sw=4:
