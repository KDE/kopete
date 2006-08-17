/*
    kopetestdaction.cpp  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming          <ryan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetestdaction.h"

#include <qapplication.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kguiitem.h>
#include <klocale.h>
#include <ksettings/dialog.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kwin.h>
#include <kcmultidialog.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"

KSettings::Dialog *KopetePreferencesAction::s_settingsDialog = 0L;

KopetePreferencesAction::KopetePreferencesAction( KActionCollection *parent, const char *name )
#if KDE_IS_VERSION( 3, 3, 90 )
: KAction( KStdGuiItem::configure(), 0, 0, 0, parent, name )
#else
: KAction( KGuiItem( i18n( "&Configure Kopete..." ),
	QString::fromLatin1( "configure" ) ), 0, 0, 0, parent, name )
#endif
{
	connect( this, SIGNAL( activated() ), this, SLOT( slotShowPreferences() ) );
}

KopetePreferencesAction::~KopetePreferencesAction()
{
}

void KopetePreferencesAction::slotShowPreferences()
{
	// FIXME: Use static deleter - Martijn
	if ( !s_settingsDialog )
		s_settingsDialog = new KSettings::Dialog( KSettings::Dialog::Static, Kopete::UI::Global::mainWidget() );
	s_settingsDialog->show();

	s_settingsDialog->dialog()->raise();

	KWin::activateWindow( s_settingsDialog->dialog()->winId() );
}

KAction * KopeteStdAction::preferences( KActionCollection *parent, const char *name )
{
	return new KopetePreferencesAction( parent, name );
}

KAction * KopeteStdAction::chat( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "Start &Chat..." ), QString::fromLatin1( "mail_generic" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::sendMessage( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "&Send Single Message..." ), QString::fromLatin1( "mail_generic" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::contactInfo( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "User &Info" ), QString::fromLatin1( "messagebox_info" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::sendFile( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "Send &File..." ), QString::fromLatin1( "attach" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::viewHistory( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "View &History..." ), QString::fromLatin1( "history" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::addGroup( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "&Create Group..." ), QString::fromLatin1( "folder" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::changeMetaContact( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "Cha&nge Meta Contact..." ), QString::fromLatin1( "move" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::deleteContact( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "&Delete Contact" ), QString::fromLatin1( "delete_user" ), Qt::Key_Delete, recvr, slot, parent, name );
}

KAction * KopeteStdAction::changeAlias( const QObject *recvr, const char *slot, QObject *parent, const char *name )
{
	return new KAction( i18n( "Change A&lias..." ), QString::fromLatin1( "signature" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::blockContact( const QObject *recvr, const char *slot, QObject* parent, const char *name )
{
	return new KAction( i18n( "&Block Contact" ), QString::fromLatin1( "player_pause" ), 0, recvr, slot, parent, name );
}

KAction * KopeteStdAction::unblockContact( const QObject *recvr, const char *slot, QObject* parent, const char *name )
{
	return new KAction( i18n( "Un&block Contact" ), QString::fromLatin1( "player_play" ), 0, recvr, slot, parent, name );
}

#include "kopetestdaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

