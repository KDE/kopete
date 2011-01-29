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

#ifndef WEBCAMIMGFORMAT_H
#define WEBCAMIMGFORMAT_H

#include <QPixmap>
#include <QImage>
#include <QByteArray>

#include "libkyahoo_export.h"

/**
  Helper class to convert webcam images in what Yahoo is using. Singleton.
  */
class LIBKYAHOO_EXPORT WebcamImgFormat
{
public:
	/** Returns true if image was successfully loaded. */
	bool fromYahoo(QPixmap& pixmap, const char * data, unsigned size);

	/** Returns true if image was successfully created */
	bool forYahoo(QByteArray& result, const QImage* src);

	static WebcamImgFormat* instance();

private:
	WebcamImgFormat();

	bool initOk;

	/** format to convert yahoo received image for QPixmap */
	int fromYahooFmtID;

	/** format to convert QImage to, as input for jasper library */
	char forYahooFmtQt[4];

	/**  jasper format ID for JPC */
	int jpcFmtID;
};

#endif // WEBCAMIMGFORMAT_H
