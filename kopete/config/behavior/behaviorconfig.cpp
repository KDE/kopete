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
#include "behaviorconfig_notifications.h"
#include "behaviorconfig_chat.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qcombobox.h>

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
	mBehaviorTabCtl->addTab(mPrfsGeneral, i18n("&General"));

	// "Notifications" TAB ============================================================
	mPrfsNotifications = new BehaviorConfig_Notifications(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mPrfsNotifications, i18n("&Notifications"));

	// "Away" TAB ===============================================================
	mAwayConfigUI = new KopeteAwayConfigBaseUI(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mAwayConfigUI, i18n("A&way Settings"));

	// "Chat" TAB ===============================================================
	mPrfsChat = new BehaviorConfig_Chat(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mPrfsChat, i18n("Cha&t"));

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
	connect(mPrfsGeneral->mAutoConnect, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mMouseNavigation, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));


	// "Notifications" TAB ============================================================
	connect(mPrfsNotifications->mQueueOnlyHighlightedMessagesInGroupChatsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mQueueOnlyMessagesOnAnotherDesktopChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mBalloonNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mBalloonNotifyIgnoreClosesChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mTrayflashNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mTrayflashNotifyLeftClickOpensMessageChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mTrayflashNotifySetCurrentDesktopToChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mSoundIfAwayChk, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mEventIfActive, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsNotifications->mRaiseMsgWindowChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));


	// "Chat" TAB ===============================================================
	connect( mPrfsChat->cb_ShowEventsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->highlightEnabled, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->cb_SpellCheckChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mPrfsChat->cmbChatGroupingPolicy, SIGNAL(activated(int)),
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
	p->setAutoConnect(mPrfsGeneral->mAutoConnect->isChecked());
	p->setContactListMouseNavigation(mPrfsGeneral->mMouseNavigation->isChecked());

	// "Notifications" TAB ============================================================
	p->setQueueOnlyHighlightedMessagesInGroupChats(mPrfsNotifications->mQueueOnlyHighlightedMessagesInGroupChatsChk->isChecked());
	p->setQueueOnlyMessagesOnAnotherDesktop(mPrfsNotifications->mQueueOnlyMessagesOnAnotherDesktopChk->isChecked());
	p->setBalloonNotify(mPrfsNotifications->mBalloonNotifyChk->isChecked());
	p->setBalloonNotifyIgnoreClosesChatView(mPrfsNotifications->mBalloonNotifyIgnoreClosesChatViewChk->isChecked());
	p->setTrayflashNotify(mPrfsNotifications->mTrayflashNotifyChk->isChecked());
	p->setTrayflashNotifyLeftClickOpensMessage(mPrfsNotifications->mTrayflashNotifyLeftClickOpensMessageChk->isChecked());
	p->setTrayflashNotifySetCurrentDesktopToChatView(mPrfsNotifications->mTrayflashNotifySetCurrentDesktopToChatViewChk->isChecked());
	p->setSoundIfAway(mPrfsNotifications->mSoundIfAwayChk->isChecked());
	p->setRaiseMsgWindow(mPrfsNotifications->mRaiseMsgWindowChk->isChecked());
	config->setGroup("General");
	config->writeEntry("EventIfActive", mPrfsNotifications->mEventIfActive->isChecked());

	// "Away" TAB ===============================================================
	p->setRememberedMessages( mAwayConfigUI->rememberedMessages->value() );

	config->setGroup("AutoAway");
	config->writeEntry("Timeout", mAwayConfigUI->mAutoAwayTimeout->value() * 60);
	config->writeEntry("GoAvailable", mAwayConfigUI->mGoAvailable->isChecked());
	config->writeEntry("UseAutoAway", mAwayConfigUI->mUseAutoAway->isChecked() );
	config->sync();

	// "Chat" TAB ===============================================================
	p->setShowEvents(mPrfsChat->cb_ShowEventsChk->isChecked());
	p->setHighlightEnabled(mPrfsChat->highlightEnabled->isChecked());
	p->setSpellCheck(mPrfsChat->cb_SpellCheckChk->isChecked());
	p->setInterfacePreference( viewPlugins[mPrfsChat->viewPlugin->currentItem()]->pluginName() );
	p->setChatWindowPolicy(mPrfsChat->cmbChatGroupingPolicy->currentItem());
	
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
	mPrfsGeneral->mAutoConnect->setChecked( p->autoConnect() );
	mPrfsGeneral->mMouseNavigation->setChecked( p->contactListMouseNavigation() );

	// "Notifications" TAB ============================================================
	mPrfsNotifications->mQueueOnlyHighlightedMessagesInGroupChatsChk->setChecked ( p->queueOnlyHighlightedMessagesInGroupChats() );
	mPrfsNotifications->mQueueOnlyMessagesOnAnotherDesktopChk->setChecked ( p->queueOnlyMessagesOnAnotherDesktop() );
	mPrfsNotifications->mBalloonNotifyChk->setChecked ( p->balloonNotify() );
	mPrfsNotifications->mBalloonNotifyIgnoreClosesChatViewChk->setChecked ( p->balloonNotifyIgnoreClosesChatView() );
	mPrfsNotifications->mTrayflashNotifyChk->setChecked ( p->trayflashNotify() );
	mPrfsNotifications->mTrayflashNotifyLeftClickOpensMessageChk->setChecked ( p->trayflashNotifyLeftClickOpensMessage() );
	mPrfsNotifications->mTrayflashNotifySetCurrentDesktopToChatViewChk->setChecked ( p->trayflashNotifySetCurrentDesktopToChatView() );
	mPrfsNotifications->mSoundIfAwayChk->setChecked( p->soundIfAway() );
	mPrfsNotifications->mRaiseMsgWindowChk->setChecked(p->raiseMsgWindow());
	config->setGroup("General");
	mPrfsNotifications->mEventIfActive->setChecked(config->readBoolEntry("EventIfActive", true));

	// "Away" TAB ===============================================================
	config->setGroup("AutoAway");
	mAwayConfigUI->mAutoAwayTimeout->setValue(config->readNumEntry("Timeout", 600)/60);
	mAwayConfigUI->mGoAvailable->setChecked(config->readBoolEntry("GoAvailable", true));
	mAwayConfigUI->mUseAutoAway->setChecked(config->readBoolEntry("UseAutoAway", true));
	mAwayConfigUI->rememberedMessages->setValue( p->rememberedMessages() );

	// "Chat" TAB ===============================================================
	mPrfsChat->cb_ShowEventsChk->setChecked(p->showEvents());
	mPrfsChat->highlightEnabled->setChecked(p->highlightEnabled());
	mPrfsChat->cb_SpellCheckChk->setChecked(p->spellCheck());
	mPrfsChat->cmbChatGroupingPolicy->setCurrentItem(p->chatWindowPolicy());

	mPrfsChat->mChatViewBufferSize->setValue(p->chatViewBufferSize());
	mPrfsChat->truncateContactNameEnabled->setChecked(p->truncateContactNames());
	mPrfsChat->mMaxContactNameLength->setValue(p->maxConactNameLength());


	mPrfsChat->viewPlugin->clear();
	int selectedIdx = 0, i = 0;
	for(  QValueList<KPluginInfo*>::iterator it = viewPlugins.begin(); it != viewPlugins.end(); ++it )
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
