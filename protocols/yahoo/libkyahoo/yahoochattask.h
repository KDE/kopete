/*
    Kopete Yahoo Protocol
    yahoochattask.h - Handle Yahoo Chat

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

#ifndef YAHOOCHATTASK_H
#define YAHOOCHATTASK_H

#include "task.h"
#include "yahootypes.h"
#include <QMap>
#include <QList>
#include <QPair>

class QByteArray;
class QDomDocument;
class KJob;
class YMSGTransfer;
namespace KIO {
	class Job;
}

struct YahooChatJob {
	QByteArray data;
	Yahoo::ChatCategory category;
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
	virtual bool forMe( const Transfer *transfer ) const;
	bool take(Transfer *transfer);

	void getYahooChatCategories();
	void getYahooChatRooms( const Yahoo::ChatCategory &category );

	void joinRoom( const Yahoo::ChatRoom &room );

	void sendYahooChatMessage( const QString &msg, const QString &handle );

	void logout();

Q_SIGNALS:
	void gotYahooChatCategories( const QDomDocument & );
	void gotYahooChatRooms( const Yahoo::ChatCategory &, const QDomDocument & );

	void chatRoomJoined( int roomId, int categoryId, const QString &comment, const QString &handle );
	void chatBuddyHasJoined( const QString &nick, const QString &handle, bool suppressNotification );
	void chatBuddyHasLeft( const QString &nick, const QString &handle );
	void chatMessageReceived( const QString &nick, const QString &message, const QString &handle );
private:
	void login();
	void parseLoginResponse( YMSGTransfer *t );
	void parseJoin( YMSGTransfer *t );
	void parseChatMessage( YMSGTransfer * );
	void parseChatExit( YMSGTransfer * );
	void parseLogout( YMSGTransfer * );

private Q_SLOTS:
	void slotData( KIO::Job *, const QByteArray & );
	void slotCategoriesComplete( KJob * );
	void slotChatRoomsComplete( KJob * );
private:
	QMap< KIO::Job *, YahooChatJob > m_jobs;
	QList< Yahoo::ChatRoom > m_pendingJoins;
	bool m_loggedIn;
};

#endif
