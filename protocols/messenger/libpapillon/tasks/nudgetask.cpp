/*
   nudgetask.cpp - Windows Live Messenger send Nudge task

    Copyright (c) 2007		by Zhang Panyong  <pyzhang8@gmail.com>
    Copyright (c) 2006 		by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

namespace Papillon
{

class NudgeTask::Private
{
public:
	Private()
	{}
};

NudgeTask::NudgeTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
}

NudgeTask::~NudgeTask()
{
	delete d;
}

bool NudgeTask::take(Transfer *transfer)
{
	return false;
}

void NudgeTask::sendNudgeCommand()
{
	qDebug() << PAPILLON_FUNCINFO << "Sending Nudge";
}

void NudgeTask::onGo()
{
	sendNudgeCommand();
}

}

#include "nudgetask.moc"
