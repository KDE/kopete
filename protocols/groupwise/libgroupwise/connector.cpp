#include "connector.h"

Connector::Connector(QObject *parent)
:QObject(parent)
{
	setUseSSL(false);
	setPeerAddressNone();
}

Connector::~Connector()
{
}

bool Connector::useSSL() const
{
	return ssl;
}

bool Connector::havePeerAddress() const
{
	return haveaddr;
}

QHostAddress Connector::peerAddress() const
{
	return addr;
}

Q_UINT16 Connector::peerPort() const
{
	return port;
}

void Connector::setUseSSL(bool b)
{
	ssl = b;
}

void Connector::setPeerAddressNone()
{
	haveaddr = false;
	addr = QHostAddress();
	port = 0;
}

void Connector::setPeerAddress(const QHostAddress &_addr, Q_UINT16 _port)
{
	haveaddr = true;
	addr = _addr;
	port = _port;
}

#include "connector.moc"
