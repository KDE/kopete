/*
    filetransfersession.cpp - Peer to Peer File Transfer Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "filetransfersession.h"
#include <qdatastream.h>
#include <qfile.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace PeerToPeer
{

class FileTransferSession::FileTransferSessionPrivate
{
	public:
		FileTransferSessionPrivate() : autoAccept(false), file(0l), size(0) {}

		bool autoAccept;
		QFile *file;
		Q_INT64 size;
};

QUuid FileTransferSession::uuid()
{
	return QUuid("5D3E02AB-6190-11D3-BBBB-00C04F795683");
}

FileTransferSession::FileTransferSession(const Q_UINT32 identifier, Direction direction, QObject *parent) : Session(identifier, direction, parent), d(new FileTransferSessionPrivate())
{
}

FileTransferSession::~FileTransferSession()
{
	if (d->file != 0l)
	{
		d->file->close();
		delete d->file;
	}

	delete d;
}

void FileTransferSession::handleInvite(const Q_UINT32 appId, const QByteArray& context)
{
	if (appId == 2)
	{
		// Retrieve the file information from the context.
		QDataStream stream(context, IO_ReadOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		// File Size (Int64) [8..15]
		stream.device()->at(8);
		stream >> d->size;
		// type (Int32) [15..18]
		// 0x00 File transfer with preview data.
		// 0x01 File transfer without preview data.
		// 0x02 Background sharing.
		Q_INT32 type;
		stream >> type;

		// FileName UTF16 (Unicode) [19..539]
		QByteArray bytes(520);
		stream.readRawBytes(bytes.data(), bytes.size());
		const QString file = QString::fromUcs2((unsigned short*)((void*)bytes.data()));

		setDataStore(new QFile(locateLocal("tmp", file)));

		if (type == 1 || type == 0)
		{
			kdDebug() << "Contact sending file, " << file << " (size " << d->size << ")" << endl;
			accept();
		}
		else
		if (type == 2)
		{
			kdDebug() << "Contact sending background sharing file, " << file << " (size " << d->size << ")" << endl;
			decline();
		}
		else
		{
			kdDebug() << k_funcinfo << "Contact sending file with unknown type -- declining" << endl;
			decline();
		}
	}
	else
	{
		// Otherwise, decline the session.
		decline();
	}
}

QFile* FileTransferSession::dataStore() const
{
	return d->file;
}

void FileTransferSession::setDataStore(QFile *file) const
{
	d->file = file;
}

void FileTransferSession::onStart()
{
	if (direction() == Session::Outgoing)
	{
		if (!d->file->open(IO_ReadOnly))
		{
			kdDebug() << k_funcinfo << "Session=" << id() << " cancelling -- unable to open file "
			<< d->file->name() << endl;

			cancel();
			return;
		}

		QByteArray data = d->file->readAll();
		d->file->close();

		kdDebug() << k_funcinfo << "Session=" << id() << ", Sending DATA size(" << data.size() << ")" << endl;
		emit sendData(data);
	}
	else
	if (direction() == Session::Incoming)
	{
		kdDebug() << k_funcinfo << "Session=" << id() << ", ready to receive data" << endl;
		d->file->open(IO_WriteOnly);

		// TODO Start timer and wait for 30 seconds.
		// If we do not received data by that time,
		// cancel the session.
	}
}

void FileTransferSession::onEnd()
{
	kdDebug() << k_funcinfo << endl;
	if (direction() == Session::Incoming)
	{
		emit ended();
	}
}

void FileTransferSession::onFaulted()
{
}

void FileTransferSession::onDataReceived(const QByteArray& data, const Q_INT32 identifier, bool lastChunk)
{
	kdDebug() << k_funcinfo << "Session=" << id() << ", Receiving DATA size(" << data.size() << ")" << endl;

	// Write the received bytes to the file.
	d->file->writeBlock(data, data.size());
	d->file->flush();

	if (lastChunk)
	{
		kdDebug() << k_funcinfo << "Session=" << id() << ", END OF DATA" << endl;

		d->file->close();
		end();
	}
}

void FileTransferSession::onSend(const Q_INT32 identifier)
{
	Q_UNUSED(identifier);
	kdDebug() << k_funcinfo << endl;

	// TODO Start a timer. if the BYE request is not received in 15 sec send one.
// 	emit ended();
}

}

#include "filetransfersession.moc"
