/*
   typingtask.cpp - Windows Live Messenger SwitchBoard Typing Task processing

   Copyright (c) 2007      by Zhang Panyong  <pyzhang@gmail.com>
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
#include "Papillon/Tasks/TypingTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class TypingTask::Private
{
	public:
		Private()
		{
		}
		QString m_myHandle;
}

TypingTask::TypingTask(Task *parent, QString handle)
	:Task(parent), d(new Private)
{
	d->m_myHandle = handle;
}

TypingTask::~TypingTask()
{
	delete d;
}

void TypingTask::sendTyping(bool isTyping)
{
	NetworkMessage *typingMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	MimeHeader typingHeader;

	if(!isTyping)
	{
		return;
	}
	typingHeader.setMimeVersion();
	typingHeader.setContentType(QLatinString("text/x-msmsgscontrol"));
	typingHeader.setValue(QLatin1String("TypingUser", m_myHandle));

	typingMessage->setCommand("MSG");
	typingMessage->setArguments(QString("U"));
	typingMessage->setPayloadData(typingHeader.toString().toUtf8());

	send( typingMessage );
}

bool TypingTask::take(NetworkMessage *networkMessage)
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

		if(mimeContentType == QString("text/x-msmsgscontrol"))
		{
			emit receiveTypingMsg(mime_msg.value("TypingUser").toString(),true);
		}
	}
}

}
#include "typingtask.moc"
