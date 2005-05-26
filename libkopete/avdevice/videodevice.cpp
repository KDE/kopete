//
// C++ Implementation: videodevicelistitem
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

//#include <assert.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>

#include "videoinput.h"
#include "videodevice.h"

#undef HAVE_V4L2
#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice::VideoDevice()
{
//	kdDebug() << "libkopete (avdevice): VideoDevice() called" << endl;
	descriptor = -1;
	n_buffers  = 0;
//	kdDebug() << "libkopete (avdevice): VideoDevice() exited successfuly" << endl;
}


VideoDevice::~VideoDevice()
{
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
    \fn VideoDevice::errnoReturn(const char* s)
 */
int VideoDevice::errnoReturn(const char* s)
{
    /// @todo implement me
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevice::setFileName(QString name)
 */
int VideoDevice::setFileName(QString filename)
{
    /// @todo implement me
	full_filename=filename;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::open()
 */
int VideoDevice::open()
{
    /// @todo implement me

	kdDebug() << "libkopete (avdevice): VideoDevice::open() called" << endl;
	if(-1 != descriptor)
	{
		kdDebug() << "libkopete (avdevice): VideoDevice::open() Device is already open" << endl;
		return EXIT_SUCCESS;
	}
	descriptor = ::open (full_filename.local8Bit(), O_RDWR /* required */ | O_NONBLOCK, 0);
	if(isOpen())
	{
		kdDebug() << "libkopete (avdevice): VideoDevice::open() File " << full_filename << " was opened successfuly" << endl;
		if(EXIT_FAILURE==checkDevice())
		{
			kdDebug() << "libkopete (avdevice): File " << full_filename << " could not be opened" << endl;
			close();
			return EXIT_FAILURE;
		}
	}
	initDevice();
	kdDebug() << "libkopete (avdevice): VideoDevice::open() exited successfuly" << endl;
	return EXIT_SUCCESS;
}

bool VideoDevice::isOpen()
{
	if(-1 == descriptor)
	{
		kdDebug() << "libkopete (avdevice): VideoDevice::isOpen() File is closed" << endl;
		return false;
	}
	kdDebug() << "libkopete (avdevice): VideoDevice::isOpen() File is open" << endl;
	return true;
}

int VideoDevice::checkDevice()
{
	m_driver=VIDEODEV_DRIVER_NONE;
	kdDebug() << "libkopete (avdevice): checkDevice() called." << endl;
#ifdef __linux__
#ifdef HAVE_V4L2
	memset(&V4L2_capabilities, 0, sizeof(V4L2_capabilities));

	if (-1 != xioctl (VIDIOC_QUERYCAP, &V4L2_capabilities))
	{
		if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			kdDebug() << "libkopete (avdevice): checkDevice(): " << full_filename << " is not a video capture device." << endl;
			m_driver = VIDEODEV_DRIVER_NONE;
			return EXIT_FAILURE;
		}
		kdDebug() << "libkopete (avdevice): checkDevice(): " << full_filename << " is a V4L2 device." << endl;
		m_driver = VIDEODEV_DRIVER_V4L2;
		name=QString::fromLocal8Bit((const char*)V4L2_capabilities.card);




		CLEAR (fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
//			return errnoReturn ("VIDIOC_S_FMT");
			kdDebug() << "libkopete (avdevice): VIDIOC_G_FMT failed (" << errno << ")." << endl;
/*		fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width       = 32767;
		fmt.fmt.pix.height      = 32767;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
		fmt.fmt.pix.field       = V4L2_FIELD_ANY;
		if (-1 == xioctl (VIDIOC_TRY_FMT, &fmt))
//			return errnoReturn ("VIDIOC_S_FMT");
			kdDebug() << "libkopete (avdevice): VIDIOC_TRY_FMT failed (" << errno << ")." << endl;
			// Note VIDIOC_S_FMT may change width and height.
		maxwidth  = fmt.fmt.pix.width;
		maxheight = fmt.fmt.pix.height;

		if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
//			return errnoReturn ("VIDIOC_S_FMT");
			kdDebug() << "libkopete (avdevice): VIDIOC_G_FMT failed (" << errno << ")." << endl;
		fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width       = 1;
		fmt.fmt.pix.height      = 1;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
		fmt.fmt.pix.field       = V4L2_FIELD_ANY;
		if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
//			return errnoReturn ("VIDIOC_S_FMT");
			kdDebug() << "libkopete (avdevice): VIDIOC_S_FMT failed (" << errno << ")." << endl;
			// Note VIDIOC_S_FMT may change width and height.*/
		minwidth  = fmt.fmt.pix.width;
		minheight = fmt.fmt.pix.height;

/*		else
		{
// Buggy driver paranoia
			min = fmt.fmt.pix.width * 2;
			if (fmt.fmt.pix.bytesperline < min)
				fmt.fmt.pix.bytesperline = min;
			min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
			if (fmt.fmt.pix.sizeimage < min)
				fmt.fmt.pix.sizeimage = min;
			m_buffer_size=fmt.fmt.pix.sizeimage ;
		}*/



		int inputisok=EXIT_SUCCESS;
		input.clear();
		for(unsigned int loop=0; inputisok==EXIT_SUCCESS; loop++)
		{
			struct v4l2_input videoinput;
			memset(&videoinput, 0, sizeof(videoinput));
			videoinput.index = loop;
			inputisok=xioctl(VIDIOC_ENUMINPUT, &videoinput);
			if(inputisok==EXIT_SUCCESS)
			{
				VideoInput tempinput;
				tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
				tempinput.hastuner=videoinput.type & V4L2_INPUT_TYPE_TUNER;
				input.push_back(tempinput);
				kdDebug() << "libkopete (avdevice): checkDevice() Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
				if((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0)
				{
//					_tunerForInput[name] = desc.tuner;
//					_isTuner = true;
				}
				else
				{
//					_tunerForInput[name] = -1;
				}
			}
		}


	}
	else
	{
		if (EINVAL == errno)
		{

			kdDebug() << "libkopete (avdevice): checkDevice(): " << full_filename << " is no V4L2 device." << endl;
		}
		else
		{
			return errnoReturn ("VIDIOC_QUERYCAP");
		}
	}
#endif

	memset(&V4L_capabilities, 0, sizeof(V4L_capabilities));

	if(m_driver==VIDEODEV_DRIVER_NONE)
	{
		kdDebug() << "libkopete (avdevice): checkDevice(): " << full_filename << " Trying V4L API." << endl;
		if (-1 == xioctl (VIDIOCGCAP, &V4L_capabilities))
		{
			perror ("ioctl (VIDIOCGCAP)");
			m_driver = VIDEODEV_DRIVER_NONE;
			return EXIT_FAILURE;
		}
		else
		{
			kdDebug() << "libkopete (avdevice): checkDevice(): " << full_filename << " is a V4L device." << endl;
			m_driver = VIDEODEV_DRIVER_V4L;
			name=QString::fromLocal8Bit((const char*)V4L_capabilities.name);

			if(V4L_capabilities.type & VID_TYPE_CAPTURE)
				kdDebug() << "libkopete (avdevice):     Video capture" << endl;
			if(V4L_capabilities.type & VID_TYPE_CHROMAKEY)
				kdDebug() << "libkopete (avdevice):     Video chromakey" << endl;
			if(V4L_capabilities.type & VID_TYPE_SCALES)
				kdDebug() << "libkopete (avdevice):     Video scales" << endl;
			if(V4L_capabilities.type & VID_TYPE_OVERLAY)
				kdDebug() << "libkopete (avdevice):     Video overlay" << endl;
//			kdDebug() << "libkopete (avdevice):     Inputs : " << V4L_capabilities.channels << endl;
			kdDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
			minwidth  = V4L_capabilities.minwidth;
			maxwidth  = V4L_capabilities.maxwidth;
			minheight = V4L_capabilities.minheight;
			maxheight = V4L_capabilities.maxheight;


			int inputisok=EXIT_SUCCESS;
			input.clear();
			for(unsigned int loop=0; loop < V4L_capabilities.channels; loop++)
			{
				struct video_channel videoinput;
				memset(&videoinput, 0, sizeof(videoinput));
				videoinput.channel = loop;
				videoinput.norm    = 1;
				inputisok=xioctl(VIDIOCGCHAN, &videoinput);
				if(inputisok==EXIT_SUCCESS)
				{
					VideoInput tempinput;
					tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
					tempinput.hastuner=videoinput.flags & VIDEO_VC_TUNER;
					input.push_back(tempinput);
//					kdDebug() << "libkopete (avdevice): Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.flags & VIDEO_VC_TUNER) != 0) << ")" << endl;
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

		}
	}
#endif
	showDeviceCapabilities();

	// Now we must execute the proper initialization according to the type of the driver.
	kdDebug() << "libkopete (avdevice): checkDevice() exited successfuly." << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::showDeviceCapabilities()
 */
int VideoDevice::showDeviceCapabilities()
{
	kdDebug() << "libkopete (avdevice): showDeviceCapabilities() called." << endl;
/*	kdDebug() << "libkopete (avdevice): Driver: " << (const char*)V4L2_capabilities.driver << " "
		<< ((V4L2_capabilities.version>>16) & 0xFF) << "."
		<< ((V4L2_capabilities.version>> 8) & 0xFF) << "."
		<< ((V4L2_capabilities.version    ) & 0xFF) << endl;
	kdDebug() << "libkopete (avdevice): Card: " << name << endl;
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
;*/
	kdDebug() << "libkopete (avdevice): Card: " << name << endl;
	kdDebug() << "libkopete (avdevice): Capabilities:" << endl;
	if(V4L_capabilities.type & VID_TYPE_CAPTURE)
		kdDebug() << "libkopete (avdevice):     Video capture" << endl;
	if(V4L_capabilities.type & VID_TYPE_CHROMAKEY)
		kdDebug() << "libkopete (avdevice):     Video chromakey" << endl;
	if(V4L_capabilities.type & VID_TYPE_SCALES)
		kdDebug() << "libkopete (avdevice):     Video scales" << endl;
	if(V4L_capabilities.type & VID_TYPE_OVERLAY)
		kdDebug() << "libkopete (avdevice):     Video overlay" << endl;
	kdDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios << endl;
	kdDebug() << "libkopete (avdevice):     Max res: " << maxWidth() << " x " << maxHeight() << endl;
	kdDebug() << "libkopete (avdevice):     Min res: " << minWidth() << " x " << minHeight() << endl;
	kdDebug() << "libkopete (avdevice):     Inputs : " << inputs() << endl;
	for (unsigned int loop=0; loop < inputs(); loop++)
		kdDebug() << "libkopete (avdevice): Input " << loop << ": " << input[loop].name << " (tuner: " << input[loop].hastuner << ")" << endl;
	kdDebug() << "libkopete (avdevice): showDeviceCapabilities() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevicePool::initDevice()
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

	m_io_method = IO_METHOD_NONE;
	switch(m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
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
				kdDebug() << "libkopete (avdevice): initDevice() Found no suitable input/output method for " << full_filename << endl;
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

// Select video input, video standard and tune here.
#ifdef __linux__
#ifdef HAVE_V4L2
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
#endif
#endif

	kdDebug() << "libkopete (avdevice): initDevice() exited successfuly" << endl;
	return EXIT_SUCCESS;
}

unsigned int VideoDevice::inputs()
{
	return input.size();
}


int VideoDevice::width()
{
	return currentwidth;
}

int VideoDevice::minWidth()
{
	return minwidth;
}

int VideoDevice::maxWidth()
{
	return maxwidth;
}

int VideoDevice::height()
{
	return currentheight;
}

int VideoDevice::minHeight()
{
	return minheight;
}

int VideoDevice::maxHeight()
{
	return maxheight;
}

int VideoDevice::setSize( int newwidth, int newheight)
{
kdDebug() << "libkopete: VideoDevice::SetSize(" << newwidth << ", " << newheight << ") called." << endl;
	if(newwidth  > maxwidth ) newwidth  = maxwidth;
	if(newheight > maxheight) newheight = maxheight;
	if(newwidth  < minwidth ) newwidth  = minwidth;
	if(newheight < minheight) newheight = minheight;

	currentwidth  = newwidth;
	currentheight = newheight;

// Change resolution and colorspace for the video device
	switch(m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			CLEAR (fmt);
			if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
//				return errnoReturn ("VIDIOC_S_FMT");
				kdDebug() << "libkopete (avdevice): VIDIOC_G_FMT failed (" << errno << ")." << endl;
			fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			fmt.fmt.pix.width       = width();
			fmt.fmt.pix.height      = height();
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
			fmt.fmt.pix.field       = V4L2_FIELD_ANY;
			if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
//				return errnoReturn ("VIDIOC_S_FMT");
				kdDebug() << "libkopete (avdevice): VIDIOC_S_FMT failed (" << errno << ")." << endl;
				// Note VIDIOC_S_FMT may change width and height.
			else
			{
// Buggy driver paranoia.
				min = fmt.fmt.pix.width * 2;
				if (fmt.fmt.pix.bytesperline < min)
					fmt.fmt.pix.bytesperline = min;
				min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
				if (fmt.fmt.pix.sizeimage < min)
					fmt.fmt.pix.sizeimage = min;
				m_buffer_size=fmt.fmt.pix.sizeimage ;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			{
				struct video_window V4L_videowindow;

kdDebug() << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

			if (xioctl (VIDIOCGWIN, &V4L_videowindow)== -1)
			{
				perror ("ioctl VIDIOCGWIN");
//				return (NULL);
			}
			V4L_videowindow.width  = width();
			V4L_videowindow.height = height();
			V4L_videowindow.clipcount=0;
			if (xioctl (VIDIOCSWIN, &V4L_videowindow)== -1)
			{
				perror ("ioctl VIDIOCSWIN");
//				return (NULL);
			}
kdDebug() << "------------- width: " << V4L_videowindow.width << " Height: " << V4L_videowindow.height << " Clipcount: " << V4L_videowindow.clipcount << " -----------------" << endl;

			struct video_picture V4L_picture;
			if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
				kdDebug() << "libkopete (avdevice): VIDIOCGPICT failed (" << errno << ")." << endl;
			kdDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;
			V4L_picture.palette = VIDEO_PALETTE_RGB32;
			V4L_picture.depth   = 32;
			if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
				kdDebug() << "libkopete (avdevice): VIDIOCSPICT failed (" << errno << ")." << endl;
			kdDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth << endl;

/*			if(-1 == xioctl(VIDIOCGFBUF,&V4L_videobuffer))
				kdDebug() << "libkopete (avdevice): VIDIOCGFBUF failed (" << errno << "): Card cannot stream" << endl;*/

			m_buffer_size = width() * height() * V4L_picture.depth / 8;
kdDebug() << "------------------------- ------- -- m_buffer_size: " << m_buffer_size << " !!! -- ------- -----------------------------------------" << endl;
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

kdDebug() << "libkopete: VideoDevice::SetSize(" << newwidth << ", " << newheight << ") exited successfulycalled." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::selectInput(int input)
 */
int VideoDevice::selectInput(int newinput)
{
    /// @todo implement me
	switch (m_driver)
	{
#ifdef __linux__
#ifdef HAVE_V4L2
		case VIDEODEV_DRIVER_V4L2:
			if (-1 == ioctl (descriptor, VIDIOC_S_INPUT, &newinput))
			{
				perror ("VIDIOC_S_INPUT");
				return EXIT_FAILURE;
			}
			break;
#endif
		case VIDEODEV_DRIVER_V4L:
			struct video_channel V4L_input;
			V4L_input.channel=newinput;
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
	kdDebug() << "libkopete (avdevice): selectInput: Selected input " << newinput << " (" << input[newinput].name << ")" << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::startCapturing()
 */
int VideoDevice::startCapturing()
{

	kdDebug() << "libkopete (avdevice): startCapturing() called." << endl;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
#ifdef __linux__
#ifdef HAVE_V4L2
			{
				unsigned int loop;
				for (loop = 0; loop < n_buffers; ++loop)
				{
					struct v4l2_buffer buf;
					CLEAR (buf);
					buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory = V4L2_MEMORY_MMAP;
					buf.index  = loop;
					if (-1 == xioctl (VIDIOC_QBUF, &buf))
						return errnoReturn ("VIDIOC_QBUF");
				}
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (-1 == xioctl (VIDIOC_STREAMON, &type))
					return errnoReturn ("VIDIOC_STREAMON");
			}
#endif
#endif
			break;
		case IO_METHOD_USERPTR:
#ifdef __linux__
#ifdef HAVE_V4L2
			{
				unsigned int loop;
				for (loop = 0; loop < n_buffers; ++loop)
				{
					struct v4l2_buffer buf;
					CLEAR (buf);
					buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory    = V4L2_MEMORY_USERPTR;
					buf.m.userptr = (unsigned long) buffers[loop].start;
					buf.length    = buffers[loop].length;
					if (-1 == xioctl (VIDIOC_QBUF, &buf))
						return errnoReturn ("VIDIOC_QBUF");
				}
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (-1 == xioctl (VIDIOC_STREAMON, &type))
					return errnoReturn ("VIDIOC_STREAMON");
			}
#endif
#endif
			break;
	}

	kdDebug() << "libkopete (avdevice): startCapturing() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevice::readFrame()
 */
int VideoDevice::readFrame()
{
    /// @todo implement me
	unsigned int i;
	ssize_t bytesread;

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
			bytesread = read (descriptor, &currentbuffer.data[0], currentbuffer.data.size());
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
    \fn VideoDevice::processImage(const void *p)
 */
int VideoDevice::processImage(const void * /* p */)
{
    /// @todo implement me
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevice::getImage(const QImage *qimage)
 */
int VideoDevice::getImage(QImage *qimage)
{
    /// @todo implement me
	unsigned long long result=0;
	unsigned long long R=0, G=0, B=0, A=0;
	int Rmax=0, Gmax=0, Bmax=0, Amax=0;
	int Rmin=255, Gmin=255, Bmin=255, Amin=0;

	for(unsigned int loop=0;loop < currentbuffer.data.size();loop++)
		result+=currentbuffer.data[loop];
	kdDebug() << "libkopete getImage(QImage *qimage) Data size: " << currentbuffer.data.size() << " Result: " << result << endl;
	for(unsigned int loop=0;loop < currentbuffer.data.size();loop+=4)
	{
		R+=currentbuffer.data[loop];
		G+=currentbuffer.data[loop+1];
		B+=currentbuffer.data[loop+2];
//		A+=currentbuffer.data[loop+3];
		if (currentbuffer.data[loop]   < Rmin) Rmin = currentbuffer.data[loop];
		if (currentbuffer.data[loop+1] < Gmin) Gmin = currentbuffer.data[loop+1];
		if (currentbuffer.data[loop+2] < Bmin) Bmin = currentbuffer.data[loop+2];
//		if (currentbuffer.data[loop+3] < Amin) Amin = currentbuffer.data[loop+3];
		if (currentbuffer.data[loop]   > Rmax) Rmax = currentbuffer.data[loop];
		if (currentbuffer.data[loop+1] > Gmax) Gmax = currentbuffer.data[loop+1];
		if (currentbuffer.data[loop+2] > Bmax) Bmax = currentbuffer.data[loop+2];
//		if (currentbuffer.data[loop+3] > Amax) Amax = currentbuffer.data[loop+3];
	}
	kdDebug() << " R: " << R << " G: " << G << " B: " << B << " A: " << A <<
		" Rmin: " << Rmin << " Gmin: " << Gmin << " Bmin: " << Bmin << " Amin: " << Amin <<
		" Rmax: " << Rmax << " Gmax: " << Gmax << " Bmax: " << Bmax << " Amax: " << Amax << endl;

	qimage->create(width(), height(),32, QImage::IgnoreEndian);

	memcpy(qimage->bits(),&currentbuffer.data[0], currentbuffer.data.size());
	return EXIT_SUCCESS;
}









/*!
    \fn VideoDevice::stopCapturing()
 */
int VideoDevice::stopCapturing()
{
    /// @todo implement me
	kdDebug() << "libkopete (avdevice): stopCapturing() called." << endl;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Card cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
#ifdef HAVE_V4L2
			{
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (-1 == xioctl (VIDIOC_STREAMOFF, &type))
					return errnoReturn ("VIDIOC_STREAMOFF");
			}
#endif
			break;
	}
	kdDebug() << "libkopete (avdevice): stopCapturing() exited successfuly." << endl;
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::close()
 */
int VideoDevice::close()
{
    /// @todo implement me
	if(isOpen())
	{
		stopCapturing();
		::close(descriptor);
	}
	descriptor = -1;
	return EXIT_SUCCESS;
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
			kdDebug() << "libkopete: initMmap () " << full_filename << " does not support memory mapping" << endl;
			return EXIT_FAILURE;
		}
		else
		{
			return errnoReturn ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		kdDebug() << "libkopete: initMmap () Insufficient buffer memory on " << full_filename << endl;
		return EXIT_FAILURE;
	}

	buffers.resize(req.count);

	if (buffers.size()==0)
	{
		kdDebug() <<  "libkopete: initMmap () Out of memory" << endl;
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
			kdDebug() << "libkopete: initUserPtr () " << full_filename << " does not support memory mapping" << endl;
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



}

}
