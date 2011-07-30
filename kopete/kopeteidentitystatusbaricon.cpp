/*
    kopeteidentitystatusbaricon.cpp  -  Kopete Identity StatusBar Dock Icon

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2001-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidentitystatusbaricon.h"
#include <kopeteidentity.h>
#include <kopetestatusmessage.h>
#include <kopetestatusrootaction.h>
#include <kactionmenu.h>

#include <KMenu>

KopeteIdentityStatusBarIcon::KopeteIdentityStatusBarIcon( Kopete::Identity *identity, QWidget *parent )
: QLabel( parent )
{
	setFixedSize ( 16, 16 );
	setCursor(QCursor(Qt::PointingHandCursor));
	show();

	m_identity = identity;

}

void KopeteIdentityStatusBarIcon::mousePressEvent( QMouseEvent *me )
{
	if( me->button() == Qt::LeftButton )
	{
		emit leftClicked( m_identity, me->globalPos() );
	}
	else if( me->button() == Qt::RightButton )
	{
		// show the context menu for the left click
		// creates the action menu
		KActionMenu *statusMenu = new KActionMenu(m_identity->label(), this);

		// add a title to the popup menu before the online action
		statusMenu->menu()->addTitle(m_identity->label());

		// Will be automatically deleted when the statusMenu is deleted.
		Kopete::StatusRootAction* statusAction = new Kopete::StatusRootAction( statusMenu );

		connect( statusAction, SIGNAL(changeStatus(uint,Kopete::StatusMessage)),
		         m_identity, SLOT(setOnlineStatus(uint,Kopete::StatusMessage)) );
		connect( statusAction, SIGNAL(updateMessage(Kopete::StatusRootAction*)),
		         this, SLOT(updateMessage(Kopete::StatusRootAction*)) );
		connect( statusAction, SIGNAL(changeMessage(Kopete::StatusMessage)),
		         m_identity, SLOT(setStatusMessage(Kopete::StatusMessage)) );

		connect( statusMenu->menu(), SIGNAL(aboutToHide()), statusMenu, SLOT(deleteLater()) );
		statusMenu->menu()->popup( me->globalPos() );
	}
}

void KopeteIdentityStatusBarIcon::updateMessage( Kopete::StatusRootAction *statusRootAction )
{
	statusRootAction->setCurrentMessage( m_identity->statusMessage() );
}

#include "kopeteidentitystatusbaricon.moc"

// vim: set noet ts=4 sts=4 sw=4:

