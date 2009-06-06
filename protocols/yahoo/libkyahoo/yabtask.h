/*
    Kopete Yahoo Protocol
    yabtask.h - Handles the Yahoo Address Book

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
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

#ifndef YABTASK_H
#define YABTASK_H

#include "task.h"
#include "yabentry.h"

class KJob;
class YMSGTransfer;

namespace KIO	{ 
	class Job;
	class TransferJob; 
}

/**
@author André Duffeck
*/
class YABTask : public Task
{
	Q_OBJECT
public:
	YABTask(Task *parent);
	~YABTask();
	
	bool take(Transfer *transfer);

	void getAllEntries( long lastMerge, long lastRemoteRevision );
	void saveEntry( const YABEntry & );
signals:
	void gotEntry( YABEntry * );
	void gotRevision( long rev, bool merged );
protected:
	virtual bool forMe( const Transfer* transfer ) const;
	void parseContactDetails( YMSGTransfer* t );
private slots:
	void slotData( KIO::Job*, const QByteArray & );
	void slotResult( KJob* );
private:
	KIO::TransferJob *m_transferJob;
	QString m_data;
};

#endif
