/*
    privacypreferences.h

    Copyright (c) 2006 by Andre Duffeck             <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PRIVACY_PREFERENCES_H
#define PRIVACY_PREFERENCES_H

#include <kcmodule.h>

#include <QVariantList>

namespace Ui { class PrivacyPrefsUI; }
class PrivacyAccountListModel;

class PrivacyPreferences : public KCModule
{
	Q_OBJECT
public:
	enum SenderMode { AllowAllMessages, AllowNoMessagesExceptWhiteList, AllowAllMessagesExceptBlackList };

	explicit PrivacyPreferences(QWidget *parent=0, const QVariantList &args = QVariantList());
	~PrivacyPreferences();

	virtual void save();
	virtual void load();	
private Q_SLOTS:
	void slotConfigChanged();
	void slotModified();
	void slotChkDropAtLeastOneToggled(bool);
	void slotChkDropAllToggled(bool);

	void slotBtnAddToWhiteListClicked();
	void slotBtnAddToBlackListClicked();	
	void slotBtnClearWhiteListClicked();
	void slotBtnClearBlackListClicked();
	void slotBtnRemoveFromWhiteListClicked();
	void slotBtnRemoveFromBlackListClicked();

	void slotSetupViews();
private:
	Ui::PrivacyPrefsUI *prefUi;
	PrivacyAccountListModel *m_whiteListModel;
	PrivacyAccountListModel *m_blackListModel;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

