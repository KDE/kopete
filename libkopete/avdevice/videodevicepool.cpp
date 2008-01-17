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

#define ENABLE_AV

#include <assert.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>
#include <klocale.h>
#include <qdir.h>

#include "videodevice.h"
#include "videodevicepool.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevicePool *VideoDevicePool::s_self = NULL;
__u64 VideoDevicePool::m_clients = 0;

VideoDevicePool* VideoDevicePool::self()
{
	kdDebug(14010) << "libkopete (avdevice): self() called" << endl;
	if (s_self == NULL)
	{
		s_self = new VideoDevicePool;
		if (s_self)
			m_clients = 0;
	}
	kdDebug(14010) << "libkopete (avdevice): self() exited successfuly. m_clients = " << m_clients << endl;
	return s_self;
}

VideoDevicePool::VideoDevicePool()
{
}


VideoDevicePool::~VideoDevicePool()
{
}




/*!
    \fn VideoDevicePool::open()
 */
int VideoDevicePool::open()
{
    /// @todo implement me

	m_ready.lock();
	if(!m_videodevice.size())
	{
		kdDebug(14010) <<  k_funcinfo << "open(): No devices found. Must scan for available devices." << m_current_device << endl;
		scanDevices();
	}
	if(!m_videodevice.size())
	{
		kdDebug(14010) <<  k_funcinfo << "open(): No devices found. bailing out." << m_current_device << endl;
		m_ready.unlock();
		return EXIT_FAILURE;
	}
	if(m_current_device >= m_videodevice.size())
	{
		kdDebug(14010) <<  k_funcinfo << "open(): Device out of scope (" << m_current_device << "). Defaulting to the first one." << endl;
		m_current_device = 0;
	}
	int isopen = m_videodevice[currentDevice()].open();
	if ( isopen == EXIT_SUCCESS)
	{
		loadConfig(); // Temporary hack. The open() seems to clean the input parameters. Need to find a way to fix it.
		
	}
	m_clients++;
	kdDebug(14010) << k_funcinfo << "Number of clients: " << m_clients << endl;
	m_ready.unlock();
	return isopen;
}

/*!
    \fn VideoDevicePool::open(int device)
 */
int VideoDevicePool::open(unsigned int device)
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "open(" << device << ") called." << endl;
	if(device >= m_videodevice.size())
	{
		kdDebug(14010) <<  k_funcinfo << "open(" << device <<"): Device does not exist." << endl;
		return EXIT_FAILURE;
	}
	close();
	kdDebug(14010) <<  k_funcinfo << "open(" << device << ") Setting m_current_Device to " << device << endl;
	m_current_device = device;
	saveConfig();
	kdDebug(14010) <<  k_funcinfo << "open(" << device << ") Calling open()." << endl;
	return open();
}

bool VideoDevicePool::isOpen()
{
	return m_videodevice[currentDevice()].isOpen();
}

/*!
    \fn VideoDevicePool::showDeviceCapabilities(int device)
 */
int VideoDevicePool::showDeviceCapabilities(unsigned int device)
{
	return m_videodevice[device].showDeviceCapabilities();
}

int VideoDevicePool::width()
{
	return m_videodevice[currentDevice()].width();
}

int VideoDevicePool::minWidth()
{
	return m_videodevice[currentDevice()].minWidth();
}

int VideoDevicePool::maxWidth()
{
	return m_videodevice[currentDevice()].maxWidth();
}

int VideoDevicePool::height()
{
	return m_videodevice[currentDevice()].height();
}

int VideoDevicePool::minHeight()
{
	return m_videodevice[currentDevice()].minHeight();
}

int VideoDevicePool::maxHeight()
{
	return m_videodevice[currentDevice()].maxHeight();
}

int VideoDevicePool::setSize( int newwidth, int newheight)
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].setSize(newwidth, newheight);
	else
	{
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::setSize() fallback for no device." << endl;
		m_buffer.width=newwidth;
		m_buffer.height=newheight;
		m_buffer.pixelformat=	PIXELFORMAT_RGB24;
		m_buffer.data.resize(m_buffer.width*m_buffer.height*3);
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::setSize() buffer size: "<< m_buffer.data.size() << endl;
	}
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevicePool::close()
 */
int VideoDevicePool::close()
{
    /// @todo implement me
	if(m_clients)
		m_clients--;
	if((currentDevice() < m_videodevice.size())&&(!m_clients))
		return m_videodevice[currentDevice()].close();
	if(m_clients)
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::close() The video device is still in use." << endl;
	if(currentDevice() >= m_videodevice.size())
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::close() Current device out of range." << endl;
	return EXIT_FAILURE;
}

/*!
    \fn VideoDevicePool::startCapturing()
 */
int VideoDevicePool::startCapturing()
{
	kdDebug(14010) <<  k_funcinfo << "startCapturing() called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].startCapturing();
	return EXIT_FAILURE;
}


/*!
    \fn VideoDevicePool::stopCapturing()
 */
int VideoDevicePool::stopCapturing()
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].stopCapturing();
	return EXIT_FAILURE;
}

// Implementation of the methods that get / set input's adjustment parameters
/*!
    \fn VideoDevicePool::getBrightness()
 */
float VideoDevicePool::getBrightness()
{
  if (currentDevice() < m_videodevice.size() )
	return m_videodevice[currentDevice()].getBrightness();
  else
	return 0;
}

/*!
    \fn VideoDevicePool::setBrightness(float brightness)
 */
float VideoDevicePool::setBrightness(float brightness)
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].setBrightness(brightness);
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::getContrast()
 */
float VideoDevicePool::getContrast()
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].getContrast();
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::setContrast(float contrast)
 */
float VideoDevicePool::setContrast(float contrast)
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].setContrast(contrast);
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::getSaturation()
 */
float VideoDevicePool::getSaturation()
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].getSaturation();
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::setSaturation(float saturation)
 */
float VideoDevicePool::setSaturation(float saturation)
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].setSaturation(saturation);
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::getWhiteness()
 */
float VideoDevicePool::getWhiteness()
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].getWhiteness();
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::setWhiteness(float whiteness)
 */
float VideoDevicePool::setWhiteness(float whiteness)
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].setWhiteness(whiteness);
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::getHue()
 */
float VideoDevicePool::getHue()
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].getHue();
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::setHue(float hue)
 */
float VideoDevicePool::setHue(float hue)
{
    if (currentDevice() < m_videodevice.size() )
	  return m_videodevice[currentDevice()].setHue(hue);
	else
	  return 0;
}

/*!
    \fn VideoDevicePool::getAutoBrightnessContrast()
 */
bool VideoDevicePool::getAutoBrightnessContrast()
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].getAutoBrightnessContrast();
	return false;
}

/*!
    \fn VideoDevicePool::setAutoBrightnessContrast(bool brightnesscontrast)
 */
bool VideoDevicePool::setAutoBrightnessContrast(bool brightnesscontrast)
{
	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::setAutoBrightnessContrast(" << brightnesscontrast << ") called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].setAutoBrightnessContrast(brightnesscontrast);
	return false;
}

/*!
    \fn VideoDevicePool::getAutoColorCorrection()
 */
bool VideoDevicePool::getAutoColorCorrection()
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].getAutoColorCorrection();
	return false;
}

/*!
    \fn VideoDevicePool::setAutoColorCorrection(bool colorcorrection)
 */
bool VideoDevicePool::setAutoColorCorrection(bool colorcorrection)
{
	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::setAutoColorCorrection(" << colorcorrection << ") called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].setAutoColorCorrection(colorcorrection);
	return false;
}

/*!
    \fn VideoDevicePool::getIMageAsMirror()
 */
bool VideoDevicePool::getImageAsMirror()
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].getImageAsMirror();
	return false;
}

/*!
    \fn VideoDevicePool::setImageAsMirror(bool imageasmirror)
 */
bool VideoDevicePool::setImageAsMirror(bool imageasmirror)
{
	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::setImageAsMirror(" << imageasmirror << ") called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].setImageAsMirror(imageasmirror);
	return false;
}

/*!
    \fn VideoDevicePool::getFrame()
 */
int VideoDevicePool::getFrame()
{
//	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getFrame() called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].getFrame();
	else
	{
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getFrame() fallback for no device." << endl;
		for(unsigned int loop=0; loop < m_buffer.data.size(); loop+=3)
		{
			m_buffer.data[loop]   = 255;
			m_buffer.data[loop+1] = 0;
			m_buffer.data[loop+2] = 0;
		}
	}
//	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getFrame() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn VideoDevicePool::getQImage(QImage *qimage)
 */
int VideoDevicePool::getImage(QImage *qimage)
{
//	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getImage() called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].getImage(qimage);
	else
	{
		kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getImage() fallback for no device." << endl;
		qimage->create(m_buffer.width, m_buffer.height,32, QImage::IgnoreEndian);
		uchar *bits=qimage->bits();
		switch(m_buffer.pixelformat)
		{
			case PIXELFORMAT_NONE	: break;
			case PIXELFORMAT_GREY	: break;
			case PIXELFORMAT_RGB332	: break;
			case PIXELFORMAT_RGB555	: break;
			case PIXELFORMAT_RGB555X: break;
			case PIXELFORMAT_RGB565	: break;
			case PIXELFORMAT_RGB565X: break;
			case PIXELFORMAT_RGB24	:
				{
					kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getImage() fallback for no device - RGB24." << endl;
					int step=0;
					for(int loop=0;loop < qimage->numBytes();loop+=4)
					{
						bits[loop]   = m_buffer.data[step];
						bits[loop+1] = m_buffer.data[step+1];
						bits[loop+2] = m_buffer.data[step+2];
						bits[loop+3] = 255;
						step+=3;
					}
				}
				break;
			case PIXELFORMAT_BGR24	: break;
				{
					int step=0;
					for(int loop=0;loop < qimage->numBytes();loop+=4)
					{
						bits[loop]   = m_buffer.data[step+2];
						bits[loop+1] = m_buffer.data[step+1];
						bits[loop+2] = m_buffer.data[step];
						bits[loop+3] = 255;
						step+=3;
					}
				}
				break;
			case PIXELFORMAT_RGB32	: memcpy(bits,&m_buffer.data[0], m_buffer.data.size());
				break;
			case PIXELFORMAT_BGR32	: break;
		}
	}
	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::getImage() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::selectInput(int input)
 */
int VideoDevicePool::selectInput(int newinput)
{
	kdDebug(14010) <<  k_funcinfo << "VideoDevicePool::selectInput(" << newinput << ") called." << endl;
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].selectInput(newinput);
	else
		return 0;
}

/*!
    \fn Kopete::AV::VideoDevicePool::setInputParameters()
 */
int VideoDevicePool::setInputParameters()
{
	if(m_videodevice.size())
		return m_videodevice[currentDevice()].setInputParameters();
	else
		return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice.size())
	{
		for (unsigned int loop=0; loop < m_videodevice.size(); loop++)
		{
			combobox->insertItem(m_videodevice[loop].m_name);
			kdDebug(14010) <<  k_funcinfo << "DeviceKCombobox: Added device " << loop << ": " << m_videodevice[loop].m_name << endl;
		}
		combobox->setCurrentItem(currentDevice());
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice.size())
	{
		if(m_videodevice[currentDevice()].inputs()>0)
		{
			for (unsigned int loop=0; loop < m_videodevice[currentDevice()].inputs(); loop++)
			{
				combobox->insertItem(m_videodevice[currentDevice()].m_input[loop].name);
				kdDebug(14010) <<  k_funcinfo << "InputKCombobox: Added input " << loop << ": " << m_videodevice[currentDevice()].m_input[loop].name << " (tuner: " << m_videodevice[currentDevice()].m_input[loop].hastuner << ")" << endl;
			}
			combobox->setCurrentItem(currentInput());
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevice.size())
	{
		if(m_videodevice[currentDevice()].inputs()>0)
		{
			for (unsigned int loop=0; loop < 25; loop++)
			{
				if ( (m_videodevice[currentDevice()].m_input[currentInput()].m_standards) & (1 << loop) )
					combobox->insertItem(m_videodevice[currentDevice()].signalStandardName( 1 << loop));
/*
				case STANDARD_PAL_B1	: return V4L2_STD_PAL_B1;	break;
				case STANDARD_PAL_G	: return V4L2_STD_PAL_G;	break;
				case STANDARD_PAL_H	: return V4L2_STD_PAL_H;	break;
				case STANDARD_PAL_I	: return V4L2_STD_PAL_I;	break;
				case STANDARD_PAL_D	: return V4L2_STD_PAL_D;	break;
				case STANDARD_PAL_D1	: return V4L2_STD_PAL_D1;	break;
				case STANDARD_PAL_K	: return V4L2_STD_PAL_K;	break;
				case STANDARD_PAL_M	: return V4L2_STD_PAL_M;	break;
				case STANDARD_PAL_N	: return V4L2_STD_PAL_N;	break;
				case STANDARD_PAL_Nc	: return V4L2_STD_PAL_Nc;	break;
				case STANDARD_PAL_60	: return V4L2_STD_PAL_60;	break;
				case STANDARD_NTSC_M	: return V4L2_STD_NTSC_M;	break;
				case STANDARD_NTSC_M_JP	: return V4L2_STD_NTSC_M_JP;	break;
				case STANDARD_NTSC_443	: return V4L2_STD_NTSC;		break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_SECAM_B	: return V4L2_STD_SECAM_B;	break;
				case STANDARD_SECAM_D	: return V4L2_STD_SECAM_D;	break;
				case STANDARD_SECAM_G	: return V4L2_STD_SECAM_G;	break;
				case STANDARD_SECAM_H	: return V4L2_STD_SECAM_H;	break;
				case STANDARD_SECAM_K	: return V4L2_STD_SECAM_K;	break;
				case STANDARD_SECAM_K1	: return V4L2_STD_SECAM_K1;	break;
				case STANDARD_SECAM_L	: return V4L2_STD_SECAM_L;	break;
				case STANDARD_SECAM_LC	: return V4L2_STD_SECAM;	break; // Using workaround value because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
				case STANDARD_ATSC_8_VSB	: return V4L2_STD_ATSC_8_VSB;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_ATSC_16_VSB	: return V4L2_STD_ATSC_16_VSB;	break; // ATSC/HDTV Standard officially not supported by V4L2 but exists in videodev2.h
				case STANDARD_PAL_BG	: return V4L2_STD_PAL_BG;	break;
				case STANDARD_PAL_DK	: return V4L2_STD_PAL_DK;	break;
				case STANDARD_PAL	: return V4L2_STD_PAL;		break;
				case STANDARD_NTSC	: return V4L2_STD_NTSC;		break;
				case STANDARD_SECAM_DK	: return V4L2_STD_SECAM_DK;	break;
				case STANDARD_SECAM	: return V4L2_STD_SECAM;	break;
				case STANDARD_525_60	: return V4L2_STD_525_60;	break;
				case STANDARD_625_50	: return V4L2_STD_625_50;	break;
				case STANDARD_ALL	: return V4L2_STD_ALL;		break;

				combobox->insertItem(m_videodevice[currentDevice()].m_input[loop].name);
				kdDebug(14010) <<  k_funcinfo << "StandardKCombobox: Added input " << loop << ": " << m_videodevice[currentDevice()].m_input[loop].name << " (tuner: " << m_videodevice[currentDevice()].m_input[loop].hastuner << ")" << endl;*/
			}
			combobox->setCurrentItem(currentInput());
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::scanDevices()
 */
int VideoDevicePool::scanDevices()
{
    /// @todo implement me

	kdDebug(14010) <<  k_funcinfo << "called" << endl;
#if defined(__linux__) && defined(ENABLE_AV)
	QDir videodevice_dir;
	const QString videodevice_dir_path=QString::fromLocal8Bit("/dev/v4l/");
	const QString videodevice_dir_filter=QString::fromLocal8Bit("video*");
	VideoDevice videodevice;

	m_videodevice.clear();
	m_modelvector.clear();

	videodevice_dir.setPath(videodevice_dir_path);
	videodevice_dir.setNameFilter(videodevice_dir_filter);
        videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks | QDir::Readable | QDir::Writable );
        videodevice_dir.setSorting( QDir::Name );

	kdDebug(14010) <<  k_funcinfo << "Looking for devices in " << videodevice_dir_path << endl;
	const QFileInfoList *list = videodevice_dir.entryInfoList();

	if (!list)
	{
		kdDebug(14010) << k_funcinfo << "Found no suitable devices in " << videodevice_dir_path << endl;
		QDir videodevice_dir;
		const QString videodevice_dir_path=QString::fromLocal8Bit("/dev/");
		const QString videodevice_dir_filter=QString::fromLocal8Bit("video*");
		VideoDevice videodevice;

		videodevice_dir.setPath(videodevice_dir_path);
		videodevice_dir.setNameFilter(videodevice_dir_filter);
        	videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks | QDir::Readable | QDir::Writable );
        	videodevice_dir.setSorting( QDir::Name );

		kdDebug(14010) <<  k_funcinfo << "Looking for devices in " << videodevice_dir_path << endl;
		const QFileInfoList *list = videodevice_dir.entryInfoList();

		if (!list)
		{
			kdDebug(14010) << k_funcinfo << "Found no suitable devices in " << videodevice_dir_path << endl;
			return EXIT_FAILURE;
		}

		QFileInfoListIterator fileiterator ( *list );
		QFileInfo *fileinfo;

		kdDebug(14010) <<  k_funcinfo << "scanning devices in " << videodevice_dir_path << "..." << endl;
		while ( (fileinfo = fileiterator.current()) != 0 )
		{
			videodevice.setFileName(fileinfo->absFilePath());
			kdDebug(14010) <<  k_funcinfo << "Found device " << videodevice.full_filename << endl;
			videodevice.open(); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
			if(videodevice.isOpen())
			{
				kdDebug(14010) <<  k_funcinfo << "File " << videodevice.full_filename << " was opened successfuly" << endl;

// This must be changed to proper code to handle multiple devices of the same model. It currently simply add models without proper checking
				videodevice.close();
				videodevice.m_modelindex=m_modelvector.addModel (videodevice.m_model); // Adds device to the device list and sets model number
				m_videodevice.push_back(videodevice);
			}
			++fileiterator;
		}


		m_current_device = 0;
		loadConfig();
		kdDebug(14010) <<  k_funcinfo << "exited successfuly" << endl;
		return EXIT_SUCCESS;

	}
	QFileInfoListIterator fileiterator ( *list );
	QFileInfo *fileinfo;

	kdDebug(14010) <<  k_funcinfo << "scanning devices in " << videodevice_dir_path << "..." << endl;
	while ( (fileinfo = fileiterator.current()) != 0 )
	{
		videodevice.setFileName(fileinfo->absFilePath());
		kdDebug(14010) <<  k_funcinfo << "Found device " << videodevice.full_filename << endl;
		videodevice.open(); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
		if(videodevice.isOpen())
		{
			kdDebug(14010) <<  k_funcinfo << "File " << videodevice.full_filename << " was opened successfuly" << endl;

// This must be changed to proper code to handle multiple devices of the same model. It currently simply add models without proper checking
			videodevice.close();
			videodevice.m_modelindex=m_modelvector.addModel (videodevice.m_model); // Adds device to the device list and sets model number
			m_videodevice.push_back(videodevice);
		}
		++fileiterator;
	}
	m_current_device = 0;
	loadConfig();
#endif
	kdDebug(14010) <<  k_funcinfo << "exited successfuly" << endl;
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::hasDevices()
 */
bool VideoDevicePool::hasDevices()
{
    /// @todo implement me
	if(m_videodevice.size())
		return true;
	return false;
}

/*!
    \fn Kopete::AV::VideoDevicePool::size()
 */
size_t VideoDevicePool::size()
{
    /// @todo implement me
	return m_videodevice.size();
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentDevice()
 */
unsigned int VideoDevicePool::currentDevice()
{
    /// @todo implement me
	return m_current_device;
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
int VideoDevicePool::currentInput()
{
    /// @todo implement me
	return m_videodevice[currentDevice()].currentInput();
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
unsigned int VideoDevicePool::inputs()
{
    /// @todo implement me
	return m_videodevice[currentDevice()].inputs();
}

/*!
    \fn Kopete::AV::VideoDevicePool::loadConfig()
 */
void VideoDevicePool::loadConfig()
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "called" << endl;
	if((hasDevices())&&(m_clients==0))
	{
		KConfig *config = KGlobal::config();
		config->setGroup("Video Device Settings");
		const QString currentdevice = config->readEntry("Current Device", QString::null);
		kdDebug(14010) << k_funcinfo << "Current device: " << currentdevice << endl;

//		m_current_device = 0; // Must check this thing because of the fact that multiple loadConfig in other methodas can do bad things. Watch out!

		VideoDeviceVector::iterator vditerator;
		for( vditerator = m_videodevice.begin(); vditerator != m_videodevice.end(); ++vditerator )
		{
			const QString modelindex = QString::fromLocal8Bit ( "Model %1 Device %2")  .arg ((*vditerator).m_name ) .arg ((*vditerator).m_modelindex);
			if(modelindex == currentdevice)
			{
				m_current_device = vditerator - m_videodevice.begin();
//				kdDebug(14010) << k_funcinfo << "This place will be used to set " << modelindex << " as the current device ( " << (vditerator - m_videodevice.begin()) << " )." << endl;
			}
			const QString name                = config->readEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Name")  .arg ((*vditerator).m_name ) .arg ((*vditerator).m_modelindex)), (*vditerator).m_model);
			const int currentinput            = config->readNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Current input")  .arg ((*vditerator).m_name ) .arg ((*vditerator).m_modelindex)), 0);
			kdDebug(14010) << k_funcinfo << "Device name: " << name << endl;
			kdDebug(14010) << k_funcinfo << "Device current input: " << currentinput << endl;
			(*vditerator).selectInput(currentinput);

			for (size_t input = 0 ; input < (*vditerator).m_input.size(); input++)
			{
				const float brightness = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Brightness").arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , 0.5 );
				const float contrast   = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Contrast")  .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , 0.5 );
				const float saturation = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Saturation").arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , 0.5 );
				const float whiteness  = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Whiteness") .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , 0.5 );
				const float hue        = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Hue")       .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , 0.5 );
				const bool  autobrightnesscontrast = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoBrightnessContrast") .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , false );
				const bool  autocolorcorrection    = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoColorCorrection")    .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , false );
				const bool  imageasmirror          = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 mageAsMirror")           .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input)) , false );
				(*vditerator).setBrightness(brightness);
				(*vditerator).setContrast(contrast);
				(*vditerator).setSaturation(saturation);
				(*vditerator).setHue(hue);
				(*vditerator).setAutoBrightnessContrast(autobrightnesscontrast);
				(*vditerator).setAutoColorCorrection(autocolorcorrection);
				(*vditerator).setImageAsMirror(imageasmirror);
				kdDebug(14010) <<  k_funcinfo << "Brightness:" << brightness << endl;
				kdDebug(14010) <<  k_funcinfo << "Contrast  :" << contrast   << endl;
				kdDebug(14010) <<  k_funcinfo << "Saturation:" << saturation << endl;
				kdDebug(14010) <<  k_funcinfo << "Whiteness :" << whiteness << endl;
				kdDebug(14010) <<  k_funcinfo << "Hue       :" << hue        << endl;
				kdDebug(14010) <<  k_funcinfo << "AutoBrightnessContrast:" << autobrightnesscontrast << endl;
				kdDebug(14010) <<  k_funcinfo << "AutoColorCorrection   :" << autocolorcorrection    << endl;
				kdDebug(14010) <<  k_funcinfo << "ImageAsMirror         :" << imageasmirror          << endl;
			}
		}
	}
}

/*!
    \fn Kopete::AV::VideoDevicePool::saveConfig()
 */
void VideoDevicePool::saveConfig()
{
    /// @todo implement me
	kdDebug(14010) <<  k_funcinfo << "called" << endl;
	if(hasDevices())
	{
		KConfig *config = KGlobal::config();
		config->setGroup("Video Device Settings");

/*		if(m_modelvector.size())
		{
			VideoDeviceModelPool::m_devicemodel::iterator vmiterator;
			for( vmiterator = m_modelvector.begin(); vmiterator != m_modelvector.end(); ++vmiterator )
			{
				kdDebug(14010) << "Device Model: " << (*vmiterator).model << endl;
				kdDebug(14010) << "Device Count: " << (*vmiterator).count << endl;
			}
		}
*/
// Stores what is the current video device in use
		const QString currentdevice = QString::fromLocal8Bit ( "Model %1 Device %2" ) .arg(m_videodevice[m_current_device].m_model) .arg(m_videodevice[m_current_device].m_modelindex);
		config->writeEntry( "Current Device", currentdevice);

		VideoDeviceVector::iterator vditerator;
		for( vditerator = m_videodevice.begin(); vditerator != m_videodevice.end(); ++vditerator )
		{
			kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Name:" << (*vditerator).m_name << endl;
			kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Current input:" << (*vditerator).currentInput() << endl;

// Stores current input for the given video device
			const QString name                   = QString::fromLocal8Bit ( "Model %1 Device %2 Name")  .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex);
			const QString currentinput           = QString::fromLocal8Bit ( "Model %1 Device %2 Current input")  .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex);
			config->writeEntry( name,                   (*vditerator).m_name);
			config->writeEntry( currentinput,           (*vditerator).currentInput());

			for (size_t input = 0 ; input < (*vditerator).m_input.size(); input++)
			{
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Brightness: " << (*vditerator).m_input[input].getBrightness() << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Contrast  : " << (*vditerator).m_input[input].getContrast()   << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Saturation: " << (*vditerator).m_input[input].getSaturation() << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Whiteness : " << (*vditerator).m_input[input].getWhiteness()  << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Hue       : " << (*vditerator).m_input[input].getHue()        << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Automatic brightness / contrast: " << (*vditerator).m_input[input].getAutoBrightnessContrast() << endl;
				kdDebug(14010) << "Model:" << (*vditerator).m_model << ":Index:" << (*vditerator).m_modelindex << ":Input:" << input << ":Automatic color correction     : " << (*vditerator).m_input[input].getAutoColorCorrection() << endl;

// Stores configuration about each channel
				const QString brightness             = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Brightness")             .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString contrast               = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Contrast")               .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString saturation             = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Saturation")             .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString whiteness              = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Whiteness")              .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString hue                    = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Hue")                    .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString autobrightnesscontrast = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoBrightnessContrast") .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString autocolorcorrection    = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoColorCorrection")    .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				const QString imageasmirror          = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 ImageAsMirror")          .arg ((*vditerator).m_model ) .arg ((*vditerator).m_modelindex) .arg (input);
				config->writeEntry( brightness,             (*vditerator).m_input[input].getBrightness());
				config->writeEntry( contrast,               (*vditerator).m_input[input].getContrast());
				config->writeEntry( saturation,             (*vditerator).m_input[input].getSaturation());
				config->writeEntry( whiteness,              (*vditerator).m_input[input].getWhiteness());
				config->writeEntry( hue,                    (*vditerator).m_input[input].getHue());
				config->writeEntry( autobrightnesscontrast, (*vditerator).m_input[input].getAutoBrightnessContrast());
				config->writeEntry( autocolorcorrection,    (*vditerator).m_input[input].getAutoColorCorrection());
				config->writeEntry( imageasmirror,          (*vditerator).m_input[input].getImageAsMirror());
			}
		}
		config->sync();
		kdDebug(14010) << endl;
	}
}



}

}
