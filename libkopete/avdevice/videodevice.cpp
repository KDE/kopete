//
// C++ Implementation: videodevice
//
// Description:
//
//
// Author: Cláudio da Silveira Pinheiro <taupter@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <asm/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif

#include <linux/videodev.h>


#include "videodevice.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice::VideoDevice()
{
	io_type    = IO_METHOD_MMAP;
	descriptor = -1;
	n_buffers  = 0;
}


VideoDevice::~VideoDevice()
{
}




/*!
    \fn VideoDevice::openDevice()
 */
int VideoDevice::openDevice()
{
    /// @todo implement me
	struct stat st;

	if (-1 == stat (path.c_str(), &st))
	{
		fprintf (stderr, "Cannot identify '%s': %d, %s\n", path.c_str(), errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
	if (!S_ISCHR (st.st_mode))
	{
		fprintf (stderr, "%s is no device\n", path.c_str());
		exit (EXIT_FAILURE);
	}
	descriptor = open (path.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);
	if (-1 == descriptor)
	{
		fprintf (stderr, "Cannot open '%s': %d, %s\n", path.c_str(), errno, strerror (errno));
		exit (EXIT_FAILURE);
	}

	return 0;
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
void VideoDevice::processImage(const void * /* p */)
{
    /// @todo implement me
}


/*!
    \fn VideoDevice::getFrame()
 */
int VideoDevice::getFrame()
{
    /// @todo implement me
	return 1;
}


/*!
    \fn VideoDevice::initDevice()
 */
void VideoDevice::initDevice()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl (VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			fprintf (stderr, "%s is no V4L2 device\n", path.c_str());
			exit (EXIT_FAILURE);
		}
		else
		{
			errnoExit ("VIDIOC_QUERYCAP");
		}
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf (stderr, "%s is no video capture device\n", path.c_str());
		exit (EXIT_FAILURE);
	}
	switch (io_type)
	{
		case IO_METHOD_READ:
			if (!(cap.capabilities & V4L2_CAP_READWRITE))
			{
				fprintf (stderr, "%s does not support read i/o\n", path.c_str());
				exit (EXIT_FAILURE);
			}
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			if (!(cap.capabilities & V4L2_CAP_STREAMING))
			{
				fprintf (stderr, "%s does not support streaming i/o\n", path.c_str());
				exit (EXIT_FAILURE);
			}
			break;
	}

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
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 320;
	fmt.fmt.pix.height      = 240;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
		errnoExit ("VIDIOC_S_FMT");

  // Note VIDIOC_S_FMT may change width and height.

  // Buggy driver paranoia.
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io_type)
	{
		case IO_METHOD_READ:    initRead (fmt.fmt.pix.sizeimage);    break;
		case IO_METHOD_MMAP:    initMmap ();                         break;
		case IO_METHOD_USERPTR: initUserptr (fmt.fmt.pix.sizeimage); break;
	}
#endif
}


/*!
    \fn VideoDevice::closeDevice()
 */
void VideoDevice::closeDevice()
{
    /// @todo implement me
	if (-1 == close (descriptor))
		errnoExit ("close");
	descriptor = -1;
}


/*!
    \fn VideoDevice::errnoExit(const char* s)
 */
void VideoDevice::errnoExit(const char* s)
{
    /// @todo implement me
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	exit (EXIT_FAILURE);
}


/*!
    \fn VideoDevice::initRead(unsigned int buffer_size)
 */
void VideoDevice::initRead(unsigned int buffer_size)
{
    /// @todo implement me
	buffers.resize(1);

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc (buffer_size);

	if (!buffers[0].start)
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
}


/*!
    \fn VideoDevice::initMmap()
 */
void VideoDevice::initMmap()
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
			exit (EXIT_FAILURE);
		}
		else
		{
			errnoExit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		fprintf (stderr, "Insufficient buffer memory on %s\n", path.c_str());
		exit (EXIT_FAILURE);
	}

	buffers.resize(req.count);

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = n_buffers;

		if (-1 == xioctl (VIDIOC_QUERYBUF, &buf))
			errnoExit ("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap (NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, descriptor, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
		errnoExit ("mmap");
	}
#endif
}


/*!
    \fn VideoDevice::initUserptr()
 */
void VideoDevice::initUserptr(unsigned int buffer_size)
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
			exit (EXIT_FAILURE);
		}
		else
		{
			errnoExit ("VIDIOC_REQBUFS");
		}
	}

	buffers.resize(4);

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers)
	{
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = malloc (buffer_size);

		if (!buffers[n_buffers].start)
		{
			fprintf (stderr, "Out of memory\n");
			exit (EXIT_FAILURE);
		}
	}
#endif
}


/*!
    \fn VideoDevice::setDevice(int device)
 */
void VideoDevice::setDevice(int device)
{
    /// @todo implement me
	std::ostringstream temp;

	temp << "/dev/video" << device;
	path = temp.str();
	std::cout << path;
}


/*!
    \fn VideoDevice::startCapturing()
 */
void VideoDevice::startCapturing()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io_type)
	{
		case IO_METHOD_READ: /* Nothing to do. */
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
					errnoExit ("VIDIOC_QBUF");
			}
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMON, &type))
				errnoExit ("VIDIOC_STREAMON");
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
					errnoExit ("VIDIOC_QBUF");
			}
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMON, &type))
				errnoExit ("VIDIOC_STREAMON");
			break;
	}
#endif
}


/*!
    \fn VideoDevice::stopCapturing()
 */
void VideoDevice::stopCapturing()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	enum v4l2_buf_type type;

	switch (io_type)
	{
		case IO_METHOD_READ: /* Nothing to do. */
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMOFF, &type))
				errnoExit ("VIDIOC_STREAMOFF");
			break;
	}
#endif
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

	switch (io_type)
	{
		case IO_METHOD_READ:
			if (-1 == read (descriptor, buffers[0].start, buffers[0].length))
			{
				switch (errno)
				{
					case EAGAIN:
						return 0;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
					errnoExit ("read");
				}
			}
			processImage (buffers[0].start);
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
						return 0;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
						errnoExit ("VIDIOC_DQBUF");
				}
			}
			assert (buf.index < n_buffers);
			processImage (buffers[buf.index].start);
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				errnoExit ("VIDIOC_QBUF");
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
						return 0;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
						errnoExit ("VIDIOC_DQBUF");
				}
			}
			for (i = 0; i < n_buffers; ++i)
				if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length == buffers[i].length)
					break;
			assert (i < n_buffers);
			processImage ((void *) buf.m.userptr);
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				errnoExit ("VIDIOC_QBUF");
			break;
	}
#endif
	return 1;
}


}

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
	return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
#endif
}


/*!
    \fn Kopete::AV::VideoDevice::setResolution(int width, int height)
 */
int Kopete::AV::VideoDevice::setResolution(int /* width */, int /* height */)
{
    /// @todo implement me
	return (EXIT_SUCCESS);
}


/*!
    \fn Kopete::AV::VideoDevice::scanDevices()
 */
int Kopete::AV::VideoDevice::scanDevices()
{
    /// @todo implement me
	return (EXIT_SUCCESS);
}
