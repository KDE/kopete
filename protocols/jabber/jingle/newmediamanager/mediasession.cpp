#include <QDebug>

#include "mediasession.h"
#include "mediamanager.h"
#include "abstractio.h"
#include "speexio.h"

class MediaSession::Private
{
public :
	AbstractIO *plugin;
	MediaManager *mediaManager;
	QString codecName;
	QByteArray encodedData;
};

MediaSession::MediaSession(MediaManager *mm, const QString& codecName)
 : d(new Private())
{
	d->mediaManager = mm;
	d->codecName = codecName;

	if (d->codecName == "speex")
		d->plugin = new SpeexIO();

	qDebug() << "Created Media Session for codec" << codecName;
}

MediaSession::~MediaSession()
{
	delete d->plugin;
	qDebug() << "Deleted Media Session";
}

void MediaSession::setSamplingRate(int sr)
{
	static_cast<SpeexIO*>(d->plugin)->setSamplingRate(sr);
}

void MediaSession::setQuality(int q)
{
	static_cast<SpeexIO*>(d->plugin)->setQuality(q);
}

bool MediaSession::start()
{
	connect((QObject*) d->mediaManager->alsaIn(), SIGNAL(readyRead()), (QObject*) this, SLOT(slotReadyRead()));
	connect((QObject*) d->plugin, SIGNAL(encoded()), (QObject*) this, SLOT(slotEncoded()));
	connect((QObject*) d->plugin, SIGNAL(decoded()), (QObject*) this, SLOT(slotDecoded()));

	return d->mediaManager->start() && d->plugin->start();
}

void MediaSession::write(const QByteArray& sData)
{
	//decoding speex data.
	d->plugin->decode(sData);
}

void MediaSession::slotReadyRead()
{
	//qDebug() << "Ready read";
	d->plugin->encode(d->mediaManager->read());
}

void MediaSession::slotEncoded()
{
	//d->encodedData.clear();
	d->encodedData = d->plugin->encodedData(); //FIXME:what about this QByteArray lifetime ?
	//FIXME:speexData lifetime is until encode() is called again.
	
	//qDebug() << "speexData =" << d->encodedData.toBase64() << "(" << d->encodedData.size() << "bytes)";
	
	emit readyRead(); // Encoded data is ready to be read and sent over the network.
}

QByteArray MediaSession::read() const
{
	return d->encodedData;
}

void MediaSession::slotDecoded()
{
	QByteArray rawData = d->plugin->decodedData(); //FIXME:what about this QByteArray lifetime ?
	if (rawData.isNull())
	{
		qDebug() << "rawData is NULL !";
		return;
	}
	//MediaManager always writes and reads from Alsa device(s)
	//qDebug() << "rawData =" << rawData.toBase64() << "(" << rawData.size() << "bytes)";
	d->mediaManager->write(rawData);
}
