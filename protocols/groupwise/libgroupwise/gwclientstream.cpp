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

#include <qguardedptr.h> 
#include <qobject.h>
#include <qtimer.h>

#include "bytestream.h"
#include "connector.h"
#include "coreprotocol.h"
#include "securestream.h"
#include "tlshandler.h"

#include "iostream.h"

#include "gwclientstream.h"

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
		newFields = false;
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
	CoreProtocol srv;

	QString defRealm;

	int mode;
	int state;
	int notify;
	bool newFields;
// 	int sasl_ssf;
	bool tls_warned, using_tls;
	bool doAuth;

// 	QStringList sasl_mechlist;

	int errCond;
	QString errText;

	QPtrList<Field::FieldList> in;

	QTimer noopTimer; // probably not needed
	int noop_time;
};

ClientStream::ClientStream(Connector *conn, TLSHandler *tlsHandler, QObject *parent)
:Stream(parent)
{
	d = new Private;
	d->mode = Client;
	d->conn = conn;
	connect(d->conn, SIGNAL(connected()), SLOT(cr_connected()));
	connect(d->conn, SIGNAL(error()), SLOT(cr_error()));

	d->noop_time = 0;
	connect(&d->noopTimer, SIGNAL(timeout()), SLOT(doNoop()));

	d->tlsHandler = tlsHandler;		// all the extra stuff happening in the larger ctor happens at connect time :)
}

ClientStream::ClientStream(const QString &host, const QString &defRealm, ByteStream *bs, QCA::TLS *tls, QObject *parent)
:Stream(parent)
{
/*	d = new Private;
	d->mode = Server;
	d->bs = bs;
	connect(d->bs, SIGNAL(connectionClosed()), SLOT(bs_connectionClosed()));
	connect(d->bs, SIGNAL(delayedCloseFinished()), SLOT(bs_delayedCloseFinished()));
	connect(d->bs, SIGNAL(error(int)), SLOT(bs_error(int)));

	QByteArray spare = d->bs->read();

	d->ss = new SecureStream(d->bs);
	connect(d->ss, SIGNAL(readyRead()), SLOT(ss_readyRead()));
	connect(d->ss, SIGNAL(bytesWritten(int)), SLOT(ss_bytesWritten(int)));
	connect(d->ss, SIGNAL(tlsHandshaken()), SLOT(ss_tlsHandshaken()));
	connect(d->ss, SIGNAL(tlsClosed()), SLOT(ss_tlsClosed()));
	connect(d->ss, SIGNAL(error(int)), SLOT(ss_error(int)));

	d->server = host;
	d->defRealm = defRealm;

	d->tls = tls;

	d->srv.startClientIn(genId());
	//d->srv.startServerIn(genId());
	//d->state = Connecting;
	//d->jid = Jid();
	//d->server = QString();
	*/
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
	// server
	else {
		if(d->tls)
			d->tls->reset();

		if(d->bs) {
			d->bs->close();
			d->bs = 0;
		}

		d->srv.reset();
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
// 	return d->errAppSpec;
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

bool ClientStream::fieldsAvailable() const
{
	return ( !d->in.isEmpty() );
}

Field::FieldList ClientStream::read()
{
	if(d->in.isEmpty())	
		return Field::FieldList(); //first from queue...
	else {
		// read from secure socket
		
	}
}

NMERR_T ClientStream::write(const Field::FieldList &fields)
{
	// test output for comparison with Gaim
	
	// write to secure socket
	
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
	reset();
	error(ErrConnection);
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
	QByteArray a = d->ss->read();

#ifdef LIBGW_DEBUG
	QCString cs(a.data(), a.size()+1);
	fprintf(stderr, "ClientStream: recv: %d [%s]\n", a.size(), cs.data());
#endif

	d->client.addIncomingData(a);
/*	if(d->notify & CoreProtocol::NRecv) { */
#ifdef LIBGW_DEBUG
	printf("We needed data, so let's process it\n");
#endif
	processNext();
}

void ClientStream::ss_bytesWritten(int bytes)
{
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
	reset();
	emit connectionClosed();
}

void ClientStream::ss_error(int x)
{
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

// void ClientStream::sasl_clientFirstStep(const QString &mech, const QByteArray *stepData)
// {
// 	d->client.setSASLFirst(mech, stepData ? *stepData : QByteArray());
// 	//d->client.sasl_mech = mech;
// 	//d->client.sasl_firstStep = stepData ? true : false;
// 	//d->client.sasl_step = stepData ? *stepData : QByteArray();
// 
// 	processNext();
// }
// 
// void ClientStream::sasl_nextStep(const QByteArray &stepData)
// {
// 	if(d->mode == Client)
// 		d->client.setSASLNext(stepData);
// 		//d->client.sasl_step = stepData;
// 	else
// 		d->srv.setSASLNext(stepData);
// 		//d->srv.sasl_step = stepData;
// 
// 	processNext();
// }
// 
// void ClientStream::sasl_needParams(bool user, bool authzid, bool pass, bool realm)
// {
// #ifdef LIBGW_DEBUG
// 	printf("need params: %d,%d,%d,%d\n", user, authzid, pass, realm);
// #endif
// 	if(authzid && !user) {
// 		d->sasl->setAuthzid(d->jid.bare());
// 		//d->sasl->setAuthzid("infiniti.homelesshackers.org");
// 	}
// 	if(user || pass || realm) {
// 		d->state = NeedParams;
// 		needAuthParams(user, pass, realm);
// 	}
// 	else
// 		d->sasl->continueAfterParams();
// }
// 
// void ClientStream::sasl_authCheck(const QString &user, const QString &)
// {
// //#ifdef LIBGW_DEBUG
// //	printf("authcheck: [%s], [%s]\n", user.latin1(), authzid.latin1());
// //#endif
// 	QString u = user;
// 	int n = u.find('@');
// 	if(n != -1)
// 		u.truncate(n);
// 	d->srv.user = u;
// 	d->sasl->continueAfterAuthCheck();
// }

// void ClientStream::sasl_authenticated()
// {
// #ifdef LIBGW_DEBUG
// 	printf("sasl authed!!\n");
// #endif
// 	d->sasl_ssf = d->sasl->ssf();
// 
// 	if(d->mode == Server) {
// 		d->srv.setSASLAuthed();
// 		processNext();
// 	}
// }
// 
// void ClientStream::sasl_error(int)
// {
// //#ifdef LIBGW_DEBUG
// //	printf("sasl error: %d\n", c);
// //#endif
// 	// has to be auth error
// 	int x = convertedSASLCond();
// 	reset();
// 	d->errCond = x;
// 	error(ErrAuth);
// }

void ClientStream::srvProcessNext()
{
/*	while(1) {
		printf("Processing step...\n");
		if(!d->srv.processStep()) {
			int need = d->srv.need;
			if(need == CoreProtocol::NNotify) {
				d->notify = d->srv.notify;
				if(d->notify & CoreProtocol::NSend)
					printf("More data needs to be written to process next step\n");
				if(d->notify & CoreProtocol::NRecv)
					printf("More data is needed to process next step\n");
			}
			else if(need == CoreProtocol::NSASLMechs) {
				if(!d->sasl) {
					d->sasl = new QCA::SASL;
					connect(d->sasl, SIGNAL(authCheck(const QString &, const QString &)), SLOT(sasl_authCheck(const QString &, const QString &)));
					connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
					connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
					connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

					//d->sasl->setAllowAnonymous(false);
					//d->sasl->setRequirePassCredentials(true);
					//d->sasl->setExternalAuthID("localhost");

				while(1) {
		printf("Processing step...\n");
		if(!d->srv.processStep()) {
			int need = d->srv.need;
			if(need == CoreProtocol::NNotify) {
				d->notify = d->srv.notify;
				if(d->notify & CoreProtocol::NSend)
					printf("More data needs to be written to process next step\n");
				if(d->notify & CoreProtocol::NRecv)
					printf("More data is needed to process next step\n");
			}
			else if(need == CoreProtocol::NSASLMechs) {
				if(!d->sasl) {
					d->sasl = new QCA::SASL;
					connect(d->sasl, SIGNAL(authCheck(const QString &, const QString &)), SLOT(sasl_authCheck(const QString &, const QString &)));
					connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
					connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
					connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

					//d->sasl->setAllowAnonymous(false);
					//d->sasl->setRequirePassCredentials(true);
					//d->sasl->setExternalAuthID("localhost");

					d->sasl->setMinimumSSF(0);
					d->sasl->setMaximumSSF(256);

					QStringList list;
					// TODO: d->server is probably wrong here
					if(!d->sasl->startServer("xmpp", d->server, d->defRealm, &list)) {
						printf("Error initializing SASL\n");
						return;
					}
					d->sasl_mechlist = list;
				}
				d->srv.setSASLMechList(d->sasl_mechlist);
				continue;
			}
			else if(need == CoreProtocol::NStartTLS) {
				printf("Need StartTLS\n");
				if(!d->tls->startServer()) {
					printf("unable to start server!\n");
					// TODO
					return;
				}
				QByteArray a = d->srv.spare;
				d->ss->startTLSServer(d->tls, a);
			}
			else if(need == CoreProtocol::NSASLFirst) {
				printf("Need SASL First Step\n");
				QByteArray a = d->srv.saslStep();
				d->sasl->putServerFirstStep(d->srv.saslMech(), a);
			}
			else if(need == CoreProtocol::NSASLNext) {
				printf("Need SASL Next Step\n");
				QByteArray a = d->srv.saslStep();
				QCString cs(a.data(), a.size()+1);
				printf("[%s]\n", cs.data());
				d->sasl->putStep(a);
			}
			else if(need == CoreProtocol::NSASLLayer) {
			}

			// now we can announce stanzas
			//if(!d->in.isEmpty())
			//	readyRead();
			return;
		}

		d->notify = 0;

		int event = d->srv.event;
		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
				printf("Error! Code=%d\n", d->srv.errorCode);
				reset();
				error(ErrProtocol);
				//handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->srv.takeOutgoingData();
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
				printf("Break (RecvOpen)\n");

				// calculate key
				QCString str = QCA::SHA1::hashToString("secret").utf8();
				str = QCA::SHA1::hashToString(str + "im.pyxa.org").utf8();
				str = QCA::SHA1::hashToString(str + d->srv.id.utf8()).utf8();
				d->srv.setDialbackKey(str);

				//d->srv.setDialbackKey("3c5d721ea2fcc45b163a11420e4e358f87e3142a");

				if(d->srv.to != d->server) {
					// host-gone, host-unknown, see-other-host
					d->srv.shutdownWithError(CoreProtocol::HostUnknown);
				}
				else
					d->srv.setFrom(d->server);
				break;
			}
			case CoreProtocol::ESASLSuccess: {
				printf("Break SASL Success\n");
				disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
				QByteArray a = d->srv.spare;
				d->ss->setLayerSASL(d->sasl, a);
				break;
			}
			case CoreProtocol::EPeerClosed: {
				// TODO: this isn' an error
				printf("peer closed\n");
				reset();
				error(ErrProtocol);
				return;
			}
		}
	}
		d->sasl->setMinimumSSF(0);
					d->sasl->setMaximumSSF(256);

					QStringList list;
					// TODO: d->server is probably wrong here
					if(!d->sasl->startServer("xmpp", d->server, d->defRealm, &list)) {
						printf("Error initializing SASL\n");
						return;
					}
					d->sasl_mechlist = list;
				}
				d->srv.setSASLMechList(d->sasl_mechlist);
				continue;
			}
			else if(need == CoreProtocol::NStartTLS) {
				printf("Need StartTLS\n");
				if(!d->tls->startServer()) {
					printf("unable to start server!\n");
					// TODO
					return;
				}
				QByteArray a = d->srv.spare;
				d->ss->startTLSServer(d->tls, a);
			}
			else if(need == CoreProtocol::NSASLFirst) {
				printf("Need SASL First Step\n");
				QByteArray a = d->srv.saslStep();
				d->sasl->putServerFirstStep(d->srv.saslMech(), a);
			}
			else if(need == CoreProtocol::NSASLNext) {
				printf("Need SASL Next Step\n");
				QByteArray a = d->srv.saslStep();
				QCString cs(a.data(), a.size()+1);
				printf("[%s]\n", cs.data());
				d->sasl->putStep(a);
			}
			else if(need == CoreProtocol::NSASLLayer) {
			}

			// now we can announce stanzas
			//if(!d->in.isEmpty())
			//	readyRead();
			return;
		}

		d->notify = 0;

		int event = d->srv.event;
		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
				printf("Error! Code=%d\n", d->srv.errorCode);
				reset();
				error(ErrProtocol);
				//handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->srv.takeOutgoingData();
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
				printf("Break (RecvOpen)\n");

				// calculate key
				QCString str = QCA::SHA1::hashToString("secret").utf8();
				str = QCA::SHA1::hashToString(str + "im.pyxa.org").utf8();
				str = QCA::SHA1::hashToString(str + d->srv.id.utf8()).utf8();
				d->srv.setDialbackKey(str);

				//d->srv.setDialbackKey("3c5d721ea2fcc45b163a11420e4e358f87e3142a");

				if(d->srv.to != d->server) {
	while(1) {
		printf("Processing step...\n");
		if(!d->srv.processStep()) {
			int need = d->srv.need;
			if(need == CoreProtocol::NNotify) {
				d->notify = d->srv.notify;
				if(d->notify & CoreProtocol::NSend)
					printf("More data needs to be written to process next step\n");
				if(d->notify & CoreProtocol::NRecv)
					printf("More data is needed to process next step\n");
			}
			else if(need == CoreProtocol::NSASLMechs) {
				if(!d->sasl) {
					d->sasl = new QCA::SASL;
				while(1) {
		printf("Processing step...\n");
		if(!d->srv.processStep()) {
			int need = d->srv.need;
			if(need == CoreProtocol::NNotify) {
				d->notify = d->srv.notify;
				if(d->notify & CoreProtocol::NSend)
					printf("More data needs to be written to process next step\n");
				if(d->notify & CoreProtocol::NRecv)
					printf("More data is needed to process next step\n");
			}
			else if(need == CoreProtocol::NSASLMechs) {
				if(!d->sasl) {
					d->sasl = new QCA::SASL;
					connect(d->sasl, SIGNAL(authCheck(const QString &, const QString &)), SLOT(sasl_authCheck(const QString &, const QString &)));
					connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
					connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
					connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

					//d->sasl->setAllowAnonymous(false);
					//d->sasl->setRequirePassCredentials(true);
					//d->sasl->setExternalAuthID("localhost");

					d->sasl->setMinimumSSF(0);
					d->sasl->setMaximumSSF(256);

					QStringList list;
					// TODO: d->server is probably wrong here
					if(!d->sasl->startServer("xmpp", d->server, d->defRealm, &list)) {
						printf("Error initializing SASL\n");
						return;
					}
					d->sasl_mechlist = list;
				}
				d->srv.setSASLMechList(d->sasl_mechlist);
				continue;
			}
			else if(need == CoreProtocol::NStartTLS) {
				printf("Need StartTLS\n");
				if(!d->tls->startServer()) {
					printf("unable to start server!\n");
					// TODO
					return;
				}
				QByteArray a = d->srv.spare;
				d->ss->startTLSServer(d->tls, a);
			}
			else if(need == CoreProtocol::NSASLFirst) {
				printf("Need SASL First Step\n");
				QByteArray a = d->srv.saslStep();
				d->sasl->putServerFirstStep(d->srv.saslMech(), a);
			}
			else if(need == CoreProtocol::NSASLNext) {
				printf("Need SASL Next Step\n");
				QByteArray a = d->srv.saslStep();
				QCString cs(a.data(), a.size()+1);
				printf("[%s]\n", cs.data());
				d->sasl->putStep(a);
			}
			else if(need == CoreProtocol::NSASLLayer) {
			}

			// now we can announce stanzas
			//if(!d->in.isEmpty())
			//	readyRead();
			return;
		}

		d->notify = 0;

		int event = d->srv.event;
		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
				printf("Error! Code=%d\n", d->srv.errorCode);
				reset();
				error(ErrProtocol);
				//handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->srv.takeOutgoingData();
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
				printf("Break (RecvOpen)\n");

				// calculate key
				QCString str = QCA::SHA1::hashToString("secret").utf8();
				str = QCA::SHA1::hashToString(str + "im.pyxa.org").utf8();
				str = QCA::SHA1::hashToString(str + d->srv.id.utf8()).utf8();
				d->srv.setDialbackKey(str);

				//d->srv.setDialbackKey("3c5d721ea2fcc45b163a11420e4e358f87e3142a");

				if(d->srv.to != d->server) {
					// host-gone, host-unknown, see-other-host
					d->srv.shutdownWithError(CoreProtocol::HostUnknown);
				}
				else
					d->srv.setFrom(d->server);
				break;
			}
			case CoreProtocol::ESASLSuccess: {
				printf("Break SASL Success\n");
				disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
				QByteArray a = d->srv.spare;
				d->ss->setLayerSASL(d->sasl, a);
				break;
			}
			case CoreProtocol::EPeerClosed: {
				// TODO: this isn' an error
				printf("peer closed\n");
				reset();
				error(ErrProtocol);
				return;
			}
		}
	}
		connect(d->sasl, SIGNAL(authCheck(const QString &, const QString &)), SLOT(sasl_authCheck(const QString &, const QString &)));
					connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
					connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
					connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

					//d->sasl->setAllowAnonymous(false);
					//d->sasl->setRequirePassCredentials(true);
					//d->sasl->setExternalAuthID("localhost");

					d->sasl->setMinimumSSF(0);
					d->sasl->setMaximumSSF(256);

					QStringList list;
					// TODO: d->server is probably wrong here
					if(!d->sasl->startServer("xmpp", d->server, d->defRealm, &list)) {
						printf("Error initializing SASL\n");
						return;
					}
					d->sasl_mechlist = list;
				}
				d->srv.setSASLMechList(d->sasl_mechlist);
				continue;
			}
			else if(need == CoreProtocol::NStartTLS) {
				printf("Need StartTLS\n");
				if(!d->tls->startServer()) {
					printf("unable to start server!\n");
					// TODO
					return;
				}
				QByteArray a = d->srv.spare;
				d->ss->startTLSServer(d->tls, a);
			}
			else if(need == CoreProtocol::NSASLFirst) {
				printf("Need SASL First Step\n");
				QByteArray a = d->srv.saslStep();
				d->sasl->putServerFirstStep(d->srv.saslMech(), a);
			}
			else if(need == CoreProtocol::NSASLNext) {
				printf("Need SASL Next Step\n");
				QByteArray a = d->srv.saslStep();
				QCString cs(a.data(), a.size()+1);
				printf("[%s]\n", cs.data());
				d->sasl->putStep(a);
			}
			else if(need == CoreProtocol::NSASLLayer) {
			}

			// now we can announce stanzas
			//if(!d->in.isEmpty())
			//	readyRead();
			return;
		}

		d->notify = 0;

		int event = d->srv.event;
		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
				printf("Error! Code=%d\n", d->srv.errorCode);
				reset();
				error(ErrProtocol);
				//handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->srv.takeOutgoingData();
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
				printf("Break (RecvOpen)\n");

				// calculate key
				QCString str = QCA::SHA1::hashToString("secret").utf8();
				str = QCA::SHA1::hashToString(str + "im.pyxa.org").utf8();
				str = QCA::SHA1::hashToString(str + d->srv.id.utf8()).utf8();
				d->srv.setDialbackKey(str);

				//d->srv.setDialbackKey("3c5d721ea2fcc45b163a11420e4e358f87e3142a");

				if(d->srv.to != d->server) {
					// host-gone, host-unknown, see-other-host
					d->srv.shutdownWithError(CoreProtocol::HostUnknown);
				}
				else
					d->srv.setFrom(d->server);
				break;
			}
			case CoreProtocol::ESASLSuccess: {
				printf("Break SASL Success\n");
				disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
				QByteArray a = d->srv.spare;
				d->ss->setLayerSASL(d->sasl, a);
				break;
			}
			case CoreProtocol::EPeerClosed: {
				// TODO: this isn' an error
				printf("peer closed\n");
				reset();
				error(ErrProtocol);
				return;
			}
		}
	}
					// host-gone, host-unknown, see-other-host
					d->srv.shutdownWithError(CoreProtocol::HostUnknown);
				}
				else
					d->srv.setFrom(d->server);
				break;
			}
			case CoreProtocol::ESASLSuccess: {
				printf("Break SASL Success\n");
				disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
				QByteArray a = d->srv.spare;
				d->ss->setLayerSASL(d->sasl, a);
				break;
			}
			case CoreProtocol::EPeerClosed: {
				// TODO: this isn' an error
				printf("peer closed\n");
				reset();
				error(ErrProtocol);
				return;
			}
		}
	}*/
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
/*	int need = d->client.need;
	if(need == CoreProtocol::NNotify) {
		d->notify = d->client.notify;
#ifdef LIBGW_DEBUG
		if(d->notify & CoreProtocol::NSend)
			printf("More data needs to be written to process next step\n");
		if(d->notify & CoreProtocol::NRecv)
			printf("More data is needed to process next step\n");
#endif
		return false;
	}

	d->notify = 0;
	switch(need) {
		case CoreProtocol::NStartTLS: {
#ifdef LIBGW_DEBUG
			printf("Need StartTLS\n");
#endif
			d->using_tls = true;
			d->ss->startTLSClient(d->tlsHandler, d->server, d->client.spare);
			return false;
		}
		case CoreProtocol::NSASLFirst: {
#ifdef LIBGW_DEBUG
			printf("Need SASL First Step\n");
#endif
			// no SASL plugin?  fall back to Simple SASL
			if(!QCA::isSupported(QCA::CAP_SASL)) {
				// Simple SASL needs MD5.  do we have that either?
				if(!QCA::isSupported(QCA::CAP_MD5))	while(1) {
		printf("Processing step...\n");
		if(!d->srv.processStep()) {
			int need = d->srv.need;
			if(need == CoreProtocol::NNotify) {
				d->notify = d->srv.notify;
				if(d->notify & CoreProtocol::NSend)
					printf("More data needs to be written to process next step\n");
				if(d->notify & CoreProtocol::NRecv)
					printf("More data is needed to process next step\n");
			}
			else if(need == CoreProtocol::NSASLMechs) {
				if(!d->sasl) {
					d->sasl = new QCA::SASL;
					connect(d->sasl, SIGNAL(authCheck(const QString &, const QString &)), SLOT(sasl_authCheck(const QString &, const QString &)));
					connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
					connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
					connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

					//d->sasl->setAllowAnonymous(false);
					//d->sasl->setRequirePassCredentials(true);
					//d->sasl->setExternalAuthID("localhost");

					d->sasl->setMinimumSSF(0);
					d->sasl->setMaximumSSF(256);

					QStringList list;
					// TODO: d->server is probably wrong here
					if(!d->sasl->startServer("xmpp", d->server, d->defRealm, &list)) {
						printf("Error initializing SASL\n");
						return;
					}
					d->sasl_mechlist = list;
				}
				d->srv.setSASLMechList(d->sasl_mechlist);
				continue;
			}
			else if(need == CoreProtocol::NStartTLS) {
				printf("Need StartTLS\n");
				if(!d->tls->startServer()) {
					printf("unable to start server!\n");
					// TODO
					return;
				}
				QByteArray a = d->srv.spare;
				d->ss->startTLSServer(d->tls, a);
			}
			else if(need == CoreProtocol::NSASLFirst) {
				printf("Need SASL First Step\n");
				QByteArray a = d->srv.saslStep();
				d->sasl->putServerFirstStep(d->srv.saslMech(), a);
			}
			else if(need == CoreProtocol::NSASLNext) {
				printf("Need SASL Next Step\n");
				QByteArray a = d->srv.saslStep();
				QCString cs(a.data(), a.size()+1);
				printf("[%s]\n", cs.data());
				d->sasl->putStep(a);
			}
			else if(need == CoreProtocol::NSASLLayer) {
			}

			// now we can announce stanzas
			//if(!d->in.isEmpty())
			//	readyRead();
			return;
		}

		d->notify = 0;

		int event = d->srv.event;
		printf("event: %d\n", event);
		switch(event) {
			case CoreProtocol::EError: {
				printf("Error! Code=%d\n", d->srv.errorCode);
				reset();
				error(ErrProtocol);
				//handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->srv.takeOutgoingData();
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
				printf("Break (RecvOpen)\n");

				// calculate key
				QCString str = QCA::SHA1::hashToString("secret").utf8();
				str = QCA::SHA1::hashToString(str + "im.pyxa.org").utf8();
				str = QCA::SHA1::hashToString(str + d->srv.id.utf8()).utf8();
				d->srv.setDialbackKey(str);

				//d->srv.setDialbackKey("3c5d721ea2fcc45b163a11420e4e358f87e3142a");

				if(d->srv.to != d->server) {
					// host-gone, host-unknown, see-other-host
					d->srv.shutdownWithError(CoreProtocol::HostUnknown);
				}
				else
					d->srv.setFrom(d->server);
				break;
			}
			case CoreProtocol::ESASLSuccess: {
				printf("Break SASL Success\n");
				disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
				QByteArray a = d->srv.spare;
				d->ss->setLayerSASL(d->sasl, a);
				break;
			}
			case CoreProtocol::EPeerClosed: {
				// TODO: this isn' an error
				printf("peer closed\n");
				reset();
				error(ErrProtocol);
				return;
			}
		}
	}

					QCA::insertProvider(createProviderHash());
				QCA::insertProvider(createProviderSimpleSASL());
			}

			d->sasl = new QCA::SASL;
			connect(d->sasl, SIGNAL(clientFirstStep(const QString &, const QByteArray *)), SLOT(sasl_clientFirstStep(const QString &, const QByteArray *)));
			connect(d->sasl, SIGNAL(nextStep(const QByteArray &)), SLOT(sasl_nextStep(const QByteArray &)));
			connect(d->sasl, SIGNAL(needParams(bool, bool, bool, bool)), SLOT(sasl_needParams(bool, bool, bool, bool)));
			connect(d->sasl, SIGNAL(authenticated()), SLOT(sasl_authenticated()));
			connect(d->sasl, SIGNAL(error(int)), SLOT(sasl_error(int)));

			if(d->haveLocalAddr)
				d->sasl->setLocalAddr(d->localAddr, d->localPort);
			if(d->conn->havePeerAddress())
				d->sasl->setRemoteAddr(d->conn->peerAddress(), d->conn->peerPort());

			d->sasl->setAllowAnonymous(false);

			//d->sasl_mech = "ANONYMOUS";
			//d->sasl->setRequirePassCredentials(true);

			//d->sasl->setExternalAuthID("localhost");
			//d->sasl->setExternalSSF(64);
			//d->sasl_mech = "EXTERNAL";

			d->sasl->setAllowPlain(d->allowPlain);
			d->sasl->setRequireMutualAuth(d->mutualAuth);

			d->sasl->setMinimumSSF(d->minimumSSF);
			d->sasl->setMaximumSSF(d->maximumSSF);

			QStringList ml;
			if(!d->sasl_mech.isEmpty())
				ml += d->sasl_mech;
			else
				ml = d->client.features.sasl_mechs;

			if(!d->sasl->startClient("xmpp", d->server, ml, true)) {
				int x = convertedSASLCond();
				reset();
				d->errCond = x;
				error(ErrAuth);
				return false;
			}
			return false;
		}
		case CoreProtocol::NSASLNext: {
#ifdef LIBGW_DEBUG
			printf("Need SASL Next Step\n");
#endif
			QByteArray a = d->client.saslStep();
			d->sasl->putStep(a);
			return false;
		}
		case CoreProtocol::NSASLLayer: {
			// SecureStream will handle the errors from this point
			disconnect(d->sasl, SIGNAL(error(int)), this, SLOT(sasl_error(int)));
			d->ss->setLayerSASL(d->sasl, d->client.spare);
			if(d->sasl_ssf > 0) {
				QGuardedPtr<QObject> self = this;
				securityLayerActivated(LayerSASL);
				if(!self)
					return false;
			}
			break;
		}
		case CoreProtocol::NPassword: {
#ifdef LIBGW_DEBUG
			printf("Need Password\n");
#endif
			d->state = NeedParams;
			needAuthParams(false, true, false);
			return false;
		}
	}

	return true;*/
}

// int ClientStream::convertedSASLCond() const
// {
// 	int x = d->sasl->errorCondition();
// 	if(x == QCA::SASL::NoMech)
// 		return NoMech;
// 	else if(x == QCA::SASL::BadProto)
// 		return BadProto;
// 	else if(x == QCA::SASL::BadServ)
// 		return BadServ;
// 	else if(x == QCA::SASL::TooWeak)
// 		return MechTooWeak;
// 	else
// 		return GenericAuthError;
// }

void ClientStream::doNoop()
{
/*	if(d->state == Active) {
#ifdef LIBGW_DEBUG
		printf("doPing\n");
#endif
		d->client.sendWhitespace();
		processNext();
	}*/
}

void ClientStream::handleError()
{
/*	int c = d->client.errorCode;
	if(c == CoreProtocol::ErrParse) {
		reset();
		error(ErrParse);
	}
	else if(c == CoreProtocol::ErrProtocol) {
		reset();
		error(ErrProtocol);
	}
	else if(c == CoreProtocol::ErrStream) {
		int x = d->client.errCond;
		QString text = d->client.errText;
		QDomElement appSpec = d->client.errAppSpec;

		int connErr = -1;
		int strErr = -1;

		switch(x) {
			case CoreProtocol::BadFormat: { break; } // should NOT happen (we send the right format)
			case CoreProtocol::BadNamespacePrefix: { break; } // should NOT happen (we send prefixes)
			case CoreProtocol::Conflict: { strErr = Conflict; break; }
			case CoreProtocol::ConnectionTimeout: { strErr = ConnectionTimeout; break; }
			case CoreProtocol::HostGone: { connErr = HostGone; break; }
			case CoreProtocol::HostUnknown: { connErr = HostUnknown; break; }
			case CoreProtocol::ImproperAddressing: { break; } // should NOT happen (we aren't a server)
			case CoreProtocol::InternalServerError: { strErr = InternalServerError;  break; }
			case CoreProtocol::InvalidFrom: { strErr = InvalidFrom; break; }
			case CoreProtocol::InvalidId: { break; } // should NOT happen (clients don't specify id)
			case CoreProtocol::InvalidNamespace: { break; } // should NOT happen (we set the right ns)
			case CoreProtocol::InvalidXml: { strErr = InvalidXml; break; } // shouldn't happen either, but just in case ...
			case CoreProtocol::StreamNotAuthorized: { break; } // should NOT happen (we're not stupid)
			case CoreProtocol::PolicyViolation: { strErr = PolicyViolation; break; }
			case CoreProtocol::RemoteConnectionFailed: { connErr = RemoteConnectionFailed; break; }
			case CoreProtocol::ResourceConstraint: { strErr = ResourceConstraint; break; }
			case CoreProtocol::RestrictedXml: { strErr = InvalidXml; break; } // group with this one
			case CoreProtocol::SeeOtherHost: { connErr = SeeOtherHost; break; }
			case CoreProtocol::SystemShutdown: { strErr = SystemShutdown; break; }
			case CoreProtocol::UndefinedCondition: { break; } // leave as null error
			case CoreProtocol::UnsupportedEncoding: { break; } // should NOT happen (we send good encoding)
			case CoreProtocol::UnsupportedStanzaType: { break; } // should NOT happen (we're not stupid)
			case CoreProtocol::UnsupportedVersion: { connErr = UnsupportedVersion; break; }
			case CoreProtocol::XmlNotWellFormed: { strErr = InvalidXml; break; } // group with this one
			default: { break; }
		}

		reset();

		d->errText = text;
		d->errAppSpec = appSpec;
		if(connErr != -1) {
			d->errCond = connErr;
			error(ErrNeg);
		}
		else {
			if(strErr != -1)
				d->errCond = strErr;
			else
				d->errCond = GenericStreamError;
			error(ErrStream);
		}
	}
	else if(c == CoreProtocol::ErrStartTLS) {
		reset();
		d->errCond = TLSStart;
		error(ErrTLS);
	}
	else if(c == CoreProtocol::ErrAuth) {
		int x = d->client.errCond;
		int r = GenericAuthError;
		if(d->client.old) {
			if(x == 401) // not authorized
				r = NotAuthorized;
			else if(x == 409) // conflict
				r = GenericAuthError;
			else if(x == 406) // not acceptable (this should NOT happen)
				r = GenericAuthErint ClientStream::convertedSASLCond() const
{
	int x = d->sasl->errorCondition();
	if(x == QCA::SASL::NoMech)
		return NoMech;
	else if(x == QCA::SASL::BadProto)
		return BadProto;
	else if(x == QCA::SASL::BadServ)
		return BadServ;
	else if(x == QCA::SASL::TooWeak)
		return MechTooWeak;
	else
		return GenericAuthError;
}
ror;
		}
		else {
			switch(x) {
				case CoreProtocol::Aborted: { r = GenericAuthError; break; } // should NOT happen (we never send <abort/>)
				case CoreProtocol::IncorrectEncoding: { r = GenericAuthError; break; } // should NOT happen
				case CoreProtocol::InvalidAuthzid: { r = InvalidAuthzid; break; }
				case CoreProtocol::InvalidMech: { r = InvalidMech; break; }
				case CoreProtocol::MechTooWeak: { r = MechTooWeak; break; }
				case CoreProtocol::NotAuthorized: { r = NotAuthorized; break; }
				case CoreProtocol::TemporaryAuthFailure: { r = TemporaryAuthFailure; break; }
			}
		}
		reset();
		d->errCond = r;
		error(ErrAuth);
	}
	else if(c == CoreProtocol::ErrPlain) {
		reset();
		d->errCond = NoMech;
		error(ErrAuth);
	}
	else if(c == CoreProtocol::ErrBind) {
		int r = -1;
		if(d->client.errCond == CoreProtocol::BindBadRequest) {
			// should NOT happen
		}
		else if(d->client.errCond == CoreProtocol::BindNotAllowed) {
			r = BindNotAllowed;
		}
		else if(d->client.errCond == CoreProtocol::BindConflict) {
			r = BindConflict;
		}

		if(r != -1) {
			reset();
			d->errCond = r;
			error(ErrBind);
		}
		else {
			reset();
			error(ErrProtocol);
		}
	}*/
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
		cout << "ClientStream::Buffer::take( unsigned blockSize ) " << "Buffer size " << size() << " < asked size " << blockSize << "!" << endl;
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
