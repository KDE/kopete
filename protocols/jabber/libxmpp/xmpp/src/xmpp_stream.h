/*
 * stream.h - handles a Jabber XML stream
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef JABBER_STREAM_H
#define JABBER_STREAM_H

/****************************************************************************
  Stream

  This class handles a Jabber XML stream.  It collects XML data into
  QDomElements and signals the chunks back to the caller.  There is no
  actual Jabber logic in this class.  It simply takes care of XML and
  a socket connection.

  HOWTO:

    Stream stream;
    stream.connectToHost("jabber.org");

  The class communicates back to you via signals:

    connected()                       - connection success
    error(int)                        - connection/stream error
    packetReady(const QDomElement &)  - new XML chunk ready

****************************************************************************/

#include<qobject.h>
#include<qstring.h>

class QCString;
class QDomElement;
class QSSLCert;

namespace Jabber
{
	class StreamProxy
	{
	public:
		enum { None, HTTPS, SOCKS4, SOCKS5 };
		StreamProxy(int type=None, const QString &host="", int port=0);
		StreamProxy(const StreamProxy &);
		StreamProxy & operator=(const StreamProxy &);
		~StreamProxy();

		int type() const;
		const QString &host() const;
		int port() const;
		bool useAuth() const;
		const QString &user() const;
		const QString &pass() const;

		void setType(int);
		void setHost(const QString &);
		void setPort(int);
		void setUseAuth(bool);
		void setUser(const QString &);
		void setPass(const QString &);

	private:
		class StreamProxyPrivate;
		StreamProxyPrivate *d;
	};

	class StreamError
	{
	public:
		enum { DNS, Refused, Timeout, Socket, Disconnected, Handshake, SSL, Proxy, Unknown };
		StreamError(int type=Unknown, const QString &details="", bool isWarning=false);

		bool isWarning() const;
		int type() const;
		const QString & details() const;
		QString toString() const;

	private:
		bool v_isWarning;
		int v_type;
		QString v_string;
	};

	class Stream : public QObject
	{
		Q_OBJECT
	public:
		Stream(QObject *parent=0);
		~Stream();

		bool isActive() const;
		bool isConnected() const;
		bool isHandshaken() const;
		void connectToHost(const QString &host, int port=-1, const QString &virtualHost="");
		void setProxy(const StreamProxy &);
		void close();

		QString id() const;

		void setNoopTime(int);

		bool isSSLEnabled() const;
		void setSSLEnabled(bool);
		void setSSLTrustedCertStoreDir(const QString &);

		static bool loadSSL(const QStringList &dirs);
		static void unloadSSL();

		static bool isSSLSupported();
		static QString SSLUnsupportedReason();
		static QCString encodeXML(const QString &);
		static QCString elemToString(const QDomElement &);

	signals:
		void connected();
		void handshaken();
		void error(const StreamError &);
		void sslCertificateReady(const QSSLCert &);
		void closeFinished();
		void receivePacket(const QDomElement &);

	public slots:
		void continueAfterCert();
		void sendPacket(const QDomElement &);
		void sendString(const QCString &);

	private slots:
		// NDns
		void ndns_done();

		// QSocket
		void sock_connected();
		void sock_disconnected();
		void sock_readyRead();
		void sock_error(int);
		void sock_bytesWritten(int);
		void sock_delayedCloseFinished();

		// Proxy
		void sock_https_connected();
		void sock_https_readyRead();

		// SSL
		void ssl_outgoingReady();
		void ssl_readyRead();
		void ssl_handshaken(bool);

		// Xml
		void xml_packetReady(const QDomElement &);
		void xml_handshake(bool, const QString &);

		// delayed functions so we can "get out" of a QSocket slot before continuing
		void delayedProcessError();
		void delayedProcessReceived();
		void delayedProcessHandShake();

		// prevent NAT/connection timeouts
		void doNoop();

		void afterClose();

	private:
		void cleanup();
		void startHandshake();
		void processIncomingData(const QByteArray &);
		QCString base64Encode(const QCString &);

		class StreamPrivate;
		StreamPrivate *d;
	};
}

#endif
