/*
    contactselectorwidget.cpp

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "contactselectorwidget.h"
#include <kopetemetacontact.h>
#include <kopeteplugin.h>
#include <kopetepluginmanager.h>
#include <kopeteprotocol.h>
#include <kiconloader.h>
#include <kplugininfo.h>
#include <metacontactselectorwidget.h>
#include "ui_contactselectorwidget_base.h"

ContactSelectorWidget::ContactSelectorWidget( QWidget *parent )
	: QWidget(parent)
{
	mUi = new Ui_ContactSelectorWidget_Base;

	QBoxLayout *layout = new QVBoxLayout(this);
	QWidget *widget = new QWidget(this);
	mUi->setupUi(widget);
	layout->addWidget(widget);

	QList<KPluginInfo> plugins = Kopete::PluginManager::self()->availablePlugins("Protocols");
	for( QList<KPluginInfo>::Iterator it = plugins.begin(); it != plugins.end(); ++it )
	{
		Kopete::Plugin *plugin = Kopete::PluginManager::self()->plugin( it->pluginName() );
		if( plugin )
			mUi->comboProtocol->addItem( SmallIcon(plugin->pluginIcon()), plugin->displayName(), plugin->pluginId() );
	}
	
	connect(mUi->radioAddExistingMetaContact, SIGNAL(toggled(bool)), mUi->metaContactSelector, SLOT(setEnabled(bool)));
	connect(mUi->radioAnotherContact, SIGNAL(toggled(bool)), mUi->editContact, SLOT(setEnabled(bool)));
	connect(mUi->radioAnotherContact, SIGNAL(toggled(bool)), mUi->comboProtocol, SLOT(setEnabled(bool)));
}

ContactSelectorWidget::~ContactSelectorWidget()
{
       delete mUi;
}

QList<AccountListEntry> ContactSelectorWidget::contacts()
{
	if( mUi->radioAddExistingMetaContact->isChecked() )
	{
		QList<AccountListEntry> list;
		if(!mUi->metaContactSelector->metaContact())
			return list;
		foreach( Kopete::Contact *contact, mUi->metaContactSelector->metaContact()->contacts() )
		{
			list.append( AccountListEntry( contact->contactId(), contact->protocol() ) );
		}
		return list;
	}
	else
	{
		QList<AccountListEntry> list;
		Kopete::Protocol *protocol = static_cast<Kopete::Protocol *>( Kopete::PluginManager::self()->plugin( 
				mUi->comboProtocol->itemData( mUi->comboProtocol->currentIndex() ).toString() ) );
		
		if( protocol )
			list.append( AccountListEntry( mUi->editContact->text(), protocol ) );
		
		return list;
	}
}
