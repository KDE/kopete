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

#include <kdebug.h>
#include <klocale.h>
#include <qstring.h>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#undef __STRICT_ANSI__
#include <asm/types.h>
#include <linux/fs.h>
#include <linux/kernel.h>

#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif

#include <linux/videodev.h>
#define __STRICT_ANSI__

//#include "videoinput.h"
#include "videodevice.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevice *VideoDevice::s_self = NULL;

VideoInput::VideoInput()
{
}


VideoInput::~VideoInput()
{
}

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
	io_type    = IO_METHOD_MMAP;
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
	return EXIT_SUCCESS;
}


/*!
    \fn VideoDevice::initDevice()
 */
int VideoDevice::initDevice()
{
    /// @todo implement me
#ifdef HAVE_V4L2
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	int inputisok=EXIT_SUCCESS;

	memset(&cap, 0, sizeof(cap));

	if (-1 == xioctl (VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			fprintf (stderr, "%s is no V4L2 device\n", path.c_str());
			return EXIT_FAILURE;
		}
		else
		{
			return errnoReturn ("VIDIOC_QUERYCAP");
		}
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf (stderr, "%s is no video capture device\n", path.c_str());
		return EXIT_FAILURE;
	}
	switch (io_type)
	{
		case IO_METHOD_READ:
			if (!(cap.capabilities & V4L2_CAP_READWRITE))
			{
				fprintf (stderr, "%s does not support read i/o\n", path.c_str());
				return EXIT_FAILURE;
			}
			break;
		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
			if (!(cap.capabilities & V4L2_CAP_STREAMING))
			{
				fprintf (stderr, "%s does not support streaming i/o\n", path.c_str());
				return EXIT_FAILURE;
			}
			break;
	}

//	kdDebug() << "V4L2Dev: device \"" << cap << "\" capabilities: " << endl;
	kdDebug() << "  Driver: " << (const char*)cap.driver << " "
		<< ((cap.version>>16) & 0xFF) << "."
		<< ((cap.version>> 8) & 0xFF) << "."
		<< ((cap.version    ) & 0xFF) << endl;
	kdDebug() << "  Card: " << (const char*)cap.card << endl;
	kdDebug() << "  Bus info: " << (const char*)cap.card << endl;

	kdDebug() << "  Capabilities:" << endl;
	if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
		kdDebug() << "    Video capture" << endl;
	if(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
		kdDebug() << "    Video output" << endl;
	if(cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
		kdDebug() << "    Video overlay" << endl;
	if(cap.capabilities & V4L2_CAP_VBI_CAPTURE)
		kdDebug() << "    VBI capture" << endl;
	if(cap.capabilities & V4L2_CAP_VBI_OUTPUT)
		kdDebug() << "    VBI output" << endl;
	if(cap.capabilities & V4L2_CAP_RDS_CAPTURE)
		kdDebug() << "    RDS capture" << endl;
	if(cap.capabilities & V4L2_CAP_TUNER)
		kdDebug() << "    Tuner IO" << endl;
	if(cap.capabilities & V4L2_CAP_AUDIO)
		kdDebug() << "    Audio IO" << endl;
	if(cap.capabilities & V4L2_CAP_READWRITE)
		kdDebug() << "    Read/Write interface" << endl;
	if(cap.capabilities & V4L2_CAP_ASYNCIO)
		kdDebug() << "    Async IO interface" << endl;
	if(cap.capabilities & V4L2_CAP_STREAMING)
		kdDebug() << "    Streaming interface" << endl;

	kdDebug() << "Enumerating video inputs: " << endl;
//    ok = true;
	m_video_input.clear();
	for(m_video_inputs=0; inputisok==EXIT_SUCCESS; m_video_inputs++)
	{
		struct v4l2_input input;
		memset(&input, 0, sizeof(input));
		input.index = m_video_inputs;
		inputisok=xioctl(VIDIOC_ENUMINPUT, &input);
		if(inputisok==EXIT_SUCCESS)
		{
			Kopete::AV::VideoInput tempinput;
			tempinput.name = QString::fromLatin1((const char*)input.name);
			tempinput.hastuner=input.type & V4L2_INPUT_TYPE_TUNER;
			m_video_input.push_back(tempinput);
			kdDebug() << "Input " << m_video_inputs << ": " << tempinput.name << " (tuner: " << ((input.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")" << endl;
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
	kdDebug() << "Grand total of " << m_video_inputs << " video inputs for this device." << endl;
	for (int loop=0; loop < m_video_inputs; loop++)
		kdDebug() << "Input " << loop << ": " << m_video_input[loop].name << " (tuner: " << m_video_input[loop].hastuner << ")" << endl;



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
		return errnoReturn ("VIDIOC_S_FMT");

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

	if (buffers.size()==0)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc (buffer_size);

	if (!buffers[0].start)
	{
		fprintf (stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}
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
		buffers[n_buffers].start = mmap (NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, descriptor, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
		return errnoReturn ("mmap");
	}
#endif
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
		buffers[n_buffers].start = malloc (buffer_size);

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

	switch (io_type)
	{
		case IO_METHOD_READ: /* Nothing to do. */
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

	switch (io_type)
	{
		case IO_METHOD_READ:
			if (-1 == read (descriptor, buffers[0].start, buffers[0].length))
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
			if (-1 == xioctl (VIDIOC_QBUF, &buf))
				return errnoReturn ("VIDIOC_QBUF");
			break;
	}
#endif
// put frame copy operation here
	return EXIT_SUCCESS;
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
	return EXIT_FAILURE;
	}
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
    \fn Kopete::AV::VideoDevice::scanDevices()
 */
int Kopete::AV::VideoDevice::scanDevices()
{
    /// @todo implement me
/*
int vpScan(struct VideoDevicePool *devicepool)
{
  unsigned char currentdirectory[STRING_BUFFER];
  char path[]="/dev/v4l/";
  char devname[]="video";
  struct dirent **eps;
  struct stat status;
  int n=0;
  int cnt;

  getcwd(currentdirectory,STRING_SIZE);
    
  devicepool->numdevices=0;
  devicepool->numinputs=0;

  chdir(path);
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
//            devicepool->numinputs+=devicepool->device[devicepool->numdevices].capabilities.channels;
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
  chdir(currentdirectory);
  return 0;
}
*/
	return EXIT_SUCCESS;
}
