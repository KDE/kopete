/*
    kirctask.cpp - IRC Task.

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

#include "kirctask.moc"

#include <kdebug.h>

using namespace KIrc;

Task::Task(QObject *parent)
	: QObject(parent)
	, d(0)
{
}

Task::~Task()
{
//	delete d;
}

Task::Status Task::doCommand(KIrc::Message)
{
	return NotHandled;
}

Task::Status Task::doEvent(KIrc::Event)
{
	return NotHandled;
}

Task::Status Task::doMessage(KIrc::Message)
{
	return NotHandled;
}

