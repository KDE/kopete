/*
    Kopete Yahoo Protocol
    Receive Messages

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

#ifndef MESSAGERECEIVERTASK_H
#define MESSAGERECEIVERTASK_H

#include "task.h"

class QString;
class YMSGTransfer;

/**
@author André Duffeck
*/
class MessageReceiverTask : public Task
{
Q_OBJECT
public:
	MessageReceiverTask(Task *parent);
	~MessageReceiverTask();
	
	bool take(Transfer *transfer);

protected:
	virtual bool forMe( const Transfer *transfer ) const;
	void parseMessage( YMSGTransfer *transfer );
	void parseAnimatedAudibleIcon( YMSGTransfer *transfer );
	void parseNotify( YMSGTransfer *transfer );
signals:
	void gotIm(const QString&, const QString&, long, int);
	void gotBuzz( const QString &who, long tm );
	void systemMessage(const QString&);
	void gotTypingNotify(const QString &, int);
	void gotWebcamInvite(const QString &);
};

#endif
