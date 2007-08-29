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
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>

#include <kdebug.h>
#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>
#include <klineedit.h>

#include "kopeteprefs.h"
#include "kopeteaway.h"
#include "kopeteawayconfigbase.h"
#include "kopetepluginmanager.h"
#include "kopeteaway.h"

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

	// "Events" TAB ============================================================
	mPrfsEvents = new BehaviorConfig_Events(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mPrfsEvents, i18n("&Events"));

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
	connect(mPrfsGeneral->mUseStackChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mQueueUnreadMessagesChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsGeneral->mAutoConnect, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));

	// "Events" TAB ============================================================
	connect(mPrfsEvents->mQueueOnlyHighlightedMessagesInGroupChatsChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mQueueOnlyMessagesOnAnotherDesktopChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mBalloonNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mBalloonNotifyIgnoreClosesChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mCloseBalloonChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mBalloonCloseDelay, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect(mPrfsEvents->mTrayflashNotifyChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mTrayflashNotifyLeftClickOpensMessageChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mTrayflashNotifySetCurrentDesktopToChatViewChk, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mSoundIfAwayChk, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mEventIfActive, SIGNAL(toggled(bool)),
			this, SLOT(slotSettingsChanged(bool)));
	connect(mPrfsEvents->mRaiseMsgWindowChk, SIGNAL(toggled(bool)),
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
	connect( mAwayConfigUI->rememberedMessages, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mAwayConfigUI->mAutoAwayTimeout, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChanged(int)));
	connect( mAwayConfigUI->mGoAvailable, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mAwayConfigUI->mUseAutoAway, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mAwayConfigUI->mDisplayLastAwayMessage, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mAwayConfigUI->mDisplayCustomAwayMessage, SIGNAL(toggled(bool)),
		this, SLOT(slotSettingsChanged(bool)));
	connect( mAwayConfigUI->mAutoAwayMessageEdit, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotTextChanged(const QString&)));
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
	p->setUseStack(mPrfsGeneral->mUseStackChk->isChecked());
	p->setQueueUnreadMessages(mPrfsGeneral->mQueueUnreadMessagesChk->isChecked());
	p->setAutoConnect(mPrfsGeneral->mAutoConnect->isChecked());

	// "Events" TAB ============================================================
	p->setQueueOnlyHighlightedMessagesInGroupChats(mPrfsEvents->mQueueOnlyHighlightedMessagesInGroupChatsChk->isChecked());
	p->setQueueOnlyMessagesOnAnotherDesktop(mPrfsEvents->mQueueOnlyMessagesOnAnotherDesktopChk->isChecked());
	p->setBalloonNotify(mPrfsEvents->mBalloonNotifyChk->isChecked());
	p->setBalloonNotifyIgnoreClosesChatView(mPrfsEvents->mBalloonNotifyIgnoreClosesChatViewChk->isChecked());
	p->setBalloonClose(mPrfsEvents->mCloseBalloonChk->isChecked());
	p->setBalloonDelay(mPrfsEvents->mBalloonCloseDelay->value());
	p->setTrayflashNotify(mPrfsEvents->mTrayflashNotifyChk->isChecked());
	p->setTrayflashNotifyLeftClickOpensMessage(mPrfsEvents->mTrayflashNotifyLeftClickOpensMessageChk->isChecked());
	p->setTrayflashNotifySetCurrentDesktopToChatView(mPrfsEvents->mTrayflashNotifySetCurrentDesktopToChatViewChk->isChecked());
	p->setSoundIfAway(mPrfsEvents->mSoundIfAwayChk->isChecked());
	p->setRaiseMsgWindow(mPrfsEvents->mRaiseMsgWindowChk->isChecked());
	config->setGroup("General");
	config->writeEntry("EventIfActive", mPrfsEvents->mEventIfActive->isChecked());

	// "Away" TAB ===============================================================
	p->setRememberedMessages( mAwayConfigUI->rememberedMessages->value() );

	config->setGroup("AutoAway");
	config->writeEntry("Timeout", mAwayConfigUI->mAutoAwayTimeout->value() * 60);
	config->writeEntry("GoAvailable", mAwayConfigUI->mGoAvailable->isChecked());
	config->writeEntry("UseAutoAway", mAwayConfigUI->mUseAutoAway->isChecked() );
	config->writeEntry("UseAutoAwayMessage", mAwayConfigUI->mDisplayCustomAwayMessage->isChecked() );
	config->sync();

	// Save the auto away message, if defined
	if( mAwayConfigUI->mDisplayCustomAwayMessage->isChecked() )
	{
		awayInstance->setAutoAwayMessage( mAwayConfigUI->mAutoAwayMessageEdit->text() );
	}

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
	awayInstance = Kopete::Away::getInstance();

	// "General" TAB ============================================================
	mPrfsGeneral->mShowTrayChk->setChecked( p->showTray() );
	mPrfsGeneral->mStartDockedChk->setChecked( p->startDocked() );
	mPrfsGeneral->mInstantMessageOpeningChk->setChecked( !p->useQueue() && !p->useStack());
	mPrfsGeneral->mUseQueueChk->setChecked( p->useQueue() );
	mPrfsGeneral->mUseStackChk->setChecked( p->useStack() );
	mPrfsGeneral->mQueueUnreadMessagesChk->setChecked ( p->queueUnreadMessages() );
	mPrfsGeneral->mAutoConnect->setChecked( p->autoConnect() );

	// "Events" TAB ============================================================
	mPrfsEvents->mQueueOnlyHighlightedMessagesInGroupChatsChk->setChecked ( p->queueOnlyHighlightedMessagesInGroupChats() );
	mPrfsEvents->mQueueOnlyMessagesOnAnotherDesktopChk->setChecked ( p->queueOnlyMessagesOnAnotherDesktop() );
	mPrfsEvents->mBalloonNotifyChk->setChecked ( p->balloonNotify() );
	mPrfsEvents->mBalloonNotifyIgnoreClosesChatViewChk->setChecked ( p->balloonNotifyIgnoreClosesChatView() );
	mPrfsEvents->mCloseBalloonChk->setChecked( p->balloonClose() );
	mPrfsEvents->mBalloonCloseDelay->setValue( p->balloonCloseDelay() );
	mPrfsEvents->mTrayflashNotifyChk->setChecked ( p->trayflashNotify() );
	mPrfsEvents->mTrayflashNotifyLeftClickOpensMessageChk->setChecked ( p->trayflashNotifyLeftClickOpensMessage() );
	mPrfsEvents->mTrayflashNotifySetCurrentDesktopToChatViewChk->setChecked ( p->trayflashNotifySetCurrentDesktopToChatView() );
	mPrfsEvents->mSoundIfAwayChk->setChecked( p->soundIfAway() );
	mPrfsEvents->mRaiseMsgWindowChk->setChecked(p->raiseMsgWindow());
	config->setGroup("General");
	mPrfsEvents->mEventIfActive->setChecked(config->readBoolEntry("EventIfActive", true));

	// "Away" TAB ===============================================================
	config->setGroup("AutoAway");
	mAwayConfigUI->mAutoAwayTimeout->setValue(config->readNumEntry("Timeout", 600)/60);
	mAwayConfigUI->mGoAvailable->setChecked(config->readBoolEntry("GoAvailable", true));
	mAwayConfigUI->mUseAutoAway->setChecked(config->readBoolEntry("UseAutoAway", true));
	mAwayConfigUI->rememberedMessages->setValue( p->rememberedMessages() );
	mAwayConfigUI->mAutoAwayMessageEdit->setText( awayInstance->autoAwayMessage() );

	// Always display the last away message by default
	mAwayConfigUI->mDisplayCustomAwayMessage->setChecked(config->readBoolEntry("UseAutoAwayMessage", false));

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

void BehaviorConfig::slotTextChanged(const QString&)
{
	emit changed( true );
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
