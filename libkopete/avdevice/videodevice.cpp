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

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#ifdef __linux__

#undef __STRICT_ANSI__
#include <asm/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif // __u64

#include <linux/videodev.h>
#define __STRICT_ANSI__

#endif // __linux__

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
	struct stat st;

	if (-1 == stat (path.c_str(), &st))
	{
		fprintf (stderr, "Cannot identify '%s': %d, %s\n", path.c_str(), errno, strerror (errno));
		return EXIT_FAILURE;
	}
	if (!S_ISCHR (st.st_mode))
	{
		fprintf (stderr, "%s is no device\n", path.c_str());
		return EXIT_FAILURE;
	}
	descriptor = ::open (path.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
	if (-1 == descriptor)
	{
		fprintf (stderr, "Cannot open '%s': %d, %s\n", path.c_str(), errno, strerror (errno));
		return EXIT_FAILURE;
	}

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
    \fn VideoDevice::initDevice()
 */
int VideoDevice::initDevice()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_capability V4L2_capabilities;
	struct video_capability V4L_capabilities;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	int inputisok=EXIT_SUCCESS;

	memset(&V4L2_capabilities, 0, sizeof(V4L2_capabilities));

	if (-1 == xioctl (VIDIOC_QUERYCAP, &V4L2_capabilities))
	{
		if (EINVAL == errno)
		{
			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is no V4L2 device." << endl;
			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " Trying V4L API." << endl;
			if (-1 == xioctl (VIDIOCGCAP, &V4L_capabilities))
			{
				perror ("ioctl (VIDIOCGCAP)");
				m_driver = VIDEODEV_DRIVER_NONE;
				return EXIT_FAILURE;
			}
			else
			{
				kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is a V4L device." << endl;
				m_driver = VIDEODEV_DRIVER_V4L;
			}
		}
		else
		{
			return errnoReturn ("VIDIOC_QUERYCAP");
		}
	}
	else
	{
		if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is not a video capture device." << endl;
			m_driver = VIDEODEV_DRIVER_NONE;
			return EXIT_FAILURE;
		}
		kdDebug() << "libkopete (avdevice): intDevice(): " << path.c_str() << " is a V4L2 device." << endl;
		m_driver=VIDEODEV_DRIVER_V4L2;
	}

// Now we must execute the proper initialization according to the type of the driver.
	switch(m_driver)
	{
		case VIDEODEV_DRIVER_V4L2:
			kdDebug() << "libkopete (avdevice): Driver: " << (const char*)V4L2_capabilities.driver << " "
				<< ((V4L2_capabilities.version>>16) & 0xFF) << "."
				<< ((V4L2_capabilities.version>> 8) & 0xFF) << "."
				<< ((V4L2_capabilities.version    ) & 0xFF) << endl;
			kdDebug() << "libkopete (avdevice): Card: " << (const char*)V4L2_capabilities.card << endl;

			kdDebug() << "libkopete (avdevice): Capabilities:" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)
				kdDebug() << "libkopete (avdevice):     Video capture" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OUTPUT)
				kdDebug() << "libkopete (avdevice):     Video output" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_OVERLAY)
				kdDebug() << "libkopete (avdevice):     Video overlay" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_CAPTURE)
				kdDebug() << "libkopete (avdevice):     VBI capture" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_VBI_OUTPUT)
				kdDebug() << "libkopete (avdevice):     VBI output" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_RDS_CAPTURE)
				kdDebug() << "libkopete (avdevice):     RDS capture" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_TUNER)
				kdDebug() << "libkopete (avdevice):     Tuner IO" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_AUDIO)
				kdDebug() << "libkopete (avdevice):     Audio IO" << endl;
			break;
		case VIDEODEV_DRIVER_V4L:

			break;
		case VIDEODEV_DRIVER_NONE:

			break;
	}

	m_io_method = IO_METHOD_NONE;
	switch(m_driver)
	{
		case VIDEODEV_DRIVER_V4L2:
			if(V4L2_capabilities.capabilities & V4L2_CAP_READWRITE)
			{
				m_io_method = IO_METHOD_READ;
				kdDebug() << "libkopete (avdevice):     Read/Write interface" << endl;
			}
			if(V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
				kdDebug() << "libkopete (avdevice):     Async IO interface" << endl;
			if(V4L2_capabilities.capabilities & V4L2_CAP_STREAMING)
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
		case VIDEODEV_DRIVER_V4L:
			break;
		case VIDEODEV_DRIVER_NONE:

			break;
	}

	kdDebug() << "libkopete (avdevice): Enumerating video inputs: " << endl;
	m_video_input.clear();
	for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
	{
		struct v4l2_input input;
		memset(&input, 0, sizeof(input));
		input.index = loop;
		inputisok=xioctl(VIDIOC_ENUMINPUT, &input);
		if(inputisok==EXIT_SUCCESS)
		{
			Kopete::AV::VideoInput tempinput;
			tempinput.name = QString::fromLatin1((const char*)input.name);
			tempinput.hastuner=input.type & V4L2_INPUT_TYPE_TUNER;
			m_video_input.push_back(tempinput);
			kdDebug() << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((input.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
			if((input.type & V4L2_INPUT_TYPE_TUNER) != 0)
			{
//				_tunerForInput[name] = desc.tuner;
//				_isTuner = true;
			}
			else
			{
//				_tunerForInput[name] = -1;
			}
		}
	}
	kdDebug() << "libkopete (avdevice): Grand total of " << m_video_input.size() << " video inputs for this device." << endl;
	for (unsigned int loop=0; loop < m_video_input.size(); loop++)
		kdDebug() << "libkopete (avdevice): Input " << loop << ": " << m_video_input[loop].name << " (tuner: " << m_video_input[loop].hastuner << ")" << endl;



// Select video input, video standard and tune here.

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (VIDIOC_CROPCAP, &cropcap))
	{ // Errors ignored.
	}
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; // reset to default
	if (-1 == xioctl (VIDIOC_S_CROP, &crop))
	{
		switch (errno)
		{
			case EINVAL: break;  // Cropping not supported.
			default:     break;  // Errors ignored.
		}
	}
	CLEAR (fmt);
	if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
//		return errnoReturn ("VIDIOC_S_FMT");
		kdDebug() << "libkopete (avdevice): VIDIOC_G_FMT failed (" << errno << ")." << endl;

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 320;
	fmt.fmt.pix.height      = 240;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
//		return errnoReturn ("VIDIOC_S_FMT");
		kdDebug() << "libkopete (avdevice): VIDIOC_S_FMT failed (" << errno << ")." << endl;

  // Note VIDIOC_S_FMT may change width and height.

  // Buggy driver paranoia.
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (m_io_method)
	{
		case IO_METHOD_NONE:    return EXIT_FAILURE;                 break;
		case IO_METHOD_READ:    initRead (fmt.fmt.pix.sizeimage);    break;
		case IO_METHOD_MMAP:    initMmap ();                         break;
		case IO_METHOD_USERPTR: initUserptr (fmt.fmt.pix.sizeimage); break;
	}
#endif
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::close()
 */
int VideoDevice::close()
{
    /// @todo implement me
	if (-1 == ::close (descriptor))
		return errnoReturn ("close");
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
int VideoDevice::initRead(unsigned int buffer_size)
{
    /// @todo implement me
	buffers.resize(1);

	kdDebug() << "libkopete (avdevice): initRead called." << endl;
	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}

	buffers[0].length = buffer_size;
	buffers[0].start = (uchar *)malloc (buffer_size);

	currentbuffer.size = buffer_size; // not really useful, cause currentbuffer.data.size() does the same thing
	currentbuffer.data.resize(buffer_size);

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

	req.count  = 4;
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
int VideoDevice::initUserptr(unsigned int buffer_size)
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count  = 4;
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
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = (uchar *) malloc (buffer_size);

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
    \fn VideoDevice::setDevice(int device)
 */
int VideoDevice::setDevice(int device)
{
    /// @todo implement me
	std::ostringstream temp;

	temp << "/dev/v4l/video" << device;
	path = temp.str();
	std::cout << path;
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
#ifdef HAVE_V4L2
	struct v4l2_buffer buf;
	unsigned int i;

	kdDebug() << "libkopete (avdevice): readFrame() called." << endl;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ:
//			if (-1 == read (descriptor, buffers[0].start, buffers[0].length))
			if (-1 == read (descriptor, &currentbuffer.data[0], currentbuffer.data.size()))
			{
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
					return errnoReturn ("read");
				}
			}
			processImage (buffers[0].start);
			for(size_t loop=0; loop<currentbuffer.data.size(); loop+=4)
			{
				currentbuffer.data[loop]  =currentbuffer.data[loop+3];
				currentbuffer.data[loop+3]=currentbuffer.data[loop+1];
				currentbuffer.data[loop+1]=currentbuffer.data[loop+2];
				currentbuffer.data[loop+2]=currentbuffer.data[loop+3];
				currentbuffer.data[loop+3]=255;
			}
			break;
		case IO_METHOD_MMAP:
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
			for(size_t loop=0; loop<currentbuffer.data.size(); loop+=4)
			{
				currentbuffer.data[loop]  =buffers[buf.index].start[loop+3];
				currentbuffer.data[loop+1]=buffers[buf.index].start[loop+2];
				currentbuffer.data[loop+2]=buffers[buf.index].start[loop+1];
				currentbuffer.data[loop+3]=255;
			}
//			memcpy(&currentbuffer.data[0], buffers[buf.index].start, currentbuffer.data.size());
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				return errnoReturn ("VIDIOC_QBUF");
			break;
		case IO_METHOD_USERPTR:
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
			for(size_t loop=0; loop<currentbuffer.data.size(); loop+=4)
			{
				currentbuffer.data[loop]  =buffers[i].start[loop+3];
				currentbuffer.data[loop+1]=buffers[i].start[loop+2];
				currentbuffer.data[loop+2]=buffers[i].start[loop+1];
				currentbuffer.data[loop+3]=255;
			}
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				return errnoReturn ("VIDIOC_QBUF");
			break;
	}
#endif
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
#ifdef HAVE_V4L2
	if (-1 == ioctl (descriptor, VIDIOC_S_INPUT, &input))
	{
		perror ("VIDIOC_S_INPUT");
		return EXIT_FAILURE;
	}
	kdDebug() << "libkopete (avdevice): selectInput: Selected input " << input << " (" << m_video_input[input].name << ")" << endl;
	return EXIT_SUCCESS;
#endif
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
int Kopete::AV::VideoDevice::fillInputKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_video_input.size()>0)
		for (unsigned int loop=0; loop < m_video_input.size(); loop++)
		{
			combobox->insertItem(m_video_input[loop].name);
			kdDebug() << "libkopete (avdevice): InputKCombobox: Added input" << loop << ": " << m_video_input[loop].name << " (tuner: " << m_video_input[loop].hastuner << ")" << endl;
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
	videodevice_dir.setPath(videodevice_dir_path);
	videodevice_dir.setNameFilter(videodevice_dir_filter);
        videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks | QDir::Readable | QDir::Writable );
        videodevice_dir.setSorting( QDir::Name );
	struct stat status;
	int n=0;
	int cnt;

	const QFileInfoList *list = videodevice_dir.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;

	m_videodevice.clear();
	kdDebug() << "libkopete (avdevice): scanDevices() called" << endl;
	while ( (fi = it.current()) != 0 )
	{
		kdDebug() << "libkopete (avdevice): Found device " << fi->fileName().latin1() << endl;
		++it;
	}

/*	chdir(path);
	n=scandir(path,&eps, one, alphasort);
	if (n>=0)
	{
		for (cnt=0; cnt<n;++cnt)
		{
			if (stat(eps[cnt]->d_name,&status)!=0)
			{
				fprintf(stderr,"%s: stat()\n", strerror(errno));
			}
			if((status.st_mode&S_IFMT)==S_IFCHR)
			{
				if(!(strncmp(eps[cnt]->d_name,devname,5)))
				{
					if(!(devicepool->numdevices))
						devicepool->device=malloc(sizeof (struct VideoDevice));
					else
						devicepool->device=realloc(devicepool->device,((sizeof (struct VideoDevice))*((devicepool->numdevices)+1)));

					printf("Probing device: %02d\n",devicepool->numdevices);
					snprintf(devicepool->device[devicepool->numdevices].name,STRING_SIZE,"Video device %02d",devicepool->numdevices);
					snprintf(devicepool->device[devicepool->numdevices].path,STRING_SIZE,"%s%s",path,eps[cnt]->d_name);

					devicepool->device[devicepool->numdevices].descriptor=open(devicepool->device[devicepool->numdevices].path,O_RDWR);
					if ((devicepool->device[devicepool->numdevices].descriptor)<0)
					{
						fprintf(stderr,"Could not open device: %s\n",devicepool->device[devicepool->numdevices].path);
						realloc(devicepool->device,((sizeof (struct VideoDevice))*(devicepool->numdevices)));
					}
					else
					{
						if (ioctl (devicepool->device[devicepool->numdevices].descriptor, VIDIOCGCAP, &(devicepool->device[devicepool->numdevices].capabilities)) == -1)
						{
							perror ("ioctl (VIDIOCGCAP)");
							return EXIT_FAILURE;
						}
						devicepool->device[devicepool->numdevices].newframe=
						vidStart(&devicepool->device[devicepool->numdevices],
						devicepool->device[devicepool->numdevices].capabilities.maxwidth,
						devicepool->device[devicepool->numdevices].capabilities.maxheight,1);
						devicepool->numinputs+=devicepool->device[devicepool->numdevices].capabilities.channels;
						devicepool->numdevices++;
//						devicepool->numinputs+=devicepool->device[devicepool->numdevices].capabilities.channels;
					}
				}
			}
		}
		showdevicepoolproperties(devicepool);
		chdir(currentdirectory);
		return devicepool->numdevices;
	}
	else
	{
		perror("Couldn't open the directory");
	}
	return 0;
*/
	return EXIT_SUCCESS;
}


}

}
