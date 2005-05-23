/*
    videodevice.h  -  Kopete Video Device Low-level Support

    Copyright (c) 2005 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_AVVIDEODEVICE_H
#define KOPETE_AVVIDEODEVICE_H

#include <qvaluevector.h>
#include <iostream>


#include "kopete_export.h"
#include "videoinput.h"
#include <qstring.h>
#include <qimage.h>
#include <qvaluevector.h>
#include <kcombobox.h>
#include "videodevicelistitem.h"

namespace Kopete {

namespace AV {

/**
This class allows kopete to check for the existence, open, configure, test, set parameters, grab frames from and close a given video capture card using the Video4Linux API.

@author Cláudio da Silveira Pinheiro
*/

class VideoDevicePrivate;

class KOPETE_EXPORT VideoDevice
{
public:
	static VideoDevice* self();
	int open();
	int getFrame();
	int checkDevice(int handle, VideoDeviceListItem *videodevice);
	int initDevice();
	int close();
	int selectDevice(unsigned int device);
	int startCapturing();
	int stopCapturing();
	int readFrame();
	int getImage(QImage *qimage);
	int selectInput(int input);
	int setResolution(int width, int height);
	int scanDevices();
	~VideoDevice();
	QValueVector <Kopete::AV::VideoDeviceListItem> m_videodevice;
	QValueVector<Kopete::AV::VideoInput> m_video_input;
	int fillDeviceKcomboBox(KComboBox *combobox);
	int fillInputKComboBox(KComboBox *combobox);
	int currentDevice();
	int currentInput();

protected:
	std::string name;
	std::string path;
	int descriptor;
	typedef enum
	{
		IO_METHOD_NONE,
		IO_METHOD_READ,
		IO_METHOD_MMAP,
		IO_METHOD_USERPTR,
	} io_method;
	io_method m_io_method;

	struct buffer2
	{
		int height;
		int width;
		int pixfmt;
		size_t size;
		QValueVector <uchar> data;
	};
	struct buffer
	{
		uchar * start;
		size_t length;
	};
	QValueVector<buffer> buffers;
	unsigned int     n_buffers;
	buffer2 currentbuffer;
	int m_buffer_size;

	QFile file;
	int m_current_device;
	int m_current_input;

protected:
	int xioctl(int request, void *arg);
	int processImage(const void *p);
	int errnoReturn(const char* s);
	int initRead();
	int initMmap();
	int initUserptr();
	void guessDriver();
private:
	VideoDevice();
	static VideoDevice* s_self;
};

}

}

#endif
