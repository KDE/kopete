/*
    kopeteaccountstatusbaricon.cpp  -  Kopete Account StatusBar Dock Icon

    Copyright (c) 2008      by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteaccountstatusbaricon.h"

#include <QtGui/QToolTip>
#include <QtGui/QMouseEvent>
#include <QtGui/QMovie>

#include <KMenu>
#include <KActionMenu>
#include <KGlobal>
#include <KDebug>

#include <kopeteaccount.h>
#include <kopetecontact.h>
#include <kopetestatusrootaction.h>

KopeteAccountStatusBarIcon::KopeteAccountStatusBarIcon( Kopete::Account *account, QWidget *parent )
: QLabel( parent ), mAccount(account), mMovie(0)
{
	setFixedSize ( 16, 16 );
	setCursor(QCursor(Qt::PointingHandCursor));

	connect( account, SIGNAL(colorChanged(QColor)), this, SLOT(statusIconChanged()) );
	connect( account->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
	         this, SLOT(statusIconChanged()) );

	statusIconChanged();
}

KopeteAccountStatusBarIcon::~KopeteAccountStatusBarIcon()
{
	if ( mMovie )
	{
		mMovie->stop();
		delete mMovie;
	}
}

void KopeteAccountStatusBarIcon::mousePressEvent( QMouseEvent *event )
{
	KActionMenu *actionMenu = new KActionMenu( mAccount->accountId(), mAccount );
	if ( !mAccount->hasCustomStatusMenu() )
		Kopete::StatusRootAction::createAccountStatusActions( mAccount, actionMenu );

	mAccount->fillActionMenu( actionMenu );

	actionMenu->menu()->exec( QPoint( event->globalX(), event->globalY() ) );
	delete actionMenu;
}

void KopeteAccountStatusBarIcon::statusIconChanged()
{
	Kopete::Contact* myself = mAccount->myself();

	if ( mMovie )
	{
		mMovie->stop();
		delete mMovie;
		mMovie = 0;
	}

	if ( myself->onlineStatus().status() == Kopete::OnlineStatus::Connecting && !myself->onlineStatus().overlayIcons().isEmpty() )
		mMovie = KIconLoader::global()->loadMovie( myself->onlineStatus().overlayIcons().first(), KIconLoader::Small );

	if ( !mMovie )
	{
		setPixmap( myself->onlineStatus().iconFor( myself->account() ).pixmap( 16, 16 ) );
	}
	else
	{
		mMovie->setCacheMode( QMovie::CacheAll );
		setMovie( mMovie );
		mMovie->start();
	}
}

bool KopeteAccountStatusBarIcon::event( QEvent *event )
{
	if ( event->type() == QEvent::ToolTip )
	{
		QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
		QToolTip::showText( helpEvent->globalPos(), mAccount->myself()->toolTip() );
	}

	return QLabel::event( event );
}

#include "kopeteaccountstatusbaricon.moc"
