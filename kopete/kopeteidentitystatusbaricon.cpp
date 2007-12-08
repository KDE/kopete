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

#include <KDebug>
#include <QMouseEvent>
#include <QLabel>
#include <QEvent>
#include <QCursor>
#include <KAction>
#include <KActionMenu>
#include <KIcon>
#include <KLocale>
#include <KMenu>

KopeteIdentityStatusBarIcon::KopeteIdentityStatusBarIcon( Kopete::Identity *identity, QWidget *parent )
: QLabel( parent )
{
	setFixedSize ( 16, 16 );
	setCursor(QCursor(Qt::PointingHandCursor));
	show();

	m_identity = identity;

	// initialize actions
	m_actionSetOnline = new KAction( KIcon("kopeteavailable"), i18n("&Online"), this );
	m_actionSetOnline->setData((uint)Kopete::OnlineStatusManager::Online);

	m_actionSetAway = new KAction( KIcon("kopeteaway"), i18n("&Away"), this );
	m_actionSetAway->setData((uint)Kopete::OnlineStatusManager::Away);

	m_actionSetBusy = new KAction( KIcon("kopeteaway"), i18n("&Busy"), this );
	m_actionSetBusy->setData((uint)Kopete::OnlineStatusManager::Busy);

	m_actionSetInvisible = new KAction( KIcon("kopeteavailable"), i18n( "&Invisible" ), this );
	m_actionSetInvisible->setData((uint)Kopete::OnlineStatusManager::Invisible);

	m_actionSetOffline = new KAction( KIcon("connect-no"), i18n( "Offline" ), this );
	m_actionSetOffline->setData((uint)Kopete::OnlineStatusManager::Offline);

	// create the actionGroup
	m_statusGroup = new QActionGroup(this);
	m_statusGroup->addAction(m_actionSetOnline);
	m_statusGroup->addAction(m_actionSetAway);
	m_statusGroup->addAction(m_actionSetBusy);
	m_statusGroup->addAction(m_actionSetInvisible);
	m_statusGroup->addAction(m_actionSetOffline);

	connect(m_statusGroup, SIGNAL(triggered(QAction*)), 
			this, SLOT(slotChangeStatus(QAction *)));

}

KopeteIdentityStatusBarIcon::~KopeteIdentityStatusBarIcon()
{
	delete m_actionSetOnline;
	delete m_actionSetAway;
	delete m_actionSetBusy;
	delete m_actionSetInvisible;
	delete m_actionSetOffline;
	delete m_statusGroup;
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

		// add the status actions to the menu
		foreach(QAction *action, m_statusGroup->actions())
			statusMenu->addAction(action);

		connect( statusMenu->menu(), SIGNAL( aboutToHide() ), statusMenu, SLOT( deleteLater() ) );
		statusMenu->menu()->popup( me->globalPos() );
	}
}

void KopeteIdentityStatusBarIcon::slotChangeStatus(QAction *a)
{
	m_identity->setOnlineStatus(a->data().toUInt(), m_identity->statusMessage());
}
#include "kopeteidentitystatusbaricon.moc"

// vim: set noet ts=4 sts=4 sw=4:

