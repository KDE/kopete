#include "abstractio.h"

AbstractIO::AbstractIO()
{
	//Should open and configure Alsa device(s)
	kDebug() << "Create AbstractIO";
}

AbstractIO::~AbstractIO()
{

}

int AbstractIO::start()
{
	return 0;
}

void AbstractIO::write(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray AbstractIO::read()
{
	kDebug() << "called";
//	return m_alsaIn->data();
	return QByteArray();
}

void AbstractIO::setFormat(AlsaIO::Format f)
{
	Q_UNUSED(f)
//	m_alsaIn->setFormat(f);
//	m_alsaOut->setFormat(f);
}
