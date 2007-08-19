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
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <kopetecontact.h>
#include <kstandarddirs.h>

namespace PeerToPeer
{

class FileTransferSession::FileTransferSessionPrivate
{
	public:
		FileTransferSessionPrivate() : autoAccept(false), file(0l), offset(0), size(0), transfer(0l) {}

		bool autoAccept;
		Kopete::Contact* contact;
		QFile *file;
		Q_INT64 offset;
		Q_INT64 size;
		Kopete::Transfer *transfer;
};

FileTransferSession::FileTransferSession(const Q_UINT32 id, DataTransferDirection direction, Kopete::Contact *contact, QObject *parent) : Session(id, direction, parent), d(new FileTransferSessionPrivate())
{
	d->contact = contact;
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

const Q_UINT32 FileTransferSession::applicationId() const
{
	return 2;
}

void FileTransferSession::handleInvite(const Q_UINT32 appId, const QByteArray& context)
{
	if (appId == applicationId())
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

		if (type == 1 || type == 0)
		{
			kdDebug() << "Contact sending file, " << file << " (size " << d->size << ")" << endl;
			Kopete::TransferManager *manager = Kopete::TransferManager::transferManager();
			manager->askIncomingTransfer(d->contact, file, d->size, QString::null, QString::number(id()));
			QObject::connect(manager, SIGNAL(accepted(Kopete::Transfer*, const QString&)), this,
			SLOT(sessionAccepted(Kopete::Transfer*, const QString&)));
			QObject::connect(manager, SIGNAL(refused(const Kopete::FileTransferInfo&)), this,
			SLOT(sessionDeclined(const Kopete::FileTransferInfo&)));
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
			kdDebug() << k_funcinfo << "Session " << id() << " cancelling -- unable to open file "
			<< d->file->name() << endl;

			cancel();
			return;
		}

		kdDebug() << k_funcinfo << "Session " << id() << ", sending DATA size(" << d->file->size() << ")" << endl;
		emit sendFile(d->file);

		d->transfer = Kopete::TransferManager::transferManager()->addTransfer(d->contact, d->file->name(), d->file->size(), d->contact->contactId(), Kopete::FileTransferInfo::Outgoing);
		// Connect the signal/slot
		QObject::connect(d->transfer , SIGNAL(transferCanceled()), this, SLOT(cancel()));
	}
	else
	if (direction() == Session::Incoming)
	{
		kdDebug() << k_funcinfo << "Session " << id() << ", ready to receive data" << endl;
		d->file->open(IO_WriteOnly);

		// TODO Start timer and wait for 30 seconds.
		// If we do not received data by that time,
		// cancel the session.
	}
}

void FileTransferSession::onEnd()
{
	kdDebug() << k_funcinfo << endl;
	if (d->file && d->file->isOpen())
	{
		d->file->close();
	}
}

void FileTransferSession::onFaulted()
{
}

void FileTransferSession::onDataReceived(const QByteArray& data, bool lastChunk)
{
	kdDebug() << k_funcinfo << "Session " << id() << ", receiving DATA size(" << data.size() << ")" << endl;

	// Write the received bytes to the file.
	d->file->writeBlock(data, data.size());
	d->file->flush();

	d->offset += data.size();
	d->transfer->slotProcessed(d->offset + data.size());

	if (lastChunk)
	{
		kdDebug() << k_funcinfo << "Session " << id() << ", END OF DATA" << endl;
		d->transfer->slotComplete();

		// End the session.
		end(true);
	}
}

void FileTransferSession::onDataSendProgress(const Q_UINT32 progress)
{
	d->transfer->slotProcessed(progress);
	kdDebug() << k_funcinfo << "Session " << id() << ", sent DATA size(" << progress << ")" << endl;

	if (progress == d->file->size())
	{
		d->transfer->slotComplete();
	}
}

void FileTransferSession::onSend(const Q_INT32 id)
{
	Q_UNUSED(id);
	kdDebug() << k_funcinfo << endl;

	if (direction() == Session::Outgoing)
	{
		// End the session if a BYE request is not received within 3 sec.
		QTimer::singleShot(3000, this, SLOT(onSessionEndTimeout()));
	}
}

void FileTransferSession::onSessionEndTimeout()
{
	end(true);
}

void FileTransferSession::sessionAccepted(Kopete::Transfer *transfer, const QString& file)
{
	Q_UNUSED(file);

	if (transfer != 0l && id() == transfer->info().internalId().toUInt())
	{
		QObject::disconnect(Kopete::TransferManager::transferManager(), 0l, this, 0l);
		QObject::connect(transfer , SIGNAL(transferCanceled()), this,
		SLOT(cancel()));

		d->transfer = transfer;
		setDataStore(new QFile(d->transfer->destinationURL().path()));
		accept();
	}
}

void FileTransferSession::sessionDeclined(const Kopete::FileTransferInfo& info)
{
	if (id() == info.internalId().toUInt())
	{
		QObject::disconnect(Kopete::TransferManager::transferManager(), 0l, this, 0l);
		decline();
	}
}

}

#include "filetransfersession.moc"
