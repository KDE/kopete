/*
    ircaddcontactpage.cpp - IRC Add Contact Widget

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include "ircaddcontactpage.h"
#include "ircadd.h"
#include "ircaccount.h"
#include "kirc.h"

class ChannelListItem : public QListViewItem
{
	public:
		ChannelListItem( QListView *parent, QString arg1, QString arg2, QString arg3 );
		virtual int compare( QListViewItem *i, int col, bool ascending ) const;
};

ChannelListItem::ChannelListItem( QListView *parent, QString arg1, QString arg2, QString arg3 ) :
	QListViewItem( parent, arg1, arg2, arg3 ) {}

int ChannelListItem::compare( QListViewItem *i, int col, bool ascending ) const
{
	if( col == 1 )
	{
		if( text(1).toUInt() < i->text(1).toUInt() )
			return -1;
		else if ( text(1).toUInt() == i->text(1).toUInt() )
			return 0;
		else
			return 1;
	}
	else
		return QListViewItem::compare( i, col, ascending );
}


IRCAddContactPage::IRCAddContactPage( QWidget *parent, IRCAccount *a ) : AddContactPage(parent, 0)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	mAccount = a;

	connect( ircdata->searchButton, SIGNAL( clicked() ), this, SLOT( slotSearch() ) );
	connect( ircdata->searchText, SIGNAL( returnPressed() ), this, SLOT( slotSearch() ) );
	connect( ircdata->searchResults, SIGNAL( selectionChanged( QListViewItem*)), this,
		SLOT( slotSelectionChanged( QListViewItem *) ) );
}
IRCAddContactPage::~IRCAddContactPage()
{
}

void IRCAddContactPage::slotSearch()
{
	ircdata->searchResults->clear();
	ircdata->searchResults->setEnabled(false);

	if( mAccount->isConnected() )
	{
		ircdata->searchButton->setEnabled(false);
		search = ircdata->searchText->text();
		connect( mAccount->engine(), SIGNAL( incomingListedChan( const QString &, uint, const QString & ) ), this,
			SLOT( slotListedChannel( const QString &, uint, const QString & ) ) );
		connect( mAccount->engine(), SIGNAL( incomingEndOfList() ), this, SLOT( slotListEnd() ) );
		mAccount->engine()->list();
		ircdata->searchButton->setEnabled(true);
	}
	else
		KMessageBox::error( this, i18n("You must be connected to the IRC server to perform a search."), i18n("Not Connected") );
}

void IRCAddContactPage::slotListedChannel( const QString &channel, uint users, const QString &topic )
{
	if( search.isEmpty() || channel.contains( search, false ) || topic.contains( search, false ) )
	{
		ChannelListItem *i = new ChannelListItem( ircdata->searchResults, channel, QString::number(users), topic );
		ircdata->searchResults->insertItem( i );
	}
}

void IRCAddContactPage::slotListEnd()
{
	disconnect( mAccount->engine(), 0, this, 0 );
	ircdata->searchResults->setEnabled(true);
}

void IRCAddContactPage::slotSelectionChanged( QListViewItem *i )
{
	ircdata->addID->setText( i->text(0) );
}

bool IRCAddContactPage::apply(KopeteAccount *account , KopeteMetaContact *m)
{
	QString name = ircdata->addID->text();
	return account->addContact(name, name, m, KopeteAccount::ChangeKABC );
}

bool IRCAddContactPage::validateData()
{
	QString name = ircdata->addID->text();
	if (name.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a channel to join, or query to open.</qt>"), i18n("You Must Specify a Channel"));
		return false;
	}
	return true;
}

#include "ircaddcontactpage.moc"

// vim: set noet ts=4 sts=4 sw=4:

