#ifndef SPEEX_IO_H
#define SPEEX_IO_H

#include <abstractio.h>
#include <speex/speex.h>

class SpeexIO : public AbstractIO
{
	Q_OBJECT
public :
	SpeexIO(int samplingRate);
	~SpeexIO();

	virtual void write(const QByteArray& data);
	virtual QByteArray read();
	virtual int start();

private slots:
	void slotReadyRead();
	void slotBytesWritten();

private :
	SpeexBits encodeBits;
	SpeexBits decodeBits;
	void *speexEncoder;
	void *speexDecoder;
	int decoderFrameSize;
	QByteArray speexData;
	QByteArray rawData;

	AlsaIO *m_alsaIn;
	AlsaIO *m_alsaOut;
};

#endif
