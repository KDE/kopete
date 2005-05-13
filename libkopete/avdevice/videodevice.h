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

#include <vector>
#include <iostream>
#include <string>

#include "kopete_export.h"

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
	int initDevice();
	int close();
	int setDevice(int device);
	int startCapturing();
	int stopCapturing();
	int readFrame();
	int selectInput(int input);
	int setResolution(int width, int height);
	int scanDevices();
	~VideoDevice();

protected:
	std::string name;
	std::string path;
	int descriptor;
	typedef enum
	{
		IO_METHOD_READ,
		IO_METHOD_MMAP,
		IO_METHOD_USERPTR,
	} io_method;
	io_method io_type;

	struct buffer
	{
		void * start;
		size_t length;
	};
		std::vector<buffer> buffers;
		unsigned int     n_buffers;
protected:
	int xioctl(int request, void *arg);
	int processImage(const void *p);
	int errnoReturn(const char* s);
	int initRead(unsigned int buffer_size);
	int initMmap();
	int initUserptr(unsigned int buffer_size);
private:
	VideoDevice();
	static VideoDevice* s_self;
};

}

}

#endif
