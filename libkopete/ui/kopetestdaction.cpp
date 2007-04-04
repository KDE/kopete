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
#include <kstandardaction.h>
#include <KStandardGuiItem>
#include <kwm.h>
#include <kcmultidialog.h>
#include <kicon.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include <kactioncollection.h>

KSettings::Dialog *KopetePreferencesAction::s_settingsDialog = 0L;

KopetePreferencesAction::KopetePreferencesAction( KActionCollection *parent, const char *name )
: KAction( KIcon(KStandardGuiItem::configure().iconName()), KStandardGuiItem::configure().text(), parent )
{
	connect( this, SIGNAL( triggered(bool) ), this, SLOT( slotShowPreferences() ) );
        parent->addAction( name, this );
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
	KWM::activateWindow( s_settingsDialog->dialog()->winId() );
}

KAction * KopeteStdAction::preferences( KActionCollection *parent, const char *name )
{
	return new KopetePreferencesAction( parent, name );
}

KAction * KopeteStdAction::createAction(const QString &text, const KIcon &icon, const QObject *receiver, const char *slot, KActionCollection* parent, const char *name)
{
	KAction *newAction = new KAction(icon, text, parent);
	if(receiver && slot)
	{
		QObject::connect(newAction, SIGNAL(triggered(bool)), receiver, slot);
	}

	if( parent )
		parent->addAction(name, newAction);

	return newAction;
}

KAction * KopeteStdAction::chat( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "Start &Chat..." ), KIcon("mail"), recvr, slot, parent, name );
}

KAction * KopeteStdAction::sendMessage( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "&Send Single Message..." ), KIcon( "mail" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::contactInfo( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "User &Info" ), KIcon( "dialog-information" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::sendFile( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "Send &File..." ), KIcon( "attach" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::viewHistory( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "View &History..." ), KIcon( "history" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::addGroup( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "&Create Group..." ), KIcon( "folder" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::changeMetaContact( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "Cha&nge Meta Contact..." ), KIcon( "move" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::deleteContact( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	KAction *deleteAction = createAction( i18n( "&Delete Contact" ), KIcon( "delete_user" ), recvr, slot, parent, name );
	deleteAction->setShortcut( KShortcut(Qt::Key_Delete) );

	return deleteAction;
}

KAction * KopeteStdAction::changeAlias( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "Change A&lias..." ), KIcon( "signature" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::blockContact( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "&Block Contact" ), KIcon( "media-playback-pause" ), recvr, slot, parent, name );
}

KAction * KopeteStdAction::unblockContact( const QObject *recvr, const char *slot, KActionCollection* parent, const char *name )
{
	return createAction( i18n( "Un&block Contact" ), KIcon( "media-playback-start" ), recvr, slot, parent, name );
}

#include "kopetestdaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

