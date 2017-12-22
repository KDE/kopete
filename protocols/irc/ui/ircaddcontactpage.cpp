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

#include "ircaddcontactpage.h"
#include "channellist.h"

#include "ircaccount.h"

#include <kdebug.h>
#include <KLocalizedString>
#include <kmessagebox.h>

struct IRCAddContactPage::Private
{
    ChannelList *search;
    IRCAccount *account;
};

IRCAddContactPage::IRCAddContactPage(QWidget *parent, IRCAccount *a)
    : AddContactPage(parent)
    , Ui::ircAddUI()
    , d(new Private)
{
    setupUi(this);

//  d->search = new ChannelList( hbox, a->client() );
//  QVBoxLayout *layout = new QVBoxLayout( hbox );
//  hbox->setLayout(layout);
//  layout->addWidget(d->search);

    d->account = a;

    connect(d->search, SIGNAL(channelSelected(QString)),
            this, SLOT(slotChannelSelected(QString)));

    connect(d->search, SIGNAL(channelDoubleClicked(QString)),
            this, SLOT(slotChannelDoubleClicked(QString)));
}

IRCAddContactPage::~IRCAddContactPage()
{
    delete d;
}

void IRCAddContactPage::slotChannelSelected(const QString &channel)
{
    addID->setText(channel);
}

void IRCAddContactPage::slotChannelDoubleClicked(const QString &channel)
{
    addID->setText(channel);
    tabWidget3->setCurrentIndex(0);
}

bool IRCAddContactPage::apply(Kopete::Account *account, Kopete::MetaContact *m)
{
    QString name = addID->text();
    return account->addContact(name, m, Kopete::Account::ChangeKABC);
}

bool IRCAddContactPage::validateData()
{
    QString name = addID->text();
    if (name.isEmpty() == true) {
        KMessageBox::sorry(this, i18n("<qt>You need to specify a channel to join, or a query to open.</qt>"), i18n("You Must Specify a Channel"));
        return false;
    }
    return true;
}
