// taken from jabber
// encode_method taken from gaim plugin
// Buffer taken from MSN

//#include<qtextstream.h>
//#include<qguardedptr.h>
// #include<qca.h>
// #include<stdlib.h>
// #include"bytestream.h"
// #include"base64.h"
// #include"hash.h"
// #include"simplesasl.h"
// #include"securestream.h"
// #include"protocol.h"

#include <qapplication.h>  // for qdebug
#include <qguardedptr.h> 
#include <qobject.h>
#include <qptrqueue.h>
#include <qtimer.h>

#include "bytestream.h"
#include "connector.h"
#include "coreprotocol.h"
#include "request.h"
#include "securestream.h"
#include "tlshandler.h"

//#include "iostream.h"

#include "gwclientstream.h"

#define LIBGW_DEBUG 1

void cs_dump( const QByteArray &bytes );

enum {
	Idle,
	Connecting,
	WaitVersion,
	WaitTLS,
	NeedParams,
	Active,
	Closing
};

enum {
	Client,
	Server
};

class ClientStream::Private
{
public:
	Private()
	{
		conn = 0;
		bs = 0;
		ss = 0;
		tlsHandler = 0;
		tls = 0;
//		sasl = 0;
		in.setAutoDelete(true);

		allowPlain = false;
		mutualAuth = false;
		haveLocalAddr = false;
/*		minimumSSF = 0;
		maximumSSF = 0;*/
		doBinding = true;

		in_rrsig = false;

		reset();
	}
	void reset()
	{
		state = Idle;
		notify = 0;
		newTransfers = false;
// 		sasl_ssf = 0;
		tls_warned = false;
		using_tls = false;
	}
	
	NovellDN id;
	QString server;
	bool oldOnly;
	bool allowPlain, mutualAuth;
	bool haveLocalAddr;
	QHostAddress localAddr;
	Q_UINT16 localPort;
// 	int minimumSSF, maximumSSF;
// 	QString sasl_mech;
	bool doBinding;

	bool in_rrsig;

	Connector *conn;
	ByteStream *bs;
	TLSHandler *tlsHandler;
	QCA::TLS *tls;
// 	QCA::SASL *sasl;
	SecureStream *ss;
	CoreProtocol client;
	Buffer inBuffer;
	//CoreProtocol srv;

	QString defRealm;

	int mode;
	int state;
	int notify;
	bool newTransfers;
// 	int sasl_ssf;
	bool tls_warned, using_tls;
	bool doAuth;

// 	QStringList sasl_mechlist;

	int errCond;
	QString errText;

	QPtrQueue<Transfer> in;

	QTimer noopTimer; // probably not needed
	int noop_time;
};

ClientStream::ClientStream(Connector *conn, TLSHandler *tlsHandler, QObject *parent)
:Stream(parent)
{
	qDebug("CLIENTSTREAM::ClientStream");

	d = new Private;
	d->mode = Client;
	d->conn = conn;
	connect( d->conn, SIGNAL(connected()), SLOT(cr_connected()) );
	connect( d->conn, SIGNAL(error()), SLOT(cr_error()) );
	connect( &d->client, SIGNAL( outgoingData( const QByteArray& ) ), SLOT ( cp_outgoingData( const QByteArray & ) ) );
	connect( &d->client, SIGNAL( incomingData() ), SLOT ( cp_incomingData() ) );

	d->noop_time = 0;
	connect(&d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));

	d->tlsHandler = tlsHandler;		// all the extra stuff happening in the larger ctor happens at connect time :)
}

ClientStream::~ClientStream()
{
	reset();
	delete d;
}

void ClientStream::reset(bool all)
{
	d->reset();
	d->noopTimer.stop();

	// delete securestream
	delete d->ss;
	d->ss = 0;

	// reset sasl
// 	delete d->sasl;
// 	d->sasl = 0;

	// client
	if(d->mode == Client) {
		// reset tls
		if(d->tlsHandler)
			d->tlsHandler->reset();

		// reset connector
		if(d->bs) {
			d->bs->close();
			d->bs = 0;
		}
		d->conn->done();

		// reset state machine
		d->client.reset();
	}
	if(all)
		d->in.clear();
}

// Jid ClientStream::jid() const
// {
// 	return d->jid;
// }

void ClientStream::connectToServer(const NovellDN &id, bool auth)
{
	reset(true);
	d->state = Connecting;
	d->id = id;
	d->doAuth = auth;
	d->server = d->id.server;

	d->conn->connectToServer( d->server );
}

void ClientStream::continueAfterWarning()
{
	if(d->state == WaitVersion) {
		// if we don't have TLS yet, then we're never going to get it
		if(!d->tls_warned && !d->using_tls) {
			d->tls_warned = true;
			d->state = WaitTLS;
			emit warning(WarnNoTLS);
			return;
		}
		d->state = Connecting;
		processNext();
	}
	else if(d->state == WaitTLS) {
		d->state = Connecting;
		processNext();
	}
}

void ClientStream::accept()
{
/*	d->srv.host = d->server;
	processNext();*/
}

bool ClientStream::isActive() const
{
	return (d->state != Idle);
}

bool ClientStream::isAuthenticated() const
{
	return (d->state == Active);
}

// void ClientStream::setPassword(const QString &s)
// {
// 	if(d->client.old) {
// 		d->client.setPassword(s);
// 	}
// 	else {
// 		if(d->sasl)
// 			d->sasl->setPassword(s);
// 	}
// }

// void ClientStream::setRealm(const QString &s)
// {
// 	if(d->sasl)
// 		d->sasl->setRealm(s);
// }

void ClientStream::continueAfterParams()
{
/*	if(d->state == NeedParams) {
		d->state = Connecting;
		if(d->client.old) {
			processNext();
		}
		else {
			if(d->sasl)
				d->sasl->continueAfterParams();
		}
	}*/
}

void ClientStream::setNoopTime(int mills)
{
	d->noop_time = mills;

	if(d->state != Active)
		return;

	if(d->noop_time == 0) {
		d->noopTimer.stop();
		return;
	}
	d->noopTimer.start(d->noop_time);
}

void ClientStream::setLocalAddr(const QHostAddress &addr, Q_UINT16 port)
{
	d->haveLocalAddr = true;
	d->localAddr = addr;
	d->localPort = port;
}

int ClientStream::errorCondition() const
{
	return d->errCond;
}

QString ClientStream::errorText() const
{
	return d->errText;
}

// QDomElement ClientStream::errorAppSpec() const
// {
// 	return d->errAppSpec;cr_error
// }

// bool ClientStream::old() const
// {
// 	return d->client.old;
// }

void ClientStream::close()
{
	if(d->state == Active) {
		d->state = Closing;
//		d->client.shutdown();
		processNext();
	}
	else if(d->state != Idle && d->state != Closing) {
		reset();
	}
}

void ClientStream::setAllowPlain(bool b)
{
	d->allowPlain = b;
}

void ClientStream::setRequireMutualAuth(bool b)
{
	d->mutualAuth = b;
}

// void ClientStream::setSSFRange(int low, int high)
// {
// 	d->minimumSSF = low;
// 	d->maximumSSF = high;
// }

// void ClientStream::setOldOnly(bool b)
// {
// 	d->oldOnly = b;
// }

bool ClientStream::transfersAvailable() const
{
	return ( !d->in.isEmpty() );
}

Transfer * ClientStream::read()
{
	if(d->in.isEmpty())
		return 0; //first from queue...
	else 
		return d->in.dequeue();
}

void ClientStream::write( Request *request )
{
	qDebug( "ClientStream::write()" );

	// pass to CoreProtocol for transformation into wire format
	d->client.outgoingTransfer( request );
}
	
void cs_dump( const QByteArray &bytes )
{
#ifdef GW_CLIENTSTREAM_DEBUG
	qDebug( "contains: %i bytes ", bytes.count() );
	uint count = 0;
	while ( count < bytes.count() )
	{
		int dword = 0;
		for ( int i = 0; i < 8; ++i )
		{
			if ( count + i < bytes.count() )
				printf( "%02x ", bytes[ count + i ] );
			else
				printf( "   " );
			if ( i == 3 )
				printf( " " );
		}
		printf(" | ");
		dword = 0;
		for ( int i = 0; i < 8; ++i )
		{
			if ( count + i < bytes.count() )
			{
				int j = bytes [ count + i ];
				if ( j >= 0x20 && j <= 0x7e ) 
					printf( "%2c ", j );
				else
					printf( "%2c ", '.' );
			}
			else
				printf( "   " );
			if ( i == 3 )
				printf( " " );
		}
		printf( "\n" );
		count += 8;
	}
	printf( "\n" );
#endif
}

void ClientStream::cp_outgoingData( const QByteArray& outgoingBytes )
{
	// take formatted bytes from CoreProtocol and put them on the wire
#ifdef LIBGW_DEBUG
	qDebug( "ClientStream::cp_outgoingData:" );
	cs_dump( outgoingBytes );
#endif	
	d->ss->write( outgoingBytes );
}

void ClientStream::cp_incomingData()
{
	qDebug( "ClientStream::cp_incomingData:" );
	Transfer * incoming = d->client.incomingTransfer();
	if ( incoming )
	{
		qDebug( " - got a new transfer" );
		d->in.enqueue( incoming );
		d->newTransfers = true;
		emit doReadyRead();
	}
	else
		qDebug( " - client signalled incomingData but none was available, state is: %i", d->client.state() );
}

void ClientStream::cr_connected()
{
	d->bs = d->conn->stream();
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));

	QByteArray spare = d->bs->read();

	d->ss = new SecureStream(d->bs);
	connect(d->ss, SIGNAL(readyRead()), SLOT(ss_readyRead()));
	connect(d->ss, SIGNAL(bytesWritten(int)), SLOT(ss_bytesWritten(int)));
	connect(d->ss, SIGNAL(tlsHandshaken()), SLOT(ss_tlsHandshaken()));
	connect(d->ss, SIGNAL(tlsClosed()), SLOT(ss_tlsClosed()));
	connect(d->ss, SIGNAL(error(int)), SLOT(ss_error(int)));

	//d->client.startDialbackOut("andbit.net", "im.pyxa.org");
	//d->client.startServerOut(d->server);

// 	d->client.startClientOut(d->jid, d->oldOnly, d->conn->useSSL(), d->doAuth);
// 	d->client.setAllowTLS(d->tlsHandler ? true: false);
// 	d->client.setAllowBind(d->doBinding);
// 	d->client.setAllowPlain(d->allowPlain);

	/*d->client.jid = d->jid;
	d->client.server = d->server;
	d->client.allowPlain = d->allowPlain;
	d->client.oldOnly = d->oldOnly;
	d->client.sasl_mech = d->sasl_mech;
	d->client.doTLS = d->tlsHandler ? true: false;
	d->client.doBinding = d->doBinding;*/

	QGuardedPtr<QObject> self = this;
	emit connected();
	if(!self)
		return;

	// immediate SSL?
	if(d->conn->useSSL()) {
		qDebug("CLIENTSTREAM: cr_connected(), starting TLS");
		d->using_tls = true;
		d->ss->startTLSClient(d->tlsHandler, d->server, spare);
	}
	else {
/*		d->client.addIncomingData(spare);
		processNext();*/
	}
}

void ClientStream::cr_error()
{
	qDebug("CLIENTSTREAM: cr_error()");
	reset();
	emit error(ErrConnection);
}

void ClientStream::bs_connectionClosed()
{
	reset();
	emit connectionClosed();
}

void ClientStream::bs_delayedCloseFinished()
{
	// we don't care about this (we track all important data ourself)
}

void ClientStream::bs_error(int)
{
	// TODO
}

void ClientStream::ss_readyRead()
{
	QByteArray a;
	//qDebug( "size of storage for incoming data is %i bytes.", a.size() );
	a = d->ss->read();

#ifdef LIBGW_DEBUG
	QCString cs(a.data(), a.size()+1);
	//qDebug("ClientStream: recv: %d [%s]\n", a.size(), cs.data());
	qDebug("ClientStream: ss_readyRead() recv: %d bytes\n", a.size() );
	cs_dump( a );
#endif

	d->client.addIncomingData(a);
/*	if(d->notify & CoreProtocol::NRecv) { */
	//processNext();
}

void ClientStream::ss_bytesWritten(int bytes)
{
#ifdef LIBGW_DEBUG
	qDebug( "ClientStream::ss_bytesWritten: %i bytes written", bytes );
#endif
/*	if(d->mode == Client)
		d->client.outgoingDataWritten(bytes);
	else
		d->srv.outgoingDataWritten(bytes);
*/
/*	if(d->notify & CoreProtocol::NSend) {
#ifdef LIBGW_DEBUG
		printf("We were waiting for data to be written, so let's process\n");
#endif
		processNext();
	}*/
}

void ClientStream::ss_tlsHandshaken()
{
	QGuardedPtr<QObject> self = this;
	emit securityLayerActivated(LayerTLS);
	if(!self)
		return;
	processNext();
}

void ClientStream::ss_tlsClosed()
{
	qDebug( "ClientStream::ss_tlsClosed()" );
	reset();
	emit connectionClosed();
}

void ClientStream::ss_error(int x)
{
	qDebug( "ClientStream::ss_error() x=%i ", x );
	if(x == SecureStream::ErrTLS) {
		reset();
		d->errCond = TLSFail;
		emit error(ErrTLS);
	}
	else {
		reset();
		emit error(ErrSecurityLayer);
	}
}

void ClientStream::srvProcessNext()
{
}

void ClientStream::doReadyRead()
{
	//QGuardedPtr<QObject> self = this;
	emit readyRead();
	//if(!self)
	//	return;
	//d->in_rrsig = false;
}

void ClientStream::processNext()
{
	if( !d->in.isEmpty() ) {
		//d->in_rrsig = true;
		QTimer::singleShot(0, this, SLOT(doReadyRead()));
	}
}

/*	if(d->mode == Server) {
		srvProcessNext();
		return;
	}

	QGuardedPtr<QObject> self = this;

	while(1) {
#ifdef LIBGW_DEBUG
		printf("Processing step...\n");
#endif
		bool ok = d->client.processStep();
		// deal with send/received items
		for(QValueList<XmlProtocol::TransferItem>::ConstIterator it = d->client.transferItemList.begin(); it != d->client.transferItemList.end(); ++it) {
			const XmlProtocol::TransferItem &i = *it;
			if(i.isExternal)
				continue;
			QString str;
			if(i.isString) {
				// skip whitespace pings
				if(i.str.stripWhiteSpace().isEmpty())
					continue;
				str = i.str;
			}
			else
				str = d->client.elementToString(i.elem);
			if(i.isSent)
				outgoingXml(str);
			else
				incomingXml(str);
		}

		if(!ok) {
			bool cont = handleNeed();

			// now we can announce stanzas
			//if(!d->in_rrsig && !d->in.isEmpty()) {
			if(!d->in.isEmpty()) {
				//d->in_rrsig = true;
				QTimer::singleShot(0, this, SLOT(doReadyRead()));
			}

			if(cont)
				continue;
			return;
		}

		int event = d->client.event;
		d->notify = 0;
		switch(event) {
			case CoreProtocol::EError: {
#ifdef LIBGW_DEBUG
				printf("Error! Code=%d\n", d->client.errorCode);
#endif
				handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->client.takeOutgoingData();
#ifdef LIBGW_DEBUG
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
#endif
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
#ifdef LIBGW_DEBUG
				printf("Break (RecvOpen)\n");
#endif

#ifdef XMPP_TEST
				QString s = QString("handshake success (lang=[%1]").arg(d->client.lang);
				if(!d->client.from.isEmpty())
					s += QString(", from=[%1]").arg(d->client.from);
				s += ')';
				TD::msg(s);
#endif

				if(d->client.old) {
					d->state = WaitVersion;
					warning(WarnOldVersion);
					return;
				}
				break;
			}
			case CoreProtocol::EFeatures: {
#ifdef LIBGW_DEBUG
				printf("Break (Features)\n");
#endif
				if(!d->tls_warned && !d->using_tls && !d->client.features.tls_supported) {
					d->tls_warned = true;
					d->state = WaitTLS;
					warning(WarnNoTLS);
					return;
				}
				break;
			}
			case CoreProtocol::ESASLSuccess: {
#ifdef LIBGW_DEBUG
				printf("Break SASL Success\n");
#endif
				break;
			}
			case CoreProtocol::EReady: {
#ifdef LIBGW_DEBUG
				printf("Done!\n");
#endif
				// grab the JID, in case it changed
				// TODO: d->jid = d->client.jid;
				d->state = Active;
				setNoopTime(d->noop_time);
				authenticated();
				if(!self)
					return;
				break;
			}
			case CoreProtocol::EPeerClosed: {
#ifdef LIBGW_DEBUG
				printf("DocumentClosed\n");
#endif
				reset();
				connectionClosed();
				return;
			}
			case CoreProtocol::EStanzaReady: {
#ifdef LIBGW_DEBUG
				printf("StanzaReady\n");
#endif
				// store the stanza for now, announce after processing all events
				Stanza s = createStanza(d->client.recvStanza());
				if(s.isNull())
					break;
				d->in.append(new Stanza(s));
				break;
			}
			case CoreProtocol::EStanzaSent: {
#ifdef LIBGW_DEBUG
				printf("StanzasSent\n");
#endif
				stanzaWritten();
				if(!self)
					return;
				break;
			}
			case CoreProtocol::EClosed: {
#ifdef LIBGW_DEBUG
				printf("Closed\n");
#endif
				reset();
				delayedCloseFinished();
				return;
			}
		}
	}*/

bool ClientStream::handleNeed()
{
	return false;
}


void ClientStream::doNoop()
{
}

void ClientStream::handleError()
{
}

ClientStream::Buffer::Buffer( unsigned int sz )
: QByteArray( sz )
{
}

ClientStream::Buffer::~Buffer()
{
}

void ClientStream::Buffer::add( char *str, unsigned int sz )
{
	char *b = new char[ size() + sz ];
	for ( uint f = 0; f < size(); f++ )
		b[ f ] = data()[ f ];
	for ( uint f = 0; f < sz; f++ )
		b[ size() + f ] = str[ f ];

	duplicate( b, size() + sz );
	delete[] b;
}

QByteArray ClientStream::Buffer::take( unsigned blockSize )
{
	if ( size() < blockSize )
	{
		qDebug( "ClientStream::Buffer::take( unsigned blockSize ) Buffer size: %i  asked size: %i!",  size(), blockSize  );
		return QByteArray();
	}

	QByteArray rep( blockSize );
	for( uint i = 0; i < blockSize; i++ )
		rep[ i ] = data()[ i ];

	char *str = new char[ size() - blockSize ];
	for ( uint i = 0; i < size() - blockSize; i++ )
		str[ i ] = data()[ blockSize + i ];
	duplicate( str, size() - blockSize );
	delete[] str;

	return rep;
}

#include "gwclientstream.moc"
