/*
 * stream.cpp - handles a Jabber XML stream
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

#define QSSL_STATIC

#include"xmpp_stream.h"

#include<qlibrary.h>
#include<qregexp.h>
#include<qptrqueue.h>
#include<qsocket.h>
#include<qtimer.h>
#include<qfileinfo.h>

#include"ndns.h"
#include"xmpp_xmlfilter.h"
#include"qssl.h"
#include"qssl_p.h"

using namespace Jabber;

//----------------------------------------------------------------------------
// StreamProxy
//----------------------------------------------------------------------------
//! \if _hide_doc_
class StreamProxy::StreamProxyPrivate
{
public:
	StreamProxyPrivate() {}

	int type;
	QString host;
	int port;
	bool useAuth;
	QString user, pass;
};
//! \endif

//! \enum Jabber::StreamProxy::None
//! \brief Normal connection

//! \enum Jabber::StreamProxy::HTTPS
//! \brief Make secure SSL connection

//! \enum Jabber::StreamProxy::SOCKS4
//! \brief Make Sock 4 proxy connection

//! \enum Jabber::StreamProxy::SOCKS5
//! \brief Make Sock 5 proxy connection

//! \brief Create StreamProxy object.
//!
//! \param type - What type of connection you will be establishing.
//! \param host - Where to make the connection.
//! \param port - port to connect too.
StreamProxy::StreamProxy(int type, const QString &host, int port)
{
	d = new StreamProxyPrivate;
	d->type = type;
	d->host = host;
	d->port = port;
}

//! \brief Create StreamProxy object.
//!
//! \param StreamProxy
StreamProxy::StreamProxy(const StreamProxy &from)
{
	d = new StreamProxyPrivate;
	*this = from;
}

//! \brief Overloaded member function.
StreamProxy & StreamProxy::operator=(const StreamProxy &from)
{
	*d = *from.d;
	return *this;
}

//! \brief Destroyes StreamProxy object.
StreamProxy::~StreamProxy()
{
	delete d;
}

//! \brief Return the type of connection
//!
//! \return int - None, HTTPS, SOCKS4 or SOCKS5
//! \sa None HTTPS SOCKS4 SOCKS5
int StreamProxy::type() const
{
	return d->type;
}

//! \brief Return host information.
const QString & StreamProxy::host() const
{
	return d->host;
}

//! \brief Returns port information
int StreamProxy::port() const
{
	return d->port;
}

//! \brief Return if Use Auth is set or not
//!
//! By Default this is set to false
bool StreamProxy::useAuth() const
{
	return d->useAuth;
}

//! \brief Return the username to be used for authentication
const QString & StreamProxy::user() const
{
	return d->user;
}

//! \brief Return the password to be used for authentication
const QString & StreamProxy::pass() const
{
	return d->pass;
}

//! \brief Sets the type of connection
//!
//! \param type - Choose from the enum lists (None, HTTPS, SOCK4, SOCK5)
//! \sa None HTTPS SOCKS4 SOCKS5
void StreamProxy::setType(int type)
{
	d->type = type;
}

//! \brief Sets the host to connect.
//!
//! \param host - The host you will be establishing connection.
void StreamProxy::setHost(const QString &host)
{
	d->host = host;
}

//! \brief Sets the port to connect to.
//!
//! \param port - prot to connect to.
void StreamProxy::setPort(int port)
{
	d->port = port;
}

//! \brief Set if you want to use authentication for proxy.
//!
//! The default for this param is false. You must set this to true if you
//! would like this object to use authentication. Make sure you set
//! the username ans password.
//! \param use - true: use authentication\n
//!   - false: don't use authentication
//! \sa setUser() setPass()
void StreamProxy::setUseAuth(bool use)
{
	d->useAuth = use;
}

//! \brief Sets the username this method will use for authentication
void StreamProxy::setUser(const QString &user)
{
	d->user = user;
}

//! \brief Sets the password this method will use for authentication
void StreamProxy::setPass(const QString &pass)
{
	d->pass = pass;
}


//----------------------------------------------------------------------------
// StreamError
//----------------------------------------------------------------------------

//! \brief Creates StreamError object
//!
//! \param _type - type of error
//! \param _string - error message for that type
//! \param _isWarning - if this is a warning or an error
StreamError::StreamError(int _type, const QString &_string, bool _isWarning)
{
	v_type = _type;
	v_string = _string;
	v_isWarning = _isWarning;
}

//! \brief Returns if this is a warning or error
//!
//! \return true: if warning\n
//!   false: if error
bool StreamError::isWarning() const
{
	return v_isWarning;
}

//! \brief Returns the error/warrning type.
int StreamError::type() const
{
	return v_type;
}

//! \brief Returns the error/warning information.
const QString & StreamError::details() const
{
	return v_string;
}

//! \brief creates a warning/error string for you.
//!
//! This function is here for your convenience. This will return a neatly
//! formated error/warrning message.
QString StreamError::toString() const
{
	QString str;
	if(isWarning())
		str += Stream::tr("Warning");
	else
		str += Stream::tr("Error");
	str += ": ";
	enum { DNS, Refused, Timeout, Socket, Disconnected, Handshake, SSL, Proxy, Unknown };
	switch(type()) {
		case DNS:
			str += Stream::tr("DNS"); break;
		case Refused:
			str += Stream::tr("Connection Refused"); break;
		case Timeout:
			str += Stream::tr("Connection Timeout"); break;
		case Socket:
			str += Stream::tr("Socket"); break;
		case Disconnected:
			str += Stream::tr("Disconnected"); break;
		case Handshake:
			str += Stream::tr("Handshake"); break;
		case SSL:
			str += Stream::tr("SSL"); break;
		case Proxy:
			str += Stream::tr("Proxy"); break;
		case Unknown:
		default:
			break;
	}
	if(!details().isEmpty()) {
		str += ": ";
		str += details();
	}

	return str;
}


//----------------------------------------------------------------------------
// Stream
//----------------------------------------------------------------------------
//! \if _hide_doc_
class Stream::StreamPrivate
{
public:
	StreamPrivate() {}

	NDns ndns;
	XmlFilter xml;
	QSSLFilter *ssl;
	QSocket *sock;

	QTimer *noopTimer;
	bool isActive, isConnected, isHandshaken, closing;
	QString host, realhost;
	int port;
	bool use_ssl;
	StreamProxy proxy;

	bool http_inHeader;

	StreamError err;
	int noop_time;

	QString id;

	QPtrQueue<QDomElement> in;

	QString certDir;
	QString sslErrorLog;
};
//! \endif

static QLibrary *lib = 0;
static QSSL *qssl = 0;
static QString *ssl_error = 0;
static QString sslErrorLog;
static bool ssl_tried = false;

//! QSSL plugin
static QSSL * (*_createQSSL)();
static int (*_version)();

//! \brief Creates QSSL object
//!
//! This function will load the QSSL plugin and create the QSSL object for you.
static bool QSSL_load(const QString &_path)
{
	if(qssl)
		return true;
	ssl_tried = true;

#if defined(QSSL_STATIC)
	qssl = new _QSSL;
#else
	if(lib)
		return true;
	ssl_error = new QString("");

	QString name;
#if defined(Q_WS_WIN)
	name = "qssl.dll";
#elif defined(Q_WS_MACX)
	name = "libqssl.dylib";
#else
	name = "libqssl.so";
#endif

	bool check_exist = true;
	QString fullname;
	if(_path.isEmpty()) {
		check_exist = false;
		fullname = name;
	}
	else {
		QString path = _path;
		if(path.at(path.length()-1) != '/')
			path += '/';
		fullname = path + name;
	}

	QFileInfo fi(fullname);
	if(check_exist) {
		if(!fi.exists()) {
			*ssl_error = Stream::tr("File not found: %1").arg(fi.filePath());
			return false;
		}
	}

	lib = new QLibrary(fi.filePath());

	// load the lib
	if(!lib->load()) {
		delete lib;
		lib = 0;
		*ssl_error = Stream::tr("Unable to load: %1").arg(fi.filePath());
		return false;
	}

	// look for the hook
	void *p = lib->resolve("createQSSL");
	if(!p) {
		delete lib;
		lib = 0;
		*ssl_error = Stream::tr("Invalid plugin: %1").arg(fi.filePath());
		return false;
	}
	_createQSSL = (QSSL*(*)())p;

	p = lib->resolve("version");
	if(!p) {
		delete lib;
		lib = 0;
		*ssl_error = Stream::tr("Invalid plugin: %1").arg(fi.filePath());
		return false;
	}
	_version = (int(*)())p;

	if(_version() != 2) {
		delete lib;
		lib = 0;
		*ssl_error = Stream::tr("Wrong plugin version: %1").arg(fi.filePath());
		return false;
	}

	// get the object
	qssl = _createQSSL();
#endif

	return true;
}

#if !defined(QSSL_STATIC)
static void QSSL_unload()
{
	if(lib) {
		delete qssl;
		delete lib;
		delete ssl_error;
		qssl = 0;
		lib = 0;
		ssl_error = 0;
		ssl_tried = false;
	}
}
#endif

//! \class Jabber::Stream stream.h
//! \brief Makes tcp connection to host/port you specify.
//!
//! This class is capable of connecting with normal tcp connection,
//! HTTPS (SSL), SOCK4, and SOCK5.

//! \fn void Jabber::Stream::connected()
//! \brief Signals when the connction has been made.

//! \fn void Jabber::Stream::handshaken()
//! \brief Signals when the handshakeing process has finished.

//! \fn void Jabber::Stream::error(const StreamError &)
//! \brief Signals when there was an error in the stream.

//! \fn void Jabber::Stream::receivePacket(QDomElement &)
//! \brief Signals when you recive something through the stream.

bool Stream::loadSSL(const QStringList &dirs)
{

#if !defined(QSSL_STATIC)
	if(qssl)
		QSSL_unload();
#endif

	sslErrorLog = "";
	for(QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
		if(QSSL_load(*it))
			return true;
		else
			sslErrorLog += QString("-> ") + (*::ssl_error) + '\n';
	}

	return false;
}

void Stream::unloadSSL()
{
#if !defined(QSSL_STATIC)
	QSSL_unload();
#endif
}

//! \brief Check if SSL is supported
//!
//! Checks to see if SSL is supported.  If you are not under KDE, you will need to call loadSSL() first.
//! \return true: if SSL is supported.\n
//!  false: if SSL is not supported.
bool Stream::isSSLSupported()
{
	if(qssl)
		return true;
	else
		return false;
}

//! \brief Returns the reason for QSSL not loading.
//!
//! This will tell you what went wrong while the SSL plugin was being loaded.
QString Stream::SSLUnsupportedReason()
{
	return sslErrorLog;
}

//! \brief Create Stream object.
//!
//! \param QObject - pass on to QObject
Stream::Stream(QObject *par)
:QObject(par)
{
	d = new StreamPrivate;

	d->closing = false;
	d->sock = 0;
	d->isActive = d->isConnected = d->isHandshaken = false;

	d->noop_time = 0;
	d->noopTimer = new QTimer;
	connect(d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));

	d->in.setAutoDelete(true);

	connect(&d->ndns, SIGNAL(resultsReady()), SLOT(ndns_done()));

	connect(&d->xml, SIGNAL(packetReady(const QDomElement &)), SLOT(xml_packetReady(const QDomElement &)));
	connect(&d->xml, SIGNAL(handshake(bool, const QString &)), SLOT(xml_handshake(bool, const QString &)));

	d->use_ssl = false;
	if(isSSLSupported()) {
		d->ssl = qssl->createFilter();
		connect(d->ssl, SIGNAL(outgoingSSLDataReady()), SLOT(ssl_outgoingReady()));
		connect(d->ssl, SIGNAL(readyRead()), SLOT(ssl_readyRead()));
		connect(d->ssl, SIGNAL(handshaken(bool)), SLOT(ssl_handshaken(bool)));
	}
	else
		d->ssl = 0;
}

//! \brief Destroys Stream object
Stream::~Stream()
{
	close();

	delete d->noopTimer;

	if(d->ssl)
		delete d->ssl;
	delete d;
}

//! \brief Tells you if the connection is busy.
//!
//! \return true: is busy\n
//!   false: nothings going on
bool Stream::isActive() const
{
	return d->isActive;
}

//! \brief Tells you if the connection is establised.
bool Stream::isConnected() const
{
	return d->isConnected;
}

//! \brief Tells you if the handshakeing process has been completed.
bool Stream::isHandshaken() const
{
	return d->isHandshaken;
}

//! \brief Closes socket connection.
//!
//! This function will disconnect and clean this object out.
void Stream::close()
{
	if(!d->isActive)
		return;

	if(d->closing)
		return;

	//printf("stream: closing...\n");

	if(d->ndns.isBusy()) {
		d->ndns.stop();
		QTimer::singleShot(0, this, SLOT(afterClose()));
	}
	else if(d->sock) {
		if(d->sock->state() == QSocket::Connected) {
			if(d->isHandshaken)
				sendString("</stream:stream>\n");

			d->closing = true;

			int bytesLeft = d->sock->bytesToWrite();
			//printf("stream: bytesToWrite: %d\n", bytesLeft);

			d->sock->close();

			if(bytesLeft == 0)
				QTimer::singleShot(0, this, SLOT(afterClose()));
			else {
				//printf("stream: socket shutting down...\n");
			}
		}
		else {
			cleanup();
		}
	}
	else
		QTimer::singleShot(0, this, SLOT(afterClose()));
}

void Stream::afterClose()
{
	//printf("stream: socket closed.\n");
	cleanup();
	closeFinished();
}

void Stream::cleanup()
{
	//printf("stream: cleanup()\n");
	if(d->sock) {
		delete d->sock;
		d->sock = 0;

		if(d->isConnected)
			d->xml.reset();

		if(d->use_ssl)
			d->ssl->reset();
	}

	d->noopTimer->stop();
	d->isActive = d->isConnected = d->isHandshaken = false;
	d->closing = false;
}

//! \brief Sets the host to connect to.
//!
//! \param _host - Sets the host you want to connect to.
//! \param _port - Sets the port to connect.
//! \param _virtual - If the server name differ from _host, specify here.
void Stream::connectToHost(const QString &_host, int _port, const QString &_virtualHost)
{
	if(d->isActive)
		return;

	d->host = _host;

	if(_port == -1) {
		if(d->use_ssl)
			d->port = 5223;
		else
			d->port = 5222;
	}
	else
		d->port = _port;

	if(_virtualHost.isEmpty())
		d->realhost = _host;
	else
		d->realhost = _virtualHost;

	d->isActive = true;

	QString str;
	if(d->proxy.type() == StreamProxy::HTTPS)
		str = d->proxy.host();
	else
		str = d->host;

	d->ndns.resolve(str.local8Bit());
}

//! \brief sets proxy
//!
//! Setup the proxy information.
//! \sa StreamProxy
void Stream::setProxy(const StreamProxy &p)
{
	d->proxy = p;
}

void Stream::sock_https_connected()
{
	QString b;
	QCString buf;

	d->http_inHeader = true;

	b = QString( "CONNECT %1:%2 HTTP/1.0\r\n" ).arg( d->host ).arg( d->port );
	buf = b.local8Bit();
	d->sock->writeBlock( buf, buf.length() );

	if( d->proxy.useAuth() )
	{
		b = QString( "%1:%2" ).arg( d->proxy.user() ).arg( d->proxy.pass() );
		buf = QCString( "Proxy-Authorization: Basic " ) + base64Encode( b.local8Bit() ) + "\r\n";
		d->sock->writeBlock( buf, buf.length() );
	}

	buf = QCString( "Proxy-Connection: Keep-Alive\r\nPragma: no-cache\r\n\r\n" );
	d->sock->writeBlock( buf, buf.length() );
}

void Stream::sock_https_readyRead()
{
	QString buf;
	bool first_line = true;

	if( d->http_inHeader )
	{
		while( d->sock->canReadLine() )
		{
			buf = d->sock->readLine();
			if( first_line )
			{
				first_line = false;
				if( buf.find( "200" ) < 0 ) // !OK
				{
					if( buf.find( "407" ) >= 0 ) // Authentication failed
						d->err = StreamError(StreamError::Proxy, tr("Authentication failed"));
					else if( buf.find( "404" ) >= 0 ) // Host not found
						d->err = StreamError(StreamError::Proxy, tr("Host not found"));
					else if( buf.find( "403" ) >= 0 ) // Access denied
						d->err = StreamError(StreamError::Proxy, tr("Access denied"));
					else if( buf.find( "503" ) >= 0 ) // Connection refused
						d->err = StreamError(StreamError::Proxy, tr("Connection refused"));
					else // invalid reply
						d->err = StreamError(StreamError::Proxy, tr("Invalid reply"));

					// process error later
					QTimer::singleShot(0, this, SLOT(delayedProcessError()));
					return;
				}
			}
			if( buf.compare( "\r\n" ) == 0 )
			{
				break;
			}
		}
		d->http_inHeader = false;
		sock_connected();
	}
	else
	{
		sock_readyRead();
	}
}

QCString Stream::base64Encode( const QCString &s )
{
	int i;
	int len = s.length();
	char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	int a, b, c;

	QCString p = QCString( "" );
	for( i = 0; i < len; i += 3 ) {
		a = (s[i] & 3) << 4;
		if(i + 1 < len) {
			a += s[i + 1] >> 4;
			b = (s[i + 1] & 0xF) << 2;
			if(i + 2 < len) {
				b += s[i + 2] >> 6;
				c = s[i + 2] & 0x3F;
			}
			else
				c = 64;
		}
		else
			b = c = 64;

		p += tbl[s[i] >> 2];
		p += tbl[a];
		p += tbl[b];
		p += tbl[c];
	}
	return p;
}

void Stream::ndns_done()
{
	if(d->ndns.result() == 0) {
		d->err = StreamError(StreamError::DNS);

		// process error later
		QTimer::singleShot(0, this, SLOT(delayedProcessError()));
	}
	else {
		d->sock = new QSocket;
		connect(d->sock, SIGNAL(connectionClosed()), SLOT(sock_disconnected()));
		connect(d->sock, SIGNAL(error(int)),  SLOT(sock_error(int)));
		connect(d->sock, SIGNAL(bytesWritten(int)), SLOT(sock_bytesWritten(int)));
		connect(d->sock, SIGNAL(delayedCloseFinished()), SLOT(sock_delayedCloseFinished()));

		if(d->proxy.type() == StreamProxy::HTTPS) {
			connect(d->sock, SIGNAL(connected()), SLOT(sock_https_connected()));
			connect(d->sock, SIGNAL(readyRead()), SLOT(sock_https_readyRead()));
			d->sock->connectToHost(d->ndns.resultString(), d->proxy.port());
		}
		else {
			connect(d->sock, SIGNAL(connected()), SLOT(sock_connected()));
			connect(d->sock, SIGNAL(readyRead()), SLOT(sock_readyRead()));
			d->sock->connectToHost(d->ndns.resultString(), d->port);
		}
	}
}

//! \brief Return stream id.
QString Stream::id() const
{
	return d->id;
}

//! \brief Set ping interval.
//!
//! This section will set the interval to send new lines to keep the conenction
//! alive.
void Stream::setNoopTime(int mills)
{
	d->noop_time = mills;

	if(!d->isHandshaken)
		return;

	if(d->noop_time == 0) {
		d->noopTimer->stop();
		return;
	}

	d->noopTimer->start(d->noop_time);
}

//! \brief Check if SSL is enable on this connection.
bool Stream::isSSLEnabled() const
{
	return d->use_ssl;
}

//! \brief Make the stream use SSL connection.
void Stream::setSSLEnabled(bool use)
{
	d->use_ssl = use;

	if(use && d->ssl)
		d->use_ssl = true;
	else
		d->use_ssl = false;
}

void Stream::setSSLTrustedCertStoreDir(const QString &dir)
{
	d->certDir = dir;
}

//! \brief Send xml to host.
void Stream::sendPacket(const QDomElement &e)
{
	sendString(elemToString(e));
}

//! \brief send String to host.
void Stream::sendString(const QCString &str)
{
	if(d->isConnected) {
		if(d->use_ssl) {
			QByteArray a = str;
			a.detach();
			a.resize(a.size()-1); // kick off the trailing zero
			d->ssl->send(a);
		}
		else {
			d->sock->writeBlock(str, str.length());
		}
	}
}

#include<qfile.h>
#include<qdir.h>
void Stream::sock_connected()
{
	if(d->use_ssl) {
		QStringList certDirs;
		certDirs += d->certDir;

		QPtrList<QSSLCert> list;
		list.setAutoDelete(true);

		// read *.xml in the certs folder
		if(!d->certDir.isEmpty()) {
			QDir dir(d->certDir);
			dir.setNameFilter("*.xml");
			QStringList entries = dir.entryList();
			for(QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
				QFile f(dir.filePath(*it));
				if(!f.open(IO_ReadOnly))
					continue;
				QDomDocument doc;
				doc.setContent(&f);
				f.close();

				QDomElement base = doc.documentElement();
				if(base.tagName() == "store") {
					QDomNodeList certs = base.elementsByTagName("certificate");
					for(int n = 0; n < (int)certs.count(); ++n) {
						QDomElement ce = certs.item(n).toElement();
						// get data
						QDomElement data = ce.elementsByTagName("data").item(0).toElement();
						if(!data.isNull()) {
							QSSLCert *cert = qssl->createCert();
							if(cert->fromString(data.text()))
								list.append(cert);
						}
					}
				}
			}
		}

		if(!d->ssl->begin(d->realhost, list)) {
			d->err = StreamError(StreamError::SSL, tr("Unable to initialize SSL"));
			QTimer::singleShot(0, this, SLOT(delayedProcessError()));
		}
	}
	else {
		d->isConnected = true;
		d->xml.begin();
		connected();
		startHandshake();
	}
}

void Stream::sock_disconnected()
{
	if(d->closing)
		return;

	d->err = StreamError(StreamError::Disconnected);

	// process error later
	QTimer::singleShot(0, this, SLOT(delayedProcessError()));
}

void Stream::sock_readyRead()
{
	int size;
	QByteArray buf;

	size = d->sock->bytesAvailable();
	buf.resize(size);
	d->sock->readBlock(buf.data(), size);

	if(d->use_ssl)
		d->ssl->putIncomingSSLData(buf);
	else
		processIncomingData(buf);
}

void Stream::sock_error(int x)
{
	if(x == QSocket::ErrConnectionRefused)
		d->err = StreamError(StreamError::Refused);
	else if(x == QSocket::ErrHostNotFound)
		d->err = StreamError(StreamError::DNS);
	else if(x == QSocket::ErrSocketRead)
		d->err = StreamError(StreamError::Socket);
	else
		d->err = StreamError(StreamError::Timeout);

	// process error later
	QTimer::singleShot(0, this, SLOT(delayedProcessError()));
}

void Stream::sock_bytesWritten(int)
{
	//printf("stream: wrote %d. left=%d\n", (int)x, (int)d->sock->bytesToWrite());
}

void Stream::sock_delayedCloseFinished()
{
	//printf("stream: sock::delayedCloseFinished()\n");
	afterClose();
}

void Stream::ssl_handshaken(bool ok)
{
	if(ok) {
		const QSSLCert &cert = d->ssl->peerCertificate();
		sslCertificateReady(cert);
	}
	else {
		d->err = StreamError(StreamError::SSL, tr("SSL handshake failed"));
		QTimer::singleShot(0, this, SLOT(delayedProcessError()));
	}
}

void Stream::continueAfterCert()
{
	d->isConnected = true;
	d->xml.begin();
	connected();
	startHandshake();
}

void Stream::ssl_outgoingReady()
{
	QByteArray a = d->ssl->getOutgoingSSLData();
	d->sock->writeBlock(a.data(), a.size());
}

void Stream::ssl_readyRead()
{
	processIncomingData(d->ssl->recv());
}

void Stream::startHandshake()
{
	// Start the handshake
	QCString str;
	str.sprintf("<stream:stream to=\"%s\" xmlns=\"jabber:client\" xmlns:stream=\"http://etherx.jabber.org/streams\">\n", encodeXML(d->realhost).data());
	sendString(str);
}

void Stream::processIncomingData(const QByteArray &buf)
{
	d->xml.putIncomingXmlData(buf);
}

void Stream::delayedProcessError()
{
	if(!d->err.isWarning())
		cleanup();
	error(d->err);
}

void Stream::delayedProcessReceived()
{
	// process chunks
	while(!d->in.isEmpty()) {
		QDomElement *e = d->in.dequeue();
		receivePacket(*e);
		delete e;
	}
}

void Stream::delayedProcessHandShake()
{
	d->isHandshaken = true;

	setNoopTime(d->noop_time);

	handshaken();
}

void Stream::doNoop()
{
	if(d->isHandshaken)
		sendString("\n");
}

void Stream::xml_packetReady(const QDomElement &e)
{
	d->in.enqueue(new QDomElement(e));
	QTimer::singleShot(0, this, SLOT(delayedProcessReceived()));
}

void Stream::xml_handshake(bool ok, const QString &id)
{
	if(!ok) {
		d->err = StreamError(StreamError::Handshake);

		// process error later
		QTimer::singleShot(0, this, SLOT(delayedProcessError()));
		return;
	}

	d->id = id;

	// process handshake later
	QTimer::singleShot(0, this, SLOT(delayedProcessHandShake()));
}

//! \brief Set the XML string.
QCString Stream::encodeXML(const QString &_str)
{
	QString str = _str;

	str.replace(QRegExp("&"), "&amp;");
	str.replace(QRegExp("<"), "&lt;");
	str.replace(QRegExp(">"), "&gt;");
	str.replace(QRegExp("\""), "&quot;");
	str.replace(QRegExp("'"), "&apos;");

	return str.utf8();
}

//! \brief Convert QDomElemnt to QCString.
QCString Stream::elemToString(const QDomElement &e)
{
	QString out;
	QTextStream ts(&out, IO_WriteOnly);
	e.save(ts, 0);
	return out.utf8();
}

#include "xmpp_stream.moc"
