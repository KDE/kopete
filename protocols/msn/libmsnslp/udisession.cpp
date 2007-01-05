/*
    udisession.cpp - Peer To Peer User Display Icon Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "udisession.h"
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qfileinfo.h>

namespace PeerToPeer
{

UdiSession::UdiSession(const Q_UINT32 id, Session::Direction direction, QObject *parent) : Session(id, direction, parent)
{
}

UdiSession::~UdiSession()
{
}

void UdiSession::onStart()
{
	QFile *store = 0l;
	if (direction() == Session::Outgoing)
	{
		kdDebug() << k_funcinfo << "Session=" << id() << ", Sending DATA PREPARATION message" << endl;
		QByteArray bytes(4);
		bytes[0] = '\0';
		bytes[1] = '\0';
		bytes[2] = '\0';
		bytes[3] = '\0';

		emit sendMessage(bytes);

		store = dataStore();
		if (store != 0l && store->open(IO_ReadOnly))
		{
			QByteArray dataBuffer = store->readAll();
			store->close();

			kdDebug() << k_funcinfo << "Session=" << id() << ", Sending DATA size(" << dataBuffer.size() << ")" << endl;
			emit readyWrite(dataBuffer);
		}
		else
		{
			kdDebug() << k_funcinfo << "Unable to open resource" << endl;
		}
	}
	else
	if (direction() == Session::Incoming)
	{
		store = dataStore();
		if (store != 0l && store->open(IO_WriteOnly))
		{
			kdDebug() << k_funcinfo << "Session=" << id() << ", Ready to receive data" << endl;
		}
		else
		{
			kdDebug() << k_funcinfo << "Unable to open resource" << endl;
		}
	}
}

void UdiSession::onEnd()
{
	kdDebug() << k_funcinfo << endl;
	if (direction() == Session::Incoming)
	{
		emit terminate();
	}
}

void UdiSession::onDataReceived(const QByteArray& data)
{
	kdDebug() << k_funcinfo << "Session=" << id() << ", Receiving DATA size(" << data.size() << ")" << endl;
	QFile *store = dataStore();
	if (store != 0 && store->isOpen())
	{
		store->writeBlock(data, data.size());
	}
	else
	{
		kdDebug() << k_funcinfo << "store not open" << endl;
	}
}

void UdiSession::onEndOfData(const Q_INT32 identifier)
{
	kdDebug() << k_funcinfo << "Session=" << id() << ", END OF DATA" << endl;
	emit sendAcknowledge(identifier);

	QFile *file = dataStore();
	if (file != 0l)
	{
		emit transferComplete(QFileInfo(*file).filePath());
	}

	end();
}

void  UdiSession::onMessageSent(const Q_INT32 identifier)
{
	kdDebug() << k_funcinfo << endl;
}

void UdiSession::onMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	QByteArray buffer(4);
	buffer.fill('\0');
	if (message.size() == 4 && message == buffer)
	{
		kdDebug() << k_funcinfo << "Session=" << id() << ", Received DATA PREPARATION message" << endl;
		emit sendAcknowledge(identifier);
	}
}

}

#include "udisession.moc"
