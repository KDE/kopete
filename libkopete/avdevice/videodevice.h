//
// C++ Interface: videodevice
//
// Description: 
//
//
// Author: Cláudio da Silveira Pinheiro <taupter@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KOPETE_AVVIDEODEVICE_H
#define KOPETE_AVVIDEODEVICE_H

#include <vector>
#include <iostream>
#include <string>

namespace Kopete {

namespace AV {

/**
This class allows kopete to check for the existence, open, configure, test, set parameters, grab frames from and close a given video capture card using the Video4Linux API.

@author Cláudio da Silveira Pinheiro
*/

class VideoDevicePrivate;

class VideoDevice
{
public:
	VideoDevice();
	~VideoDevice();
	int openDevice();
	int getFrame();
	void initDevice();
	void closeDevice();
	void setDevice(int device);
	void startCapturing();
	void stopCapturing();
	int readFrame();
	int selectInput(int input);
	int setResolution(int width, int height);
	int scanDevices();

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
	void processImage(const void *p);
	void errnoExit(const char* s);
	void initRead(unsigned int buffer_size);
	void initMmap();
	void initUserptr(unsigned int buffer_size);
};

}

}

#endif
