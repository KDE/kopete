/*
    Kopete Yahoo Protocol
    Handles several lists such as buddylist, ignorelist and so on

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LISTTASK_H
#define LISTTASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class ListTask : public Task
{
Q_OBJECT
public:
	ListTask(Task *parent);
	~ListTask();
	
	bool take(Transfer *transfer);

	const QString &yCookie() const;
	const QString &cCookie() const;
	const QString &tCookie() const;
	const QString &loginCookie() const;

protected:
	bool forMe( Transfer *transfer ) const;
	void parseCookies( Transfer *transfer );
	void parseBuddyList( Transfer *transfer );

signals:
	void gotCookies();
	void gotBuddy(const QString&, const QString&, const QString&);

private:
	QString m_yCookie;
	QString m_tCookie;
	QString m_cCookie;
	QString m_loginCookie;
};

#endif
