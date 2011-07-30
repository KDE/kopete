/*
    kopetestatusrootaction.cpp - Kopete Status Root Action

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
#include "kopetestatusrootaction.h"

#include <KMenu>
#include <KCMultiDialog>

#include <kopeteonlinestatusmanager.h>
#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopeteuiglobal.h>
#include <kopetestatusmanager.h>
#include <kopetestatusitems.h>
#include <kopetestatussettings.h>

#include "kopetestatusgroupaction.h"
#include "kopetestatusaction.h"
#include "kopeteonlinestatusaction.h"
#include "kopetestatuseditaction.h"


namespace Kopete {

class StatusRootAction::Private
{
public:
	Private() : group(0), menu(0), account(0), statusDialog(0),
		statusSeparator(0), insertBefore(0)
	{}
	
	Status::StatusGroup *group;
	KActionMenu* menu;
	Filter filter;
	QMap<Status::StatusItem *, QAction* > childMap;

	OnlineStatus onlineStatus;
	Account *account;
	QPointer<Kopete::UI::StatusEditDialog> statusDialog;
	QAction *statusSeparator;
	QAction *insertBefore;
};

StatusRootAction::StatusRootAction( KActionMenu* menu )
	: QObject( menu ), d(new Private())
{
	d->menu = menu;
	d->filter = UseCategory;
	d->account = 0;

	init();
}

StatusRootAction::StatusRootAction( KActionMenu* menu, Filter filter, Account *account,
                                    const OnlineStatus &onlineStatus, QAction *before )
	: QObject( menu ), d(new Private())
{
	d->menu = menu;
	d->insertBefore = before;
	d->filter = filter;
	
	d->onlineStatus = onlineStatus;
	d->account = account;

	init();
}

void StatusRootAction::createAccountStatusActions( Account *account , KActionMenu *parent, QAction * before )
{
	Kopete::StatusRootAction::Filter statusFilter = Kopete::StatusRootAction::UseCategory;
	switch( Kopete::StatusSettings::self()->protocolStatusMenuType() )
	{
	case Kopete::StatusSettings::EnumProtocolStatusMenuType::StatusesNotFiltered:
		statusFilter = Kopete::StatusRootAction::UseCategory;
		break;
	case Kopete::StatusSettings::EnumProtocolStatusMenuType::StatusesWithSameCategory:
		statusFilter = Kopete::StatusRootAction::UseStatusAndCategory;
		break;
	case Kopete::StatusSettings::EnumProtocolStatusMenuType::UseParentStatusCategory:
		statusFilter = Kopete::StatusRootAction::UseStatus;
		break;
	};

	if ( statusFilter == Kopete::StatusRootAction::UseCategory )
	{
		new StatusRootAction( parent, statusFilter, account, Kopete::OnlineStatus(), before );
	}
	else
	{
		OnlineStatusManager *osm = OnlineStatusManager::self();
		QList< OnlineStatus > statusList = osm->registeredStatusList( account->protocol() );
		QList< OnlineStatus >::Iterator it = statusList.end();
		while ( it != statusList.begin() )
		{
			--it;

			if ( (*it).options() & OnlineStatusManager::HideFromMenu )
				continue;

			OnlineStatus status = (*it);
			KAction *action;

			// Any existing actions owned by the account are reused by recovering them
			// from the parent's child list.
			// The description of the onlinestatus is used as the qobject name
			// This is safe as long as OnlineStatus are immutable
			QByteArray actionName = status.description().toAscii();
			if ( !( action = account->findChild<KAction*>( actionName ) ) )
			{
				if( status.options() & OnlineStatusManager::HasStatusMessage )
				{
					KActionMenu *actionMenu = new KActionMenu( KIcon(status.iconFor(account)), status.caption(), 0 );
					action = actionMenu;

					// Will be automatically deleted when the actionMenu is deleted.
					new StatusRootAction( actionMenu, statusFilter, account, status );
				}
				else
				{
					action = new OnlineStatusAction( status, status.caption(), status.iconFor(account), account );
					connect(action, SIGNAL(activated(Kopete::OnlineStatus)) ,
					        account, SLOT(setOnlineStatus(Kopete::OnlineStatus)));
				}
				action->setObjectName( actionName ); // for the lookup by name above
			}

			if( status.options() & OnlineStatusManager::DisabledIfOffline )
				action->setEnabled( account->isConnected() );

			if(parent)
				parent->insertAction( before, action );

		}
	}
}

void StatusRootAction::init()
{
	Kopete::StatusManager *statusManager = Kopete::StatusManager::self();
	connect( statusManager, SIGNAL(changed()), this, SLOT(rootChanged()) );
	d->group = statusManager->getRootGroup();

	connect( d->group, SIGNAL(childRemoved(Kopete::Status::StatusItem*)),
	         this, SLOT(childRemoved(Kopete::Status::StatusItem*)) );
	connect( d->group, SIGNAL(childInserted(int,Kopete::Status::StatusItem*)),
	         this, SLOT(childInserted(int,Kopete::Status::StatusItem*)) );

	foreach( Kopete::Status::StatusItem* child, d->group->childList() )
		insertChild( d->insertBefore, child );

	d->statusSeparator = d->menu->insertSeparator( d->insertBefore );

	QAction *statusAction = new QAction( i18n( "Edit Message..." ), this );
	connect (statusAction, SIGNAL(triggered(bool)), this, SLOT(showEditStatusDialog()) );
	d->menu->insertAction( d->insertBefore, statusAction );

	QAction *action = new QAction( i18n("Edit Statuses..."), this );
	connect( action, SIGNAL(triggered(bool)), this, SLOT(editStatuses()) );
	d->menu->insertAction( d->insertBefore, action );
}

void StatusRootAction::showEditStatusDialog()
{
	if ( d->statusDialog ) {
		d->statusDialog->activateWindow();
		return;
	}
	d->statusDialog = new Kopete::UI::StatusEditDialog( Kopete::UI::Global::mainWidget() );
	connect( d->statusDialog, SIGNAL(finished(int)), SLOT(editStatusDialogFinished(int)) );

	if ( d->account ) {
		d->statusDialog->setStatusMessage( d->account->myself()->statusMessage() );
	} else {
		emit updateMessage( this );
	}

	d->statusDialog->exec();
}

void StatusRootAction::editStatusDialogFinished(int code)
{
	if (code == QDialog::Accepted ) {
		setStatusMessage( d->statusDialog->statusMessage() );
	}
	d->statusDialog->deleteLater();
}

void StatusRootAction::editStatuses()
{
	KCMultiDialog *kcm = new KCMultiDialog( Kopete::UI::Global::mainWidget() );
	kcm->setFaceType( KCMultiDialog::Plain );
	kcm->setPlainCaption( i18n( "Configure Statuses" ) );
	kcm->addModule( "kopete_statusconfig" );
	kcm->exec();
}

StatusRootAction::~StatusRootAction()
{
	delete d;
}

StatusRootAction::Filter StatusRootAction::filter() const
{
	return d->filter;
}

int StatusRootAction::category() const
{
	return d->onlineStatus.categories();
}

OnlineStatus StatusRootAction::onlineStatus() const
{
	return d->onlineStatus;
}

Account *StatusRootAction::account() const
{
	return d->account;
}

void StatusRootAction::setCurrentMessage( const Kopete::StatusMessage &statusMessage )
{
	if ( d->statusDialog )
		d->statusDialog->setStatusMessage( statusMessage );
}

void StatusRootAction::childInserted( int i, Kopete::Status::StatusItem* child )
{
	Status::StatusItem* before = d->group->child( i + 1 );
	if ( before )
		insertChild( d->childMap.value( before, 0 ), child );
	else
		insertChild( 0, child );
}

void StatusRootAction::insertChild( QAction * before, Status::StatusItem* child )
{
	if ( child->isGroup() )
	{
		Kopete::Status::StatusGroup *group = qobject_cast<Kopete::Status::StatusGroup*>(child);
		StatusGroupAction *groupAction = new StatusGroupAction( group, this, this );
		if ( groupAction->childCount() == 0 )
		{
			delete groupAction;
			return;
		}

		d->childMap.insert( group, groupAction );
		d->menu->insertAction( before, groupAction );
	}
	else
	{
		if ( d->filter == UseStatusAndCategory && child->category() != 0 &&
		     (child->category() & d->onlineStatus.categories()) == 0 )
			return;

		Kopete::Status::Status* status = qobject_cast<Kopete::Status::Status*>(child);
		StatusAction *action = new StatusAction( status, this, this );
		d->childMap.insert( status, action );
		d->menu->insertAction( before, action );
	}
}

void StatusRootAction::childRemoved( Kopete::Status::StatusItem* item )
{
	QAction *action = d->childMap.value( item );
	d->menu->removeAction( action );
	d->childMap.remove( item );
	delete action;
}

void StatusRootAction::rootChanged()
{
	QMap<Status::StatusItem*, QAction* >::const_iterator it = d->childMap.constBegin();
	while ( it != d->childMap.constEnd() )
	{
		d->menu->removeAction( it.value() );
		delete it.value();
		++it;
	}
	d->childMap.clear();

	Kopete::StatusManager *statusManager = Kopete::StatusManager::self();
	d->group = statusManager->getRootGroup();
	
	connect( d->group, SIGNAL(childRemoved(Kopete::Status::StatusItem*)),
	         this, SLOT(childRemoved(Kopete::Status::StatusItem*)) );
	connect( d->group, SIGNAL(childInserted(int,Kopete::Status::StatusItem*)),
	         this, SLOT(childInserted(int,Kopete::Status::StatusItem*)) );

	foreach( Kopete::Status::StatusItem* child, d->group->childList() )
		insertChild( d->statusSeparator, child );
}

void StatusRootAction::setStatusMessage( const Kopete::StatusMessage &statusMessage )
{
	if ( d->account )
	{	// Set status for this account only
		d->account->setOnlineStatus( d->account->myself()->onlineStatus(), statusMessage, Kopete::Account::KeepSpecialFlags );
	}
	else
	{	// Set global status
		emit changeMessage( statusMessage );
	}
}

void StatusRootAction::changeStatus( const Kopete::Status::Status* status )
{
	Kopete::StatusMessage statusMessage;
	statusMessage.setTitle( status->title() );
	statusMessage.setMessage( status->message() );

	if ( d->account )
	{	// Set status for this account only
		if ( d->filter == UseCategory )
		{
			if ( status->category() != 0x00 )
			{
				OnlineStatusManager *osm = OnlineStatusManager::self();
				Kopete::OnlineStatus oStatus = osm->onlineStatus( d->account->protocol(), status->category() );
				d->account->setOnlineStatus( oStatus, statusMessage, Kopete::Account::KeepSpecialFlags );
			}
			else
			{
				setStatusMessage( statusMessage );
			}
		}
		else
		{
			d->account->setOnlineStatus( d->onlineStatus, statusMessage, Kopete::Account::KeepSpecialFlags );
		}
	}
	else
	{	// Set global status
		if ( status->category() != 0x00 )
			emit changeStatus( status->category(), statusMessage );
		else
			setStatusMessage( statusMessage );
	}
}

}

#include "kopetestatusrootaction.moc"
