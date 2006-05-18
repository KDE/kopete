/*
    behaviorconfig.cpp  -  Kopete Look Feel Config

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "behaviorconfig.h"
#include "behaviorconfig_general.h"
#include "behaviorconfig_events.h"
#include "behaviorconfig_chat.h"

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <QVBoxLayout>

#include <kdebug.h>
#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>
#include <klineedit.h>

#include "kopetebehaviorsettings.h"
#include "kopeteaway.h"
#include "kopeteawayconfigbase.h"
#include "kopetepluginmanager.h"
#include "kopeteaway.h"

#include <qtabwidget.h>

typedef KGenericFactory<BehaviorConfig, QWidget> KopeteBehaviorConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_behaviorconfig, KopeteBehaviorConfigFactory( "kcm_kopete_behaviorconfig" ) )


BehaviorConfig::BehaviorConfig(QWidget *parent, const QStringList &args) :
		KCModule( KopeteBehaviorConfigFactory::instance(), parent, args )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	mBehaviorTabCtl = new QTabWidget(this);
	mBehaviorTabCtl->setObjectName("mBehaviorTabCtl");

	// "General" TAB ============================================================
	mPrfsGeneral = new BehaviorConfig_General(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsGeneral );
	mBehaviorTabCtl->addTab(mPrfsGeneral, i18n("&General"));

	// "Events" TAB ============================================================
	mPrfsEvents = new BehaviorConfig_Events(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsEvents );
	mBehaviorTabCtl->addTab(mPrfsEvents, i18n("&Events"));

	// "Away" TAB ===============================================================
	mAwayConfigUI = new KopeteAwayConfigBaseUI(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mAwayConfigUI );
	mBehaviorTabCtl->addTab(mAwayConfigUI, i18n("A&way Settings"));

	// "Chat" TAB ===============================================================
	mPrfsChat = new BehaviorConfig_Chat(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsChat );
	mBehaviorTabCtl->addTab(mPrfsChat, i18n("Cha&t"));

	Kopete::PluginManager *pluginManager = Kopete::PluginManager::self();
	viewPlugins = pluginManager->availablePlugins("Views");

	load();

	// "Chat" TAB ===============================================================
	connect( mPrfsChat->viewPlugin, SIGNAL(activated(int)),
		 this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->viewPlugin, SIGNAL(activated(int)),
		 this, SLOT(slotUpdatePluginLabel(int)));

	// "Away" TAB ===============================================================
	connect( mAwayConfigUI->mAutoAwayTimeout, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));;
}

void BehaviorConfig::save()
{
//	kDebug(14000) << k_funcinfo << "called." << endl;

	KCModule::save();

	// "Away" TAB ===============================================================
	Kopete::BehaviorSettings::self()->setAutoAwayTimeout( mAwayConfigUI->mAutoAwayTimeout->value() * 60 );

	// "Chat" TAB ===============================================================
	Kopete::BehaviorSettings::self()->setViewPlugin(viewPlugins[mPrfsChat->viewPlugin->currentIndex()]->pluginName() );

	Kopete::BehaviorSettings::self()->writeConfig();

	load();
}

void BehaviorConfig::load()
{
//	kDebug(14000) << k_funcinfo << "called" << endl;
	awayInstance = Kopete::Away::getInstance();

	KCModule::load();
	// "Away" TAB ===============================================================
	mAwayConfigUI->mAutoAwayTimeout->setValue( Kopete::BehaviorSettings::self()->autoAwayTimeout() / 60 );

	// "Chat" TAB ===============================================================
	mPrfsChat->viewPlugin->clear();
	int selectedIdx = 0, i = 0;
	for(  QList<KPluginInfo*>::iterator it = viewPlugins.begin(); it != viewPlugins.end(); ++it )
	{
		if( (*it)->pluginName() == Kopete::BehaviorSettings::self()->viewPlugin() )
			selectedIdx = i;
		mPrfsChat->viewPlugin->insertItem( i++, (*it)->name() );
	}

	mPrfsChat->viewPlugin->setCurrentIndex(selectedIdx);
	slotUpdatePluginLabel(selectedIdx);
}

void BehaviorConfig::slotUpdatePluginLabel(int)
{
	mPrfsChat->viewPluginLabel->setText( viewPlugins[ mPrfsChat->viewPlugin->currentIndex() ]->comment() );
}

void BehaviorConfig::slotSettingsChanged(bool)
{
	emit changed(true);
}

void BehaviorConfig::slotValueChanged(int)
{
	emit changed( true );
}

void BehaviorConfig::slotTextChanged(const QString&)
{
	emit changed( true );
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
