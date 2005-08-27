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
#include "behaviorconfig_chat.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcombobox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3ValueList>

#include <kdebug.h>
#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>

#include "kopeteprefs.h"
#include "kopeteaway.h"
#include "kopeteawayconfigbase.h"
#include "kopetepluginmanager.h"

#include <qtabwidget.h>

typedef KGenericFactory<BehaviorConfig, QWidget> KopeteBehaviorConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_behaviorconfig, KopeteBehaviorConfigFactory( "kcm_kopete_behaviorconfig" ) )


BehaviorConfig::BehaviorConfig(QWidget *parent, const char * /* name */, const QStringList &args) :
		KCModule( KopeteBehaviorConfigFactory::instance(), parent, args )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	mBehaviorTabCtl = new QTabWidget(this, "mBehaviorTabCtl");

	// "General" TAB ============================================================
	mPrfsGeneral = new BehaviorConfig_General(mBehaviorTabCtl);
	connect(mPrfsGeneral->mShowTrayChk, SIGNAL(toggled(bool)),
		this, SLOT(slotShowTrayChanged(bool)));
	connect(mPrfsGeneral->mQueueUnreadMessagesChk, SIGNAL(toggled(bool)),
		this, SLOT(slotQueueUnreadMessagesChanged(bool)));
	connect(mPrfsGeneral->mBalloonNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotBalloonNotifyChanged(bool)));
	connect(mPrfsGeneral->mTrayflashNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotTrayflashNotifyChanged(bool)));
	mBehaviorTabCtl->addTab(mPrfsGeneral, i18n("&General"));

	// "Away" TAB ===============================================================
	mAwayConfigUI = new KopeteAwayConfigBaseUI(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mAwayConfigUI, i18n("&Away Settings"));

	// "Chat" TAB ===============================================================
	mPrfsChat = new BehaviorConfig_Chat(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mPrfsChat, i18n("&Chat"));

	Kopete::PluginManager *pluginManager = Kopete::PluginManager::self();
	viewPlugins = pluginManager->availablePlugins("Views");

	load();


	// "General" TAB ============================================================
	connect(mPrfsGeneral->mShowTrayChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mStartDockedChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mUseQueueChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mQueueUnreadMessagesChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mQueueOnlyHighlightedMessagesInGroupChatsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mQueueOnlyMessagesOnAnotherDesktopChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mBalloonNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mBalloonNotifyIgnoreClosesChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mTrayflashNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mTrayflashNotifyLeftClickOpensMessageChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mTrayflashNotifySetCurrentDesktopToChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mSoundIfAwayChk, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mEventIfActive, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mAutoConnect, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mMouseNavigation, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));


	// "Chat" TAB ===============================================================
	connect( mPrfsChat->cb_RaiseMsgWindowChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->cb_ShowEventsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->highlightEnabled, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->chatWindowGroup, SIGNAL(clicked(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->interfaceGroup, SIGNAL(clicked(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->mChatViewBufferSize, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->truncateContactNameEnabled, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->mMaxContactNameLength, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->viewPlugin, SIGNAL(activated(int)),
		 this, SLOT(slotValueChanged(int)));
	connect( mPrfsChat->viewPlugin, SIGNAL(activated(int)),
		 this, SLOT(slotUpdatePluginLabel(int)));

	// "Away" TAB ===============================================================
	connect( mAwayConfigUI->mAutoAwayTimeout, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mAwayConfigUI->mGoAvailable, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mAwayConfigUI->mUseAutoAway, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
}

void BehaviorConfig::save()
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;

	KopetePrefs *p = KopetePrefs::prefs();
	KConfig *config = KGlobal::config();

	// "General" TAB ============================================================
	p->setShowTray(mPrfsGeneral->mShowTrayChk->isChecked());
	p->setStartDocked(mPrfsGeneral->mStartDockedChk->isChecked());
	p->setUseQueue(mPrfsGeneral->mUseQueueChk->isChecked());
	p->setQueueUnreadMessages(mPrfsGeneral->mQueueUnreadMessagesChk->isChecked());
	p->setQueueOnlyHighlightedMessagesInGroupChats(mPrfsGeneral->mQueueOnlyHighlightedMessagesInGroupChatsChk->isChecked());
	p->setQueueOnlyMessagesOnAnotherDesktop(mPrfsGeneral->mQueueOnlyMessagesOnAnotherDesktopChk->isChecked());
	p->setBalloonNotify(mPrfsGeneral->mBalloonNotifyChk->isChecked());
	p->setBalloonNotifyIgnoreClosesChatView(mPrfsGeneral->mBalloonNotifyIgnoreClosesChatViewChk->isChecked());
	p->setTrayflashNotify(mPrfsGeneral->mTrayflashNotifyChk->isChecked());
	p->setTrayflashNotifyLeftClickOpensMessage(mPrfsGeneral->mTrayflashNotifyLeftClickOpensMessageChk->isChecked());
	p->setTrayflashNotifySetCurrentDesktopToChatView(mPrfsGeneral->mTrayflashNotifySetCurrentDesktopToChatViewChk->isChecked());
	p->setSoundIfAway(mPrfsGeneral->mSoundIfAwayChk->isChecked());
	p->setAutoConnect(mPrfsGeneral->mAutoConnect->isChecked());
	p->setContactListMouseNavigation(mPrfsGeneral->mMouseNavigation->isChecked());
	slotShowTrayChanged( mPrfsGeneral->mShowTrayChk->isChecked() );
	config->setGroup("General");
	config->writeEntry("EventIfActive", mPrfsGeneral->mEventIfActive->isChecked());

	// "Away" TAB ===============================================================
	p->setRememberedMessages( mAwayConfigUI->rememberedMessages->value() );

	config->setGroup("AutoAway");
	config->writeEntry("Timeout", mAwayConfigUI->mAutoAwayTimeout->value() * 60);
	config->writeEntry("GoAvailable", mAwayConfigUI->mGoAvailable->isChecked());
	config->writeEntry("UseAutoAway", mAwayConfigUI->mUseAutoAway->isChecked() );
	config->sync();

	// "Chat" TAB ===============================================================
	p->setRaiseMsgWindow(mPrfsChat->cb_RaiseMsgWindowChk->isChecked());
	p->setShowEvents(mPrfsChat->cb_ShowEventsChk->isChecked());
	p->setHighlightEnabled(mPrfsChat->highlightEnabled->isChecked());
	p->setChatWindowPolicy(mPrfsChat->chatWindowGroup->id(
		mPrfsChat->chatWindowGroup->selected())
	);

	p->setInterfacePreference( viewPlugins[mPrfsChat->viewPlugin->currentItem()]->pluginName() );

	p->setChatViewBufferSize(mPrfsChat->mChatViewBufferSize->value());
	p->setTruncateContactNames(mPrfsChat->truncateContactNameEnabled->isChecked());
	p->setMaxContactNameLength(mPrfsChat->mMaxContactNameLength->value());

	p->save();
	emit changed(false);
}

void BehaviorConfig::load()
{
//	kdDebug(14000) << k_funcinfo << "called" << endl;
	KopetePrefs *p = KopetePrefs::prefs();
	KConfig *config = KGlobal::config();

	// "General" TAB ============================================================
	mPrfsGeneral->mShowTrayChk->setChecked( p->showTray() );
	mPrfsGeneral->mStartDockedChk->setChecked( p->startDocked() );
	mPrfsGeneral->mUseQueueChk->setChecked( p->useQueue() );
	mPrfsGeneral->mQueueUnreadMessagesChk->setChecked ( p->queueUnreadMessages() );
	mPrfsGeneral->mQueueOnlyHighlightedMessagesInGroupChatsChk->setChecked ( p->queueOnlyHighlightedMessagesInGroupChats() );
	mPrfsGeneral->mQueueOnlyMessagesOnAnotherDesktopChk->setChecked ( p->queueOnlyMessagesOnAnotherDesktop() );
	mPrfsGeneral->mBalloonNotifyChk->setChecked ( p->balloonNotify() );
	mPrfsGeneral->mBalloonNotifyIgnoreClosesChatViewChk->setChecked ( p->balloonNotifyIgnoreClosesChatView() );
	mPrfsGeneral->mTrayflashNotifyChk->setChecked ( p->trayflashNotify() );
	mPrfsGeneral->mTrayflashNotifyLeftClickOpensMessageChk->setChecked ( p->trayflashNotifyLeftClickOpensMessage() );
	mPrfsGeneral->mTrayflashNotifySetCurrentDesktopToChatViewChk->setChecked ( p->trayflashNotifySetCurrentDesktopToChatView() );
	mPrfsGeneral->mSoundIfAwayChk->setChecked( p->soundIfAway() );
	mPrfsGeneral->mAutoConnect->setChecked( p->autoConnect() );
	mPrfsGeneral->mMouseNavigation->setChecked( p->contactListMouseNavigation() );
	config->setGroup("General");
	mPrfsGeneral->mEventIfActive->setChecked(config->readBoolEntry("EventIfActive", true));

	// "Away" TAB ===============================================================
	config->setGroup("AutoAway");
	mAwayConfigUI->mAutoAwayTimeout->setValue(config->readNumEntry("Timeout", 600)/60);
	mAwayConfigUI->mGoAvailable->setChecked(config->readBoolEntry("GoAvailable", true));
	mAwayConfigUI->mUseAutoAway->setChecked(config->readBoolEntry("UseAutoAway", true));
	mAwayConfigUI->rememberedMessages->setValue( p->rememberedMessages() );

	// "Chat" TAB ===============================================================
	mPrfsChat->cb_RaiseMsgWindowChk->setChecked(p->raiseMsgWindow());
	mPrfsChat->cb_ShowEventsChk->setChecked(p->showEvents());
	mPrfsChat->highlightEnabled->setChecked(p->highlightEnabled());
	mPrfsChat->chatWindowGroup->setButton(p->chatWindowPolicy());
	mPrfsChat->mChatViewBufferSize->setValue(p->chatViewBufferSize());
	mPrfsChat->truncateContactNameEnabled->setChecked(p->truncateContactNames());
	mPrfsChat->mMaxContactNameLength->setValue(p->maxConactNameLength());


	mPrfsChat->viewPlugin->clear();
	int selectedIdx = 0, i = 0;
	for(  Q3ValueList<KPluginInfo*>::iterator it = viewPlugins.begin(); it != viewPlugins.end(); ++it )
	{
		if( (*it)->pluginName() == p->interfacePreference() )
			selectedIdx = i;
		mPrfsChat->viewPlugin->insertItem( (*it)->name(), i++ );
	}

	mPrfsChat->viewPlugin->setCurrentItem(selectedIdx);
	slotUpdatePluginLabel(selectedIdx);
}

void BehaviorConfig::slotUpdatePluginLabel(int)
{
	mPrfsChat->viewPluginLabel->setText( viewPlugins[ mPrfsChat->viewPlugin->currentItem() ]->comment() );
}

void BehaviorConfig::slotShowTrayChanged(bool check)
{
	mPrfsGeneral->mStartDockedChk->setEnabled(check);
	mPrfsGeneral->mQueueUnreadMessagesChk->setEnabled(check);
	slotQueueUnreadMessagesChanged( mPrfsGeneral->mQueueUnreadMessagesChk->isOn() );
	mPrfsGeneral->mBalloonNotifyChk->setEnabled(check);
	slotBalloonNotifyChanged( mPrfsGeneral->mBalloonNotifyChk->isOn() );
	mPrfsGeneral->mTrayflashNotifyChk->setEnabled(check);
	slotTrayflashNotifyChanged( mPrfsGeneral->mTrayflashNotifyChk->isOn() );
	emit changed(true);
}

void BehaviorConfig::slotQueueUnreadMessagesChanged(bool check)
{
	check &= mPrfsGeneral->mShowTrayChk->isOn();
	mPrfsGeneral->mQueueOnlyHighlightedMessagesInGroupChatsChk->setEnabled(check);
	mPrfsGeneral->mQueueOnlyMessagesOnAnotherDesktopChk->setEnabled(check);
}

void BehaviorConfig::slotBalloonNotifyChanged(bool check)
{
	check &= mPrfsGeneral->mShowTrayChk->isOn();
	mPrfsGeneral->mBalloonNotifyIgnoreClosesChatViewChk->setEnabled(check);
}

void BehaviorConfig::slotTrayflashNotifyChanged(bool check)
{
	check &= mPrfsGeneral->mShowTrayChk->isOn();
	mPrfsGeneral->mTrayflashNotifyLeftClickOpensMessageChk->setEnabled(check);
}

void BehaviorConfig::slotSettingsChanged(bool)
{
	emit changed(true);
}

void BehaviorConfig::slotValueChanged(int)
{
	emit changed( true );
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
