/*
    kopeteaddedinfoevent.cpp - Kopete Added Info Event

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteaddedinfoevent.h"

#include <klocale.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "ui/contactaddednotifydialog.h"

namespace Kopete {

class AddedInfoEvent::Private
{
public:
	QString contactId;
	Kopete::Account *account;
	ShowActionOptions actions;

	QString contactNickname;

	bool suppressClose;
	UI::ContactAddedNotifyDialog* addDialog;
};

AddedInfoEvent::AddedInfoEvent( const QString& contactId, Kopete::Account *account )
	: InfoEvent(account), d( new Private() )
{
	d->suppressClose = false;
	d->addDialog = 0;
	d->contactId = contactId;
	d->account = account;
	d->actions = AllActions;
}

AddedInfoEvent::~AddedInfoEvent()
{
	if( d->addDialog )
		d->addDialog->deleteLater();

	delete d;
}

QString AddedInfoEvent::contactId() const
{
	return d->contactId;
}

Kopete::Account* AddedInfoEvent::account() const
{
	return d->account;
}

void AddedInfoEvent::showActions( ShowActionOptions actions )
{
	d->actions = actions;
}

void AddedInfoEvent::setContactNickname( const QString& nickname )
{
	d->contactNickname = nickname;
}

void AddedInfoEvent::activate( uint actionId )
{
	if ( actionId == AddAction )
	{
		if ( d->addDialog )
		{
			d->addDialog->raise();
		}
		else
		{
			UI::ContactAddedNotifyDialog::HideWidgetOptions hideFlags = UI::ContactAddedNotifyDialog::DefaultHide;
			if ( !(d->actions & AuthorizeAction) )
				hideFlags |= UI::ContactAddedNotifyDialog::AuthorizeCheckBox;
			if ( !(d->actions & InfoAction) )
				hideFlags |= UI::ContactAddedNotifyDialog::InfoButton;

			d->addDialog = new UI::ContactAddedNotifyDialog( d->contactId, d->contactNickname, d->account, hideFlags );
			d->addDialog->setAttribute( Qt::WA_DeleteOnClose, false );

			connect( d->addDialog, SIGNAL(finished()), this, SLOT(addDialogFinished()) );
			connect( d->addDialog, SIGNAL(applyClicked(QString)), this, SLOT(addDialogOk()) );
			connect( d->addDialog, SIGNAL(infoClicked(QString)), this, SLOT(addDialogInfo()) );
			d->addDialog->show();
		}
	}
	else
	{
		InfoEvent::activate( actionId );

		if ( !d->suppressClose && actionId != InfoAction && d->account->isConnected() )
			close();
	}
}

MetaContact* AddedInfoEvent::addContact() const
{
	if( !d->addDialog )
		return 0L;

	return d->addDialog->addContact();
}

void AddedInfoEvent::sendEvent()
{
	setTitle( i18n( "You have been added" ) );

	if ( d->actions & AddAction )
		addAction( AddAction, i18n("Add...") );
	if ( d->actions & AuthorizeAction )
		addAction( AuthorizeAction, i18n("Authorize") );
	if ( d->actions & BlockAction )
		addAction( BlockAction, i18n("Block") );
	if ( d->actions & InfoAction )
		addAction( InfoAction, i18n("Info...") );

	setText( i18n( "The contact <b>%1</b> has added you to his/her contact list.",
	               ( d->contactNickname.isEmpty() ) ? d->contactId : d->contactNickname ) );

	InfoEvent::sendEvent();
}

void AddedInfoEvent::addDialogOk()
{
	if( !d->addDialog )
		return;

	d->suppressClose = true;
	if ( d->addDialog->authorized() )
		activate( AuthorizeAction );


	if ( d->addDialog->added() )
		activate( AddContactAction );

	if ( d->account->isConnected() )
		close();
}

void AddedInfoEvent::addDialogInfo()
{
	activate( InfoAction );
}

void AddedInfoEvent::addDialogFinished()
{
	if( d->addDialog )
	{
		d->addDialog->deleteLater();
		d->addDialog = 0;
	}
}

}

#include "kopeteaddedinfoevent.moc"
