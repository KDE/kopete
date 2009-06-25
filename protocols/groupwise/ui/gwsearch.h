/*
    Kopete Groupwise Protocol
    gwsearch.h - logic for server side search widget

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

#ifndef GWSEARCH_H
#define GWSEARCH_H
#include <QList>
#include "ui_gwcontactsearch.h"
#include "gwaccount.h"


class GroupWiseContactSearchModel;
class GroupWiseContactSearchSortProxyModel;
class GroupWiseAccount;
class GroupWiseContactProperties;

/**
Logic for searching for and displaying users and chat rooms using a GroupWiseContactSearchWidget

@author SUSE Linux Products GmbH
*/
class GroupWiseContactSearch : public QWidget, public Ui::GroupWiseContactSearchWidget
{
Q_OBJECT
public:
	GroupWiseContactSearch( GroupWiseAccount * account, QAbstractItemView::SelectionMode mode, bool onlineOnly, 
			QWidget *parent = 0 );
	virtual ~GroupWiseContactSearch();
	QList< GroupWise::ContactDetails > selectedResults();
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
	GroupWise::ContactDetails detailsAtIndex( const QModelIndex & ) const;
	GroupWiseContactSearchModel * m_model;
	GroupWiseContactSearchSortProxyModel * m_proxyModel;
	GroupWiseAccount * m_account;
	QList< GroupWise::ContactDetails > m_lastSearchResults;
};

#endif
