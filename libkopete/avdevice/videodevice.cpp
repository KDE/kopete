/*
    videodevice.cpp  -  Kopete Video Device Low-level Support

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

#include <assert.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>
#include <klocale.h>
#include <qdir.h>

#include "videoinput.h"
#include "videodevice.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice *VideoDevice::s_self = NULL;

VideoDevice* VideoDevice::self()
{
	if (s_self == NULL)
	{
		s_self = new VideoDevice;
	}
	return s_self;
}

VideoDevice::VideoDevice()
{
	descriptor = -1;
	n_buffers  = 0;
}


VideoDevice::~VideoDevice()
{
}




/*!
    \fn VideoDevice::open()
 */
int VideoDevice::open()
{
    /// @todo implement me

	kdDebug() << "libkopete (avdevice): open() called" << endl;
	if(-1 != descriptor)
	{
		kdDebug() << "libkopete (avdevice): open() Device is already open" << endl;
		return EXIT_SUCCESS;
	}
	file.setName(m_videodevice[m_current_device].full_filename);
	file.open( IO_ReadWrite | IO_Raw ); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
	if(file.isOpen())
	{
		kdDebug() << "libkopete (avdevice): File " << file.name() << " was opened successfuly" << endl;
		descriptor = file.handle();
		if(EXIT_FAILURE==checkDevice(descriptor, &(m_videodevice[m_current_device])))
		{
			kdDebug() << "libkopete (avdevice): File " << file.name() << " could not be opened" << endl;
			descriptor = -1;
			return EXIT_FAILURE;
		}
	}
	kdDebug() << "libkopete (avdevice): open() exited successfuly" << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::xioctl(int fd, int request, void *arg)
 */
int VideoDevice::xioctl(int request, void *arg)
{
	int r;

	do r = ioctl (descriptor, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}


/*!
    \fn VideoDevice::processImage(const void *p)
 */
int VideoDevice::processImage(const void * /* p */)
{
    /// @todo implement me
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::getFrame()
 */
int VideoDevice::getFrame()
{
    /// @todo implement me
	readFrame();
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::checkDevice()
 */
int VideoDevice::checkDevice(int handle, VideoDeviceListItem *videodevice)
{
	videodevice->m_driver=VIDEODEV_DRIVER_NONE;
#ifdef __linux__
#ifdef HAVE_V4L2
	memset(&videodevice->V4L2_capabilities, 0, sizeof(videodevice->V4L2_capabilities));

	if (-1 != ioctl (handle, VIDIOC_QUERYCAP, &videodevice->V4L2_capabilities))
	{
		if (!(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is not a video capture device." << endl;
			videodevice->m_driver = VIDEODEV_DRIVER_NONE;
			return EXIT_FAILURE;
		}
		kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is a V4L2 device." << endl;
		videodevice->m_driver = VIDEODEV_DRIVER_V4L2;
	}
	else
	{
		if (EINVAL == errno)
		{

			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is no V4L2 device." << endl;

		}
		else
		{
			return errnoReturn ("VIDIOC_QUERYCAP");
		}
	}
#endif
	if(videodevice->m_driver==VIDEODEV_DRIVER_NONE)
	{
		kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " Trying V4L API." << endl;
		if (-1 == ioctl (handle, VIDIOCGCAP, &videodevice->V4L_capabilities))
		{
			perror ("ioctl (VIDIOCGCAP)");
			videodevice->m_driver = VIDEODEV_DRIVER_NONE;
			return EXIT_FAILURE;
		}
		else
		{
			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is a V4L device." << endl;
			videodevice->m_driver = VIDEODEV_DRIVER_V4L;
		}
	}
#endif

// Now we must execute the proper initialization according to the type of the driver.
	switch(videodevice->m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			kdDebug() << "libkopete (avdevice): Driver: " << (const char*)videodevice->V4L2_capabilities.driver << " "
				<< ((videodevice->V4L2_capabilities.version>>16) & 0xFF) << "."
				<< ((videodevice->V4L2_capabilities.version>> 8) & 0xFF) << "."
				<< ((videodevice->V4L2_capabilities.version    ) & 0xFF) << endl;
			videodevice->name=QString::fromLocal8Bit((const char*)videodevice->V4L2_capabilities.card);
			kdDebug() << "libkopete (avdevice): Card: " << videodevice->name << endl;
			kdDebug() << "libkopete (avdevice): Capabilities:" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
				kdDebug() << "libkopete (avdevice):     Video capture" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
				kdDebug() << "libkopete (avdevice):     Video output" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
				kdDebug() << "libkopete (avdevice):     Video overlay" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
				kdDebug() << "libkopete (avdevice):     VBI capture" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
				kdDebug() << "libkopete (avdevice):     VBI output" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
				kdDebug() << "libkopete (avdevice):     RDS capture" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_TUNER)
				kdDebug() << "libkopete (avdevice):     Tuner IO" << endl;
			if(videodevice->V4L2_capabilities.capabilities & V4L2_CAP_AUDIO)
				kdDebug() << "libkopete (avdevice):     Audio IO" << endl;
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			videodevice->name=QString::fromLocal8Bit((const char*)videodevice->V4L_capabilities.name);
			kdDebug() << "libkopete (avdevice): Card: " << videodevice->name << endl;
			kdDebug() << "libkopete (avdevice): Capabilities:" << endl;
			break;
#endif
		case VIDEODEV_DRIVER_NONE:

			break;
	}
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::initDevice()
 */
int VideoDevice::initDevice()
{
    /// @todo implement me
	unsigned int min;

	kdDebug() << "libkopete (avdevice): initDevice() started" << endl;
	if(-1 == descriptor)
	{
		kdDebug() << "libkopete (avdevice): initDevice() Device is not open" << endl;
		return EXIT_FAILURE;
	}
	int inputisok=EXIT_SUCCESS;

	checkDevice(descriptor, &(m_videodevice[m_current_device]));

	m_io_method = IO_METHOD_NONE;
	switch(m_videodevice[m_current_device].m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			if(m_videodevice[m_current_device].V4L2_capabilities.capabilities & V4L2_CAP_READWRITE)
			{
				m_io_method = IO_METHOD_READ;
				kdDebug() << "libkopete (avdevice):     Read/Write interface" << endl;
			}
			if(m_videodevice[m_current_device].V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
				kdDebug() << "libkopete (avdevice):     Async IO interface" << endl;
			if(m_videodevice[m_current_device].V4L2_capabilities.capabilities & V4L2_CAP_STREAMING)
			{
				m_io_method = IO_METHOD_MMAP;
//				m_io_method = IO_METHOD_USERPTR;
				kdDebug() << "libkopete (avdevice):     Streaming interface" << endl;
			}
			if(m_io_method==IO_METHOD_NONE)
			{
				fprintf (stderr, "Found no suitable input/output method for %s\n", path.c_str());
				return EXIT_FAILURE;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			m_io_method=IO_METHOD_READ;
			break;
#endif
		case VIDEODEV_DRIVER_NONE:

			break;
	}

	kdDebug() << "libkopete (avdevice): Enumerating video inputs: " << endl;
	m_video_input.clear();

	switch(m_videodevice[m_current_device].m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
			{
				struct v4l2_input input;
				memset(&input, 0, sizeof(input));
				input.index = loop;
				inputisok=xioctl(VIDIOC_ENUMINPUT, &input);
				if(inputisok==EXIT_SUCCESS)
				{
					Kopete::AV::VideoInput tempinput;
					tempinput.name = QString::fromLocal8Bit((const char*)input.name);
					tempinput.hastuner=input.type & V4L2_INPUT_TYPE_TUNER;
					m_video_input.push_back(tempinput);
					kdDebug() << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((input.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
					if((input.type & V4L2_INPUT_TYPE_TUNER) != 0)
					{
//						_tunerForInput[name] = desc.tuner;
//						_isTuner = true;
					}
					else
					{
//						_tunerForInput[name] = -1;
					}
				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
//			m_video_input.resize(m_videodevice[m_current_device].V4L_capabilities.channels);
			for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
			{
				struct video_channel input;
				memset(&input, 0, sizeof(input));
				input.channel = loop;
				input.norm    = 1;
				inputisok=xioctl(VIDIOCGCHAN, &input);
				if(inputisok==EXIT_SUCCESS)
				{
					Kopete::AV::VideoInput tempinput;
					tempinput.name = QString::fromLocal8Bit((const char*)input.name);
					tempinput.hastuner=input.flags & VIDEO_VC_TUNER;
					m_video_input.push_back(tempinput);
					kdDebug() << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((input.flags & VIDEO_VC_TUNER) != 0) << ")" << endl;
/*					if((input.type & V4L2_INPUT_TYPE_TUNER) != 0)
					{
//						_tunerForInput[name] = desc.tuner;
//						_isTuner = true;
					}
					else
					{
//						_tunerForInput[name] = -1;
					}
*/				}
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
			break;
	}
	kdDebug() << "libkopete (avdevice): Grand total of " << m_video_input.size() << " video inputs for this device." << endl;
	for (unsigned int loop=0; loop < m_video_input.size(); loop++)
		kdDebug() << "libkopete (avdevice): Input " << loop << ": " << m_video_input[loop].name << " (tuner: " << m_video_input[loop].hastuner << ")" << endl;



// Select video input, video standard and tune here.

	m_videodevice[m_current_device].cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (VIDIOC_CROPCAP, &m_videodevice[m_current_device].cropcap))
	{ // Errors ignored.
	}
	m_videodevice[m_current_device].crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	m_videodevice[m_current_device].crop.c = m_videodevice[m_current_device].cropcap.defrect; // reset to default
	if (-1 == xioctl (VIDIOC_S_CROP, &m_videodevice[m_current_device].crop))
	{
		switch (errno)
		{
			case EINVAL: break;  // Cropping not supported.
			default:     break;  // Errors ignored.
		}
	}

// Change resolution and colorspace for the video device
	switch(m_videodevice[m_current_device].m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			CLEAR (m_videodevice[m_current_device].fmt);
			if (-1 == xioctl (VIDIOC_G_FMT, &m_videodevice[m_current_device].fmt))
//				return errnoReturn ("VIDIOC_S_FMT");
				kdDebug() << "libkopete (avdevice): VIDIOC_G_FMT failed (" << errno << ")." << endl;
			m_videodevice[m_current_device].fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			m_videodevice[m_current_device].fmt.fmt.pix.width       = 320;
			m_videodevice[m_current_device].fmt.fmt.pix.height      = 240;
			m_videodevice[m_current_device].fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
			m_videodevice[m_current_device].fmt.fmt.pix.field       = V4L2_FIELD_ANY;
			if (-1 == xioctl (VIDIOC_S_FMT, &m_videodevice[m_current_device].fmt))
//				return errnoReturn ("VIDIOC_S_FMT");
				kdDebug() << "libkopete (avdevice): VIDIOC_S_FMT failed (" << errno << ")." << endl;
				// Note VIDIOC_S_FMT may change width and height.
			else
			{
// Buggy driver paranoia.
				min = m_videodevice[m_current_device].fmt.fmt.pix.width * 2;
				if (m_videodevice[m_current_device].fmt.fmt.pix.bytesperline < min)
					m_videodevice[m_current_device].fmt.fmt.pix.bytesperline = min;
				min = m_videodevice[m_current_device].fmt.fmt.pix.bytesperline * m_videodevice[m_current_device].fmt.fmt.pix.height;
				if (m_videodevice[m_current_device].fmt.fmt.pix.sizeimage < min)
					m_videodevice[m_current_device].fmt.fmt.pix.sizeimage = min;
				m_buffer_size=m_videodevice[m_current_device].fmt.fmt.pix.sizeimage ;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			struct video_picture V4L_picture;
			if (xioctl (VIDIOCGWIN, &m_videodevice[m_current_device].V4L_videowindow)== -1)
			{
				perror ("ioctl VIDIOCGWIN");
				return (NULL);
			}
			m_videodevice[m_current_device].V4L_videowindow.width  = 320;
			m_videodevice[m_current_device].V4L_videowindow.height = 240;
			m_videodevice[m_current_device].V4L_videowindow.clipcount=0;
			if (xioctl (VIDIOCSWIN, &m_videodevice[m_current_device].V4L_videowindow)== -1)
			{
				perror ("ioctl VIDIOCSWIN");
				return (NULL);
			}
			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug() << "libkopete (avdevice): VIDIOCGPICT failed (" << errno << ")." << endl;
			kdDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << endl;
			V4L_picture.palette = VIDEO_PALETTE_RGB32;
			if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
				kdDebug() << "libkopete (avdevice): VIDIOCSPICT failed (" << errno << ")." << endl;
			kdDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << endl;

			if(-1 == xioctl(VIDIOCGFBUF,&m_videodevice[m_current_device].V4L_videobuffer))
				kdDebug() << "libkopete (avdevice): VIDIOCGFBUF failed (" << errno << ")." << endl;
			else
			{
// Buggy driver paranoia.
				min = m_videodevice[m_current_device].V4L_videobuffer.width * 2;
				if (m_videodevice[m_current_device].V4L_videobuffer.bytesperline < min)
					m_videodevice[m_current_device].V4L_videobuffer.bytesperline = min;
				m_buffer_size = m_videodevice[m_current_device].V4L_videobuffer.height * m_videodevice[m_current_device].V4L_videobuffer.bytesperline;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
			break;
	}

	switch (m_io_method)
	{
		case IO_METHOD_NONE:                    break;
		case IO_METHOD_READ:    initRead ();    break;
		case IO_METHOD_MMAP:    initMmap ();    break;
		case IO_METHOD_USERPTR: initUserptr (); break;
	}
	kdDebug() << "libkopete (avdevice): initDevice() exited successfuly" << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::close()
 */
int VideoDevice::close()
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): initRead called." << endl;
	stopCapturing();
	if(file.isOpen())
		file.close();
	else
		kdDebug() << "libkopete (avdevice): close() Device already closed." << endl;
	descriptor = -1;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::errnoReturn(const char* s)
 */
int VideoDevice::errnoReturn(const char* s)
{
    /// @todo implement me
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevice::initRead(unsigned int buffer_size)
 */
int VideoDevice::initRead()
{
    /// @todo implement me
	buffers.resize(1);

	kdDebug() << "libkopete (avdevice): initRead called." << endl;
	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}
	kdDebug() << "libkopete (avdevice): initRead m_buffer_size: " << m_buffer_size << endl;

	buffers[0].length = m_buffer_size;
	buffers[0].start = (uchar *)malloc (m_buffer_size);

	currentbuffer.size = m_buffer_size; // not really useful, cause currentbuffer.data.size() does the same thing
	currentbuffer.data.resize(m_buffer_size);

	if (!buffers[0].start)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}
	kdDebug() << "libkopete (avdevice): initRead exited successfuly." << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::initMmap()
 */
int VideoDevice::initMmap()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count  = 2;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			fprintf (stderr, "%s does not support memory mapping\n", path.c_str());
			return EXIT_FAILURE;
		}
		else
		{
			return errnoReturn ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		fprintf (stderr, "Insufficient buffer memory on %s\n", path.c_str());
		return EXIT_FAILURE;
	}

	buffers.resize(req.count);

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = n_buffers;

		if (-1 == xioctl (VIDIOC_QUERYBUF, &buf))
			return errnoReturn ("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = (uchar *) mmap (NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, descriptor, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
		return errnoReturn ("mmap");
	}
#endif
	currentbuffer.size = buffers[0].length; // not really useful, cause currentbuffer.data.size() does the same thing
	currentbuffer.data.resize(buffers[0].length);
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::initUserptr()
 */
int VideoDevice::initUserptr()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count  = 2;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl (VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			fprintf (stderr, "%s does not support user pointer i/o\n", path.c_str());
			return EXIT_FAILURE;
		}
		else
		{
			return errnoReturn ("VIDIOC_REQBUFS");
		}
	}

	buffers.resize(4);

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers)
	{
		buffers[n_buffers].length = m_buffer_size;
		buffers[n_buffers].start = (uchar *) malloc (m_buffer_size);

		if (!buffers[n_buffers].start)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
	}
#endif
return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::selectDevice(int device)
 */
int VideoDevice::selectDevice(unsigned int device)
{
    /// @todo implement me
	if(device >= m_videodevice.size())
	{
		kdDebug() << "libkopete (avdevice): selectDevice(" << device <<"): Device does not exist." << endl;
		return EXIT_FAILURE;
	}
	close();
	scanDevices();
	m_current_device = device;

	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::startCapturing()
 */
int VideoDevice::startCapturing()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	unsigned int i;
	enum v4l2_buf_type type;

	kdDebug() << "libkopete (avdevice): startCapturing() called." << endl;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
			for (i = 0; i < n_buffers; ++i)
			{
				struct v4l2_buffer buf;
				CLEAR (buf);
				buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index  = i;
				if (-1 == xioctl (VIDIOC_QBUF, &buf))
					return errnoReturn ("VIDIOC_QBUF");
			}
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMON, &type))
				return errnoReturn ("VIDIOC_STREAMON");
			break;
		case IO_METHOD_USERPTR:
			for (i = 0; i < n_buffers; ++i)
			{
				struct v4l2_buffer buf;
				CLEAR (buf);
				buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory    = V4L2_MEMORY_USERPTR;
				buf.m.userptr = (unsigned long) buffers[i].start;
				buf.length    = buffers[i].length;
				if (-1 == xioctl (VIDIOC_QBUF, &buf))
					return errnoReturn ("VIDIOC_QBUF");
			}
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMON, &type))
				return errnoReturn ("VIDIOC_STREAMON");
			break;
	}
#endif
	kdDebug() << "libkopete (avdevice): startCapturing() exited successfuly." << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::stopCapturing()
 */
int VideoDevice::stopCapturing()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	enum v4l2_buf_type type;

	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMOFF, &type))
				return errnoReturn ("VIDIOC_STREAMOFF");
			break;
	}
#endif
return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::readFrame()
 */
int VideoDevice::readFrame()
{
    /// @todo implement me
	unsigned int i;
	unsigned int bytesread;

#ifdef __linux__
#ifdef HAVE_V4L2
	struct v4l2_buffer buf;
#endif
#endif
	kdDebug() << "libkopete (avdevice): readFrame() called." << endl;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ:
			kdDebug() << "libkopete (avdevice): readFrame() Using IO_METHOD_READ.File descriptor: " << descriptor << "Buffer address: " << &currentbuffer.data[0] << "Size: " << currentbuffer.data.size() << endl;
			bytesread =read (descriptor, &currentbuffer.data[0], currentbuffer.data.size());
			if (-1 == bytesread)
			{
				kdDebug() << "libkopete (avdevice): readFrame() IO_METHOD_READ failed." << endl;
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
					return errnoReturn ("read");
				}
			}
			if(currentbuffer.data.size() < bytesread)
			{
				kdDebug() << "libkopete (avdevice): readFrame() IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << currentbuffer.data.size() <<")." << endl;
			}
			processImage (buffers[0].start);
			break;
		case IO_METHOD_MMAP:
#ifdef __linux__
#ifdef HAVE_V4L2
			CLEAR (buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			if (-1 == xioctl (VIDIOC_DQBUF, &buf))
			{
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
						return errnoReturn ("VIDIOC_DQBUF");
				}
			}
			assert (buf.index < n_buffers);
			processImage (buffers[buf.index].start);
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				return errnoReturn ("VIDIOC_QBUF");
#endif
#endif
			break;
		case IO_METHOD_USERPTR:
#ifdef __linux__
#ifdef HAVE_V4L2
			CLEAR (buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			if (-1 == xioctl (VIDIOC_DQBUF, &buf))
			{
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
						return errnoReturn ("VIDIOC_DQBUF");
				}
			}
			for (i = 0; i < n_buffers; ++i)
				if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length == buffers[i].length)
					break;
			assert (i < n_buffers);
			processImage ((void *) buf.m.userptr);
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				return errnoReturn ("VIDIOC_QBUF");
#endif
#endif
			break;
	}
// put frame copy operation here
	kdDebug() << "libkopete (avdevice): readFrame() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::getImage(const QImage *qimage)
 */

int Kopete::AV::VideoDevice::getImage(QImage *qimage)
{
    /// @todo implement me
	unsigned long long result=0;
	unsigned long long R=0, G=0, B=0, A=0;
	uchar Rmax=0, Gmax=0, Bmax=0, Amax=0;
	uchar Rmin=255, Gmin=255, Bmin=255, Amin=0;

	for(unsigned int loop=0;loop < currentbuffer.data.size();loop++)
	result+=currentbuffer.data[loop];
	kdDebug() << " ------ Data size: " << currentbuffer.data.size() << "----------------- Result: " << result << " -----------------------------" << endl;
	for(unsigned int loop=0;loop < currentbuffer.data.size();loop+=4)
	{
		R+=currentbuffer.data[loop];
		G+=currentbuffer.data[loop+1];
		B+=currentbuffer.data[loop+2];
		A+=currentbuffer.data[loop+3];
		if (currentbuffer.data[loop]   < Rmin) Rmin = currentbuffer.data[loop];
		if (currentbuffer.data[loop+1] < Gmin) Gmin = currentbuffer.data[loop+1];
		if (currentbuffer.data[loop+2] < Bmin) Bmin = currentbuffer.data[loop+2];
		if (currentbuffer.data[loop+3] < Amin) Amin = currentbuffer.data[loop+3];
		if (currentbuffer.data[loop]   > Rmax) Rmax = currentbuffer.data[loop];
		if (currentbuffer.data[loop+1] > Gmax) Gmax = currentbuffer.data[loop+1];
		if (currentbuffer.data[loop+2] > Bmax) Bmax = currentbuffer.data[loop+2];
		if (currentbuffer.data[loop+3] > Amax) Amax = currentbuffer.data[loop+3];
	}
	kdDebug() << " ------ R: " << R << "------ G: " << G << "------ B: " << B << "------ A: " << A << "------" << endl;
	kdDebug() << " ------ Rmin: " << Rmin << "------ Gmin: " << Gmin << "------ Bmin: " << Bmin << "------ Amin: " << Amin << "------" << endl;
	kdDebug() << " ------ Rmax: " << Rmax << "------ Gmax: " << Gmax << "------ Bmax: " << Bmax << "------ Amax: " << Amax << "------" << endl;

	qimage->create(320,240,32,QImage::IgnoreEndian);
	memcpy(qimage->bits(),&currentbuffer.data[0], currentbuffer.data.size());
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::selectInput(int input)
 */
int Kopete::AV::VideoDevice::selectInput(int input)
{
    /// @todo implement me
	switch (m_videodevice[m_current_device].m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			if (-1 == ioctl (descriptor, VIDIOC_S_INPUT, &input))
			{
				perror ("VIDIOC_S_INPUT");
				return EXIT_FAILURE;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			struct video_channel V4L_input;
			V4L_input.channel=input;
			V4L_input.norm=1;
			if (-1 == ioctl (descriptor, VIDIOCSCHAN, &V4L_input))
			{
				perror ("ioctl (VIDIOCSCHAN)");
				return EXIT_FAILURE;
			}
			break;
#endif
		case VIDEODEV_DRIVER_NONE:
			break;
	}
	kdDebug() << "libkopete (avdevice): selectInput: Selected input " << input << " (" << m_video_input[input].name << ")" << endl;
	m_current_input=input;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::setResolution(int width, int height)
 */
int Kopete::AV::VideoDevice::setResolution(int /* width */, int /* height */)
{
    /// @todo implement me
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::fillInputKComboBox(KComboBox *combobox)
 */
int Kopete::AV::VideoDevice::fillDeviceKcomboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice.size()>0)
		for (unsigned int loop=0; loop < m_videodevice.size(); loop++)
		{
			combobox->insertItem(m_videodevice[loop].name);
			kdDebug() << "libkopete (avdevice): DeviceKCombobox: Added device " << loop << ": " << m_videodevice[loop].name << endl;
		}
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::fillInputKComboBox(KComboBox *combobox)
 */
int Kopete::AV::VideoDevice::fillInputKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_video_input.size()>0)
		for (unsigned int loop=0; loop < m_video_input.size(); loop++)
		{
			combobox->insertItem(m_video_input[loop].name);
			kdDebug() << "libkopete (avdevice): InputKCombobox: Added input " << loop << ": " << m_video_input[loop].name << " (tuner: " << m_video_input[loop].hastuner << ")" << endl;
		}
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::scanDevices()
 */
int Kopete::AV::VideoDevice::scanDevices()
{
    /// @todo implement me

	QDir videodevice_dir;
	const QString videodevice_dir_path=QString::fromLatin1("/dev/v4l/");
	const QString videodevice_dir_filter=QString::fromLatin1("video*");
	VideoDeviceListItem videodevice;

	videodevice_dir.setPath(videodevice_dir_path);
	videodevice_dir.setNameFilter(videodevice_dir_filter);
        videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks | QDir::Readable | QDir::Writable );
        videodevice_dir.setSorting( QDir::Name );

	const QFileInfoList *list = videodevice_dir.entryInfoList();
	QFileInfoListIterator fileiterator ( *list );
	QFileInfo *fileinfo;
	QFile file;

	m_videodevice.clear();
	kdDebug() << "libkopete (avdevice): scanDevices() called" << endl;
	while ( (fileinfo = fileiterator.current()) != 0 )
	{
		kdDebug() << "libkopete (avdevice): Found device " << fileinfo->fileName().latin1() << endl;
		file.setName(fileinfo->absFilePath());
		file.open( IO_ReadWrite | IO_Raw ); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
		if(file.isOpen())
		{
			kdDebug() << "libkopete (avdevice): File " << file.name() << " was opened successfuly" << endl;
			if(EXIT_SUCCESS==checkDevice(file.handle(), &videodevice))
			{
				videodevice.full_filename=fileinfo->absFilePath();
				m_videodevice.push_back(videodevice);
			}
			file.close();
		}
		++fileiterator;
	}
	return EXIT_SUCCESS;
}


}

}


/*!
    \fn Kopete::AV::VideoDevice::currentDevice()
 */
int Kopete::AV::VideoDevice::currentDevice()
{
    /// @todo implement me
	return m_current_device;
}

/*!
    \fn Kopete::AV::VideoDevice::currentInput()
 */
int Kopete::AV::VideoDevice::currentInput()
{
    /// @todo implement me
	return m_current_input;
}
