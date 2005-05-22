// -*- c++ -*-

/*
 *
 * Copyright (C) 2002 George Staikos <staikos@kde.org>
 *               2004 Dirk Ziegelmeier <dziegel@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _QVIDEOSTREAM_H
#define _QVIDEOSTREAM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GL
#include <qgl.h>
#include <GL/gl.h>
#endif

#include <qwidget.h>
#include "qvideo.h"

class QVideoStreamPrivate;

/**
 * QT-style video stream driver.
 */
class QVideoStream : public QObject, public QVideo
{
	Q_OBJECT

public:
	QVideoStream(QWidget *widget, const char* name = 0);
	~QVideoStream();

	/* output method */
	bool haveMethod(VideoMethod method) const;
	VideoMethod method() const;
	VideoMethod setMethod(VideoMethod method);

	/* max output sizes */
	QSize maxSize() const;
	int   maxWidth() const;
	int   maxHeight() const;

	/* output sizes */
	QSize size() const;
	int   width() const;
	int   height() const;

	QSize setSize(const QSize& sz);
	int   setWidth(int width);
	int   setHeight(int height);

	/* input sizes */
	QSize inputSize() const;
	int   inputWidth() const;
	int   inputHeight() const;

	QSize setInputSize(const QSize& sz);
	int setInputWidth(int width);
	int setInputHeight(int height);

    /* input format */
    ImageFormat format() const;
    bool setFormat(ImageFormat format);

    /* functions to find out about formats */
    ImageFormat formatsForMethod(VideoMethod method);
    bool supportsFormat(VideoMethod method, ImageFormat format);

    /* Display image */
	QVideoStream& operator<<(const unsigned char *const img);
    
public slots:
	int displayFrame(const unsigned char *const img);
	int displayFrame(const unsigned char *const img, int x, int y, int sw, int sh);

private:
    QVideoStreamPrivate* d;

	QWidget*    _w;
	VideoMethod _methods; // list of methods
	VideoMethod _method;  // the current method
    ImageFormat _format;
	QSize       _size;
	QSize       _inputSize;
	bool        _init;
    ImageFormat _xFormat;

	void deInit();
	void init();
};

#endif

