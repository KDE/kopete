/*
 * Copy of QCATLSHandler class found in xmpp.h
 * Copyright (C) 2003  Justin Karneges
 */


#ifndef GWQCATLSHANDLER_H
#define GWQCATLSHANDLER_H

//#include <qtimer.h>
#include "tlshandler.h"

class QCA::TLS;

class QCATLSHandler : public TLSHandler
{
	Q_OBJECT
public:
	QCATLSHandler(QCA::TLS *parent);
	~QCATLSHandler();

	QCA::TLS *tls() const;
	int tlsError() const;

	void reset();
	void startClient(const QString &host);
	void write(const QByteArray &a);
	void writeIncoming(const QByteArray &a);

signals:
	void tlsHandshaken();

public slots:
	void continueAfterHandshake();

private slots:
	void tls_handshaken();
	void tls_readyRead();
	void tls_readyReadOutgoing(int);
	void tls_closed();
	void tls_error(int);

private:
	class Private;
	Private *d;
};

#endif
