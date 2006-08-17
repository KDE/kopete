/*
 * stream.cpp - handles a client stream
 * Copyright (C) 2003  Justin Karneges
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

/*
  Notes:
    - For Non-SASL auth (JEP-0078), username and resource fields are required.

  TODO:
    - sasl needParams is totally jacked?  PLAIN requires authzid, etc
    - server error handling
      - reply with protocol errors if the client send something wrong
      - don't necessarily disconnect on protocol error.  prepare for more.
    - server function
      - deal with stream 'to' attribute dynamically
      - flag tls/sasl/binding support dynamically (have the ability to specify extra stream:features)
      - inform the caller about the user authentication information
      - sasl security settings
      - resource-binding interaction
      - timeouts
    - allow exchanges of non-standard stanzas
    - send </stream:stream> even if we close prematurely?
    - ensure ClientStream and child classes are fully deletable after signals
    - xml:lang in root (<stream>) element
    - sasl external
    - sasl anonymous
*/

#include"xmpp.h"

#include<qtextstream.h>
#include<qguardedptr.h>
#include<qtimer.h>
#include<qca.h>
#include<stdlib.h>
#include"bytestream.h"
#include"base64.h"
#include"hash.h"
#include"simplesasl.h"
#include"securestream.h"
#include"protocol.h"

#ifdef XMPP_TEST
#include"td.h"
#endif

//#define XMPP_DEBUG

using namespace XMPP;

static Debug *debug_ptr = 0;
void XMPP::setDebug(Debug *p)
{
	debug_ptr = p;
}

static QByteArray randomArray(int size)
{
	QByteArray a(size);
	for(int n = 0; n < size; ++n)
		a[n] = (char)(256.0*rand()/(RAND_MAX+1.0));
	return a;
}

static QString genId()
{
	// need SHA1 here
	if(!QCA::isSupported(QCA::CAP_SHA1))
		QCA::insertProvider(createProviderHash());

	return QCA::SHA1::hashToString(randomArray(128));
}

//----------------------------------------------------------------------------
// Stanza
//----------------------------------------------------------------------------
Stanza::Error::Error(int _type, int _condition, const QString &_text, const QDomElement &_appSpec)
{
	type = _type;
	condition = _condition;
	text = _text;
	appSpec = _appSpec;
}

class Stanza::Private
{
public:
	struct ErrorTypeEntry
	{
		const char *str;
		int type;
	};
	static ErrorTypeEntry errorTypeTable[];

	struct ErrorCondEntry
	{
		const char *str;
		int cond;
	};
	static ErrorCondEntry errorCondTable[];

	static int stringToKind(const QString &s)
	{
		if(s == "message")
			return Message;
		else if(s == "presence")
			return Presence;
		else if(s == "iq")
			return IQ;
		else
			return -1;
	}

	static QString kindToString(Kind k)
	{
		if(k == Message)
			return "message";
		else if(k == Presence)
			return "presence";
		else
			return "iq";
	}

	static int stringToErrorType(const QString &s)
	{
		for(int n = 0; errorTypeTable[n].str; ++n) {
			if(s == errorTypeTable[n].str)
				return errorTypeTable[n].type;
		}
		return -1;
	}

	static QString errorTypeToString(int x)
	{
		for(int n = 0; errorTypeTable[n].str; ++n) {
			if(x == errorTypeTable[n].type)
				return errorTypeTable[n].str;
		}
		return QString();
	}

	static int stringToErrorCond(const QString &s)
	{
		for(int n = 0; errorCondTable[n].str; ++n) {
			if(s == errorCondTable[n].str)
				return errorCondTable[n].cond;
		}
		return -1;
	}

	static QString errorCondToString(int x)
	{
		for(int n = 0; errorCondTable[n].str; ++n) {
			if(x == errorCondTable[n].cond)
				return errorCondTable[n].str;
		}
		return QString();
	}

	Stream *s;
	QDomElement e;
};

Stanza::Private::ErrorTypeEntry Stanza::Private::errorTypeTable[] =
{
	{ "cancel",   Cancel },
	{ "continue", Continue },
	{ "modify",   Modify },
	{ "auth",     Auth },
	{ "wait",     Wait },
	{ 0, 0 },
};

Stanza::Private::ErrorCondEntry Stanza::Private::errorCondTable[] =
{
	{ "bad-request",             BadRequest },
	{ "conflict",                Conflict },
	{ "feature-not-implemented", FeatureNotImplemented },
	{ "forbidden",               Forbidden },
	{ "internal-server-error",   InternalServerError },
	{ "item-not-found",          ItemNotFound },
	{ "jid-malformed",           JidMalformed },
	{ "not-allowed",             NotAllowed },
	{ "payment-required",        PaymentRequired },
	{ "recipient-unavailable",   RecipientUnavailable },
	{ "registration-required",   RegistrationRequired },
	{ "remote-server-not-found", ServerNotFound },
	{ "remote-server-timeout",   ServerTimeout },
	{ "resource-constraint",     ResourceConstraint },
	{ "service-unavailable",     ServiceUnavailable },
	{ "subscription-required",   SubscriptionRequired },
	{ "undefined-condition",     UndefinedCondition },
	{ "unexpected-request",      UnexpectedRequest },
	{ 0, 0 },
};

Stanza::Stanza()
{
	d = 0;
}

Stanza::Stanza(Stream *s, Kind k, const Jid &to, const QString &type, const QString &id)
{
	d = new Private;

	Kind kind;
	if(k == Message || k == Presence || k == IQ)
		kind = k;
	else
		kind = Message;

	d->s = s;
	d->e = d->s->doc().createElementNS(s->baseNS(), Private::kindToString(kind));
	if(to.isValid())
		setTo(to);
	if(!type.isEmpty())
		setType(type);
	if(!id.isEmpty())
		setId(id);
}

Stanza::Stanza(Stream *s, const QDomElement &e)
{
	d = 0;
	if(e.namespaceURI() != s->baseNS())
		return;
	int x = Private::stringToKind(e.tagName());
	if(x == -1)
		return;
	d = new Private;
	d->s = s;
	d->e = e;
}

Stanza::Stanza(const Stanza &from)
{
	d = 0;
	*this = from;
}

Stanza & Stanza::operator=(const Stanza &from)
{
	delete d;
	d = 0;
	if(from.d)
		d = new Private(*from.d);
	return *this;
}

Stanza::~Stanza()
{
	delete d;
}

bool Stanza::isNull() const
{
	return (d ? false: true);
}

QDomElement Stanza::element() const
{
	return d->e;
}

QString Stanza::toString() const
{
	return Stream::xmlToString(d->e);
}

QDomDocument & Stanza::doc() const
{
	return d->s->doc();
}

QString Stanza::baseNS() const
{
	return d->s->baseNS();
}

QString Stanza::xhtmlImNS() const
{
	return d->s->xhtmlImNS();
}

QString Stanza::xhtmlNS() const
{
	return d->s->xhtmlNS();
}

QDomElement Stanza::createElement(const QString &ns, const QString &tagName)
{
	return d->s->doc().createElementNS(ns, tagName);
}

QDomElement Stanza::createTextElement(const QString &ns, const QString &tagName, const QString &text)
{
	QDomElement e = d->s->doc().createElementNS(ns, tagName);
	e.appendChild(d->s->doc().createTextNode(text));
	return e;
}

QDomElement Stanza::createXHTMLElement(const QString &xHTML)
{
	QDomDocument doc;

  	doc.setContent(xHTML, true);
	QDomElement root = doc.documentElement();
	//QDomElement e;
	return (root);
}

void Stanza::appendChild(const QDomElement &e)
{
	d->e.appendChild(e);
}

Stanza::Kind Stanza::kind() const
{
	return (Kind)Private::stringToKind(d->e.tagName());
}

void Stanza::setKind(Kind k)
{
	d->e.setTagName(Private::kindToString(k));
}

Jid Stanza::to() const
{
	return Jid(d->e.attribute("to"));
}

Jid Stanza::from() const
{
	return Jid(d->e.attribute("from"));
}

QString Stanza::id() const
{
	return d->e.attribute("id");
}

QString Stanza::type() const
{
	return d->e.attribute("type");
}

QString Stanza::lang() const
{
	return d->e.attributeNS(NS_XML, "lang", QString());
}

void Stanza::setTo(const Jid &j)
{
	d->e.setAttribute("to", j.full());
}

void Stanza::setFrom(const Jid &j)
{
	d->e.setAttribute("from", j.full());
}

void Stanza::setId(const QString &id)
{
	d->e.setAttribute("id", id);
}

void Stanza::setType(const QString &type)
{
	d->e.setAttribute("type", type);
}

void Stanza::setLang(const QString &lang)
{
	d->e.setAttribute("xml:lang", lang);
}

Stanza::Error Stanza::error() const
{
	Error err;
	QDomElement e = d->e.elementsByTagNameNS(d->s->baseNS(), "error").item(0).toElement();
	if(e.isNull())
		return err;

	// type
	int x = Private::stringToErrorType(e.attribute("type"));
	if(x != -1)
		err.type = x;

	// condition: find first element
	QDomNodeList nl = e.childNodes();
	QDomElement t;
	uint n;
	for(n = 0; n < nl.count(); ++n) {
		QDomNode i = nl.item(n);
		if(i.isElement()) {
			t = i.toElement();
			break;
		}
	}
	if(!t.isNull() && t.namespaceURI() == NS_STANZAS) {
		x = Private::stringToErrorCond(t.tagName());
		if(x != -1)
			err.condition = x;
	}

	// text
	t = e.elementsByTagNameNS(NS_STANZAS, "text").item(0).toElement();
	if(!t.isNull())
		err.text = t.text();
	else
		err.text = e.text();

	// appspec: find first non-standard namespaced element
	nl = e.childNodes();
	for(n = 0; n < nl.count(); ++n) {
		QDomNode i = nl.item(n);
		if(i.isElement() && i.namespaceURI() != NS_STANZAS) {
			err.appSpec = i.toElement();
			break;
		}
	}
	return err;
}

void Stanza::setError(const Error &err)
{
	// create the element if necessary
	QDomElement errElem = d->e.elementsByTagNameNS(d->s->baseNS(), "error").item(0).toElement();
	if(errElem.isNull()) {
		errElem = d->e.ownerDocument().createElementNS(d->s->baseNS(), "error");
		d->e.appendChild(errElem);
	}

	// error type/condition
	if(d->s->old()) {
		errElem.setAttribute("code", QString::number(err.condition));
	}
	else {
		QString stype = Private::errorTypeToString(err.type);
		if(stype.isEmpty())
			return;
		QString scond = Private::errorCondToString(err.condition);
		if(scond.isEmpty())
			return;

		errElem.setAttribute("type", stype);
		errElem.appendChild(d->e.ownerDocument().createElementNS(d->s->baseNS(), scond));
	}

	// text
	if(d->s->old()) {
		errElem.appendChild(d->e.ownerDocument().createTextNode(err.text));
	}
	else {
		QDomElement te = d->e.ownerDocument().createElementNS(d->s->baseNS(), "text");
		te.appendChild(d->e.ownerDocument().createTextNode(err.text));
		errElem.appendChild(te);
	}

	// application specific
	errElem.appendChild(err.appSpec);
}

void Stanza::clearError()
{
	QDomElement errElem = d->e.elementsByTagNameNS(d->s->baseNS(), "error").item(0).toElement();
	if(!errElem.isNull())
		d->e.removeChild(errElem);
}

//----------------------------------------------------------------------------
// Stream
//----------------------------------------------------------------------------
static XmlProtocol *foo = 0;
Stream::Stream(QObject *parent)
:QObject(parent)
{
}

Stream::~Stream()
{
}

Stanza Stream::createStanza(Stanza::Kind k, const Jid &to, const QString &type, const QString &id)
{
	return Stanza(this, k, to, type, id);
}

Stanza Stream::createStanza(const QDomElement &e)
{
	return Stanza(this, e);
}

QString Stream::xmlToString(const QDomElement &e, bool clip)
{
	if(!foo)
		foo = new CoreProtocol;
	return foo->elementToString(e, clip);
}

//----------------------------------------------------------------------------
// ClientStream
//----------------------------------------------------------------------------
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
		sasl = 0;
		in.setAutoDelete(true);

		oldOnly = false;
		allowPlain = false;
		mutualAuth = false;
		haveLocalAddr = false;
		minimumSSF = 0;
		maximumSSF = 0;
		doBinding = true;

		in_rrsig = false;

		reset();
	}

	void reset()
	{
		state = Idle;
		notify = 0;
		newStanzas = false;
		sasl_ssf = 0;
		tls_warned = false;
		using_tls = false;
	}

	Jid jid;
	QString server;
	bool oldOnly;
	bool allowPlain, mutualAuth;
	bool haveLocalAddr;
	QHostAddress localAddr;
	Q_UINT16 localPort;
	int minimumSSF, maximumSSF;
	QString sasl_mech;
	bool doBinding;

	bool in_rrsig;

	Connector *conn;
	ByteStream *bs;
	TLSHandler *tlsHandler;
	QCA::TLS *tls;
	QCA::SASL *sasl;
	SecureStream *ss;
	CoreProtocol client;
	CoreProtocol srv;

	QString defRealm;

	int mode;
	int state;
	int notify;
	bool newStanzas;
	int sasl_ssf;
	bool tls_warned, using_tls;
	bool doAuth;

	QStringList sasl_mechlist;

	int errCond;
	QString errText;
	QDomElement errAppSpec;

	QPtrList<Stanza> in;

	QTimer noopTimer;
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

	d->tlsHandler = tlsHandler;
}

ClientStream::ClientStream(const QString &host, const QString &defRealm, ByteStream *bs, QCA::TLS *tls, QObject *parent)
:Stream(parent)
{
	d = new Private;
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
	delete d->sasl;
	d->sasl = 0;

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

Jid ClientStream::jid() const
{
	return d->jid;
}

void ClientStream::connectToServer(const Jid &jid, bool auth)
{
	reset(true);
	d->state = Connecting;
	d->jid = jid;
	d->doAuth = auth;
	d->server = d->jid.domain();

	d->conn->connectToServer(d->server);
}

void ClientStream::continueAfterWarning()
{
	if(d->state == WaitVersion) {
		// if we don't have TLS yet, then we're never going to get it
		if(!d->tls_warned && !d->using_tls) {
			d->tls_warned = true;
			d->state = WaitTLS;
			warning(WarnNoTLS);
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
	d->srv.host = d->server;
	processNext();
}

bool ClientStream::isActive() const
{
	return (d->state != Idle) ? true: false;
}

bool ClientStream::isAuthenticated() const
{
	return (d->state == Active) ? true: false;
}

void ClientStream::setUsername(const QString &s)
{
	if(d->sasl)
		d->sasl->setUsername(s);
}

void ClientStream::setPassword(const QString &s)
{
	if(d->client.old) {
		d->client.setPassword(s);
	}
	else {
		if(d->sasl)
			d->sasl->setPassword(s);
	}
}

void ClientStream::setRealm(const QString &s)
{
	if(d->sasl)
		d->sasl->setRealm(s);
}

void ClientStream::continueAfterParams()
{
	if(d->state == NeedParams) {
		d->state = Connecting;
		if(d->client.old) {
			processNext();
		}
		else {
			if(d->sasl)
				d->sasl->continueAfterParams();
		}
	}
}

void ClientStream::setResourceBinding(bool b)
{
	d->doBinding = b;
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

QString ClientStream::saslMechanism() const
{
	return d->client.saslMech();
}

int ClientStream::saslSSF() const
{
	return d->sasl_ssf;
}

void ClientStream::setSASLMechanism(const QString &s)
{
	d->sasl_mech = s;
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

QDomElement ClientStream::errorAppSpec() const
{
	return d->errAppSpec;
}

bool ClientStream::old() const
{
	return d->client.old;
}

void ClientStream::close()
{
	if(d->state == Active) {
		d->state = Closing;
		d->client.shutdown();
		processNext();
	}
	else if(d->state != Idle && d->state != Closing) {
		reset();
	}
}

QDomDocument & ClientStream::doc() const
{
	return d->client.doc;
}

QString ClientStream::baseNS() const
{
	return NS_CLIENT;
}

QString ClientStream::xhtmlImNS() const
{
	return NS_XHTML_IM;
}

QString ClientStream::xhtmlNS() const
{
	return NS_XHTML;
}

void ClientStream::setAllowPlain(bool b)
{
	d->allowPlain = b;
}

void ClientStream::setRequireMutualAuth(bool b)
{
	d->mutualAuth = b;
}

void ClientStream::setSSFRange(int low, int high)
{
	d->minimumSSF = low;
	d->maximumSSF = high;
}

void ClientStream::setOldOnly(bool b)
{
	d->oldOnly = b;
}

bool ClientStream::stanzaAvailable() const
{
	return (!d->in.isEmpty());
}

Stanza ClientStream::read()
{
	if(d->in.isEmpty())
		return Stanza();
	else {
		Stanza *sp = d->in.getFirst();
		Stanza s = *sp;
		d->in.removeRef(sp);
		return s;
	}
}

void ClientStream::write(const Stanza &s)
{
	if(d->state == Active) {
		d->client.sendStanza(s.element());
		processNext();
	}
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

	d->client.startClientOut(d->jid, d->oldOnly, d->conn->useSSL(), d->doAuth);
	d->client.setAllowTLS(d->tlsHandler ? true: false);
	d->client.setAllowBind(d->doBinding);
	d->client.setAllowPlain(d->allowPlain);

	/*d->client.jid = d->jid;
	d->client.server = d->server;
	d->client.allowPlain = d->allowPlain;
	d->client.oldOnly = d->oldOnly;
	d->client.sasl_mech = d->sasl_mech;
	d->client.doTLS = d->tlsHandler ? true: false;
	d->client.doBinding = d->doBinding;*/

	QGuardedPtr<QObject> self = this;
	connected();
	if(!self)
		return;

	// immediate SSL?
	if(d->conn->useSSL()) {
		d->using_tls = true;
		d->ss->startTLSClient(d->tlsHandler, d->server, spare);
	}
	else {
		d->client.addIncomingData(spare);
		processNext();
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
	connectionClosed();
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

#ifdef XMPP_DEBUG
	QCString cs(a.data(), a.size()+1);
	fprintf(stderr, "ClientStream: recv: %d [%s]\n", a.size(), cs.data());
#endif

	if(d->mode == Client)
		d->client.addIncomingData(a);
	else
		d->srv.addIncomingData(a);
	if(d->notify & CoreProtocol::NRecv) {
#ifdef XMPP_DEBUG
		printf("We needed data, so let's process it\n");
#endif
		processNext();
	}
}

void ClientStream::ss_bytesWritten(int bytes)
{
	if(d->mode == Client)
		d->client.outgoingDataWritten(bytes);
	else
		d->srv.outgoingDataWritten(bytes);

	if(d->notify & CoreProtocol::NSend) {
#ifdef XMPP_DEBUG
		printf("We were waiting for data to be written, so let's process\n");
#endif
		processNext();
	}
}

void ClientStream::ss_tlsHandshaken()
{
	QGuardedPtr<QObject> self = this;
	securityLayerActivated(LayerTLS);
	if(!self)
		return;
	processNext();
}

void ClientStream::ss_tlsClosed()
{
	reset();
	connectionClosed();
}

void ClientStream::ss_error(int x)
{
	if(x == SecureStream::ErrTLS) {
		reset();
		d->errCond = TLSFail;
		error(ErrTLS);
	}
	else {
		reset();
		error(ErrSecurityLayer);
	}
}

void ClientStream::sasl_clientFirstStep(const QString &mech, const QByteArray *stepData)
{
	d->client.setSASLFirst(mech, stepData ? *stepData : QByteArray());
	//d->client.sasl_mech = mech;
	//d->client.sasl_firstStep = stepData ? true : false;
	//d->client.sasl_step = stepData ? *stepData : QByteArray();

	processNext();
}

void ClientStream::sasl_nextStep(const QByteArray &stepData)
{
	if(d->mode == Client)
		d->client.setSASLNext(stepData);
		//d->client.sasl_step = stepData;
	else
		d->srv.setSASLNext(stepData);
		//d->srv.sasl_step = stepData;

	processNext();
}

void ClientStream::sasl_needParams(bool user, bool authzid, bool pass, bool realm)
{
#ifdef XMPP_DEBUG
	printf("need params: %d,%d,%d,%d\n", user, authzid, pass, realm);
#endif
	if(authzid && !user) {
		d->sasl->setAuthzid(d->jid.bare());
		//d->sasl->setAuthzid("infiniti.homelesshackers.org");
	}
	if(user || pass || realm) {
		d->state = NeedParams;
		needAuthParams(user, pass, realm);
	}
	else
		d->sasl->continueAfterParams();
}

void ClientStream::sasl_authCheck(const QString &user, const QString &)
{
//#ifdef XMPP_DEBUG
//	printf("authcheck: [%s], [%s]\n", user.latin1(), authzid.latin1());
//#endif
	QString u = user;
	int n = u.find('@');
	if(n != -1)
		u.truncate(n);
	d->srv.user = u;
	d->sasl->continueAfterAuthCheck();
}

void ClientStream::sasl_authenticated()
{
#ifdef XMPP_DEBUG
	printf("sasl authed!!\n");
#endif
	d->sasl_ssf = d->sasl->ssf();

	if(d->mode == Server) {
		d->srv.setSASLAuthed();
		processNext();
	}
}

void ClientStream::sasl_error(int)
{
//#ifdef XMPP_DEBUG
//	printf("sasl error: %d\n", c);
//#endif
	// has to be auth error
	int x = convertedSASLCond();
	reset();
	d->errCond = x;
	error(ErrAuth);
}

void ClientStream::srvProcessNext()
{
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
}

void ClientStream::doReadyRead()
{
	//QGuardedPtr<QObject> self = this;
	readyRead();
	//if(!self)
	//	return;
	//d->in_rrsig = false;
}

void ClientStream::processNext()
{
	if(d->mode == Server) {
		srvProcessNext();
		return;
	}

	QGuardedPtr<QObject> self = this;

	while(1) {
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
				printf("Error! Code=%d\n", d->client.errorCode);
#endif
				handleError();
				return;
			}
			case CoreProtocol::ESend: {
				QByteArray a = d->client.takeOutgoingData();
#ifdef XMPP_DEBUG
				QCString cs(a.size()+1);
				memcpy(cs.data(), a.data(), a.size());
				printf("Need Send: {%s}\n", cs.data());
#endif
				d->ss->write(a);
				break;
			}
			case CoreProtocol::ERecvOpen: {
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
				printf("Break SASL Success\n");
#endif
				break;
			}
			case CoreProtocol::EReady: {
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
				printf("DocumentClosed\n");
#endif
				reset();
				connectionClosed();
				return;
			}
			case CoreProtocol::EStanzaReady: {
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
				printf("StanzasSent\n");
#endif
				stanzaWritten();
				if(!self)
					return;
				break;
			}
			case CoreProtocol::EClosed: {
#ifdef XMPP_DEBUG
				printf("Closed\n");
#endif
				reset();
				delayedCloseFinished();
				return;
			}
		}
	}
}

bool ClientStream::handleNeed()
{
	int need = d->client.need;
	if(need == CoreProtocol::NNotify) {
		d->notify = d->client.notify;
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
			printf("Need StartTLS\n");
#endif
			d->using_tls = true;
			d->ss->startTLSClient(d->tlsHandler, d->server, d->client.spare);
			return false;
		}
		case CoreProtocol::NSASLFirst: {
#ifdef XMPP_DEBUG
			printf("Need SASL First Step\n");
#endif
			// no SASL plugin?  fall back to Simple SASL
			if(!QCA::isSupported(QCA::CAP_SASL)) {
				// Simple SASL needs MD5.  do we have that either?
				if(!QCA::isSupported(QCA::CAP_MD5))
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
#ifdef XMPP_DEBUG
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
#ifdef XMPP_DEBUG
			printf("Need Password\n");
#endif
			d->state = NeedParams;
			needAuthParams(false, true, false);
			return false;
		}
	}

	return true;
}

int ClientStream::convertedSASLCond() const
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

void ClientStream::doNoop()
{
	if(d->state == Active) {
#ifdef XMPP_DEBUG
		printf("doPing\n");
#endif
		d->client.sendWhitespace();
		processNext();
	}
}

void ClientStream::writeDirect(const QString &s)
{
	if(d->state == Active) {
#ifdef XMPP_DEBUG
		printf("writeDirect\n");
#endif
		d->client.sendDirect(s);
		processNext();
	}
}

void ClientStream::handleError()
{
	int c = d->client.errorCode;
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
				r = GenericAuthError;
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
	}
}

//----------------------------------------------------------------------------
// Debug
//----------------------------------------------------------------------------
Debug::~Debug()
{
}

#ifdef XMPP_TEST
TD::TD()
{
}

TD::~TD()
{
}

void TD::msg(const QString &s)
{
	if(debug_ptr)
		debug_ptr->msg(s);
}

void TD::outgoingTag(const QString &s)
{
	if(debug_ptr)
		debug_ptr->outgoingTag(s);
}

void TD::incomingTag(const QString &s)
{
	if(debug_ptr)
		debug_ptr->incomingTag(s);
}

void TD::outgoingXml(const QDomElement &e)
{
	if(debug_ptr)
		debug_ptr->outgoingXml(e);
}

void TD::incomingXml(const QDomElement &e)
{
	if(debug_ptr)
		debug_ptr->incomingXml(e);
}
#endif
