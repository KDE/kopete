/*
    Kopete Groupwise Protocol
    gwsearch.h - logic for server side search widget

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
public slots:
	void doSearch();
protected:
	unsigned char searchOperation( int comboIndex );
protected slots:
	void slotGotSearchResults();
	void slotShowDetails();
private:
	QValueList< GroupWise::ContactDetails > m_searchResults;
	GroupWiseAccount * m_account;
};

#endif
