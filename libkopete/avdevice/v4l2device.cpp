/*
    videodevice.cpp  -  Kopete Video Device Low-level Support

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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

#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>

#include "videoinput.h"
#include "videodevice.h"
#include "v4l2device.h"

#include "bayer.h"
#include "sonix_compress.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

#ifdef V4L2_CAP_VIDEO_CAPTURE

V4l2Device::V4l2Device()
//: new VideoDevice()
{
	kDebug() << "Create a v4l2 video device.";
}


V4l2Device::~V4l2Device()
{
}

static bool qSizeSort(const QSize& a, const QSize& b)
{
	if (a.width() < b.width() || a.height() < b.height())
		return true;
	return false;
}

void V4l2Device::sortFrameSizes()
{
	qSort(m_frameSizes.begin(), m_frameSizes.end(), qSizeSort);
}

void V4l2Device::enumerateControls (void)
{
// -----------------------------------------------------------------------------------------------------------------
// This must turn up to be a proper method to check for controls' existence.
	CLEAR (queryctrl);
// v4l2_queryctrl may zero the .id in some cases, even if the IOCTL returns EXIT_SUCCESS (tested with a bttv card, when testing for V4L2_CID_AUDIO_VOLUME).
// As of 6th Aug 2007, according to the V4L2 specification version 0.21, this behavior is undocumented, and the example 1-8 code found at
// http://www.linuxtv.org/downloads/video4linux/API/V4L2_API/spec/x519.htm fails because of this behavior with a bttv card.

	int currentid = V4L2_CID_BASE;

	kDebug() << "Checking CID controls";

	for (currentid = V4L2_CID_BASE; currentid < V4L2_CID_LASTP1; currentid++)
	{
		queryctrl.id = currentid;
		if (0 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
		{
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;

			kDebug() << "Control :"
				 << QString::fromLocal8Bit((const char*)queryctrl.name)
				 << "Values from"
				 << queryctrl.minimum
				 << "to"
				 << queryctrl.maximum
				 << "with steps of"
				 << queryctrl.step
				 << ". Default :"
				 << queryctrl.default_value;

			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerateMenu ();
		}
		else
		{
			if (errno == EINVAL)
				continue;

			perror ("VIDIOC_QUERYCTRL");
		}
	}

	kDebug() << "Checking CID private controls";

	for (currentid = V4L2_CID_PRIVATE_BASE;; currentid++)
	{
		queryctrl.id = currentid;
		if ( 0 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
		{
			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				continue;

			kDebug() << "Control :"
				 << QString::fromLocal8Bit((const char*)queryctrl.name)
				 << "Values from"
				 << queryctrl.minimum
				 << "to"
				 << queryctrl.maximum
				 << "with steps of"
				 << queryctrl.step
				 << ". Default :"
				 << queryctrl.default_value;

			if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
				enumerateMenu ();
		}
		else
		{
			if (errno == EINVAL)
				break;

			perror ("VIDIOC_QUERYCTRL");
		}
	}
}

void V4l2Device::enumerateMenu (void)
{
//This only lists controls on stdout in debug mode, no need to spend time on this.

	kDebug() <<  "  Menu items:";

	memset (&querymenu, 0, sizeof (querymenu));
	querymenu.id = queryctrl.id;

	for (querymenu.index = queryctrl.minimum; querymenu.index <= queryctrl.maximum; querymenu.index++)
	{
		if (0 == xioctl (VIDIOC_QUERYMENU, &querymenu))
		{
			kDebug() <<  "  " << QString::fromLocal8Bit((const char*)querymenu.name);
                }
		else
		{
			perror ("VIDIOC_QUERYMENU");
			exit (EXIT_FAILURE);
		}
	}
}

int V4l2Device::checkDevice()
{
	kDebug() << "checkDevice() called.";
	
	if(!isOpen())
	{
		kDebug() << "File is not open, stopping";
		return EXIT_FAILURE;
	}

	m_videocapture=false;
	m_videochromakey=false;
	m_videoscale=false;
	m_videooverlay=false;
	m_videoread=false;
	m_videoasyncio=false;
	m_videostream=false;
	
	CLEAR(V4L2_capabilities);

	if (-1 != xioctl (VIDIOC_QUERYCAP, &V4L2_capabilities))
	{
		if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			kDebug() << full_filename << " is not a video capture device.";
			return EXIT_FAILURE;
		}
		m_videocapture = true;
		kDebug() << full_filename << " is a V4L2 device.";
		m_model = QString::fromLocal8Bit((const char*)V4L2_capabilities.card);
		
		if (detectPixelFormats() == EXIT_FAILURE)
		{
			kDebug() << "No pixel format detected.";
			return EXIT_FAILURE;
		}
		
#ifdef VIDIOC_ENUM_FRAMESIZES
		struct v4l2_frmsizeenum sizes;
		int i = 0;
		sizes.index = i;
		sizes.pixel_format = m_pixelformat;
		while(-1 != xioctl(VIDIOC_ENUM_FRAMESIZES, &sizes))
		{
			kDebug() << "Add res" << QSize((int)sizes.discrete.width, (int)sizes.discrete.height);
			m_frameSizes << QSize((int)sizes.discrete.width, (int)sizes.discrete.height);
			sizes.index = ++i;
		}
		sortFrameSizes();
#else
		CLEAR (fmt);
		// Checking for the most little frame size
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.type                	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width       	= 1;
		fmt.fmt.pix.height      	= 1;
		fmt.fmt.pix.field       	= V4L2_FIELD_ANY;
		fmt.fmt.pix.pixelformat 	= m_pixelformat;
		if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
		{
			kDebug() << "Detecting maximum size with VIDIOC_S_FMT failed (" << errno << ").";
			kDebug() << "Returned maxwidth: " << pixelFormatName(fmt.fmt.pix.pixelformat)
				 << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
		}
		else
		{
			m_frameSize.append(QSize(fmt.fmt.pix.width, fmt.fmt.pix.height));
		}

		// Checking for the biggest frame size.
		fmt.type                	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width       	= 32767;
		fmt.fmt.pix.height      	= 32767;
		fmt.fmt.pix.field       	= V4L2_FIELD_ANY;
		fmt.fmt.pix.pixelformat 	= m_pixelformat;
		if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
		{
			kDebug() << "Detecting maximum size with VIDIOC_S_FMT failed (" << errno << ").";
			kDebug() << "Returned maxwidth: " << pixelFormatName(fmt.fmt.pix.pixelformat)
				 << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
		}
		else
		{
			m_frameSize.append(QSize(fmt.fmt.pix.width, fmt.fmt.pix.height));
		}
#endif

		int inputisok = EXIT_SUCCESS;
		m_input.clear();
		for(unsigned int loop = 0; inputisok == EXIT_SUCCESS; loop++)
		{
			struct v4l2_input videoinput;
			CLEAR(videoinput);
			videoinput.index = loop;
			inputisok = xioctl(VIDIOC_ENUMINPUT, &videoinput);
			if(inputisok == EXIT_SUCCESS)
			{
				VideoInput tempinput;
				tempinput.name = QString::fromLocal8Bit((const char*)videoinput.name);
				tempinput.hastuner = videoinput.type & V4L2_INPUT_TYPE_TUNER;
				detectSignalStandards();
				tempinput.m_standards = videoinput.std;
				m_input.push_back(tempinput);
				kDebug() << "Input " << loop << ": " << tempinput.name << " (tuner: " << ((videoinput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")";
			}
		}
	}
	else
	{
		kDebug() << "checkDevice(): " << full_filename << " is not a V4L2 device.";
	}

	m_name = m_model; // Take care about changing the name to be different from the model itself...
	//FIXME:This comment is the opposit of the affection.

	enumerateControls();
	kDebug() << "checkDevice() exited successfuly.";

	return EXIT_SUCCESS;
}


int V4l2Device::initDevice()
{
	kDebug() << "initDevice() started";
	if(!isOpen())
	{
		kDebug() << "Device is not open";
		return EXIT_FAILURE;
	}
	
	m_io_method = IO_METHOD_NONE;
	if(V4L2_capabilities.capabilities & V4L2_CAP_READWRITE)
	{
		m_videoread = true;
		m_io_method = IO_METHOD_READ;
		kDebug() << "    Read/Write interface";
	}
	if(V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
	{
		m_videoasyncio = true;
		kDebug() << "    Async IO interface";
	}
	if(V4L2_capabilities.capabilities & V4L2_CAP_STREAMING)
	{
		m_videostream = true;
		m_io_method = IO_METHOD_MMAP;
		kDebug() << "    Streaming interface";
	}

	if(m_io_method == IO_METHOD_NONE)
	{
		kDebug() << "initDevice() Found no suitable input/output method for " << full_filename;
		return EXIT_FAILURE;
	}
	
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

	showDeviceCapabilities();
	kDebug() << "Exited successfuly";
	return EXIT_SUCCESS;
}

int V4l2Device::setSize(QSize newSize)
{
	kDebug() << "setting size" << newSize;

	if(!isOpen())
	{
		kDebug() << "The device is not opened.";
		return EXIT_FAILURE;
	}

	if (newSize.width() > m_frameSizes.last().width())
		newSize.setWidth(m_frameSizes.last().width());
	if (newSize.height() > m_frameSizes.last().height())
		newSize.setHeight(m_frameSizes.last().height());
	
	if (newSize.width() < m_frameSizes.first().width())
		newSize.setWidth(m_frameSizes.first().width());
	if (newSize.height() < m_frameSizes.first().height())
		newSize.setHeight(m_frameSizes.first().height());

	currentFrameSize = newSize;

	// Change frame size for the video device
	CLEAR (fmt);
	kDebug() << "fmt.fmt.pix.pixelformat =" << fmt.fmt.pix.pixelformat;
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = frameSize().width();
	fmt.fmt.pix.height      = frameSize().height();
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;
	fmt.fmt.pix.pixelformat = m_pixelformat;

	if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
	{
		kDebug() << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
	}
	else
	{
		// Buggy driver paranoia.
		kDebug() << "VIDIOC_S_FMT worked (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
		unsigned int min = fmt.fmt.pix.width * 2;
		if (fmt.fmt.pix.bytesperline < min)
			fmt.fmt.pix.bytesperline = min;
		min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
		if (fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;
		m_buffer_size=fmt.fmt.pix.sizeimage ;
	}

	m_pixelformat = fmt.fmt.pix.pixelformat;
	currentFrameSize.setWidth(fmt.fmt.pix.width);
	currentFrameSize.setHeight(fmt.fmt.pix.height);
	kDebug() << "pixelFormatDepth(m_pixelformat) = " << pixelFormatDepth(m_pixelformat);
	m_buffer_size = frameSize().width() * frameSize().height() * pixelFormatDepth(m_pixelformat) / 8;
	kDebug() << "------------------------- ------- -- m_buffer_size: " << m_buffer_size << " !!! -- ------- -----------------------------------------";

	m_currentbuffer.pixelformat=m_pixelformat;
	m_currentbuffer.data.resize(m_buffer_size);

	switch (m_io_method)
	{
		case IO_METHOD_NONE:                    break;
		case IO_METHOD_READ:    initRead ();    break;
		case IO_METHOD_MMAP:    initMmap ();    break;
		case IO_METHOD_USERPTR: initUserptr (); break;
	}

	kDebug() << "exited successfuly.";

	return EXIT_SUCCESS;
}

unsigned int V4l2Device::setPixelFormat(unsigned int newformat)
{
	// Change the pixel format for the video device
	int ret = 0;
	kDebug() << "Setting new pixel format :" << pixelFormatName(newformat);
	CLEAR (fmt);
	if (-1 == xioctl (VIDIOC_G_FMT, &fmt))
	{
		kDebug() << "VIDIOC_G_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
	}
	else
		m_pixelformat = fmt.fmt.pix.pixelformat;

	fmt.fmt.pix.pixelformat = newformat;
	fmt.fmt.pix.width = frameSize().width();
	fmt.fmt.pix.height = frameSize().height();
	if (-1 == xioctl (VIDIOC_S_FMT, &fmt))
	{
		kDebug() << "VIDIOC_S_FMT failed (" << errno << ").Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
	}
	else
	{
		if (fmt.fmt.pix.pixelformat == newformat) // Thih "if" (not what is contained within) is a fix for a bug in sn9c102 driver.
		{
			m_pixelformat = newformat;
			ret = m_pixelformat;
		}
	}
	kDebug() << "New pixel format is" << pixelFormatName(m_pixelformat) << "==" << pixelFormatName(fmt.fmt.pix.pixelformat);
	return ret;
}

int V4l2Device::selectInput(int newinput)
{
	if (m_current_input >= inputs() || !isOpen())
		return EXIT_FAILURE;

	if (-1 == ioctl (descriptor, VIDIOC_S_INPUT, &newinput))
	{
		perror ("VIDIOC_S_INPUT");
		return EXIT_FAILURE;
	}
	kDebug() << "Selected input " << newinput << " (" << m_input[newinput].name << ")";
	
	m_current_input = newinput;
	
	setInputParameters();

	return EXIT_SUCCESS;
}

int V4l2Device::startCapturing()
{

	kDebug() << "called.";
	if(!isOpen())
	{
		return EXIT_FAILURE;
	}
	enum v4l2_buf_type type;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Device cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
			unsigned int loop;
			for (loop = 0; loop < m_streambuffers; ++loop)
			{
				struct v4l2_buffer buf;
				CLEAR (buf);
				buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				buf.index  = loop;
				if (-1 == xioctl (VIDIOC_QBUF, &buf))
					return errnoReturn ("VIDIOC_QBUF");
			}
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl (VIDIOC_STREAMON, &type))
				return errnoReturn ("VIDIOC_STREAMON");
			break;
		case IO_METHOD_USERPTR:
			{
				unsigned int loop;
				for (loop = 0; loop < m_streambuffers; ++loop)
				{
					struct v4l2_buffer buf;
					CLEAR (buf);
					buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					buf.memory    = V4L2_MEMORY_USERPTR;
					buf.m.userptr = (unsigned long) m_rawbuffers[loop].start;
					buf.length    = m_rawbuffers[loop].length;
					if (-1 == xioctl (VIDIOC_QBUF, &buf))
						return errnoReturn ("VIDIOC_QBUF");
				}
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (-1 == xioctl (VIDIOC_STREAMON, &type))
					return errnoReturn ("VIDIOC_STREAMON");
			}
			break;
	}

	kDebug() << "exited successfuly.";
	return EXIT_SUCCESS;
}

int V4l2Device::getFrame()
{
	ssize_t bytesread;

	struct v4l2_buffer v4l2buffer;
	//kDebug() << "getFrame() called.";
	if(!isOpen())
		return EXIT_FAILURE;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Device cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ:
 			//kDebug() << "Using IO_METHOD_READ.File descriptor: " << descriptor << " Buffer address: " << &m_currentbuffer.data[0] << " Size: " << m_currentbuffer.data.size();
			if (m_currentbuffer.data.isEmpty())
				return EXIT_FAILURE;

			bytesread = read (descriptor, &m_currentbuffer.data[0], m_currentbuffer.data.size());
			if (-1 == bytesread) // must verify this point with ov511 driver.
			{
				kDebug() << "IO_METHOD_READ failed.";
				switch (errno)
				{
					case EAGAIN:
						return EXIT_FAILURE;
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
					return errnoReturn ("read");
				}
			}
			if((int)m_currentbuffer.data.size() < bytesread)
			{
				kDebug() << "IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << m_currentbuffer.data.size() <<").";
			}
			break;
		case IO_METHOD_MMAP:
			CLEAR (v4l2buffer);
			v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2buffer.memory = V4L2_MEMORY_MMAP;
			if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer))
			{
				kDebug() << full_filename << " MMAPed getFrame failed.";
				switch (errno)
				{
					case EAGAIN:
					{
						kDebug() << full_filename << " MMAPed getFrame failed: EAGAIN. Pointer: ";
						return EXIT_FAILURE;
					}
					case EIO: /* Could ignore EIO, see spec. fall through */
					default:
						return errnoReturn ("VIDIOC_DQBUF");
				}
			}
			if (m_currentbuffer.data.isEmpty() || (unsigned int)m_rawbuffers.size() <= v4l2buffer.index)
				return EXIT_FAILURE;

			memcpy(&m_currentbuffer.data[0], m_rawbuffers[v4l2buffer.index].start, m_currentbuffer.data.size());
			if (-1 == xioctl (VIDIOC_QBUF, &v4l2buffer))
				return errnoReturn ("VIDIOC_QBUF");
			break;
		case IO_METHOD_USERPTR:
			{
				unsigned int i;
				CLEAR (v4l2buffer);
				v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				v4l2buffer.memory = V4L2_MEMORY_USERPTR;
				if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer))
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
				if ((unsigned int)m_rawbuffers.size() < m_streambuffers)
					return EXIT_FAILURE;
				
				for (i = 0; i < m_streambuffers; ++i)
					if (v4l2buffer.m.userptr == (unsigned long) m_rawbuffers[i].start && v4l2buffer.length == m_rawbuffers[i].length)
						break;
				if (i < m_streambuffers)
					return EXIT_FAILURE;
				if (-1 == xioctl (VIDIOC_QBUF, &v4l2buffer))
				return errnoReturn ("VIDIOC_QBUF");
			}
			break;
	}
	return EXIT_SUCCESS;
}

int V4l2Device::stopCapturing()
{
	kDebug() << "called.";
	if(!isOpen())
		return EXIT_FAILURE;
	switch (m_io_method)
	{
		case IO_METHOD_NONE: // Device cannot capture frames
			return EXIT_FAILURE;
			break;
		case IO_METHOD_READ: // Nothing to do
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			{
				enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if (-1 == xioctl (VIDIOC_STREAMOFF, &type))
					return errnoReturn ("VIDIOC_STREAMOFF");

				if (m_io_method == IO_METHOD_MMAP)
				{
					unsigned int loop;
					for (loop = 0; loop < m_streambuffers; ++loop)
					{
						if (munmap(m_rawbuffers[loop].start,m_rawbuffers[loop].length) != 0)
						{
							kDebug() << "unable to munmap.";
						}
					}
				}
			}
			break;
	}
	return EXIT_SUCCESS;
}

float V4l2Device::setBrightness(float brightness)
{
	kDebug() << "(" << brightness << ") called.";
	m_input[m_current_input].setBrightness(brightness); // Just to check bounds

	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control;

	CLEAR (queryctrl);
	queryctrl.id = V4L2_CID_BRIGHTNESS;

	if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
	{
		if (errno != EINVAL)
			kDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
		else
			kDebug() << "Device doesn't support the Brightness control.";
	}
	else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
		kDebug() << "Brightness control is disabled.";
	else
	{
		CLEAR (control);
		control.id = V4L2_CID_BRIGHTNESS;
		control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getBrightness());

		if (-1 == xioctl (VIDIOC_S_CTRL, &control))
		{
			kDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
		}
	}
	return getBrightness();
}

float V4l2Device::setContrast(float contrast)
{
	kDebug() << "(" << contrast << ") called.";
	m_input[m_current_input].setContrast(contrast); // Just to check bounds

				struct v4l2_queryctrl queryctrl;
				struct v4l2_control control;

				CLEAR (queryctrl);
				queryctrl.id = V4L2_CID_CONTRAST;

				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
				{
					if (errno != EINVAL)
					{
						kDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
					} else
					{
						kDebug() << "Device doesn't support the Contrast control.";
					}
				} else
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				{
					kDebug() << "Contrast control is disabled.";
				} else
				{
					CLEAR (control);
					control.id = V4L2_CID_CONTRAST;
					control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getContrast());

					if (-1 == xioctl (VIDIOC_S_CTRL, &control))
					{
						kDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
					}
				}
	return getContrast();
}

float V4l2Device::setSaturation(float saturation)
{
	kDebug() << "(" << saturation << ") called.";
	m_input[m_current_input].setSaturation(saturation); // Just to check bounds

				struct v4l2_queryctrl queryctrl;
				struct v4l2_control control;

				CLEAR (queryctrl);
				queryctrl.id = V4L2_CID_SATURATION;

				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
				{
					if (errno != EINVAL)
					{
						kDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
					} else
					{
						kDebug() << "Device doesn't support the Saturation control.";
					}
				} else
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				{
					kDebug() << "Saturation control is disabled.";
				} else
				{
					CLEAR (control);
					control.id = V4L2_CID_SATURATION;
					control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getSaturation());

					if (-1 == xioctl (VIDIOC_S_CTRL, &control))
					{
						kDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
					}
				}
	return getSaturation();
}

float V4l2Device::setWhiteness(float whiteness)
{
	kDebug() << "(" << whiteness << ") called.";
	m_input[m_current_input].setWhiteness(whiteness); // Just to check bounds

				struct v4l2_queryctrl queryctrl;
				struct v4l2_control control;

				CLEAR (queryctrl);
				queryctrl.id = V4L2_CID_WHITENESS;

				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
				{
					if (errno != EINVAL)
					{
						kDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
					} else
					{
						kDebug() << "Device doesn't support the Whiteness control.";
					}
				} else
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				{
					kDebug() << "Whiteness control is disabled.";
				} else
				{
					CLEAR (control);
					control.id = V4L2_CID_WHITENESS;
					control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getWhiteness());

					if (-1 == xioctl (VIDIOC_S_CTRL, &control))
					{
						kDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
					}
				}
	return getWhiteness();
}

float V4l2Device::setHue(float hue)
{
	kDebug() << "(" << hue << ") called.";
	m_input[m_current_input].setHue(hue); // Just to check bounds

				struct v4l2_queryctrl queryctrl;
				struct v4l2_control control;

				CLEAR (queryctrl);
				queryctrl.id = V4L2_CID_HUE;

				if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
				{
					if (errno != EINVAL)
					{
						kDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
					} else
					{
						kDebug() << "Device doesn't support the Hue control.";
					}
				} else
				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
				{
					kDebug() << "Hue control is disabled.";
				} else
				{
					CLEAR (control);
					control.id = V4L2_CID_HUE;
					control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getHue());

					if (-1 == xioctl (VIDIOC_S_CTRL, &control))
					{
						kDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
					}
				}
	return getHue();
}


int V4l2Device::detectPixelFormats()
{
	if (!isOpen())
		return EXIT_FAILURE;

	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	kDebug() << "Supported pixel formats :";

	while (-1 != xioctl (VIDIOC_ENUM_FMT, &fmtdesc))
	{
		kDebug() << "Detect Pixel format :" << pixelFormatName(fmtdesc.pixelformat);
		m_pixelFormats << fmtdesc.pixelformat;
		fmtdesc.index++;
	}
	
	if (m_pixelFormats.isEmpty())
		return EXIT_FAILURE;
	
	m_pixelformat = m_pixelFormats.first();
	
	return EXIT_SUCCESS;
}

int V4l2Device::detectSignalStandards()
{
	kDebug() << "called.";
	if(!isOpen())
		return EXIT_FAILURE;
				struct v4l2_input input;
				struct v4l2_standard standard;

				CLEAR(input);

				if (-1 == xioctl (VIDIOC_G_INPUT, &input.index)) {
					perror ("VIDIOC_G_INPUT");
					return EXIT_FAILURE;
				}

				if (-1 == xioctl (VIDIOC_ENUMINPUT, &input)) {
					perror ("VIDIOC_ENUM_INPUT");
					return EXIT_FAILURE;
				}

				CLEAR(standard);
				standard.index = 0;

				while (0 == xioctl (VIDIOC_ENUMSTD, &standard)) {
					if (standard.id & input.std)
						kDebug() << signalStandardName(standard.id) << " (" << standard.id << ")" << V4L2_STD_NTSC;

					standard.index++;
				}

				if (errno != EINVAL || standard.index == 0) {
					perror ("VIDIOC_ENUMSTD");
					return EXIT_FAILURE;
				}
	return EXIT_SUCCESS;
}

int V4l2Device::initRead()
{
	kDebug() << "called.";
	if(isOpen())
	{
		m_rawbuffers.resize(1);
		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kDebug() << "m_buffer_size: " << m_buffer_size;

//		m_rawbuffers[0].pixelformat=m_pixelformat;
		m_rawbuffers[0].length = m_buffer_size;
		m_rawbuffers[0].start = (uchar *)malloc (m_buffer_size);

		if (!m_rawbuffers[0].start)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}
		kDebug() << "exited successfuly.";
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


int V4l2Device::initMmap()
{
	// A better way than QTimer to fetch images each x msec, is a notifier.
	// Of course, it would work only for MMAP and maybe UserPtr
	// A simple QSocketNotifier on the file descriptor would emit a signal
	// each there is data to read, that means, each time a buffer is ready to dequeued.
	// This notifier could be set up here and started in startCapture().
#define BUFFERS 2
	if(isOpen())
	{
		kDebug() << full_filename << " Trying to MMAP";
		struct v4l2_requestbuffers req;

		CLEAR (req);

		req.count  = BUFFERS;
		req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl (VIDIOC_REQBUFS, &req))
		{
			if (EINVAL == errno)
			{
				kDebug() << full_filename << " does not support memory mapping";
				return EXIT_FAILURE;
			}
			else
			{
				return errnoReturn ("VIDIOC_REQBUFS");
			}
		}

		if (req.count < BUFFERS)
		{
			kDebug() << "Insufficient buffer memory on " << full_filename;
			return EXIT_FAILURE;
		}

		m_rawbuffers.resize(req.count);

		if (m_rawbuffers.size()==0)
		{
			kDebug() <<  "Out of memory";
			return EXIT_FAILURE;
		}

		for (m_streambuffers = 0; m_streambuffers < req.count; ++m_streambuffers)
		{
			struct v4l2_buffer v4l2buffer;

			CLEAR (v4l2buffer);

			v4l2buffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			v4l2buffer.memory = V4L2_MEMORY_MMAP;
			v4l2buffer.index  = m_streambuffers;

			if (-1 == xioctl (VIDIOC_QUERYBUF, &v4l2buffer))
				return errnoReturn ("VIDIOC_QUERYBUF");

			m_rawbuffers[m_streambuffers].length = v4l2buffer.length;
			m_rawbuffers[m_streambuffers].start = (uchar *) mmap (NULL /* start anywhere */, v4l2buffer.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, descriptor, v4l2buffer.m.offset);

			if (MAP_FAILED == m_rawbuffers[m_streambuffers].start)
			return errnoReturn ("mmap");
		}
		m_currentbuffer.data.resize(m_rawbuffers[0].length); // Makes the imagesize.data buffer size equal to the rawbuffer size
		kDebug() << full_filename << " m_currentbuffer.data.size(): " << m_currentbuffer.data.size();
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


int V4l2Device::initUserptr()
{
	if(isOpen())
	{
		struct v4l2_requestbuffers req;

		CLEAR (req);

		req.count  = 2;
		req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl (VIDIOC_REQBUFS, &req))
		{
			if (EINVAL == errno)
			{
				kDebug() << full_filename << " does not support memory mapping";
				return EXIT_FAILURE;
			}
			else
			{
				return errnoReturn ("VIDIOC_REQBUFS");
			}
		}

		m_rawbuffers.resize(4);

		if (m_rawbuffers.size()==0)
		{
			fprintf (stderr, "Out of memory\n");
			return EXIT_FAILURE;
		}

		for (m_streambuffers = 0; m_streambuffers < 4; ++m_streambuffers)
		{
			m_rawbuffers[m_streambuffers].length = m_buffer_size;
			m_rawbuffers[m_streambuffers].start = (uchar *) malloc (m_buffer_size);

			if (!m_rawbuffers[m_streambuffers].start)
			{
				kDebug() <<  "Out of memory";
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

}

}

#endif //V4L2_CAP_VIDEO_CAPTURE
