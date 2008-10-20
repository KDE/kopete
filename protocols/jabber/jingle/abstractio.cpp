#include "abstractio.h"

AbstractIO::AbstractIO()
{

}

AbstractIO::~AbstractIO()
{

}


void AbstractIO::encode(const QByteArray& data)
{
	Q_UNUSED(data)
}

void AbstractIO::decode(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray AbstractIO::encodedData() const
{
	return QByteArray();
}

QByteArray AbstractIO::decodedData() const
{
	return QByteArray();
}

int AbstractIO::tsValue()
{
	return 0;
}
