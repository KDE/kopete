//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GWPRIVACYDIALOG_H
#define GWPRIVACYDIALOG_H

#include <kdialogbase.h>

class GroupWiseAccount;
class GroupWisePrivacyWidget;
class GroupWiseSearch;
class QListBoxItem;

/**
Logic for the UI part managing the allow and deny lists, and the default privacy setting.

@author Kopete Developers
*/
class GroupWisePrivacyDialog : public KDialogBase
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
	GroupWisePrivacyWidget * m_privacy;
	GroupWiseSearch * m_search;
	QListBoxItem * m_defaultPolicy;
	bool m_dirty;
};

#endif
