/*
    Kopete Yahoo Protocol
    Handles image conversion of the webcam image, as used by Yahoo.

    Copyright (c) 2011 Cristi Posoiu <cristi.posoiu@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "webcamimgformat.h"

#include "yahootypes.h"

#include <QObject>
#include <QPixmap>
#include <QByteArray>
#include <QImage>
#include <QBuffer>
#include <QIODevice>

#include <kdebug.h>

#include <jasper/jasper.h>

bool jasperConvert(jas_image_t* &image, jas_stream_t* &out, const char* data, unsigned size, int outfmt, const char* outopts)
{
	jas_stream_t *in;

	// kDebug(YAHOO_RAW_DEBUG) << "Got data - size=" << size;

	if(!(in = jas_stream_memopen(const_cast<char*>(data), size)))
	{
		kDebug(YAHOO_RAW_DEBUG) << "Could not open jasper input stream";
		return false;
	}

	int infmt;
	infmt = jas_image_getfmt(in);

	if (infmt < 0)
	{
		jas_stream_close(in);
		kDebug(YAHOO_RAW_DEBUG) << "Failed to recognize input webcam image format";
		return false;
	}

	if (!(image = jas_image_decode(in, infmt, 0)))
	{
		kDebug(YAHOO_RAW_DEBUG) << "Unable to decode image";
		jas_stream_close(in);
		return false;
	}
	/* kDebug(YAHOO_RAW_DEBUG) << "jasper: decoded image: " << jas_image_width(image) << "x" << jas_image_height(image) << " bytes: " <<
		jas_image_rawsize(image) << " components:" << jas_image_numcmpts(image);
	*/

	char* out_img = NULL;
	if(!(out = jas_stream_memopen(out_img, 0)))
	{
		kDebug(YAHOO_RAW_DEBUG) << "Could not open output stream";
		jas_stream_close(in);
		return false;
	}

	if (jas_image_encode(image, out, outfmt, const_cast<char*>(outopts)))
	{
		kDebug(YAHOO_RAW_DEBUG) << "Unable to convert image";
		jas_stream_close(in);
		jas_stream_close(out);
		jas_image_destroy(image);
		return false;
	}
	jas_stream_flush(out);
	jas_stream_close(in);
	return true;
}

WebcamImgFormat::WebcamImgFormat()
	: initOk(false)
{
	int err = jas_init();
	if (err)
	{
		kDebug(YAHOO_RAW_DEBUG) << "Unable to initialize jasper library: code=" << err;
		return;
	}

	int fmt_id = jas_image_strtofmt(const_cast<char*>("pnm"));
	QString formats;

	if (fmt_id >= 0)
	{
		// want to not loose time w/ intermediary conversions/compressions
		formats = "PNM/PPM";
		strncpy(forYahooFmtQt, "PPM", sizeof(forYahooFmtQt) / sizeof(forYahooFmtQt[0])); // QT format
		fromYahooFmtID = fmt_id;
	} else if ((fmt_id = jas_image_strtofmt(const_cast<char*>("png"))) >= 0)
	{
		// png, but we're going to use quality = 100 anyway, so that we don't loose time w/ compression alg.
		formats = "PNG";
		strncpy(forYahooFmtQt, "PNG", sizeof(forYahooFmtQt) / sizeof(forYahooFmtQt[0])); // QT format
		fromYahooFmtID = fmt_id;
	} else if ((fmt_id = jas_image_strtofmt(const_cast<char*>("jpg"))) >= 0)
	{
		// this one is also degrading the pic :-(
		formats = "JPG";
		strncpy(forYahooFmtQt, "JPG", sizeof(forYahooFmtQt) / sizeof(forYahooFmtQt[0])); // QT format
		fromYahooFmtID = fmt_id;
	} else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Couldn't find a reasonable intermerdiary image format (ppm, png,jpg)";
		return;
	}
	forYahooFmtQt[sizeof(forYahooFmtQt) / sizeof(forYahooFmtQt[0]) - 1] = '\0'; // due to the strncpy above

	jpcFmtID = jas_image_strtofmt(const_cast<char*>("jpc"));
	if (jpcFmtID < 0)
	{
		kDebug(YAHOO_RAW_DEBUG) << "library does not support the needed JPEG2000 format";
		return;
	}

	kDebug(YAHOO_RAW_DEBUG) << "Will use intermediary image format " << formats;
	initOk = true;
}

bool WebcamImgFormat::fromYahoo(QPixmap& pixmap, const char* data, unsigned size)
{
	if (!initOk)
		return false;

	jas_image_t* image = NULL;
	jas_stream_t* out = NULL;

	bool r = jasperConvert(image, out, data, size, fromYahooFmtID, 0);
	if (!r)
		return false;
	size = ((jas_stream_memobj_t *)out->obj_)->bufsize_;
	const unsigned char *buf = static_cast<const unsigned char*>(((jas_stream_memobj_t *)out->obj_)->buf_);

	r = pixmap.loadFromData(buf, size);

	if (out)
		jas_stream_close(out);
	if (image)
		jas_image_destroy(image);
	return r;
}

bool WebcamImgFormat::forYahoo(QByteArray& result, const QImage* src)
{
	if (!initOk)
		return false;

	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);
	if (!src)
		return false;
	if (!src->save(&buffer, forYahooFmtQt, 100))
	{
		kDebug(YAHOO_RAW_DEBUG) << "Failed to write intermediary " << forYahooFmtQt << " image";
		return false;
	}

	jas_image_t* image = NULL;
	jas_stream_t* out = NULL;

	/* Note: need to pass the floating number in the current locale so that it can be parsed back.
	   And since somehow I got my dev env with a non-english locale, it took some time to figure that this was the problem :-(
	*/
	/* The 'rate' parameter: tested and worked also with rate = 0.0265. In the future - make that a property of this object
	   and change it from some GUI.
	*/

	bool r = jasperConvert(image, out, ba.constData(), ba.size(), jpcFmtID,
				QString("cblkwidth=64 cblkheight=64 numrlvls=4 rate=%L1 prcheight=128 prcwidth=2048 mode=real").arg(0.0165).toAscii().constData());
	if (!r)
		return false;
	unsigned size = ((jas_stream_memobj_t *)out->obj_)->bufsize_;
	const char *buf = (const char*)(((jas_stream_memobj_t *)out->obj_)->buf_);
	result = QByteArray(buf, size);

	if (out)
		jas_stream_close(out);
	if (image)
		jas_image_destroy(image);
	return r;
}

WebcamImgFormat* WebcamImgFormat::instance()
{
	static WebcamImgFormat* p = NULL;
	if (!p)
		p = new WebcamImgFormat();
	return p;
}
