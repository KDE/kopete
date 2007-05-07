/*
   Kopete Oscar Protocol
   ssiauthtask.h - SSI Authentication Task

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

#ifndef SSIAUTHTASK_H
#define SSIAUTHTASK_H

#include <task.h>

class ContactManager;

/**
@author Kopete Developers
*/
class SSIAuthTask : public Task
{
Q_OBJECT
public:
	SSIAuthTask( Task* parent );

	~SSIAuthTask();
	
	virtual bool forMe( const Transfer* t ) const;
	virtual bool take( Transfer* t );
	
	void grantFutureAuth( const QString& uin, const QString& reason );
	void sendAuthRequest( const QString& uin, const QString& reason );
	void sendAuthReply( const QString& uin, const QString& reason, bool auth );
signals:
	void futureAuthGranted( const QString& uin, const QString& reason );
	void authRequested( const QString& uin, const QString& reason );
	void authReplied( const QString& uin, const QString& reason, bool auth );
	void contactAddedYou( const QString& uin );
private:
	void handleFutureAuthGranted();
	void handleAuthRequested();
	void handleAuthReplied();
	void handleAddedMessage();
	
	QString parseReason( Buffer* buffer );
	
private:
	ContactManager* m_manager;
};

#endif

//kate: tab-width 4; indent-mode csands;
