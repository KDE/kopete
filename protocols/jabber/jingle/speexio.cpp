 /*
  * speexio.cpp - A coder encoder for raw audio and speex.
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include "speexio.h"
#include <KDebug>

class SpeexIO::Private
{
public :
	void *encoder;
	void *decoder;
	SpeexBits encBits;
	SpeexBits decBits;
	int samplingRate;
	int bitRate;
	int frameSize;
	
	QByteArray speexData; //encoded data
	QByteArray rawData; //decoded date
};

SpeexIO::SpeexIO()
 : AbstractIO(), d(new Private())
{
	speex_bits_init(&d->encBits);
	speex_bits_init(&d->decBits);
	
	d->samplingRate = -1;
	d->frameSize = 0;
	d->bitRate = 0;
	qDebug() << "SpeexIO : created.";
}

SpeexIO::~SpeexIO()
{
	speex_bits_destroy(&d->encBits);
	speex_encoder_destroy(d->encoder);
	
	speex_bits_destroy(&d->decBits);
	speex_decoder_destroy(d->decoder);
        delete d;
	kDebug() << "Destroyed SpeexIO";
}

void SpeexIO::setSamplingRate(int sr)
{
	if (d->samplingRate != -1)
	{
		//FIXME:this should simply change the current sampling rate (SPEEX_SET_SAMPLING_RATE)
		//FIXME:is that possible ?
		qDebug() << "Sampling rate already set... Abort";
		return;
	}
	
	switch(sr)
	{
	case 8000 : 
		d->encoder = speex_encoder_init(&speex_nb_mode);
		d->decoder = speex_decoder_init(&speex_nb_mode);
		break;
	case 16000 :
		d->encoder = speex_encoder_init(&speex_wb_mode);
		d->decoder = speex_decoder_init(&speex_wb_mode);
		break;
// Currently not supported, this is not for internet payloads anyway.
/*	case 32000 :
		d->encoder = speex_encoder_init(&speex_uwb_mode);
		d->decoder = speex_decoder_init(&speex_uwb_mode);
		break;
*/
	default :
		return;
	}
	
	d->samplingRate = sr;

	qDebug() << "encoder and decoder initiated.";
}

int SpeexIO::setQuality(int q)
{
	if (d->samplingRate == -1)
		return -1;
	
	int qualityEnc = q;
	if (0 != speex_encoder_ctl(d->encoder, SPEEX_SET_QUALITY, &qualityEnc))
		return -1;
	
	int qualityDec = q;
	if (0 != speex_decoder_ctl(d->decoder, SPEEX_SET_QUALITY, &qualityDec))
		return -1;
	
	if (qualityEnc != qualityDec)
		return -1;
	
	return qualityDec;
}

// Returns a frame size in samples.
int SpeexIO::frameSize()
{
	if (d->samplingRate == -1)
		return -1;

	if (d->frameSize != 0) //if frameSize != 0, we already got it, no need to get it again.
		return d->frameSize;
	
	int fs;
	//Encoder and decoder frame size are the same.
	if (0 != speex_encoder_ctl(d->encoder, SPEEX_GET_FRAME_SIZE, &fs))
		return -1;
	
	return (d->frameSize = fs);
}

// Returns a frame size in bytes.
int SpeexIO::frameSizeBytes()
{
	return (d->samplingRate == 8000 ? 160 * 2 : 320 * 2);
}

bool SpeexIO::start()
{
	if (d->samplingRate == -1)
		return false;
	return true;
}

void SpeexIO::encode(const QByteArray& rawData)
{
	d->speexData.clear();

	if (d->samplingRate == -1 || rawData.size() == 0)
		return;
	
	speex_bits_reset(&d->encBits);
	int ret = speex_encode_int(d->encoder, (short*) rawData.data(), &d->encBits);
	if (ret == 0)
	{
		qDebug() << "Error encoding speex data : frame needs not be transmitted";
		return;
	}
	
	int maxSize = speex_bits_nbytes(&d->encBits);
	d->speexData.resize(maxSize);
	
	int nbBytes = speex_bits_write(&d->encBits, (char*) d->speexData.data(), maxSize);

	emit encoded();
}

QByteArray SpeexIO::encodedData() const
{
	return d->speexData;
}

void SpeexIO::decode(const QByteArray& speexData)
{
	d->rawData.clear();

	if (d->samplingRate == -1 || speexData.size() == 0)
		return;
	
	speex_bits_read_from(&d->decBits, (char*) speexData.data(), speexData.size());

	if (frameSizeBytes() == -1)
		return;
	
	d->rawData.resize(frameSizeBytes());
	int ret = speex_decode_int(d->decoder, &d->decBits, (short*) d->rawData.data());
	if (ret != 0)
	{
		qDebug() << "Error decoding speex data :" << (ret == -1 ? "end of stream" : "corrupt stream");
		return;
	}
	
	emit decoded();
}

QByteArray SpeexIO::decodedData() const
{
	return d->rawData;
}

int SpeexIO::tsValue()
{
	return d->samplingRate / 50;
}

