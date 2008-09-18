#ifndef SPEEX_IO_H
#define SPEEX_IO_H

#include <abstractio.h>
#include <speex/speex.h>

class SpeexIO : public AbstractIO
{
	Q_OBJECT
public :
	SpeexIO();
	~SpeexIO();

	virtual void write(const QByteArray& data);
	virtual QByteArray read();

private slots:
	void slotReadyRead();

private :
	SpeexBits bits;
	void *speexEncoder;
};

#endif
