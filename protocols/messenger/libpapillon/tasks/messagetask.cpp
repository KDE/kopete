/*
   messagetask.cpp - Windows Live Messenger SwitchBoard MSG processing

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
#include "Papillon/Tasks/AddListTask"

// Qt includes
#include <QtDebug>
#include <QtCore/QLatin1String>

// Papillon includes
#include "Papillon/NetworkMessage"
#include "Papillon/Connection"

namespace Papillon
{
class MessageTask::Private
{
	public:
		Private()
		{
		}
		// Keep track of the expected transaction ID.
		QString currentTransactionId;
}

MessageTask::MessageTask(Task *parent)
	:Task(parent), d(new Private)
{

}

MessageTask::~MessageTask()
{
	delete d;
}

bool MessageTask::take(NetworkMessage *networkMessage)
{
	QString msg;
	MimeHeader mime_msg;
	QString mimeContentType;

	if( forMe(networkMessage) )
	{
		msg = QString::fromUtf8(networkMessage->payloadData(),
				networkMessage->payloadLength());
		mime_msg = MimeHeader::parseMimeHeader(msg);
		mimeContentType = mime_msg.contentType();

		if(mimeContentType == QString("text/plain"))
		{

		}
		else if(mimeContentType == QString("text/x-msmsgsinvite"))
		{
			emit receiveInvitation(mime_msg);
		}
		else if(mimeContentType == QString("text/x-msmsgscontrol"))
		{

		}
		else if(mimeContentType == QString("text/x-msnmsgr-datacast"))
		{
			if(mime_msg.value( "ID" ).toString() == QString("1") ){
				qDebug()<< Q_FUNC_INFO <<"Receive a Nudge!\n";
				emit nudgeReceived();
			}
		}
		else if( (mimeContentType == QString("text/x-mms-emoticon") ) ||
				(mimeContentType == QString("text/x-mms-aniemoticon"))
				)
		{

		}
		else if(mimeContentType == QString("text/x-msnmsgrp2p"))
		{

		}
		else if(mimeContentType == QString("text/x-clientcaps"))
		{

		}
		else if(mimeContentType == QString("image/gif")||
				(mime_msg.value("Message-ID") != NULL) )
		{

		}
		else
		{
			emit unknownMSG(mime_msg.contentType(), msg);
		}
		return true;
	}
}

bool MessageTask::forMe(NetworkMessage *networkMessage) const
{
	if( networkMessage->command() == QLatin1String("MSG") )
	{
		return true;
	}

	return false;
}

}

#include "messagetask.moc"
