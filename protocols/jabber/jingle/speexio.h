/*
 * "" <>
 * speexio.h - Speex audio encoder and decoder.
 * This class is able to encode and decode speex data. It works as an
 * encoder-decoder expecting to have the same type of data (same bitrate,...)
 * for encoding and decoding.
 */

#ifndef SPEEX_IO_H
#define SPEEX_IO_H

#include "abstractio.h"
#include "speex/speex.h"

class SpeexIO : public AbstractIO
{
public:
	SpeexIO();
	virtual ~SpeexIO();

	//void setBitRate();
	
	/**
	 * This is the speex sampling rate :
	 * Possible different sampling rates: 8 kHz (8000), 16 kHz (16000), and 32 kHz (32000).
	 * These are respectively refered to as narrowband, wideband and ultra-wideband.
	 *
	 * FIXME:should take an enum value instead of an int.
	 */
	void setSamplingRate(int sr);

	/**
	 * Set the encoder-decoder output quality.
	 * Returns the actual value set for encoding.
	 * -1 means there was an error setting quality parameter which can either be an error
	 * on the ctl or 2 different values for the decoder and encoder.
	 */
	int setQuality(int q);

	/**
	 * Returns the size of a speex frame (in bytes)
	 */
	int frameSize();

	virtual bool start();
	
	virtual void encode(const QByteArray& data);
	virtual void decode(const QByteArray& data);
	virtual QByteArray encodedData() const;
	virtual QByteArray decodedData() const;
	
	virtual int tsValue();

private:
	class Private;
	Private *d;

};
#endif
