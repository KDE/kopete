/*
    Kopete Yahoo Protocol
    Notifies about status changes of buddies

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATUSNOTIFIERTASK_H
#define STATUSNOTIFIERTASK_H

#include "task.h"
#include "yahootypes.h"

class QString;
class YMSGTransfer;

/**
@author André Duffeck
*/
class StatusNotifierTask : public Task
{
Q_OBJECT
public:
	StatusNotifierTask(Task *parent);
	~StatusNotifierTask();
	
	bool take(Transfer *transfer);

protected:
	virtual bool forMe( const Transfer *transfer ) const;
	void parseStatus( YMSGTransfer *transfer );
	void parseStealthStatus( YMSGTransfer *transfer );
	void parseAuthorization( YMSGTransfer *transfer );
signals:
	void statusChanged( const QString &nick, int state, const QString &message, int away, int idle, int pictureChecksum );
	void stealthStatusChanged( const QString&, Yahoo::StealthStatus );
	void loginResponse( int, const QString& );
	void authorizationAccepted( const QString & );
	void authorizationRejected( const QString &, const QString & );
	void gotAuthorizationRequest( const QString &, const QString &, const QString & );
};

#endif
