/*
    Kopete Groupwise Protocol
    gwprivacydialog.h - dialog summarising, and editing, the user's privacy settings

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GWPRIVACYDIALOG_H
#define GWPRIVACYDIALOG_H

#include <kdialog.h>
#include "gwsearch.h"
#include "ui_gwprivacy.h"

class GroupWiseAccount;
class Q3ListBoxItem;

/**
Logic for the UI part managing the allow and deny lists, and the default privacy setting.

@author Kopete Developers
*/
class GroupWisePrivacyDialog : public KDialog
{
Q_OBJECT
public:
	GroupWisePrivacyDialog( GroupWiseAccount * account, QWidget * parent, const char * name );
	~GroupWisePrivacyDialog();
protected:
	void commitChanges();
	void errorNotConnected();
	void disableWidgets();
	void populateWidgets();
	void updateButtonState();
protected slots:
	void slotAllowClicked();
	void slotBlockClicked();
	void slotAddClicked();
	void slotRemoveClicked();
	void slotAllowListClicked();
	void slotDenyListClicked();
	void slotPrivacyChanged();
	void slotSearchedForUsers();
	void slotOk();
	void slotApply();
	
private:
	GroupWiseAccount * m_account;
	Ui::GroupWisePrivacy m_privacy;
	GroupWiseContactSearch * m_search;
	Q3ListBoxItem * m_defaultPolicy;
	bool m_dirty;
	KDialog * m_searchDlg;
};

#endif
