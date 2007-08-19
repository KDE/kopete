/*
   sbkeepalivetask.cpp - Windows Live Messenger Switchboard keep alive task

   Copyright (c) 2007		by Zhang Panyong  <pyzhang@gmail.com>
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
#include "Papillon/Tasks/SBKeepAliveTask"

// Qt includes
#include <QtDebug>
#include <QTimer>

// Papillon includes
#include "Papillon/NetworkMessage"

namespace Papillon
{

class SBKeepAliveTask::Private
{
public:
	Private()
	{}
	QTimer *m_keepAlive;
	int m_keepAliveNb;
};

SBKeepAliveTask::SBKeepAliveTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{
	d->m_keepAlive = 01;
	d->m_keepAliveNb = 0;
}

SBKeepAliveTask::~SBKeepAliveTask()
{

}

SBKeepAliveTask::init()
{
	if(!d->m_keepAlive)
	{
		d->m_keepAliveNb = 20;
		d->m_keepAlive= new QTimer(this);
		QObject::connect(d->m_keepAlive, SIGNAL(timeout()) , this , SLOT(slotKeepAliveTimer()));
		d->m_keepAlive->start(50*1000);
	}
}

SBKeepAliveTask::slotKeepAliveTimer()
{
	NetworkMessage *sbkeepaliveMessage = new NetworkMessage(NetworkMessage::TransactionTransfer);
	MimeHeader keepaliveMSG;

	keepaliveMSG.setMimeVersion();
	keepaliveMSG.setContentType("text/x-keepalive");

	sbkeepaliveMessage->setCommand("MSG");
	sbkeepaliveMessage->setArguments(QString("U"));
	sbkeepaliveMessage->setPayloadData(keepaliveMSG.toString().toUtf8());

	send(sbkeepaliveMessage);

	d->m_keepAliveNb--;
	if(d->m_keepAliveNb <= 0)
	{
		d->m_keepAlive->deleteLater();
		d->m_keepAlive = 0;
	}
}

}
#include "sbkeepalivetask.moc"
