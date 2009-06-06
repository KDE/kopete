/*
    Kopete Yahoo Protocol
    Notifies about new mails

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

#ifndef MAILNOTIFIERTASK_H
#define MAILNOTIFIERTASK_H

#include "task.h"

class QString;
class YMSGTransfer;

/**
@author André Duffeck
*/
class MailNotifierTask : public Task
{
Q_OBJECT
public:
	MailNotifierTask(Task *parent);
	~MailNotifierTask();
	
	bool take(Transfer *transfer);

protected:
	virtual bool forMe( const Transfer *transfer ) const;
	void parseMail( YMSGTransfer *transfer );
signals:
	void mailNotify(const QString&, const QString&, int);
};

#endif
