/*
    videodevicepool.cpp  -  Kopete Video Device Low-level Support

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
    Copyright (c) 2010      by Frank Schaefer                 <fschaefer.oss@googlemail.com>

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
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <qdir.h>
#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <solid/deviceinterface.h>
#include <solid/video.h>


#include "videodevice.h"
#include "videodevicepool.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevicePool *VideoDevicePool::s_self = NULL;
__u64 VideoDevicePool::m_clients = 0;

VideoDevicePool* VideoDevicePool::self()
{
//	kDebug() << "libkopete (avdevice): self() called";
	if (s_self == NULL)
	{
		s_self = new VideoDevicePool;
		if (s_self)
			m_clients = 0;
	}
//	kDebug() << "libkopete (avdevice): self() exited successfuly";
	return s_self;
}

VideoDevicePool::VideoDevicePool()
: m_current_device(0)
{
	connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString&)), SLOT(deviceAdded(const QString &)) );
	connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString&)), SLOT(deviceRemoved(const QString &)) );
	/* NOTE: No locking needed as long as we don't connect with Qt::ConnectionType = Qt::DirectConnection
	         while the signals are emitted by other threads
	 */
	foreach( Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()) )
		registerDevice( device );
}


VideoDevicePool::~VideoDevicePool()
{
	foreach ( VideoDevice* vd, m_videodevices )
		delete vd;
}




/*!
    \fn VideoDevicePool::open(int device)
 */
int VideoDevicePool::open(int device)
{
    /// @todo implement me
	kDebug() << "called with device" << device;
	if (!m_videodevices.size() || (device >= m_videodevices.size()))
	{
		kDebug() << "Device not found.";
		return EXIT_FAILURE;
	}
	int current_device = m_current_device;
	if (device < 0)
	{
		kDebug() << "Trying to load saved device (using default device if not available)";
		loadSelectedDevice();	// Set m_current_device to saved device (if device available)
	}
	else
		m_current_device = device;
	int isopen = EXIT_FAILURE;
	if ((m_current_device != current_device) || !isOpen())
	{
		if (isOpen())
		{
			if (EXIT_SUCCESS == m_videodevices[current_device]->close())
				m_clients--;
			else
				return EXIT_FAILURE;
		}
		isopen = m_videodevices[m_current_device]->open();
		if (isopen == EXIT_SUCCESS)
		{
			loadDeviceConfig(); // Load and apply device parameters
			m_clients++;
		}
	}
	else
	{
		isopen = EXIT_SUCCESS;
		m_clients++;
	}
	kDebug() << "Number of clients: " << m_clients;
	return isopen;
}

bool VideoDevicePool::isOpen()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->isOpen();
	else
		return false;
}

/*!
    \fn VideoDevicePool::showDeviceCapabilities(int device)
 */
int VideoDevicePool::showDeviceCapabilities(int device)
{
	if (device < 0)
		device = m_current_device;
	if ((device >= 0) && (device < m_videodevices.size()))
		return m_videodevices[device]->showDeviceCapabilities();
	else
		return EXIT_FAILURE;
}

int VideoDevicePool::width()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->width();
	else
		return 0;
}

int VideoDevicePool::minWidth()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->minWidth();
	else
		return 0;
}

int VideoDevicePool::maxWidth()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->maxWidth();
	else
		return 0;
}

int VideoDevicePool::height()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->height();
	else
		return 0;
}

int VideoDevicePool::minHeight()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->minHeight();
	else
		return 0;
}

int VideoDevicePool::maxHeight()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->maxHeight();
	else
		return 0;
}

int VideoDevicePool::setSize( int newwidth, int newheight)
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->setSize(newwidth, newheight);
	else
	{
		kDebug() << "VideoDevicePool::setSize() fallback for no device.";
		m_buffer.width=newwidth;
		m_buffer.height=newheight;
		m_buffer.pixelformat=	PIXELFORMAT_RGB24;
		m_buffer.data.resize(m_buffer.width*m_buffer.height*3);
		kDebug() << "VideoDevicePool::setSize() buffer size: "<< m_buffer.data.size();
	}
	return EXIT_SUCCESS;
}

/*!
    \fn int VideoDevicePool::close()
    \return The success of the operation: EXIT_SUCCESS or EXIT_FAILURE
    \brief Closes the device
 */
int VideoDevicePool::close()
{
	int ret = EXIT_FAILURE;
	if ((m_current_device < 0) || (m_current_device >= m_videodevices.size()))
	{
		kDebug() << "Current device out of range.";
	}
	else if (!m_clients)
	{
		ret = EXIT_SUCCESS;
	}
	else if (m_clients > 1)
	{
		m_clients--;
		kDebug() << "The video device is still in use:" << m_clients << "clients";
		ret = EXIT_SUCCESS;
	}
	else
	{
		ret = m_videodevices[m_current_device]->close();
		if (EXIT_SUCCESS == ret)
			m_clients--;
	}
	return ret;
}

/*!
    \fn VideoDevicePool::startCapturing()
 */
int VideoDevicePool::startCapturing()
{
	kDebug() << "startCapturing() called.";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->startCapturing();
	else
		return EXIT_FAILURE;
}


/*!
    \fn VideoDevicePool::stopCapturing()
 */
int VideoDevicePool::stopCapturing()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->stopCapturing();
	else
		return EXIT_FAILURE;
}


// Implementation of the methods that get / set input's adjustment parameters

/*!
    \fn QList<NumericVideoControl> VideoDevicePool::getSupportedNumericControls()
    \return A list of all supported numeric controls for the current input
    \brief Returns the supported numeric controls for the current input
 */
QList<NumericVideoControl> VideoDevicePool::getSupportedNumericControls()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getSupportedNumericControls();
	else
		return QList<NumericVideoControl>();
}

/*!
    \fn QList<BooleanVideoControl> VideoDevicePool::getSupportedBooleanControls()
    \return A list of all supported boolean controls for the current input
    \brief Returns the supported boolean controls for the current input
 */
QList<BooleanVideoControl> VideoDevicePool::getSupportedBooleanControls()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getSupportedBooleanControls();
	else
		return QList<BooleanVideoControl>();
}

/*!
    \fn QList<MenuVideoControl> VideoDevicePool::getSupportedMenuControls()
    \return A list of all supported menu-controls for the current input
    \brief Returns the supported menu-controls for the current input
 */
QList<MenuVideoControl> VideoDevicePool::getSupportedMenuControls()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getSupportedMenuControls();
	else
		return QList<MenuVideoControl>();
}

/*!
    \fn QList<ActionVideoControl> VideoDevicePool::getSupportedActionControls()
    \return A list of all supported action-controls for the current input
    \brief Returns the supported action-controls for the current input
 */
QList<ActionVideoControl> VideoDevicePool::getSupportedActionControls()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getSupportedActionControls();
	else
		return QList<ActionVideoControl>();
}

/*!
    \fn int VideoDevicePool::getControlValue(quint32 ctrl_id, qint32 * value)
    \param ctrl_id ID of the video-control
    \param value Pointer to the variable, which recieves the value of the querried video-control.
                 For boolean controls, the value is 0 or 1.
                 For menu-controls, the value is the index of the currently selected option.
    \return The result-code, currently EXIT_SUCCESS or EXIT_FAILURE
    \brief Reads the value of a video-control
 */
int VideoDevicePool::getControlValue(quint32 ctrl_id, qint32 * value)
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getControlValue(ctrl_id, value);
	else
		return EXIT_FAILURE;
}

/*!
    \fn int VideoDevicePool::setControlValue(quint32 ctrl_id, qint32 value)
    \param ctrl_id ID of the video-control
    \param value The value that should be set.
                 For boolean controls, the value must be 0 or 1.
                 For menu-controls, the value must be the index of the option.
                 For action-controls, the value is ignored.
    \return The result-code, currently EXIT_SUCCESS or EXIT_FAILURE
    \brief Sets the value of a video-control
 */
int VideoDevicePool::setControlValue(quint32 ctrl_id, qint32 value)
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->setControlValue(ctrl_id, value);
	else
		return EXIT_FAILURE;
}


/*!
    \fn VideoDevicePool::getFrame()
 */
int VideoDevicePool::getFrame()
{
//	kDebug() << "VideoDevicePool::getFrame() called.";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getFrame();
	else
		return EXIT_FAILURE;
}

/*!
    \fn VideoDevicePool::getQImage(QImage *qimage)
 */
int VideoDevicePool::getImage(QImage *qimage)
{
//	kDebug() << "VideoDevicePool::getImage() called.";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getImage(qimage);
	else
	{
		kDebug() << "VideoDevicePool::getImage() fallback for no device.";

		// do NOT delete qimage here, as it is received as a parameter
		if (qimage->width() != width() || qimage->height() != height())
			*qimage = QImage(width(), height(), QImage::Format_RGB32);

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
					kDebug() << "VideoDevicePool::getImage() fallback for no device - RGB24.";
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
			case PIXELFORMAT_YUYV   : break;
			case PIXELFORMAT_UYVY   : break;
			case PIXELFORMAT_YUV420P: break;
			case PIXELFORMAT_YUV422P: break;
			default: break;
		}
	}
	kDebug() << "VideoDevicePool::getImage() exited successfuly.";
	return EXIT_SUCCESS;
}

/*!
    \fn Kopete::AV::VideoDevicePool::selectInput(int input)
 */
int VideoDevicePool::selectInput(int newinput)
{
	kDebug() << "VideoDevicePool::selectInput(" << newinput << ") called.";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->selectInput(newinput);
	else
		return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillDeviceKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kDebug() << "Called.";
// check if KComboBox is a valid pointer.
	if (combobox != NULL)
	{
		combobox->clear();
		kDebug() << "Combobox cleaned.";
		if(m_videodevices.size())
		{
			for (int loop=0; loop < m_videodevices.size(); loop++)
			{
				combobox->addItem(m_videodevices[loop]->m_name);
				kDebug() << "Added device " << loop << ": " << m_videodevices[loop]->m_name;
			}
			combobox->setCurrentIndex(m_current_device);
			combobox->setEnabled(true);
			return EXIT_SUCCESS;
		}
		combobox->setEnabled(false);
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillInputKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kDebug() << "Called.";
	if (combobox != NULL)
	{
		combobox->clear();
		if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		{
			if (m_videodevices[m_current_device]->inputs() > 0)
			{
				for (int loop=0; loop < m_videodevices[m_current_device]->inputs(); loop++)
				{
					combobox->addItem(m_videodevices[m_current_device]->m_input[loop].name);
					kDebug() << "Added input " << loop << ": " << m_videodevices[m_current_device]->m_input[loop].name << " (tuner: " << m_videodevices[m_current_device]->m_input[loop].hastuner << ")";
				}
				combobox->setCurrentIndex(currentInput());
				combobox->setEnabled(true);
				return EXIT_SUCCESS;
			}
		}
		combobox->setEnabled(false);
	}
	return EXIT_FAILURE;
}

/*!
    \fn Kopete::AV::VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
 */
int VideoDevicePool::fillStandardKComboBox(KComboBox *combobox)
{
    /// @todo implement me
	kDebug() << "Called.";
	if (combobox != NULL)
	{
		combobox->clear();
		if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		{
			if (m_videodevices[m_current_device]->inputs() > 0)
			{
				for (unsigned int loop=0; loop < 25; loop++)
				{
					if ( (m_videodevices[m_current_device]->m_input[currentInput()].m_standards) & (1 << loop) )
						combobox->addItem(m_videodevices[m_current_device]->signalStandardName( 1 << loop));
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

				combobox->insertItem(m_videodevices[m_current_device]->m_input[loop].name);
				kDebug() << "StandardKCombobox: Added input " << loop << ": " << m_videodevices[m_current_device]->m_input[loop].name << " (tuner: " << m_videodevices[m_current_device]->m_input[loop].hastuner << ")";*/
				}
				combobox->setCurrentIndex(0);	// FIXME: set to actual signal standard
				combobox->setEnabled(combobox->count());
				return EXIT_SUCCESS;
			}
		}
		combobox->setEnabled(false);
	}
	return EXIT_FAILURE;
}

bool VideoDevicePool::registerDevice( Solid::Device & device )
{
	kDebug() << "New video device at " << device.udi();
	const Solid::Device * vendorDevice = &device;
	while ( vendorDevice->isValid() && vendorDevice->vendor().isEmpty() )
	{
		vendorDevice = new Solid::Device( vendorDevice->parentUdi() );
	}
	if ( vendorDevice->isValid() )
	{
		kDebug() << "vendor: " << vendorDevice->vendor() << ", product: " << vendorDevice->product();
	}
	Solid::Video * solidVideoDevice = device.as<Solid::Video>();
	if ( solidVideoDevice ) {
		QStringList protocols = solidVideoDevice->supportedProtocols();
		if ( protocols.contains( "video4linux" ) )
		{
			QStringList drivers = solidVideoDevice->supportedDrivers( "video4linux" );
			if ( drivers.contains( "video4linux" ) )
			{
				kDebug() << "V4L device path is" << solidVideoDevice->driverHandle( "video4linux" ).toString();
				VideoDevice* videodevice = new VideoDevice;
				videodevice->setUdi( device.udi() );
				videodevice->setFileName(solidVideoDevice->driverHandle( "video4linux" ).toString());
				kDebug() << "Found device " << videodevice->fileName();
				videodevice->open();
				if(videodevice->isOpen())
				{
					kDebug() << "File " << videodevice->fileName() << " was opened successfuly";
					videodevice->close();
					m_videodevices.push_back(videodevice);
					return true;
				}
				else
					delete videodevice;
			}
		}
	}
	return false;
}

/*!
    \fn Kopete::AV::VideoDevicePool::size()
 */
int VideoDevicePool::size()
{
    /// @todo implement me
	return m_videodevices.size();
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentDevice()
 */
int VideoDevicePool::currentDevice()
{
    /// @todo implement me
	return m_current_device;
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentDeviceUdi()
 */
QString VideoDevicePool::currentDeviceUdi()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->udi();
	else
		return QString();
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
int VideoDevicePool::currentInput()
{
    /// @todo implement me
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->currentInput();
	else
		return -1;
}

/*!
    \fn Kopete::AV::VideoDevicePool::currentInput()
 */
int VideoDevicePool::inputs()
{
    /// @todo implement me
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->inputs();
	else
		return 0;
}

/*!
    \fn void Kopete::AV::VideoDevicePool::loadSelectedDevice()
    \brief Loads and selects the saved device
 */
void VideoDevicePool::loadSelectedDevice()
{
	kDebug() << "called";
	if (m_videodevices.size())
	{
		KConfigGroup config(KGlobal::config(), "Video Device Settings");
		QString currentdevice = config.readEntry("Current Device", QString());
		kDebug() << "Device name:" << config.readEntry( QString::fromLocal8Bit( "Device %1 Name" ).arg( currentdevice ), QString("NOT SAVED") );
		if (!currentdevice.isEmpty())
		{
			kDebug() << "Saved device:" << currentdevice;
			QVector<VideoDevice*>::iterator vditerator;
			for( vditerator = m_videodevices.begin(); vditerator != m_videodevices.end(); ++vditerator )
			{
				if ((*vditerator)->udi() == currentdevice)
				{
					m_current_device = std::distance (m_videodevices.begin(), vditerator);
					kDebug() << "Saved device is available, setting device-index to" << m_current_device;
					return;
				}
			}
			kDebug() << "Saved device is not available.";
		}
		else
			kDebug() << "No device saved.";
	}
}

/*!
    \fn void Kopete::AV::VideoDevicePool::loadDeviceConfig()
    \brief Loads and applies the configuration for the currently selected device
    
    Loads the input and the values for all video-controls and applies them.
 */
void VideoDevicePool::loadDeviceConfig()
{
	kDebug() << "called";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		KConfigGroup config(KGlobal::config(), "Video Device Settings");
		// Load input and apply
		const QString key_currentinput = QString::fromLocal8Bit( "Device %1 Current Input" ).arg( m_videodevices[m_current_device]->udi() );
		const int currentinput = config.readEntry(key_currentinput, 0);
		kDebug() << "Setting input to" << currentinput;
		if (currentinput != m_videodevices[m_current_device]->currentInput())
			m_videodevices[m_current_device]->selectInput(currentinput);
		// Load video-controls and apply
		quint32 ctrl_id;
		qint32 ctrl_value;
		QString ctrl_key;
		bool ok = false;
		const QString key_control_start = QString::fromLocal8Bit( "Device %1 Input %2 Control " ).arg( m_videodevices[m_current_device]->udi() ).arg( m_videodevices[m_current_device]->currentInput() );
		QStringList ctrl_keys = config.keyList().filter(key_control_start);
		kDebug() << "Found" << ctrl_keys.size() << "saved values for video-controls";
		foreach (ctrl_key, ctrl_keys)
		{
			ctrl_id = QString(ctrl_key).remove(key_control_start).toUInt(&ok);
			if (ok)
			{
				/* NOTE: we do not read the value as int with readEntry() directly
				  because it doesn't tell us if the saved value was valid.
				  If not, we do NOT apply a standard value.
				*/
				QString tmpstr = config.readEntry(ctrl_key, QString());
				ctrl_value = tmpstr.toInt(&ok);
				if (ok && !tmpstr.isEmpty())
				{
					kDebug() << "Setting control" << ctrl_id << "to value" << ctrl_value;
					m_videodevices[m_current_device]->setControlValue(ctrl_id, ctrl_value);
				}
				else
					kDebug() << "Saved value for control" << ctrl_id << "is invalid:" << tmpstr;
			}
			else
				kDebug() << "Invalid key:" << ctrl_key;
		}
	}
	/* TODO: load and apply signal standard */
}

/*!
    \fn void Kopete::AV::VideoDevicePool::saveCurrentDeviceConfig()
    \brief Saves the current device configuration
    
    Saves the current device, the current input and the current values for all supported video-controls.
 */
void VideoDevicePool::saveCurrentDeviceConfig()
{
	kDebug() << "called";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		KConfigGroup config(KGlobal::config(), "Video Device Settings");
		// Save current device:
		kDebug() << "Current device:" << m_videodevices[m_current_device]->udi();
		config.writeEntry( "Current Device", m_videodevices[m_current_device]->udi() );
		// Save current device name (for debugging only):
		kDebug() << "Current device name:" << m_videodevices[m_current_device]->m_name;
		const QString name = QString::fromLocal8Bit( "Device %1 Name" ).arg( m_videodevices[m_current_device]->udi() );
		config.writeEntry( name, m_videodevices[m_current_device]->m_name );
		// Open device if closed:
		bool wasClosed = false;
		if (!m_videodevices[m_current_device]->isOpen())
		{
			kDebug() << "Device is currently closed, will be opened.";
			wasClosed = true;
			if (EXIT_SUCCESS != m_videodevices[m_current_device]->open())
			{
				kDebug() << "Failed to open the device. Saving aborted.";
				config.sync();
				return;
			}
		}
		// Save current input:
		kDebug() << "Current input:" << m_videodevices[m_current_device]->currentInput();
		const QString key_currentinput = QString::fromLocal8Bit( "Device %1 Current Input" ).arg( m_videodevices[m_current_device]->udi() );
		config.writeEntry( key_currentinput, m_videodevices[m_current_device]->currentInput() );
		// --- Save values of the controls ---:
		qint32 ctrl_value;
		const QString key_control_start = QString::fromLocal8Bit( "Device %1 Input %2 Control " ).arg( m_videodevices[m_current_device]->udi() ).arg( m_videodevices[m_current_device]->currentInput() );
		// Save values of the numeric controls:
		QList<NumericVideoControl> numCtrls = m_videodevices[m_current_device]->getSupportedNumericControls();
		NumericVideoControl numCtrl;
		foreach (numCtrl, numCtrls)
		{
			if (EXIT_SUCCESS == m_videodevices[m_current_device]->getControlValue(numCtrl.id, &ctrl_value))
			{
				kDebug() << "Numeric control:" << numCtrl.id << "value" << ctrl_value;
				config.writeEntry( key_control_start + QString::number(numCtrl.id), ctrl_value );
			}
			else
				kDebug() << "Error: couldn't get current value for numeric control" << numCtrl.id;
		}
		// Save values of the boolean controls:
		QList<BooleanVideoControl> boolCtrls = m_videodevices[m_current_device]->getSupportedBooleanControls();
		BooleanVideoControl boolCtrl;
		foreach (boolCtrl, boolCtrls)
		{
			if (EXIT_SUCCESS == m_videodevices[m_current_device]->getControlValue(boolCtrl.id, &ctrl_value))
			{
				kDebug() << "Boolean control:" << boolCtrl.id << "value" << ctrl_value;
				config.writeEntry( key_control_start + QString::number(boolCtrl.id), ctrl_value );
			}
			else
				kDebug() << "Error: couldn't get current value for boolean control" << numCtrl.id;
		}
		// Save values of the menu controls:
		QList<MenuVideoControl> menuCtrls = m_videodevices[m_current_device]->getSupportedMenuControls();
		MenuVideoControl menuCtrl;
		foreach (menuCtrl, menuCtrls)
		{
			if (EXIT_SUCCESS == m_videodevices[m_current_device]->getControlValue(menuCtrl.id, &ctrl_value))
			{
				kDebug() << "Menu-control:" << menuCtrl.id << "value" << ctrl_value;
				config.writeEntry( key_control_start + QString::number(menuCtrl.id), ctrl_value );
			}
			else
				kDebug() << "Error: couldn't get current value for menu-control" << numCtrl.id;
		}
		// NOTE: Action-video-controls don't have values, so there is nothing to save.
		// Close device again (if it was closed before):
		if (wasClosed)
		{
			if (EXIT_SUCCESS == m_videodevices[m_current_device]->close())
				kDebug() << "Device successfully closed.";
			else
				kDebug() << "Error: failed to close the device.";
		}
		config.sync();
	}
	/* TODO: save signal standard */
}

void VideoDevicePool::deviceAdded( const QString & udi )
{
	kDebug() << "("<< udi << ") called";
	Solid::Device dev( udi );
	if ( dev.is<Solid::Video>() )
	{
		if ( registerDevice( dev ) )
			emit deviceRegistered( udi );
	}
}

void VideoDevicePool::deviceRemoved( const QString & udi )
{
	kDebug() << "("<< udi << ") called";
	int i = 0;
	foreach ( VideoDevice* vd, m_videodevices )
	{
		if ( vd->udi() == udi )
		{
			kDebug() << "Video device '" << udi << "' has been removed!";
			delete m_videodevices[i]; // NOTE: device is closed in destructor
			m_videodevices.remove( i );
			if (m_current_device == i)
			{
				m_current_device = 0;
				m_clients = 0;
			}
			else if (m_current_device > i)
			{
				m_current_device--;
			}
			emit deviceUnregistered( udi );
			return;
		}
		else
			i++;
	}
}

} // namespace AV

} // namespace Kopete
