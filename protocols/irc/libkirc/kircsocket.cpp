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

Socket::Socket(QObject *parent)
	: QObject(parent),
	  m_socket(0),
	  m_state(Idle),
	  m_useSSL(false)
{
	kdDebug(14120) << k_funcinfo << endl;
}

Socket::~Socket()
{
	kdDebug(14120) << k_funcinfo << endl;
}

QByteArray Socket::convert(const QString &str, QTextCodec *codec) const
{
//	static const QTextCodec *utf8 = QTextCodec::codecFromMib(4);

//	if (!codec->canEncode(str))
//	{
//		kdDebug(14121) << k_funcinfo << "Encoding problem detected:" << str << endl;
//		return;
//	}

	return codec->fromUnicode(str);
}

QByteArrayList Socket::convert(const QStringList &strlist, QTextCodec *codec) const
{
	QByteArrayList ret;

	for (QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it)
		ret.append(convert(*it, codec));

	return ret;
}

QTextCodec *Socket::defaultCodec() const
{
	return m_defaultCodec;
}

void Socket::setDefaultCodec(QTextCodec *codec)
{
	codec = m_defaultCodec;
}

KIRC::ConnectionState Socket::connectionState() const
{
	return m_state;
}

void Socket::connectToServer(const QString &host, Q_UINT16 port, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);
	m_socket->connect(host, QString::number(port));
}

void Socket::connectToServer(const KResolverEntry &entry, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);
	m_socket->connect(entry);
}

void Socket::close()
{
	m_socket->close();
}

void Socket::writeRawMessage(const QByteArray &rawMsg)
{
/*	if (!m_socket || m_socket->status())
	{
		kdDebug(14121) << k_funcinfo << "Not connected while attempting to write:" << str << endl;
		return;
	}
*/
	QCString buffer = rawMsg + QCString("\n\r"); // QT-4.0 make this a QByteArray
	int wrote = m_socket->writeBlock(buffer.data(), buffer.length());
	kdDebug(14121) << QString::fromLatin1("(%1 bytes) >> %2").arg(wrote).arg(rawMsg) << endl;
}

void Socket::writeRawMessage(const QString &msg, QTextCodec *codec)
{
	writeRawMessage(convert(msg, codec));
}

void Socket::showInfoDialog()
{
/*	if( m_useSSL )
	{
		static_cast<KSSLSocket*>( m_socket )->showInfoDialog();
	}*/
}

void Socket::setConnectionState(KIRC::ConnectionState newstate)
{
	if (m_state != newstate)
	{
		m_state = newstate;
		emit connectionStateChanged(newstate);
	}
}

void Socket::slotReadyRead()
{
	// This condition is buggy when the peer server
	// close the socket unexpectedly

//	if (m_socket->state() == KSocketBase::Connected && m_socket->canReadLine())
	if (m_socket->canReadLine())
	{
		QByteArray rawMsg = m_socket->readLine();
//		kdDebug(14121) << QString::fromLatin1("(%1 bytes) << %2").arg(wrote).arg(rawMsg) << endl;

		Message msg(rawMsg);
		if (msg.isValid())
			emit receivedMessage(msg);
		else
			emit internalError(i18n("Parse error while parsing: %1").arg(msg.rawLine()));

		QTimer::singleShot( 0, this, SLOT( slotReadyRead() ) );
	}

//	if(m_socket->socketStatus() != KExtendedSocket::connected)
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
	case KBufferedSocket::HostFound:
//	case KBufferedSocket::Bound: // Should never be Bound, unless writing a server
	case KBufferedSocket::Connecting:
		setConnectionState(Connecting);
		break;
	case KBufferedSocket::Open:
		setConnectionState(Authentifying);
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

	QString errStr = KBufferedSocket::errorString(err);
	kdDebug(14120) << k_funcinfo << "Socket error: " << errStr << endl;
	emit internalError(errStr);

	// ignore non-fatal error
	if (!KBufferedSocket::isFatalError(err))
		return;

	close();
}

bool Socket::setupSocket(bool useSSL)
{
	if (m_socket)
		delete m_socket;

	if (!m_socket || useSSL != m_useSSL)
	{
		m_useSSL = useSSL;

		if( m_useSSL )
		{
#ifdef KIRC_SSL_SUPPORT
			m_socket = new KSSLSocket(this);
			m_socket->setSocketFlags( KExtendedSocket::inetSocket );
		}
		else
#else
			kdWarning(14120) << "You tried to use SSL, but this version of Kopete was"
				" not compiled with IRC SSL support. A normal IRC connection will be attempted." << endl;
		}
#endif
		{
			m_socket = new KBufferedSocket(QString::null, QString::null, this);
//			m_socket->setSocketFlags( KExtendedSocket::inputBufferedSocket | KExtendedSocket::inetSocket );
		}

		QObject::connect(m_socket, SIGNAL(closed(int)),
				 this, SLOT(slotConnectionClosed()));
		QObject::connect(m_socket, SIGNAL(readyRead()),
				 this, SLOT(slotReadyRead()));
		QObject::connect(m_socket, SIGNAL(connectionSuccess()),
				 this, SLOT(slotConnected()));
		QObject::connect(m_socket, SIGNAL(connectionFailed(int)),
				 this, SLOT(error(int)));
	}
}

#include "kircsocket.moc"

