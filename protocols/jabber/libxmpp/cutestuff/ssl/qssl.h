/*
 * qssl.h - Qt SSL plugin
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

#ifndef QSSL_H
#define QSSL_H

#include<qobject.h>
#include<qvaluelist.h>
#include<qptrlist.h>

#ifdef Q_WS_WIN
#define QSSL_EXPORT extern "C" __declspec(dllexport)
#else
#define QSSL_EXPORT extern "C"
#endif

class QString;
class QDateTime;
class QSSLFilter;
class QSSL;
class QSSLCert;

QSSL_EXPORT QSSL *createQSSL();
QSSL_EXPORT int version();

class QSSL
{
public:
	QSSL() {}
	virtual ~QSSL() {}

	virtual QSSLCert *createCert()=0;
	virtual QSSLFilter *createFilter()=0;
};

struct QSSLCertProperty
{
	QString var;
	QString val;
};

class QSSLCert
{
public:
	enum { NoCert, Valid, HostMismatch, Rejected, Untrusted, SignatureFailed, InvalidCA, InvalidPurpose, SelfSigned, Revoked, PathLengthExceeded, Expired, Unknown };
	QSSLCert() {}
	virtual ~QSSLCert() {}

	virtual bool isNull() const=0;
	virtual bool isValid() const=0;
	virtual int validityResult() const=0;

	virtual QString serialNumber() const=0;
	virtual QDateTime notBefore() const=0;
	virtual QDateTime notAfter() const=0;
	virtual QValueList<QSSLCertProperty> subject() const=0;
	virtual QValueList<QSSLCertProperty> issuer() const=0;
	virtual QString subjectString() const=0;
	virtual QString issuerString() const=0;

	virtual QString toString() const=0;
	virtual bool fromString(const QString &)=0;

	virtual QByteArray toPEM() const=0;
};

class QSSLFilter : public QObject
{
	Q_OBJECT

public:
	QSSLFilter() {}

	virtual void reset()=0;
	virtual bool begin(const QString &host, const QPtrList<QSSLCert> &)=0;

	// send data
	virtual void send(const QByteArray &)=0;
	// check/recv data
	virtual bool isRecvData()=0;
	virtual QByteArray recv()=0;

	// pass incoming socket data to this function
	virtual void putIncomingSSLData(const QByteArray &)=0;
	// check/read outgoing socket data with this function
	virtual bool isOutgoingSSLData()=0;
	virtual QByteArray getOutgoingSSLData()=0;

	// cert related
	virtual const QSSLCert & peerCertificate() const=0;

signals:
	void handshaken(bool);
	void readyRead();
	void outgoingSSLDataReady();
};

#endif
