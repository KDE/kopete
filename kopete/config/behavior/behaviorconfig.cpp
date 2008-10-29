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
#include "behaviorconfig_away.h"

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
#include <kconfig.h>
#include <klineedit.h>

#include "kopetebehaviorsettings.h"
#include "kopetepluginmanager.h"

#include <qtabwidget.h>

K_PLUGIN_FACTORY( KopeteBehaviorConfigFactory,
		registerPlugin<BehaviorConfig>(); )
K_EXPORT_PLUGIN( KopeteBehaviorConfigFactory("kcm_kopete_behaviorconfig") )

BehaviorConfig::BehaviorConfig(QWidget *parent, const QVariantList &args) :
		KCModule( KopeteBehaviorConfigFactory::componentData(), parent, args )
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	// since KSetting::Dialog has margins here, we don't need our own.
	layout->setContentsMargins( 0, 0, 0, 0);

	mBehaviorTabCtl = new QTabWidget(this);
	mBehaviorTabCtl->setObjectName("mBehaviorTabCtl");
	layout->addWidget( mBehaviorTabCtl );

	// "General" TAB ============================================================
	mPrfsGeneral = new BehaviorConfig_General(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsGeneral );
	mBehaviorTabCtl->addTab(mPrfsGeneral, i18n("&General"));

	// "Events" TAB ============================================================
	mPrfsEvents = new BehaviorConfig_Events(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsEvents );
	mBehaviorTabCtl->addTab(mPrfsEvents, i18n("&Events"));

	// "Away" TAB ===============================================================
	mPrfsAway = new BehaviorConfig_Away(mBehaviorTabCtl);
	addConfig( Kopete::BehaviorSettings::self(), mPrfsAway );
	mBehaviorTabCtl->addTab(mPrfsAway, i18n("A&way Settings"));

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

	// "Away" TAB ===============================================================
	connect( mPrfsAway->mAutoAwayTimeout, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));;
	connect( mPrfsAway->mAutoAwayCustomMessage, SIGNAL(textChanged()),
	         this, SLOT(slotTextChanged()) );
}

void BehaviorConfig::save()
{
//	kDebug(14000);

	KCModule::save();

	// "Away" TAB ===============================================================
	Kopete::BehaviorSettings::self()->setAutoAwayTimeout( mPrfsAway->mAutoAwayTimeout->value() * 60 );
	Kopete::BehaviorSettings::self()->setAutoAwayCustomMessage( mPrfsAway->mAutoAwayCustomMessage->toPlainText() );

	// "Chat" TAB ===============================================================
	if ( viewPlugins.size() > 0 )
	{
		Kopete::BehaviorSettings::self()->setViewPlugin(viewPlugins[mPrfsChat->viewPlugin->currentIndex()].pluginName() );
	}

	Kopete::BehaviorSettings::self()->writeConfig();

	load();
}

void BehaviorConfig::load()
{
	KCModule::load();
	// "General" TAB ===============================================================
	if(!mPrfsGeneral->kcfg_useMessageQueue->isChecked()) {
		mPrfsGeneral->mInstantMessageOpeningChk->setChecked(true);
	}

	// "Away" TAB ===============================================================
	mPrfsAway->mAutoAwayTimeout->setValue( Kopete::BehaviorSettings::self()->autoAwayTimeout() / 60 );
	mPrfsAway->mAutoAwayCustomMessage->setPlainText( Kopete::BehaviorSettings::self()->autoAwayCustomMessage() );

	// "Chat" TAB ===============================================================
	mPrfsChat->viewPlugin->clear();
	int selectedIdx = 0, i = 0;
	for(  QList<KPluginInfo>::iterator it = viewPlugins.begin(); it != viewPlugins.end(); ++it )
	{
		if( it->pluginName() == Kopete::BehaviorSettings::self()->viewPlugin() )
			selectedIdx = i;
		mPrfsChat->viewPlugin->insertItem( i++, it->name() );
	}

	mPrfsChat->viewPlugin->setCurrentIndex(selectedIdx);
}

void BehaviorConfig::slotSettingsChanged(bool)
{
	emit changed(true);
}

void BehaviorConfig::slotValueChanged(int)
{
	emit changed( true );
}

void BehaviorConfig::slotTextChanged()
{
	emit changed( true );
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
