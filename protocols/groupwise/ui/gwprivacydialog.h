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
class QListBoxItem;

/**
Logic for the UI part managing the allow and deny lists, and the default privacy setting.

@author Kopete Developers
*/
class GroupWisePrivacyDialog : public KDialogBase
{
public:
	GroupWisePrivacyDialog( GroupWiseAccount * account, QWidget * parent, const char * name );
	~GroupWisePrivacyDialog();
protected:
	void disableWidgets();
private:
	GroupWiseAccount * m_account;
	GroupWisePrivacyWidget * m_widget;
	QListBoxItem * m_defaultPolicy;
};

#endif
