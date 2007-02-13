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
	kdDebug( 14010 )
	<< "libkopete (avdevice): self() called" << endl;
	if (s_self == NULL)
	{
		s_self = new VideoDevicePool;
		if (s_self)
			m_clients = 0;
	}
	kdDebug( 14010 ) << "libkopete (avdevice): self() exited successfuly. m_clients = " << m_clients << endl;
	return s_self;
}

VideoDevicePool::VideoDevicePool()
{
}

VideoDevicePool::~VideoDevicePool()
{
}

int VideoDevicePool::open()
{
	m_ready.lock();
	if( m_videodevices.empty() )
	{
		kdDebug( 14010 ) <<  k_funcinfo << "open(): No devices found. Must scan for available devices." << m_current_device << endl;
		scanDevices();
	}
	if( m_videodevices.empty() )
	{
		kdDebug( 14010 ) <<  k_funcinfo << "open(): No devices found. bailing out." << m_current_device << endl;
		m_ready.unlock();
		return EXIT_FAILURE;
	}
	if( m_current_device >= m_videodevices.size() )
	{
		kdDebug( 14010 ) <<  k_funcinfo << "open(): Device out of scope (" << m_current_device << "). Defaulting to the first one." << endl;
		m_current_device = 0;
	}
	int isopen = current()->open();
	if ( isopen == EXIT_SUCCESS )
	{
		loadConfig(); // Temporary hack. The open() seems to clean the input parameters. Need to find a way to fix it.
		
	}
	m_clients++;
	kdDebug( 14010 ) << k_funcinfo << "Number of clients: " << m_clients << endl;
	m_ready.unlock();
	return isopen;
}

int VideoDevicePool::open(unsigned int device)
{
    /// @todo implement me
	kdDebug( 14010 ) <<  k_funcinfo << "open(" << device << ") called." << endl;
	if(device >= m_videodevices.size())
	{
		kdDebug( 14010 ) <<  k_funcinfo << "open(" << device <<"): Device does not exist." << endl;
		return EXIT_FAILURE;
	}
	close();
	kdDebug() <<  k_funcinfo << "open(" << device << ") Setting m_current_Device to " << device << endl;
	m_current_device = device;
	saveConfig();
	kdDebug( 14010 ) <<  k_funcinfo << "open(" << device << ") Calling open()." << endl;
	return open();
}

VideoDevice * VideoDevicePool::current()
{
	if ( m_videodevices.empty() )
		return 0;
	else
		return m_videodevices[currentDeviceIndex()];
}

int VideoDevicePool::showDeviceCapabilities( unsigned int device )
{
	return m_videodevices[device]->showDeviceCapabilities();
}

int VideoDevicePool::setSize( int newwidth, int newheight)
{
	if( m_videodevices.empty() )
	{
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::setSize() fallback for no device." << endl;
		m_buffer.width=newwidth;
		m_buffer.height=newheight;
		m_buffer.pixelformat= PIXELFORMAT_RGB24;
		m_buffer.data.resize(m_buffer.width*m_buffer.height*3);
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::setSize() buffer size: "<< m_buffer.data.size() << endl;
	}
	else
	{
		return current()->setSize( newwidth, newheight );
	}
	return EXIT_SUCCESS;
}

int VideoDevicePool::close()
{
	if(m_clients)
		m_clients--;
	if((currentDeviceIndex() < m_videodevices.size())&&(!m_clients))
		return current()->close();
	if(m_clients)
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::close() The video device is still in use." << endl;
	if(currentDeviceIndex() >= m_videodevices.size())
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::close() Current device out of range." << endl;
	return EXIT_FAILURE;
}

int VideoDevicePool::getFrame()
{
//	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getFrame() called." << endl;
	if( m_videodevices.size() )
		return current()->getFrame();
	else
	{
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getFrame() fallback for no device." << endl;
		for(unsigned int loop=0; loop < m_buffer.data.size(); loop+=3)
		{
			m_buffer.data[loop]   = 255;
			m_buffer.data[loop+1] = 0;
			m_buffer.data[loop+2] = 0;
		}
	}
//	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getFrame() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

int VideoDevicePool::getImage(QImage *qimage)
{
//	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getImage() called." << endl;
	if(m_videodevices.size())
		return current()->getImage(qimage);
	else
	{
		kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getImage() fallback for no device." << endl;
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
					kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getImage() fallback for no device - RGB24." << endl;
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
			default:
				kdDebug( 14010 ) << "Unhandled pixel format in getImage(): " <<m_buffer.pixelformat << endl;
		}
	}
	kdDebug( 14010 ) <<  k_funcinfo << "VideoDevicePool::getImage() exited successfuly." << endl;
	return EXIT_SUCCESS;
}

int VideoDevicePool::getPreviewImage(QImage *qimage)
{
	if( m_videodevices.empty() )
		return getImage(qimage);
	else
		return current()->getPreviewImage(qimage);
}

int VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug( 14010 ) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevices.size())
	{
		for (unsigned int loop=0; loop < m_videodevices.size(); loop++)
		{
			combobox->insertItem( m_videodevices[loop]->name() );
			kdDebug( 14010 ) <<  k_funcinfo << "DeviceKCombobox: Added device " << loop << ": " << m_videodevices[loop]->name() << endl;
		}
		combobox->setCurrentItem(currentDeviceIndex());
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
{
	kdDebug( 14010 ) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevices.size())
	{
		for (unsigned int loop=0; loop < current()->inputCount(); loop++)
		{
			combobox->insertItem( current()->inputs()[loop].name() );
			kdDebug( 14010 ) <<  k_funcinfo << "InputKCombobox: Added input " << loop << ": " <<  current()->inputs()[loop].name() << " (tuner: " <<  current()->inputs()[loop].hasTuner() << ")" << endl;
		}
		combobox->setCurrentItem( current()->currentInput() );
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kdDebug( 14010 ) <<  k_funcinfo << "fillInputKComboBox: Called." << endl;
	combobox->clear();
	if(m_videodevices.size())
	{
		if( current()->inputCount()>0)
		{
			for (unsigned int loop=0; loop < 25; loop++)
			{
				if ( (current()->inputs()[currentInput()].standards()) & (1 << loop) )
					combobox->insertItem(current()->signalStandardName( 1 << loop));
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
				kdDebug( 14010 ) <<  k_funcinfo << "StandardKCombobox: Added input " << loop << ": " << m_videodevice[currentDevice()].m_input[loop].name << " (tuner: " << m_videodevice[currentDevice()].m_input[loop].hastuner << ")" << endl;*/
			}
			combobox->setCurrentItem( current()->currentInput() );
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

QStringList VideoDevicePool::videoDevicePaths( const QString & dirPath, const QString & filter )
{
	QDir videodevice_dir;
	VideoDevice videodevice;

	videodevice_dir.setPath( dirPath );
	videodevice_dir.setNameFilter( filter );
	videodevice_dir.setFilter( QDir::System | QDir::NoSymLinks |
							   QDir::Readable | QDir::Writable );
	videodevice_dir.setSorting( QDir::Name );

	kdDebug( 14010 ) <<  k_funcinfo << "Looking for devices in " << dirPath << endl;
	const QFileInfoList *list = videodevice_dir.entryInfoList();
	QStringList absolutePaths;
	if ( list )
	{
		QFileInfoListIterator fileiterator ( *list );
		QFileInfo *fileinfo;
		while ( (fileinfo = fileiterator.current()) != 0 )
		{
			++fileiterator;
			absolutePaths.append( fileinfo->absFilePath() );
		}
	}
	return absolutePaths;
}

/*!
    \fn Kopete::AV::VideoDevicePool::scanDevices()
 */
int VideoDevicePool::scanDevices()
{
	kdDebug( 14010 ) << k_funcinfo << endl;
	m_videodevices.clear();
	m_modelvector.clear();

#if defined(__linux__) && defined(ENABLE_AV)
	m_current_device = 0;

	QString videodevice_dir_path=QString::fromLocal8Bit("/dev/v4l/");
	QString videodevice_dir_filter=QString::fromLocal8Bit("video*");

	QStringList videoDevices = videoDevicePaths( videodevice_dir_path, videodevice_dir_filter );
	if ( videoDevices.empty() )
	{
		videodevice_dir_path=QString::fromLocal8Bit("/dev/");
		videodevice_dir_filter=QString::fromLocal8Bit("video*");
		videoDevices = videoDevicePaths( videodevice_dir_path, videodevice_dir_filter );
		if ( videoDevices.empty() )
		{
			kdDebug( 14010 ) << k_funcinfo << "Found no suitable devices in " << videodevice_dir_path << endl;
			return EXIT_FAILURE;
		}
	}
	for ( QStringList::Iterator it = videoDevices.begin(); it != videoDevices.end(); ++it )
	{
		VideoDevice * videodevice = new VideoDevice;
		videodevice->setDevicePath( *it );
		kdDebug( 14010 ) <<  k_funcinfo << "Found device " << videodevice->devicePath() << endl;
		videodevice->open(); // It should be opened with O_NONBLOCK (it's a FIFO) but I dunno how to do it using QFile
		// QFile::IO_Raw is probably what you need here - Bille
		if(videodevice->isOpen())
		{
			kdDebug( 14010 ) <<  k_funcinfo << "File " << videodevice->devicePath() << " was opened successfully" << endl;

	// This must be changed to proper code to handle multiple devices of the same model. It currently simply add models without proper checking
			videodevice->close();
			videodevice->m_modelindex = m_modelvector.addModel( videodevice->model() ); // Adds device to the device list and sets model number
			m_videodevices.push_back(videodevice);
		}
	}
	loadConfig();
#endif
	return ( m_videodevices.empty() ? EXIT_FAILURE : EXIT_SUCCESS );
}

bool VideoDevicePool::hasDevices()
{
	return !m_videodevices.empty();
}

size_t VideoDevicePool::size()
{
	return m_videodevices.size();
}

unsigned int VideoDevicePool::currentDeviceIndex()
{
	return m_current_device;
}

void VideoDevicePool::loadConfig()
{
	kdDebug( 14010 ) <<  k_funcinfo << "called" << endl;
	if((hasDevices())&&(m_clients==0))
	{
		KConfig *config = KGlobal::config();
		KConfigGroupSaver saver( config, "Video Device Settings" );
		const QString currentdevice = config->readEntry("Current Device", QString::null);
		kdDebug( 14010 ) << k_funcinfo << "Current device: " << currentdevice << endl;

//		m_current_device = 0; // Must check this thing because of the fact that multiple loadConfig in other methodas can do bad things. Watch out!

		VideoDeviceList::iterator vditerator;
		uint i = 0;
		for( vditerator = m_videodevices.begin(); vditerator != m_videodevices.end(); ++vditerator, ++i )
		{
			const QString modelindex = QString::fromLocal8Bit ( "Model %1 Device %2")  .arg ((*vditerator)->name() ) .arg ((*vditerator)->m_modelindex);
			if(modelindex == currentdevice)
			{
				m_current_device = i;
//				kdDebug( 14010 ) << k_funcinfo << "This place will be used to set " << modelindex << " as the current device ( " << (vditerator - m_videodevices.begin()) << " )." << endl;
			}
			const QString name                = config->readEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Name")  .arg ((*vditerator)->name() ) .arg ((*vditerator)->m_modelindex)), (*vditerator)->model());
			const int currentinput            = config->readNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Current input")  .arg ((*vditerator)->name() ) .arg ((*vditerator)->m_modelindex)), 0);
			const bool disablemmap            = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 DisableMMap")  .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex)), false );
			const bool workaroundbrokendriver = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 WorkaroundBrokenDriver")  .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex)), false );
			kdDebug( 14010 ) << k_funcinfo << "Device name: " << name << endl;
			kdDebug( 14010 ) << k_funcinfo << "Device current input: " << currentinput << endl;
			kdDebug( 14010 ) << k_funcinfo << "Disable mmap: " << disablemmap << endl;
			kdDebug( 14010 ) << k_funcinfo << "Workaround broken driver: " << workaroundbrokendriver << endl;
			(*vditerator)->selectInput(currentinput);
			(*vditerator)->setDisableMMap(disablemmap);
			(*vditerator)->setWorkaroundBrokenDriver(workaroundbrokendriver);

			for (size_t input = 0 ; input < (*vditerator)->inputCount(); input++)
			{
				const float brightness = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Brightness").arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , 0.5 );
				const float contrast   = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Contrast")  .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , 0.5 );
				const float saturation = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Saturation").arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , 0.5 );
				const float whiteness  = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Whiteness") .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , 0.5 );
				const float hue        = config->readDoubleNumEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Hue")       .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , 0.5 );
				const bool  autobrightnesscontrast = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoBrightnessContrast") .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , false );
				const bool  autocolorcorrection    = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoColorCorrection")    .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , false );
				const bool  imageasmirror          = config->readBoolEntry((QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 ImageAsMirror")          .arg ((*vditerator)->model() ) .arg ((*vditerator)->m_modelindex) .arg (input)) , false );
				(*vditerator)->setBrightness(brightness);
				(*vditerator)->setContrast(contrast);
				(*vditerator)->setSaturation(saturation);
				(*vditerator)->setHue(hue);
				(*vditerator)->setAutoBrightnessContrast(autobrightnesscontrast);
				(*vditerator)->setAutoColorCorrection(autocolorcorrection);
				(*vditerator)->setImageAsMirror(imageasmirror);
				kdDebug( 14010 ) <<  k_funcinfo << "Brightness:" << brightness << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "Contrast  :" << contrast   << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "Saturation:" << saturation << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "Whiteness :" << whiteness << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "Hue       :" << hue        << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "AutoBrightnessContrast:" << autobrightnesscontrast << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "AutoColorCorrection   :" << autocolorcorrection    << endl;
				kdDebug( 14010 ) <<  k_funcinfo << "ImageAsMirror         :" << imageasmirror          << endl;
			}
		}
	}
}

void VideoDevicePool::saveConfig()
{
	kdDebug( 14010 ) <<  k_funcinfo << "called" << endl;
	if(hasDevices())
	{
		KConfig *config = KGlobal::config();
		KConfigGroupSaver( config, "Video Device Settings");

/*		if(m_modelvector.size())
		{
			VideoDeviceModelPool::m_devicemodel::iterator vmiterator;
			for( vmiterator = m_modelvector.begin(); vmiterator != m_modelvector.end(); ++vmiterator )
			{
				kdDebug( 14010 ) << "Device Model: " << (*vmiterator).model << endl;
				kdDebug( 14010 ) << "Device Count: " << (*vmiterator).count << endl;
			}
		}
*/
// Stores what is the current video device in use
		const QString currentdevice = QString::fromLocal8Bit ( "Model %1 Device %2" ) .arg(current()->model()) .arg(current()->m_modelindex);
		config->writeEntry( "Current Device", currentdevice);

		VideoDeviceList::iterator vditerator;
		for( vditerator = m_videodevices.begin(); vditerator != m_videodevices.end(); ++vditerator )
		{
			QString model = (*vditerator)->model();
			QString name = (*vditerator)->name();
			int modelIndex = (*vditerator)->m_modelindex;
			
			kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Name:" << (*vditerator)->name() << endl;
			kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Current input:" << (*vditerator)->currentInput() << endl;

// Stores current input for the given video device
			const QString modelName                   = QString::fromLocal8Bit ( "Model %1 Device %2 Name")  .arg (model ) .arg (modelIndex);
			const QString currentinput           = QString::fromLocal8Bit ( "Model %1 Device %2 Current input")  .arg (model ) .arg (modelIndex);
			const QString disablemmap            = QString::fromLocal8Bit ( "Model %1 Device %2 DisableMMap") .arg (model ) .arg (modelIndex);
			const QString workaroundbrokendriver = QString::fromLocal8Bit ( "Model %1 Device %2 WorkaroundBrokenDriver") .arg (model ) .arg (modelIndex);
			config->writeEntry( modelName,                   (*vditerator)->name());
			config->writeEntry( currentinput,           (*vditerator)->currentInput());
			config->writeEntry( disablemmap,            (*vditerator)->mmapDisabled());
			config->writeEntry( workaroundbrokendriver, (*vditerator)->workaroundBrokenDriver());

			for (size_t input = 0 ; input < (*vditerator)->inputCount(); input++)
			{
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Brightness: " << (*vditerator)->inputs()[input].brightness() << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Contrast  : " << (*vditerator)->inputs()[input].contrast()   << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Saturation: " << (*vditerator)->inputs()[input].saturation() << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Whiteness : " << (*vditerator)->inputs()[input].whiteness()  << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Hue       : " << (*vditerator)->inputs()[input].hue()        << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Automatic brightness / contrast: " << (*vditerator)->inputs()[input].autoBrightnessContrast() << endl;
				kdDebug( 14010 ) << "Model:" << model << ":Index:" << modelIndex << ":Input:" << input << ":Automatic color correction     : " << (*vditerator)->inputs()[input].autoColorCorrection() << endl;

// Stores configuration about each channel
				const QString brightness             = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Brightness")             .arg (model ) .arg (modelIndex) .arg (input);
				const QString contrast               = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Contrast")               .arg (model ) .arg (modelIndex) .arg (input);
				const QString saturation             = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Saturation")             .arg (model ) .arg (modelIndex) .arg (input);
				const QString whiteness              = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Whiteness")              .arg (model ) .arg (modelIndex) .arg (input);
				const QString hue                    = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 Hue")                    .arg (model ) .arg (modelIndex) .arg (input);
				const QString autobrightnesscontrast = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoBrightnessContrast") .arg (model ) .arg (modelIndex) .arg (input);
				const QString autocolorcorrection    = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 AutoColorCorrection")    .arg (model ) .arg (modelIndex) .arg (input);
				const QString imageasmirror          = QString::fromLocal8Bit ( "Model %1 Device %2 Input %3 ImageAsMirror")          .arg (model ) .arg (modelIndex) .arg (input);
				config->writeEntry( brightness,             (*vditerator)->inputs()[input].brightness());
				config->writeEntry( contrast,               (*vditerator)->inputs()[input].contrast());
				config->writeEntry( saturation,             (*vditerator)->inputs()[input].saturation());
				config->writeEntry( whiteness,              (*vditerator)->inputs()[input].whiteness());
				config->writeEntry( hue,                    (*vditerator)->inputs()[input].hue());
				config->writeEntry( autobrightnesscontrast, (*vditerator)->inputs()[input].autoBrightnessContrast());
				config->writeEntry( autocolorcorrection,    (*vditerator)->inputs()[input].autoColorCorrection());
				config->writeEntry( imageasmirror,          (*vditerator)->inputs()[input].imageAsMirror());
			}
		}
		config->sync();
	}
}

} // namespace AV

} // namespace Kopete
