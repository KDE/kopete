/*
    icqreadaway.cpp  -  ICQ Protocol Plugin

    Copyright (c) 2003 by Stefan Gehn <metz@gehn.net>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "icqreadaway.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"

#include <ktextbrowser.h>
#include <klocale.h>
#include <krun.h>

#include <assert.h>

ICQReadAway::ICQReadAway(ICQContact *c, QWidget *parent, const char* name)
	: KDialog(parent, QString(), KDialog::Close | KDialog::User1, 0, KGuiItem(i18n("&Fetch Again")))
{
	assert(c);

	mAccount = static_cast<ICQAccount*>(c->account());
	mContact = c;
	setCaption(i18n("'%2' Message for %1", c->displayName(), c->onlineStatus().description()));

	QWidget *mMainWidget = makeVBoxMainWidget();

	awayMessageBrowser = new KTextBrowser(mMainWidget);
	awayMessageBrowser->setObjectName("userInfoView");
	awayMessageBrowser->setTextFormat(Qt::AutoText);
	awayMessageBrowser->setNotifyClick(true);
	awayMessageBrowser->setText(mContact->awayMessage());

	QObject::connect(awayMessageBrowser, &KTextBrowser::urlClick, this, &ICQReadAway::slotUrlClicked);
	QObject::connect(awayMessageBrowser, &KTextBrowser::mailClick, this, &ICQReadAway::slotMailClicked);

	connect(this, &ICQReadAway::user1Clicked, this, &ICQReadAway::slotFetchAwayMessage);
	connect(this, &ICQReadAway::closeClicked, this, &ICQReadAway::slotCloseClicked);

	connect(c, SIGNAL(awayMessageChanged()),
		this, SLOT(slotAwayMessageChanged()));

	slotFetchAwayMessage();
}

void ICQReadAway::slotFetchAwayMessage()
{
	if(!mAccount->isConnected())
		return;

	awayMessageBrowser->setDisabled(true);
	enableButton(User1,false);

	mAccount->engine()->requestAwayMessage(mContact);

	setCaption(i18n("Fetching '%2' Message for %1...", mContact->displayName(), mContact->onlineStatus().description()));
} // END slotFetchAwayMessage()

void ICQReadAway::slotAwayMessageChanged()
{
	setCaption(i18n("'%2' Message for %1", mContact->displayName(), mContact->onlineStatus().description()));
	awayMessageBrowser->setText(mContact->awayMessage());

	awayMessageBrowser->setDisabled(false);
	enableButton(User1,true);

} // END slotAwayMessageChanged()

void ICQReadAway::slotCloseClicked()
{
	emit closing();
}

void ICQReadAway::slotUrlClicked(const QString &url)
{
	new KRun(KUrl(url));
}

void ICQReadAway::slotMailClicked(const QString&, const QString &address)
{
	new KRun(KUrl(address));
}

