/*
    jingleswatchsessiontask.h - Watch for incoming Jingle sessions.

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLEWATCHSESSIONTASK_H
#define JINGLEWATCHSESSIONTASK_H

#include <xmpp_tasks.h>

/**
 * This task watch for incoming Jingle session and notify manager.
 * It is declared in the header to be "moc"-able.
 */
class JingleWatchSessionTask : public XMPP::Task
{
	Q_OBJECT
public:
	JingleWatchSessionTask(XMPP::Task *parent);
	~JingleWatchSessionTask();

	bool take(const QDomElement &element);

signals:
	void watchSession(const QString &sessionType, const QString &initiator);
};

#endif
