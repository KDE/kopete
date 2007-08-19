/*
   nudgetask.cpp - Windows Live Messenger send Nudge task

   Copyright (c) 2007      by Zhang Panyong  <pyzhang@gmail.com>
   Copyright (c) 2006 		by Michaël Larouche <larouche@kde.org>
   Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Tasks/NudgeTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

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

bool Task::forMe(NetworkMessage *networkMessage) const
{
	QString msg;
	MimeHeader mime_msg;
	QString mimeContentType;

	if( networkMessage->command() == QLatin1String("MSG") )
	{
		msg = QString::fromUtf8(networkMessage->payloadData(),
				networkMessage->payloadLength());
		mime_msg = MimeHeader::parseMimeHeader(msg);
		mimeContentType = mime_msg.contentType();

		if(mimeContentType == QString("text/x-datacast"))
		{
			if(msg.contain("ID:"))
			{
				QRegExp rx("ID: ([0-9]*)");
				rx.indexIn(msg);
				uint dataCastId = rx.cap(1).toUInt();
				if( dataCastId == 1 )
				{
					return true;
				}
			}
		}
	}
	return false
}

bool NudgeTask::take(NetworkMessage *networkMessage)
{
	if(forMe(networkMessage))
	{
		emit nudgeReceived();
		return true;
	}
	return false;
}

void NudgeTask::sendNudgeCommand()
{
	NetworkMessage *nudgeMessage = new NetworkMessage(NetworkMessage::NormalTransfer);
	QByteArray message = QString( "MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msnmsgr-datacast\r\n"
			"\r\n"
			"ID: 1\r\n"
			"\r\n\r\n" ).toUtf8();

	qDebug() << Q_FUNC_INFO << "Sending Nudge";

	nudgeMessage->setCommand("MSG");
	nudgeMessage->setArguments(QString("U"));
	nudgeMessage->setPayloadData(message);

	send( nudgeMessage );
}

void NudgeTask::onGo()
{
	sendNudgeCommand();
}

}

#include "nudgetask.moc"
