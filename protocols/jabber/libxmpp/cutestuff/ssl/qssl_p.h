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

#ifndef QSSL_P_H
#define QSSL_P_H

#include"qssl.h"

#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/x509.h>
#include<openssl/x509v3.h>

class _QSSL : public QSSL
{
public:
	_QSSL();
	~_QSSL();

	QSSLCert *createCert();
	QSSLFilter *createFilter();
//! \if _hide_doc_
private:
	class QSSLPrivate *d;
//! \endif _hide_doc_
};

class _QSSLCert : public QSSLCert
{
public:
	_QSSLCert();
	_QSSLCert(const _QSSLCert &);
	_QSSLCert & operator=(const _QSSLCert &);
	~_QSSLCert();

	X509 *toX509() const;
	void fromX509(X509 *);
	void setValidityResult(int);
	bool matchesAddress(const QString &) const;

	virtual bool isNull() const;
	virtual bool isValid() const;
	virtual int validityResult() const;

	virtual QString serialNumber() const;
	virtual QDateTime notBefore() const;
	virtual QDateTime notAfter() const;
	virtual QValueList<QSSLCertProperty> subject() const;
	virtual QValueList<QSSLCertProperty> issuer() const;
	virtual QString subjectString() const;
	virtual QString issuerString() const;

	virtual QString toString() const;
	virtual bool fromString(const QString &);

	virtual QByteArray toPEM() const;

private:
	class Private;
	Private *d;

	void reset();
};

class _QSSLFilter : public QSSLFilter
{
	Q_OBJECT

public:
	_QSSLFilter();
	~_QSSLFilter();

	void reset();
	bool begin(const QString &host, const QPtrList<QSSLCert> &);

	// send data
	void send(const QByteArray &);
	// check/recv data
	bool isRecvData();
	QByteArray recv();

	// pass incoming socket data to this function
	void putIncomingSSLData(const QByteArray &);
	// check/read outgoing socket data with this function
	bool isOutgoingSSLData();
	QByteArray getOutgoingSSLData();

	// cert related
	virtual const QSSLCert & peerCertificate() const;

private slots:
	void sslUpdate();

private:
	enum { Success, TryAgain, Error };
	enum { Idle, Connect, Handshake, Active };

	int doConnect();
	int doHandshake();

	void finishHandshake();
	void processSendQueue();
	void sslReadAll();
	void doError();
	void doWarning();
	int resultToCV(int) const;

	//! \if _hide_doc_
	class QSSLFilterPrivate *d;
	//! \endif
};

#endif
