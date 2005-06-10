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

#include "kircenginebase.h"
#include "ksslsocket.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qtextcodec.h>
#include <qtimer.h>

using namespace KIRC;
using namespace KNetwork;

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp EngineBase::m_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

EngineBase::EngineBase(QObject *parent)
	: QObject(parent),
	  m_socket(0),
	  m_state(Idle),
	  m_useSSL(false)
{
	kdDebug(14120) << k_funcinfo << endl;
}

EngineBase::~EngineBase()
{
	kdDebug(14120) << k_funcinfo << endl;
}

void EngineBase::connectToServer(const QString &host, Q_UINT16 port, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);

	return connectToServer(KResolverEntry(), useSSL);
}

void EngineBase::connectToServer(const KResolverEntry &entry, bool useSSL)
{
//	kdDebug(14120) << k_funcinfo << useSSL << endl;
	setupSocket(useSSL);

	kdDebug(14120) << "Trying to connect to server " << m_Host << ":" << m_Port << endl;
	kdDebug(14120) << "Sock state: " << m_socket->state() << endl;

	return m_socket->connect(m_Host, QString::number(m_Port)))
}

bool EngineBase::setupSocket(bool useSSL)
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

void EngineBase::close()
{
	m_socket->close();
}

void EngineBase::socketStateChanged(int newstate)
{
	switch ((KBufferedSocket::SocketState)newstate)
	{
	case KBufferedSocket::Idle:
		emit stateChanged(Idle);
		break;
	case KBufferedSocket::HostLookup:
	case KBufferedSocket::HostFound:
//	case KBufferedSocket::Bound: // Should never be Bound, unless writing a server
	case KBufferedSocket::Connecting:
		emit stateChanged(Connecting);
		break;
	case KBufferedSocket::Open:
		emit stateChanged(Authentifying);
	case KBufferedSocket::Closing:
		emit stateChanged(Closing);
		break;
	default:
		emit internalError(i18n("Unknown SocketState value:%1").arg(newstate));
		close();
	}
}

void EngineBase::socketGotError(int errCode)
{
	// Ignore spurious error
	if (errCode == KBufferedSocket::NoError)
		return;

	QString errStr = KBufferedSocket::errorString(errCode);
	kdDebug(14120) << k_funcinfo << "Socket error: " << errStr << endl;
	emit internalError(errStr);

	// ignore non-fatal error
	if (!KBufferedSocket::isFatalError(errCode))
		return;

	close();
}

void EngineBase::writeRawMessage(const QByteArray &rawMsg)
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
/*
void EngineBase::writeRawMessage(const QString &rawMsg, QTextCodec *codec)
{
	if (!codec)
		codec = m_defaultCodec;

//	if (!codec->canEncode(rawMsg))
//	{
//		kdDebug(14121) << k_funcinfo << "Encoding problem detected:" << str << endl;
//		return;
//	}

	writeRawMessage(codec->fromUnicode(rawMsg));
}
*/
void EngineBase::slotReadyRead()
{
	// This condition is buggy when the peer server
	// close the socket unexpectedly

//	if (m_socket->state() == KSocketBase::Connected && m_socket->canReadLine())
	if (m_socket->canReadLine())
	{
		bool parseSuccess;
		Message msg = Message::parse(this, &parseSuccess);
		if (parseSuccess)
		{
/*
		QCString raw = engine->socket()->readLine();
		raw.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
		kdDebug(14121) << "<< " << raw << endl;

		return Message(engine, raw);
*/

		if (msg.isValid())
			emit receivedMessage(msg);
		else
			emit internalError(ParsingFailed, msg);

		QTimer::singleShot( 0, this, SLOT( slotReadyRead() ) );
	}

//	if(m_socket->socketStatus() != KExtendedSocket::connected)
//		error();
}

void EngineBase::showInfoDialog()
{
/*	if( m_useSSL )
	{
		static_cast<KSSLSocket*>( m_socket )->showInfoDialog();
	}*/
}

#include "kircenginebase.moc"
