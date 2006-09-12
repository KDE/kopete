/*
    Kopete Yahoo Protocol
    yahoochattask.h - Handle Yahoo Chat

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

#ifndef YAHOOCHATTASK_H
#define YAHOOCHATTASK_H

#include "task.h"
#include <QMap>

class QByteArray;
class QDomDocument;
class KJob;
namespace KIO {
	class Job;
	class TransferJob;
}

struct YahooChatJob {
	QByteArray data;
	int category;
};

/**
@author André Duffeck
*/
class YahooChatTask : public Task
{
Q_OBJECT
public:
	YahooChatTask(Task *parent);
	virtual ~YahooChatTask();
	
	virtual void onGo();

	void getYahooChatCategories();
	void getYahooChatRooms( int category );

Q_SIGNALS:
	void gotYahooChatCategories( const QDomDocument & );
	void gotYahooChatRooms( int category, const QDomDocument & );

private:
private Q_SLOTS:
	void slotData( KIO::Job *, const QByteArray & );
	void slotCategoriesComplete( KJob * );
	void slotChatRoomsComplete( KJob * );
private:
	QMap< KIO::Job *, YahooChatJob > m_jobs;
};

#endif
