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
#include "gwcontactsearch.h"

class GroupWiseAccount;
class GroupWiseContactProperties;
class GroupWiseContactSearchWidget;

/**
Logic for searching for and displaying users and chat rooms using a GroupWiseContactSearchWidget

@author SUSE Linux Products GmbH
*/
class GroupWiseContactSearch : public GroupWiseContactSearchWidget
{
Q_OBJECT
public:
	GroupWiseContactSearch( GroupWiseAccount * account, QListView::SelectionMode mode, bool onlineOnly, 
			QWidget *parent = 0, const char *name = 0);
	~GroupWiseContactSearch();
	QValueList< GroupWise::ContactDetails > selectedResults();
signals:
	void selectionValidates( bool );
protected:
	unsigned char searchOperation( int comboIndex );
protected slots:
	void slotClear();
	void slotDoSearch();
	void slotGotSearchResults();
	// shows a GroupWiseContactProperties for the selected contact.  Dialog's parent is this instance
	void slotShowDetails();
	void slotValidateSelection();
private:
	QValueList< GroupWise::ContactDetails > m_searchResults;
	GroupWiseAccount * m_account;
	bool m_onlineOnly;
};

#endif
