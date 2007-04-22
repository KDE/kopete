/*
    msnobjectsession.cpp - Peer to Peer Msn Object Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "msnobjectsession.h"
#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

namespace PeerToPeer
{

class MsnObjectSession::MsnObjectSessionPrivate
{
	public:
		MsnObjectSessionPrivate() : file(0l) {}

		QFile *file;
		KTempFile *temporaryFile;
		QString location;
		QString object;
};

MsnObjectSession::MsnObjectSession(const Q_UINT32 id, Direction direction, QObject *parent) : Session(id, direction, parent), d(new MsnObjectSessionPrivate())
{
}

MsnObjectSession::MsnObjectSession(const QString& s, const Q_UINT32 id, Direction direction, QObject *parent) : Session(id, direction, parent), d(new MsnObjectSessionPrivate())
{
	d->object = s;

	QDomDocument document;
	const QString xml = QString("<Data>%1</Data>").arg(d->object);
	// Load the xml object.
	document.setContent(xml);
	QDomElement object = document.documentElement().namedItem("msnobj").toElement();
	QString creator = object.attribute("Creator");
	const Q_INT32 type = object.attribute("Type").toInt();

	switch(type)
	{
		case(2):
		{
			// We are requesting the peer's custom emotion.
			d->temporaryFile = new KTempFile(locateLocal("tmp", "msnemotion--"), ".png");
			d->location = d->temporaryFile->name();
			kdDebug() << d->location << endl;
			break;
		}

		case(3):
		{
			// We are requesting the peer's display picture.
			d->temporaryFile = new KTempFile(locateLocal("tmp", "msnpicture--"), ".png");
			d->location = d->temporaryFile->name();
			kdDebug() << d->location << endl;
			break;
		}

		case(11):
		{
			// We are requesting the peer's voice clip.
			d->temporaryFile = new KTempFile(locateLocal("tmp", "msnvoiceclip--"), ".wav");
			d->location = d->temporaryFile->name();
			kdDebug() << d->location << endl;
			break;
		}
	}
}

MsnObjectSession::~MsnObjectSession()
{
	if (d->file != 0l)
	{
		d->file->close();
		delete d->file;
	}

	delete d;
}

void MsnObjectSession::handleInvite(const Q_UINT32 appId, const QByteArray& context)
{
	if (appId == 1 || appId == 12)
	{
		QDomDocument document;
		d->object = QString::fromUtf8(context.data(), context.size());
		const QString xml = QString("<Data>%1</Data>").arg(d->object);
		// Load the xml object.
		document.setContent(xml);
		QDomElement object = document.documentElement().namedItem("msnobj").toElement();
		QString creator = object.attribute("Creator");
		const Q_INT32 type = object.attribute("Type").toInt();

		switch(type)
		{
			case(2):
			{
				// Peer is requesting one of our custom emotions
				break;
			}

			case(3):
			{
				// Peer is requesting our display picture.
				d->location = locate("appdata", QString("msnpicture-%1.png").arg(creator.replace(QRegExp("[./~]"),"-")));
				kdDebug() << d->location << endl;

				accept();
				break;
			}

			default:
			{
				decline();
				break;
			}
		}
	}
	else
	{
		// Otherwise, decline the session.
		decline();
	}
}

void MsnObjectSession::onStart()
{
	d->file = new QFile(d->location);

	if (direction() == Session::Outgoing)
	{
		if (!d->file->open(IO_ReadOnly))
		{
			kdDebug() << k_funcinfo << "Session=" << id() << " cancelling -- unable to open file "
			<< d->location << endl;

			cancel();
			return;
		}

		kdDebug() << k_funcinfo << "Session=" << id() << ", Sending DATA PREAMBLE message" << endl;
		// NOTE The data preparation preamble consists of
		// an empty byte array of size 4.
		QByteArray preamble(4);
		preamble.fill('\0');

		emit sendMessage(preamble);

		QByteArray data = d->file->readAll();
		d->file->close();

		kdDebug() << k_funcinfo << "Session=" << id() << ", Sending DATA size(" << data.size() << ")" << endl;
		emit sendData(data);
	}
	else
	if (direction() == Session::Incoming)
	{
		kdDebug() << k_funcinfo << "Session=" << id() << ", Ready to receive data" << endl;
	}
}

void MsnObjectSession::onEnd()
{
	kdDebug() << k_funcinfo << endl;
	if (direction() == Session::Incoming)
	{
		emit ended();
	}
}

void MsnObjectSession::onFaulted()
{
}

void MsnObjectSession::onDataReceived(const QByteArray& data, bool lastChunk)
{
	kdDebug() << k_funcinfo << "Session " << id() << ", receiving DATA size(" << data.size() << ")" << endl;
	d->temporaryFile->file()->writeBlock(data, data.size());

	if (lastChunk)
	{
		kdDebug() << k_funcinfo << "Session " << id() << ", END OF DATA" << endl;

		emit objectReceived(d->object, d->temporaryFile);
		end();
	}
}

void MsnObjectSession::onReceive(const QByteArray& bytes, const Q_INT32 /*id*/, const Q_INT32 correlationId)
{
	Q_UNUSED(correlationId);

	QByteArray buffer(4);
	buffer.fill('\0');
	if (bytes.size() == 4 && bytes == buffer)
	{
		kdDebug() << k_funcinfo << "Session " << id() << ", received DATA PREPARATION message" << endl;
	}
}

void MsnObjectSession::onSend(const Q_INT32 id)
{
	Q_UNUSED(id);

	kdDebug() << k_funcinfo << endl;
}

}

#include "msnobjectsession.moc"
