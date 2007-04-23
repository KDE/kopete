/*
    kircechotask.cpp - IRC Echo Task

    Copyright (c) 2006      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2006      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircechotask.moc"

#include "kircevent.h"

/*
class KIrc::EchoTask::Private
{
public:
};
*/
using namespace KIrc;

EchoTask::EchoTask(QObject *parent)
	: Task(parent)
	, d(0)
{
}

EchoTask::~EchoTask()
{
//	delete d;
}

Task::Status EchoTask::doMessage(Message msg)
{
	Event event;
	event.setMessage(msg);
//	event.setType(messageType);
//	event.setFrom(msg.prefix());
//	event.setCc(msg.socket().owner()); // server instead ?

	event.setText(msg.rawLine());

	postEvent(event);

	return NotHandled;
}

