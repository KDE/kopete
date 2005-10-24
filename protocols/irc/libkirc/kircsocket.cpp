/*
    kircsocket.cpp - IRC socket.

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kircsocket.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qtextcodec.h>
#include <qtimer.h>

using namespace KIRC;
using namespace KNetwork;

class KIRC::Socket::Private
{
public:
	Private()
		: socket(0),
		  useSSL(false),
		  state(Idle),
		  defaultCodec(0)
	{ }

	KNetwork::KBufferedSocket *socket;
	bool useSSL;
	KIRC::Socket::ConnectionState state;

	QTextCodec *defaultCodec;

	KIRC::Entity::Ptr owner;
};

Socket::Socket(QObject *parent)
	: QObject(parent),
	  d( new Private )
{
	kdDebug(14120) << k_funcinfo << endl;
}

Socket::~Socket()
{
	kdDebug(14120) << k_funcinfo << endl;
	delete d;
}

Socket::ConnectionState Socket::connectionState() const
{
	return d->state;
}

KNetwork::KStreamSocket *Socket::socket()
{
	return d->socket;
}

QTextCodec *Socket::defaultCodec() const
{
	return d->defaultCodec;
}

void Socket::setDefaultCodec(QTextCodec *codec)
{
	codec = d->defaultCodec;
}

Entity::Ptr Socket::owner()
{
	return d->owner;
}

void Socket::setOwner(const Entity::Ptr &newOwner)
{
	d->owner = newOwner;
}

void Socket::connectToServer(const QString &host, Q_UINT16 port, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);
	d->socket->connect(host, QString::number(port));
}

void Socket::connectToServer(const KResolverEntry &entry, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);
	d->socket->connect(entry);
}

void Socket::close()
{
	if (d->socket)
	{
		d->socket->close();
		d->socket = 0;
	}
}

void Socket::writeMessage(const char *msg)
{
	if (!d->socket || d->socket->state() != KBufferedSocket::Open)
	{
//		emit internalError(i18n("Attempting to send while not connected: %1").arg(rawMsg));
		return;
	}
/*
	QByteArray buffer = msg + QByteArray("\n\r");
	int wrote = d->socket->writeBlock(buffer.data(), buffer.length());
//	kdDebug(14121) << QString::fromLatin1("(%1 bytes) >> %2").arg(wrote).arg(rawMsg) << endl;
*/
}

void Socket::writeMessage(const QByteArray &msg)
{
	#warning implement me
}

void Socket::writeMessage(const QString &msg, QTextCodec *codec)
{
	bool encodeSuccess = false;
	writeMessage(encode(msg, &encodeSuccess, codec));
}

void Socket::writeMessage(const Message &msg)
{
	#warning Check message validity before sending it
	writeMessage(msg.rawLine());
}

void Socket::showInfoDialog()
{
/*
	if( d->useSSL )
	{
		static_cast<KSSLSocket*>( d->socket )->showInfoDialog();
	}
*/
}

void Socket::setConnectionState(ConnectionState newstate)
{
	if (d->state != newstate)
	{
		kdDebug(14121) << k_funcinfo << d->state << "->" << newstate << endl;
		d->state = newstate;
		emit connectionStateChanged(newstate);
	}
}

void Socket::onReadyRead()
{
	// The caller can also be self so lets check the socket still exist

	if (d->socket && d->socket->canReadLine())
	{
		QByteArray rawMsg = d->socket->readLine();
//		kdDebug(14121) << QString::fromLatin1("(%1 bytes) << %2").arg(wrote).arg(rawMsg) << endl;

		Message msg(rawMsg, Message::InGoing);

		if (msg.isValid())
			emit receivedMessage(msg);
//		else
//			emit internalError(i18n("Parse error while parsing: %1").arg(msg.rawLine()));

		QTimer::singleShot( 0, this, SLOT( onReadyRead() ) );
	}

//	if(d->socket->socketStatus() != KExtendedSocket::connected)
//		error();
}

void Socket::socketStateChanged(int newstate)
{
	switch ((KBufferedSocket::SocketState)newstate)
	{
	case KBufferedSocket::Idle:
		setConnectionState(Idle);
		break;
	case KBufferedSocket::HostLookup:
		setConnectionState(HostLookup);
		break;
	case KBufferedSocket::HostFound:
		setConnectionState(HostFound);
		break;
//	case KBufferedSocket::Bound: // Should never be Bound, unless writing a server
//		setConnectionState(Bound)
//		break;
	case KBufferedSocket::Connecting:
		setConnectionState(Connecting);
		break;
	case KBufferedSocket::Open:
//		d->socket->enableRead(true);
//		d->socket->enableWrite(true); // Should not be needed
		setConnectionState(Authentifying);
		break;
	case KBufferedSocket::Closing:
		setConnectionState(Closing);
		break;
	default:
		emit internalError(i18n("Unknown SocketState value:%1").arg(newstate));
		close();
	}
}

void Socket::socketGotError(int errCode)
{
	KBufferedSocket::SocketError err = (KBufferedSocket::SocketError)errCode;

	// Ignore spurious error
	if (err == KBufferedSocket::NoError)
		return;
/*
	QString errStr = KBufferedSocket::errorString(err);
	kdDebug(14120) << k_funcinfo << "Socket error: " << errStr << endl;
	emit internalError(errStr);
*/
	// ignore non-fatal error
	if (!KBufferedSocket::isFatalError(err))
		return;

	close();
}

QByteArray Socket::encode(const QString &str, bool *success, QTextCodec *codec) const
{
	*success = false;

	if (!codec || !codec->canEncode(str))
	{
		if (!d->defaultCodec || !d->defaultCodec->canEncode(str))
		{
			if (!UTF8 || !UTF8->canEncode(str))
			{
//				emit internalError(i18n("Codec Error: .").arg(codec->name()));
				return QByteArray();
			}
			else
			{
//				emit internalError(i18n("Codec Warning: Using codec %1 for:\n%2")
//					.arg(UTF8->name(), str));
				codec = UTF8;
			}
		}
		else
			codec = d->defaultCodec;
	}

	*success = true;
	return codec->fromUnicode(str);
}

bool Socket::setupSocket(bool useSSL)
{
	close();

	d->useSSL = useSSL;

	if( d->useSSL )
	{
#ifdef KIRC_SSL_SUPPORT
		d->socket = new KSSLSocket(this);
		d->socket->setSocketFlags( KExtendedSocket::inetSocket );
	}
	else
#else
		kdWarning(14120) << "You tried to use SSL, but this version of Kopete was"
			" not compiled with IRC SSL support. A normal IRC connection will be attempted." << endl;
	}
#endif
	{
		d->socket = new KBufferedSocket(QString::null, QString::null, this);
//		d->socket->setSocketFlags( KExtendedSocket::inputBufferedSocket | KExtendedSocket::inetSocket );
	}

	connect(d->socket, SIGNAL(closed(int)),
		this, SLOT(slotConnectionClosed()));
	connect(d->socket, SIGNAL(readyRead()),
		this, SLOT(onReadyRead()));
	connect(d->socket, SIGNAL(connectionSuccess()),
		this, SLOT(slotConnected()));
	connect(d->socket, SIGNAL(connectionFailed(int)),
		this, SLOT(error(int)));
}

#include "kircsocket.moc"

