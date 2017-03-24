/*
   Kopete Oscar Protocol
   usersearchtask.h - Search for contacts

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef USERSEARCHTASK_H
#define USERSEARCHTASK_H

#include "icqtask.h"
#include <qstring.h>
#include <QList>
#include "icquserinfo.h"

/**
Search for contacts

@author Kopete Developers
*/
class UserSearchTask : public ICQTask
{
Q_OBJECT
public:
	UserSearchTask( Task* parent );
	
	~UserSearchTask();
	
	enum SearchType { UINSearch, WhitepageSearch };
	
	void onGo() Q_DECL_OVERRIDE;
	bool forMe( const Transfer* t ) const Q_DECL_OVERRIDE;
	bool take( Transfer* t ) Q_DECL_OVERRIDE;
	
	/** Search by UIN */
	void searchUserByUIN( const QString& uin );
	
	void searchWhitePages( const ICQWPSearchInfo& info );
	
signals:
	void foundUser( const ICQSearchResult& result );
	void searchFinished( int );
	
private:
	QList<ICQSearchResult> m_results;
	SearchType m_type;
};

#endif
