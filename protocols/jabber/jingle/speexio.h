/*
 * speexio.cpp - A coder encoder for raw audio and speex.
 *
 * speexio.h - Speex audio encoder and decoder.
 * This class is able to encode and decode speex data. It works as an
 * encoder-decoder expecting to have the same type of data (same bitrate,...)
 * for encoding and decoding.
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
	 * This sets the speex sampling rate :
	 * Possible different sampling rates: 8 kHz (8000) and 16 kHz (16000).
	 * These are respectively referred to as narrowband and wideband.
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
	 * Returns the size of a speex frame (in samples)
	 */
	int frameSize();
	
	/**
	 * Returns the size of a speex frame (in bytes)
	 */
	virtual int frameSizeBytes();

	/**
	 * Return true if it is ready to start, false if not.
	 */
	virtual bool start();
	
	virtual void encode(const QByteArray& data);
	virtual void decode(const QByteArray& data);
	
	virtual QByteArray encodedData() const;
	virtual QByteArray decodedData() const;
	
	virtual int tsValue();

private:
	class Private;
	Private * const d;

};
#endif
