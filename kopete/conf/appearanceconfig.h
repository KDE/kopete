/*
    appearanceconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __APPEARANCE_H
#define __APPEARANCE_H

#include "configmodule.h"

#include <kdeversion.h>

class KTabCtl;

class KListBox;
class QVGroupBox;
class QVButtonGroup;
class QHButtonGroup;
class QRadioButton;
class QCheckBox;
class QFrame;
class QPushButton;
class KProcess;

class AppearanceConfig_ChatWindow;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AppearanceConfig : public ConfigModule
{
	Q_OBJECT

public:
	AppearanceConfig(QWidget * parent);
	~AppearanceConfig();

	virtual void save();
	virtual void reopen();

public slots:
	void slotSelectKind(int k);

private slots:
	void slotConfigSound(void);
	void slotSoundChanged(void);
	void slotUseEmoticonsChanged(bool);
	void slotConfigChanged(void);

private:
	KTabCtl* mAppearanceTab;

	// Widgets for General TAB
	QFrame* mGeneralTab;
	QCheckBox *mStartDockedChk;
	QCheckBox *mUseQueueChk;
	QVGroupBox *notifyGroupBox;
	QCheckBox *mBalloonNotifyChk;
	QCheckBox *mBeepNotifyChk;
	QCheckBox *mTrayflashNotifyChk;
	QCheckBox *mSoundNotifyChk;
	QCheckBox *mSoundIfAwayChk;
	QCheckBox *mUseEmoticonsChk;
	QPushButton *configSound;
	QCheckBox* cb_RaiseMsgWindowChk;
	QCheckBox* cb_ShowEventsChk;
	
	QHButtonGroup* ButtonGroup1;
	QRadioButton* cb_Enter;
	QRadioButton* cb_CtrlEnter;
	QRadioButton* cb_ShiftEnter;
    
	QVButtonGroup *chatWindowGroup;
	QRadioButton *mNewWindow;
	QRadioButton *mTabWindow;
	QRadioButton *mTabProtocolWindow;

	// Widgets for Contactlist TAB
	QFrame* mContactListTab;
	QCheckBox *mShowOfflineUsers;
	#if KDE_VERSION >= 306
	QCheckBox *mNotifyOnlineUsers;
	#endif
	QCheckBox *mTreeContactList;
	QCheckBox *mSortByGroup;
	QCheckBox *mHideMetaContacts;
	QCheckBox *mGreyIdleMetaContacts;

	// Widgets for Emoticon TAB
	QFrame* mEmoticonsTab;
	KListBox *icon_theme_list;

	// Widgets for the Chat Window TAB
	QFrame* mChatWindowTab;
	AppearanceConfig_ChatWindow *mPrfsChatWindow;

	QFrame* mChatAppearanceTab;
	
	bool mQueueChanged;
	KProcess *kcm;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

