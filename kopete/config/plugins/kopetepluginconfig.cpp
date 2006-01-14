/*
    kopetepluginconfig.cpp - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetepluginconfig.h"

#include <qlayout.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpluginselector.h>
#include <ksettings/dispatcher.h>

#include "kopetepluginmanager.h"

class KopetePluginConfigPrivate
{
public:
	KPluginSelector *pluginSelector;
	bool isChanged;
};

KopetePluginConfig::~KopetePluginConfig()
{
	delete d;
}

KopetePluginConfig::KopetePluginConfig( QWidget *parent, const char *name )
: KDialogBase( Plain, i18n( "Configure Plugins" ), /*Help |*/ Cancel | Apply | Ok | User1,
	Ok, parent, name, false, true, KGuiItem( i18n( "&Reset" ), "undo" ) )
{
	d = new KopetePluginConfigPrivate;
	showButton( User1, false );
	setChanged( false );

	// FIXME: Implement this - Martijn
	enableButton( KDialogBase::Help, false );

	setInitialSize( QSize( 640, 480 ) );

	( new QVBoxLayout( plainPage(), 0, 0 ) )->setAutoAdd( true );
	d->pluginSelector = new KPluginSelector( plainPage() );
	setMainWidget( d->pluginSelector );
	connect( d->pluginSelector, SIGNAL( changed( bool ) ), this, SLOT( setChanged( bool ) ) );
	connect( d->pluginSelector, SIGNAL( configCommitted( const QCString & ) ),
		KSettings::Dispatcher::self(), SLOT( reparseConfiguration( const QCString & ) ) );

	d->pluginSelector->addPlugins( Kopete::PluginManager::self()->availablePlugins( "Plugins" ), i18n( "General Plugins" ), "Plugins" );
}

void KopetePluginConfig::setChanged( bool c )
{
	d->isChanged = c;
	enableButton( Apply, c );
}

void KopetePluginConfig::slotDefault()
{
	d->pluginSelector->defaults();
	setChanged( false );
}

void KopetePluginConfig::slotUser1()
{
	d->pluginSelector->load();
	setChanged( false );
}

void KopetePluginConfig::apply()
{
	if( d->isChanged )
	{
		d->pluginSelector->save();
		Kopete::PluginManager::self()->loadAllPlugins();
		setChanged( false );
	}
}

void KopetePluginConfig::slotApply()
{
	apply();
}

void KopetePluginConfig::slotOk()
{
	emit okClicked();
	apply();
	accept();
}

void KopetePluginConfig::slotHelp()
{
	kdWarning() << k_funcinfo << "FIXME: Implement!" << endl;
}

void KopetePluginConfig::show()
{
	d->pluginSelector->load();

	KDialogBase::show();
}

#include "kopetepluginconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

