
#include "abstractio.h"

AbstractIO::AbstractIO()
{
	//Should open and configure Alsa device(s)
	m_alsaIn = new AlsaIO(AlsaIO::Capture);
	m_alsaOut = new AlsaIO(AlsaIO::Playback);
}

AbstractIO::~AbstractIO()
{

}

int AbstractIO::start()
{
	connect(m_alsaIn, SIGNAL(readyRead()), this, SIGNAL(readData()));
	m_alsaIn->start();
	m_alsaOut->start();
	return 0;
}

void AbstractIO::write(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray AbstractIO::read()
{
	kDebug() << "called";
	return m_alsaIn->data();
}

void AbstractIO::setFormat(AlsaIO::Format f)
{
	m_alsaIn->setFormat(f);
	m_alsaOut->setFormat(f);
}
