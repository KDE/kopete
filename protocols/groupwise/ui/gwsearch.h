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
#ifndef GWSEARCH_H
#define GWSEARCH_H
#include <qlistview.h>
#include "gwsearchwidget.h"

class GroupWiseAccount;
class GroupWiseContactProperties;
class GroupWiseSearchWidget;

/**
Logic for searching and displaying results using a GroupWiseSearchWidget

@author Kopete Developers
*/
class GroupWiseSearch : public GroupWiseSearchWidget
{
Q_OBJECT
public:
	GroupWiseSearch( GroupWiseAccount * account, QListView::SelectionMode mode, QWidget *parent = 0, const char *name = 0);
	~GroupWiseSearch();
	QValueList< GroupWise::ContactDetails > selectedResults();
protected:
	unsigned char searchOperation( int comboIndex );
protected slots:
	void slotDoSearch();
	void slotGotSearchResults();
	void slotShowDetails();
private:
	QValueList< GroupWise::ContactDetails > m_searchResults;
	GroupWiseAccount * m_account;
};

#endif
