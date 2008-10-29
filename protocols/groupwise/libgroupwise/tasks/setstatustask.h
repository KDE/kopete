/*
    Kopete Groupwise Protocol
    setstatustask.h - Sets our status on the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#ifndef SETSTATUSTASK_H
#define SETSTATUSTASK_H

#include "gwerror.h"
#include "requesttask.h"

/**
@author Kopete Developers
*/
class SetStatusTask : public RequestTask
{
Q_OBJECT
public:
	SetStatusTask(Task* parent);
	~SetStatusTask();
	void status( GroupWise::Status newStatus, const QString &awayMessage, const QString &autoReply );
	GroupWise::Status requestedStatus() const;
	QString awayMessage() const;
	QString autoReply() const; 
private:
	GroupWise::Status m_status;
	QString m_awayMessage;
	QString m_autoReply;
};

#endif
