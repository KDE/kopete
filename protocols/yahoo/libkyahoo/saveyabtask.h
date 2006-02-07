/*
    Kopete Yahoo Protocol
    saveyabtask.h - Saves a YAB entry

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>
    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef SAVEYABTASK_H
#define SAVEYABTASK_H

#include "task.h"
#include "yabentry.h"

struct KURL;
namespace KIO	{ 
	class Job;
	class TransferJob; 
}
class QDomElement;

/**
@author André Duffeck
*/
class SaveYABTask : public Task
{
	Q_OBJECT
public:
	SaveYABTask(Task *parent);
	~SaveYABTask();

	virtual void onGo();
	void setEntry( const YABEntry & );
signals:
	void gotEntry( YABEntry * );
private slots:
	void connectSucceeded();
	void connectFailed( int );
	void slotRead();
private:
	KIO::TransferJob *m_transferJob;
	QString m_data;
	QString m_postData;
};

#endif
