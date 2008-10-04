#include <QDebug>

#include "mediamanager.h"
#include "alsaio.h"

class MediaManager::Private
{
public:
	AlsaIO *alsaIn;
	AlsaIO *alsaOut;

	bool started;

};

MediaManager::MediaManager()
 : d(new Private)
{
	d->alsaIn = new AlsaIO(AlsaIO::Capture, AlsaIO::Signed16Le);
	d->alsaOut = new AlsaIO(AlsaIO::Playback, AlsaIO::Signed16Le);

	d->started = false;

	qDebug() << "Created Media Manager.";
}

MediaManager::~MediaManager()
{
	delete d->alsaIn;
	delete d->alsaOut;
	qDebug() << "Deleted Media Manager.";
}

AlsaIO *MediaManager::alsaIn() const
{
	return d->alsaIn;
}

AlsaIO *MediaManager::alsaOut() const
{
	return d->alsaOut;
}

bool MediaManager::start()
{
	if (d->started)
		return true;
	return (d->started = d->alsaIn->start() && d->alsaOut->start());
}

QByteArray MediaManager::read()
{
	return alsaIn()->data();
}

void MediaManager::write(const QByteArray& data)
{
	alsaOut()->write(data);
}

