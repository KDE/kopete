/*
 * qssl.cpp - Qt OpenSSL plugin
 * Copyright (C) 2001-2003  Justin Karneges
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

#include"qssl_p.h"

#include<qstring.h>
#include<qstringlist.h>
#include<qcstring.h>
#include<qdatetime.h>
#include<qregexp.h>

//! \brief plugin hook
QSSL *createQSSL()
{
	return new _QSSL;
}

int version()
{
	return 2;
}

//! \class QSSL qssl.h
//! \brief QT OpenSSL plug-in

//----------------------------------------------------------------------------
// QSSL
//----------------------------------------------------------------------------
//! \brief Creates QSSL object
_QSSL::_QSSL()
{
	SSL_library_init();
	SSL_load_error_strings();
}

//! \brief Destroys QSSL object
_QSSL::~_QSSL()
{
	ERR_free_strings();
	ERR_remove_state(0);
}

QSSLCert *_QSSL::createCert()
{
	return new _QSSLCert;
}

//! \brief gives you access to QSSLFilter
QSSLFilter *_QSSL::createFilter()
{
	return new _QSSLFilter;
}


//----------------------------------------------------------------------------
// QSSLCert
//----------------------------------------------------------------------------
static QByteArray base64encode(const QByteArray &s)
{
	int i;
	int len = s.size();
	char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	int a, b, c;

	QByteArray p((len+2)/3*4);
	int at = 0;
	for( i = 0; i < len; i += 3 ) {
		a = ((unsigned char)s[i] & 3) << 4;
		if(i + 1 < len) {
			a += (unsigned char)s[i + 1] >> 4;
			b = ((unsigned char)s[i + 1] & 0xF) << 2;
			if(i + 2 < len) {
				b += (unsigned char)s[i + 2] >> 6;
				c = (unsigned char)s[i + 2] & 0x3F;
			}
			else
				c = 64;
		}
		else
			b = c = 64;

		p[at++] = tbl[(unsigned char)s[i] >> 2];
		p[at++] = tbl[a];
		p[at++] = tbl[b];
		p[at++] = tbl[c];
	}
	return p;
}

QByteArray base64decode(const QByteArray &s)
{
	// return value
	QByteArray p;

	// -1 specifies invalid
	// 64 specifies eof
	// everything else specifies data

	char tbl[] = {
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,64,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	};

	// this should be a multiple of 4
	int len = s.size();

	if(len % 4)
		return p;

	p.resize(len / 4 * 3);

	int i;
	int at = 0;

	int a, b, c, d;
	c = d = 0;

	for( i = 0; i < len; i += 4 ) {
		a = tbl[s[i]];
		b = tbl[s[i + 1]];
		c = tbl[s[i + 2]];
		d = tbl[s[i + 3]];
		if((a == 64 || b == 64) || (a < 0 || b < 0 || c < 0 || d < 0)) {
			p.resize(0);
			return p;
		}
		p[at++] = ((a & 0x3F) << 2) | ((b >> 4) & 0x03);
		p[at++] = ((b & 0x0F) << 4) | ((c >> 2) & 0x0F);
		p[at++] = ((c & 0x03) << 6) | ((d >> 0) & 0x3F);
	}

	if(c & 64)
		p.resize(at - 2);
	else if(d & 64)
		p.resize(at - 1);

	return p;
}

static QValueList<QSSLCertProperty> nameToProperties(X509_NAME *name)
{
	QValueList<QSSLCertProperty> list;

	for(int n = 0; n < X509_NAME_entry_count(name); ++n) {
		X509_NAME_ENTRY *ne = X509_NAME_get_entry(name, n);
		QSSLCertProperty p;

		ASN1_OBJECT *ao = X509_NAME_ENTRY_get_object(ne);
		int nid = OBJ_obj2nid(ao);
		if(nid == NID_undef)
			continue;
		p.var = OBJ_nid2sn(nid);

		ASN1_STRING *as = X509_NAME_ENTRY_get_data(ne);
		QCString c;
		c.resize(as->length+1);
		strncpy(c.data(), (char *)as->data, as->length);
		p.val = QString::fromLatin1(c);
		list += p;
	}

	return list;
}

// (taken from kdelibs) -- Justin
//
// This code is mostly taken from OpenSSL v0.9.5a
// by Eric Young
QDateTime ASN1_UTCTIME_QDateTime(ASN1_UTCTIME *tm, int *isGmt)
{
	QDateTime qdt;
	char *v;
	int gmt=0;
	int i;
	int y=0,M=0,d=0,h=0,m=0,s=0;
	QDate qdate;
	QTime qtime;

	i = tm->length;
	v = (char *)tm->data;

	if (i < 10) goto auq_err;
	if (v[i-1] == 'Z') gmt=1;
	for (i=0; i<10; i++)
		if ((v[i] > '9') || (v[i] < '0')) goto auq_err;
	y = (v[0]-'0')*10+(v[1]-'0');
	if (y < 50) y+=100;
	M = (v[2]-'0')*10+(v[3]-'0');
	if ((M > 12) || (M < 1)) goto auq_err;
	d = (v[4]-'0')*10+(v[5]-'0');
	h = (v[6]-'0')*10+(v[7]-'0');
	m =  (v[8]-'0')*10+(v[9]-'0');
	if (    (v[10] >= '0') && (v[10] <= '9') &&
		(v[11] >= '0') && (v[11] <= '9'))
		s = (v[10]-'0')*10+(v[11]-'0');

	// localize the date and display it.
	qdate.setYMD(y+1900, M, d);
	qtime.setHMS(h,m,s);
	qdt.setDate(qdate); qdt.setTime(qtime);
auq_err:
	if (isGmt) *isGmt = gmt;
	return qdt;
}

// (adapted from kdelibs) -- Justin
static bool cnMatchesAddress(const QString &_cn, const QString &peerHost)
{
	QString cn = _cn;
	QRegExp rx;

	// Check for invalid characters
	if(QRegExp("[^a-zA-Z0-9\\.\\*\\-]").search(cn) >= 0)
		return false;

	// Domains can legally end with '.'s.  We don't need them though.
	while(cn.endsWith("."))
		cn.truncate(cn.length()-1);

	// Do not let empty CN's get by!!
	if(cn.isEmpty())
		return false;

	// Check for IPv4 address
	rx.setPattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
	if(rx.exactMatch(peerHost))
		return peerHost == cn;

	// Check for IPv6 address here...
	rx.setPattern("^\\[.*\\]$");
	if(rx.exactMatch(peerHost))
		return peerHost == cn;

	if(cn.contains('*')) {
		// First make sure that there are at least two valid parts
		// after the wildcard (*).
		QStringList parts = QStringList::split('.', cn, false);
    
		while(parts.count() > 2)
			parts.remove(parts.begin());

		if(parts.count() != 2) {
			return false;  // we don't allow *.root - that's bad
		}

		if(parts[0].contains('*') || parts[1].contains('*')) {
			return false;
		}

		// RFC2818 says that *.example.com should match against
		// foo.example.com but not bar.foo.example.com
		// (ie. they must have the same number of parts)
		if(QRegExp(cn, false, true).exactMatch(peerHost) &&
			parts.count() == QStringList::split('.', peerHost, false).count())
			return true;

		return false;
	}

	// We must have an exact match in this case (insensitive though)
	// (note we already did .lower())
	if(cn == peerHost)
		return true;
	return false;
}

class _QSSLCert::Private
{
public:
	Private() {}

	X509 *x; // openssl internal representation
	QByteArray dat;
	QString serial;
	QDateTime notBefore, notAfter;
	QString subjectString, issuerString;
	QValueList<QSSLCertProperty> subject, issuer;

	int validityResult;
};

_QSSLCert::_QSSLCert()
{
	d = new Private;
	d->x = 0;
	d->validityResult = NoCert;
}

_QSSLCert::_QSSLCert(const _QSSLCert &from)
:QSSLCert()
{
	d = new Private;
	d->x = 0;
	d->validityResult = NoCert;
	*this = from;
}

_QSSLCert & _QSSLCert::operator=(const _QSSLCert &from)
{
	reset();
	*d = *from.d;
	if(d->x)
		++d->x->references; // bump the reference count
	d->dat.detach(); // dup the QByteArray
	return *this;
}

_QSSLCert::~_QSSLCert()
{
	reset();
	delete d;
}

void _QSSLCert::reset()
{
	if(d->x) {
		X509_free(d->x);
		d->x = 0;
	}
	d->validityResult = NoCert;
}

X509 *_QSSLCert::toX509() const
{
	return d->x;
}

void _QSSLCert::fromX509(X509 *x)
{
	reset();

	// extract the raw data
	d->x = x;
	++d->x->references; // bump the reference count
	int len = i2d_X509(x, NULL);
	QByteArray dat(len);
	unsigned char *p = (unsigned char *)dat.data();
	i2d_X509(x, &p);
	d->dat = dat;

	// serial number
	ASN1_INTEGER *ai = X509_get_serialNumber(x);
	if(ai) {
		char *rep = i2s_ASN1_INTEGER(NULL, ai);
		d->serial = rep;
		OPENSSL_free(rep);
	}

	// validity dates
	d->notBefore = ASN1_UTCTIME_QDateTime(X509_get_notBefore(x), NULL);
	d->notAfter = ASN1_UTCTIME_QDateTime(X509_get_notAfter(x), NULL);

	// extract the subject/issuer strings
	X509_NAME *sn = X509_get_subject_name(x);
	X509_NAME *in = X509_get_issuer_name(x);
	char buf[1024];
	X509_NAME_oneline(sn, buf, 1024);
	d->subjectString = buf;
	X509_NAME_oneline(in, buf, 1024);
	d->issuerString = buf;

	// extract the subject/issuer contents
	d->subject = nameToProperties(sn);
	d->issuer  = nameToProperties(in);
}

void _QSSLCert::setValidityResult(int r)
{
	d->validityResult = r;
}

bool _QSSLCert::isNull() const
{
	return (d->x ? false: true);
}

bool _QSSLCert::isValid() const
{
	return (validityResult() == Valid ? true: false);
}

int _QSSLCert::validityResult() const
{
	return d->validityResult;
}

QString _QSSLCert::serialNumber() const
{
	return d->serial;
}

QDateTime _QSSLCert::notBefore() const
{
	return d->notBefore;
}

QDateTime _QSSLCert::notAfter() const
{
	return d->notAfter;
}

QValueList<QSSLCertProperty> _QSSLCert::subject() const
{
	return d->subject;
}

QValueList<QSSLCertProperty> _QSSLCert::issuer() const
{
	return d->issuer;
}

QString _QSSLCert::subjectString() const
{
	return d->subjectString;
}

QString _QSSLCert::issuerString() const
{
	return d->issuerString;
}

QString _QSSLCert::toString() const
{
	QByteArray enc = base64encode(d->dat);
	QCString c;
	c.resize(enc.size()+1);
	memcpy(c.data(), enc.data(), enc.size());
	return QString::fromLatin1(c);
}

bool _QSSLCert::fromString(const QString &str)
{
	QCString cs = str.latin1();
	QByteArray enc(cs.length());
	memcpy(enc.data(), cs.data(), enc.size());
	QByteArray dat = base64decode(enc);
	unsigned char *p = (unsigned char *)dat.data();
	X509 *x = d2i_X509(NULL, &p, dat.size());
	if(!x)
		return false;
	fromX509(x);
	return true;
}

QByteArray _QSSLCert::toPEM() const
{
	QString str = toString();
	unsigned int len = str.length() - 1;
	for(unsigned int i = 0; i < len/64; i++)
		str.insert(64*(i+1)+i, '\n');

	QString out;
	out += "-----BEGIN CERTIFICATE-----\n";
	out += str + '\n';
	out += "-----END CERTIFICATE-----\n";

	QCString cs = out.latin1();
	QByteArray buf(cs.length());
	memcpy(buf.data(), cs.data(), buf.size());
	return buf;
}

bool _QSSLCert::matchesAddress(const QString &realHost) const
{
	QString peerHost = realHost.stripWhiteSpace();
	while(peerHost.endsWith("."))
		peerHost.truncate(peerHost.length()-1);
	peerHost = peerHost.lower();

	for(QValueList<QSSLCertProperty>::ConstIterator it = d->subject.begin(); it != d->subject.end(); ++it) {
		if((*it).var == "CN") {
			if(cnMatchesAddress((*it).val.stripWhiteSpace().lower(), peerHost))
				return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------
// QSSLFilter
//----------------------------------------------------------------------------
//! \if _hide_doc_
class QSSLFilterPrivate
{
public:
	QSSLFilterPrivate() {}

	int mode;
	QByteArray sendQueue, recvQueue;

	SSL *ssl;
	SSL_METHOD *method;
	SSL_CTX *context;
	BIO *rbio, *wbio;

	_QSSLCert cert;
	QString host;
};
//! \endif

//! \class QSSLFilter qssl.h
//! \brief QSSLFilter - Main SSL wrapper
//!
//! This class should be used if you want basic ssl support.
//!

//! \fn void QSSLFilter::handshaken()
//! \brief Signals letting you know the initialization has finished
//!
//! This is when you put those pad locks at the corner of your application ;D

//! \fn void QSSLFilter::readyRead()
//! \brief Signals non-encrypted data is ready for read

//! \fn void QSSLFilter::outgoingSSLDataReady()
//! \brief Singals encrypted data is ready for read

//! \fn void QSSLFilter::error(bool isWarning, const QString &)
//! \brief Signals any error
//!
//! the isWarning = 1 if this is a warning information. You may choose to
//!   procede with the process if a warning is signaled.
//!
//! \param bool - 1 if warning and 0 if error
//! \param QString - warning/error message
//!
//! \sa continueAfterWarning()

//! \brief Creates QSSLFilter object
_QSSLFilter::_QSSLFilter()
{
	d = new QSSLFilterPrivate;
	d->ssl = 0;
	d->context = 0;
}

//! \brief Destroys Object
_QSSLFilter::~_QSSLFilter()
{
	reset();
	delete d;
}

//! \brief Reset QSSLFilter's vars.
//!
//! Reset all the variable inside QSSLFilter.
//!
//! Use this function if you want to establish a new connection.
void _QSSLFilter::reset()
{
	if(d->ssl) {
		SSL_shutdown(d->ssl);
		SSL_free(d->ssl);
		d->ssl = 0;
	}
	if(d->context) {
		SSL_CTX_free(d->context);
		d->context = 0;
	}

	d->sendQueue.resize(0);
	d->recvQueue.resize(0);
	d->mode = Idle;
}

const QSSLCert & _QSSLFilter::peerCertificate() const
{
	return d->cert;
}

//! \brief Initializes SSL filtering.
//!
//! Initializes SSL filtering.
//! \warning Make sure you establish a socket connection before you call this
//! function!
//! \return true - if success\n
//! false - if anything goes wrong
bool _QSSLFilter::begin(const QString &host, const QPtrList<QSSLCert> &list)
{
	reset();

	d->ssl = 0;
	d->method = 0;
	d->context = 0;

	// get our handles
	d->method = TLSv1_client_method();
	if(!d->method) {
		reset();
		return false;
	}
	d->context = SSL_CTX_new(d->method);
	if(!d->context) {
		reset();
		return false;
	}

	// load the certs
	if(!list.isEmpty()) {
		X509_STORE *store = SSL_CTX_get_cert_store(d->context);
		QPtrListIterator<QSSLCert> it(list);
		for(_QSSLCert *cert; (cert = (_QSSLCert *)it.current()); ++it)
			X509_STORE_add_cert(store, cert->toX509());
	}

	d->ssl = SSL_new(d->context);
	if(!d->ssl) {
		reset();
		return false;
	}
	SSL_set_ssl_method(d->ssl, d->method); // can this return error?

	// setup the memory bio
	// these could error out, but i don't see how
	d->rbio = BIO_new(BIO_s_mem());
	d->wbio = BIO_new(BIO_s_mem());

	// these always work
	SSL_set_bio(d->ssl, d->rbio, d->wbio);

	d->host = host;
	d->mode = Connect;

	sslUpdate();

	return true;
}

//! \brief Send information to decode
//!
//! Send any information you want to decode through this method.
//! \param QByteArray - data to be encrypted
void _QSSLFilter::putIncomingSSLData(const QByteArray &a)
{
	BIO_write(d->rbio, a.data(), a.size());
	sslUpdate();
}

//! \brief Tells you if any data is pending.
bool _QSSLFilter::isOutgoingSSLData()
{
	return (BIO_pending(d->wbio) > 0) ? true: false;
}

//! \brief Returns ssl encrypted data.
//!
//! This is what you want to send out your socket in most cases.
//! \return QByteArray - ssl encrypted data
QByteArray _QSSLFilter::getOutgoingSSLData()
{
	QByteArray a;

	int size = BIO_pending(d->wbio);
	if(size <= 0)
		return a;
	a.resize(size);

	int r = BIO_read(d->wbio, a.data(), size);
	if(r <= 0) {
		a.resize(0);
		return a;
	}
	if(r != size)
		a.resize(r);

	return a;
}

int _QSSLFilter::doConnect()
{
	int ret = SSL_connect(d->ssl);
	if(ret < 0) {
		int x = SSL_get_error(d->ssl, ret);
		if(x == SSL_ERROR_WANT_CONNECT || x == SSL_ERROR_WANT_READ || x == SSL_ERROR_WANT_WRITE)
			return TryAgain;
		else
			return Error;
	}
	else if(ret == 0)
		return Error;
	return Success;
}

int _QSSLFilter::doHandshake()
{
	int ret = SSL_do_handshake(d->ssl);
	if(ret < 0) {
		int x = SSL_get_error(d->ssl, ret);
		if(x == SSL_ERROR_WANT_READ || x == SSL_ERROR_WANT_WRITE)
			return TryAgain;
		else
			return Error;
	}
	else if(ret == 0)
		return Error;
	return Success;
}

void _QSSLFilter::sslUpdate()
{
	if(d->mode == Idle)
		return;

	if(d->mode == Connect) {
		int ret = doConnect();
		if(ret == Success) {
			d->mode = Handshake;
		}
		else if(ret == Error) {
			reset();
			handshaken(false);
			return;
		}
	}

	if(d->mode == Handshake) {
		int ret = doHandshake();
		if(ret == Success) {
			// verify the certificate
			int code = QSSLCert::Unknown;
			X509 *x = SSL_get_peer_certificate(d->ssl);
			if(x) {
				d->cert.fromX509(x);
				X509_free(x);
				int ret = SSL_get_verify_result(d->ssl);
				if(ret == X509_V_OK) {
					if(d->cert.matchesAddress(d->host))
						code = QSSLCert::Valid;
					else
						code = QSSLCert::HostMismatch;
				}
				else
					code = resultToCV(ret);
			}
			else {
				d->cert = _QSSLCert();
				code = QSSLCert::NoCert;
			}
			d->cert.setValidityResult(code);

			d->mode = Active;
			handshaken(true);
		}
		else if(ret == Error) {
			reset();
			handshaken(false);
			return;
		}
	}

	if(isOutgoingSSLData()) {
		outgoingSSLDataReady();
	}

	// try to read incoming unencrypted data
	sslReadAll();

	if(isRecvData())
		readyRead();
}

//! \brief send non-encrypted data to object
//!
//! \param QByteArray - non-encrypted data
void _QSSLFilter::send(const QByteArray &a)
{
	if(d->mode != Active)
		return;

	int oldsize = d->sendQueue.size();
	d->sendQueue.resize(oldsize + a.size());
	memcpy(d->sendQueue.data() + oldsize, a.data(), a.size());

	processSendQueue();
}

//! \brief Data waiting for you to take
//!
//! Tells you if there is any encrypted data in the queue for you to take.
//! \return true - if data is queued\n
//!   false - if no data is queued
bool _QSSLFilter::isRecvData()
{
	return (d->recvQueue.size() > 0) ? true: false;
}

void _QSSLFilter::sslReadAll()
{
	QByteArray a;

	while(1) {
		a.resize(4096);
		int x = SSL_read(d->ssl, a.data(), a.size());
		if(x <= 0)
			break;

		if(x != (int)a.size())
			a.resize(x);

		int oldsize = d->recvQueue.size();
		d->recvQueue.resize(oldsize + a.size());
		memcpy(d->recvQueue.data() + oldsize, a.data(), a.size());
	}
}

//! \brief Get SSL decrypted data.
//!
//! \return QByteArray - SSL decrypted data
QByteArray _QSSLFilter::recv()
{
	QByteArray a = d->recvQueue;
	a.detach();
	d->recvQueue.resize(0);
	return a;
}

void _QSSLFilter::processSendQueue()
{
	if(d->sendQueue.size() > 0) {
		SSL_write(d->ssl, d->sendQueue.data(), d->sendQueue.size());
		d->sendQueue.resize(0);
		sslUpdate();
	}
}

int _QSSLFilter::resultToCV(int ret) const
{
	int rc;

	switch(ret) {
		case X509_V_ERR_CERT_REJECTED:
			rc = QSSLCert::Rejected;
			break;
		case X509_V_ERR_CERT_UNTRUSTED:
			rc = QSSLCert::Untrusted;
			break;
		case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
		case X509_V_ERR_CRL_SIGNATURE_FAILURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			rc = QSSLCert::SignatureFailed;
			break;
		case X509_V_ERR_INVALID_CA:
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
			rc = QSSLCert::InvalidCA;
			break;
		case X509_V_ERR_INVALID_PURPOSE:
			rc = QSSLCert::InvalidPurpose;
			break;
		case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			rc = QSSLCert::SelfSigned;
			break;
		case X509_V_ERR_CERT_REVOKED:
			rc = QSSLCert::Revoked;
			break;
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
			rc = QSSLCert::PathLengthExceeded;
			break;
		case X509_V_ERR_CERT_NOT_YET_VALID:
		case X509_V_ERR_CERT_HAS_EXPIRED:
		case X509_V_ERR_CRL_NOT_YET_VALID:
		case X509_V_ERR_CRL_HAS_EXPIRED:
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
		case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
			rc = QSSLCert::Expired;
			break;
		case X509_V_ERR_APPLICATION_VERIFICATION:
		case X509_V_ERR_OUT_OF_MEM:
		case X509_V_ERR_UNABLE_TO_GET_CRL:
		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
		default:
			rc = QSSLCert::Unknown;
			break;
	}
	return rc;
}

#include "qssl.moc"
