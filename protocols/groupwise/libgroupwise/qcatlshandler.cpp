// copy; original found in iris/xmpp-core/tlshandler.cpp

#include <qtimer.h>

#include "qca.h"

#include "qcatlshandler.h"

class QCATLSHandler::Private
{
public:
	QCA::TLS *tls;
	int state, err;
};

QCATLSHandler::QCATLSHandler(QCA::TLS *parent)
:TLSHandler(parent)
{
	d = new Private;
	d->tls = parent;
	connect(d->tls, SIGNAL(handshaken()), SLOT(tls_handshaken()));
	connect(d->tls, SIGNAL(readyRead()), SLOT(tls_readyRead()));
	connect(d->tls, SIGNAL(readyReadOutgoing(int)), SLOT(tls_readyReadOutgoing(int)));
	connect(d->tls, SIGNAL(closed()), SLOT(tls_closed()));
	connect(d->tls, SIGNAL(error(int)), SLOT(tls_error(int)));
	d->state = 0;
	d->err = -1;
}

QCATLSHandler::~QCATLSHandler()
{
	delete d;
}

QCA::TLS *QCATLSHandler::tls() const
{
	return d->tls;
}

int QCATLSHandler::tlsError() const
{
	return d->err;
}

void QCATLSHandler::reset()
{
	d->tls->reset();
	d->state = 0;
}

void QCATLSHandler::startClient(const QString &host)
{
	d->state = 0;
	d->err = -1;
	if(!d->tls->startClient(host))
		QTimer::singleShot(0, this, SIGNAL(fail()));
}

void QCATLSHandler::write(const QByteArray &a)
{
	d->tls->write(a);
}

void QCATLSHandler::writeIncoming(const QByteArray &a)
{
	d->tls->writeIncoming(a);
}

void QCATLSHandler::continueAfterHandshake()
{
	if(d->state == 2) {
		success();
		d->state = 3;
	}
}

void QCATLSHandler::tls_handshaken()
{
	d->state = 2;
	tlsHandshaken();
}

void QCATLSHandler::tls_readyRead()
{
	readyRead(d->tls->read());
}

void QCATLSHandler::tls_readyReadOutgoing(int plainBytes)
{
	readyReadOutgoing(d->tls->readOutgoing(), plainBytes);
}

void QCATLSHandler::tls_closed()
{
	closed();
}

void QCATLSHandler::tls_error(int x)
{
	d->err = x;
	d->state = 0;
	fail();
}

#include "qcatlshandler.moc"
