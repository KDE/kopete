#ifndef ABSTRACT_IO_H
#define ABSTRACT_IO_H

#include <QObject>

#include "alsaio.h"

class AbstractIO : public QObject
{
	Q_OBJECT
public :
	// This constructor prepares all necessary operations to use
	// Alsa (Capture and Playback, there is no direction here,
	// both directions will be managed by the same instance.)
	AbstractIO();
	~AbstractIO();

	virtual void write(const QByteArray& data);
	virtual QByteArray read();

	int start();

	AlsaIO *alsaIn() const {return m_alsaIn;}
	AlsaIO *alsaOut() const {return m_alsaOut;}

	void setFormat(AlsaIO::Format f);

signals:
	void readyRead();

private:
	AlsaIO *m_alsaIn;
	AlsaIO *m_alsaOut;
};

#endif
