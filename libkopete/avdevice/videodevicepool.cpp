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

#include "videodevicepool.h"

#include <assert.h>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <qapplication.h>
#include <qdir.h>
#include <solid/device.h>
#include <solid/devicenotifier.h>
#include <solid/deviceinterface.h>
//#include <solid/video.h>
#include <KSharedConfig>


#include "videodevice.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

namespace Kopete {

namespace AV {

VideoDevicePool *VideoDevicePool::s_self = NULL;
__u64 VideoDevicePool::m_clients = 0;

/*!
    \fn VideoDevicePool* VideoDevicePool::self()
    \return Pointer to the VideoDevicePool object
    \brief Returns pointer to a common instance of the VideoDevicePool
 */
VideoDevicePool* VideoDevicePool::self()
{
	kDebug() << "called";
	if (s_self == NULL)
	{
		kDebug() << "Generated new instance.";
		s_self = new VideoDevicePool;
		if (s_self)
			m_clients = 0;
	}
	return s_self;
}

/*!
    \fn VideoDevicePool::VideoDevicePool()
    \brief Constructor of class VideoDevicePool
 */
VideoDevicePool::VideoDevicePool()
: QObject(qApp), m_current_device(-1)
{
	connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), SLOT(deviceAdded(QString)) );
	connect( Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), SLOT(deviceRemoved(QString)) );
	/* NOTE: No locking needed as long as we don't connect with Qt::ConnectionType = Qt::DirectConnection
	         while the signals are emitted by other threads
	 */
	/*foreach( Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()) )
		registerDevice( device );*/
}

/*!
    \fn VideoDevicePool::~VideoDevicePool()
    \brief Destructor of class VideoDevicePool
 */
VideoDevicePool::~VideoDevicePool()
{
	s_self = 0L;
	foreach ( VideoDevice* vd, m_videodevices )
		delete vd;
}



/*!
    \fn bool VideoDevicePool::isOpen()
    \return True if the device is open, false otherwise
    \brief Returns true if the currently selected device is open and false othwerise
 */
bool VideoDevicePool::isOpen()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->isOpen();
	else
		return false;
}

/*!
    \fn int VideoDevicePool::open( int device )
    \param device Index of the device that should be opened.
                  If a negative index is passed (default), the default device will be opened,
                  which is either the saved device (if available) or alternatively the device with index 0.
    \return The result-code, EXIT_SUCCESS or EXIT_FAILURE
    \brief Opens the video device with the specified index. The previously opened device is closed before.
 */
int VideoDevicePool::open( int device )
{
	kDebug() << "called with device" << device;
	if (!m_videodevices.size() || (device >= m_videodevices.size()))
	{
		kDebug() << "Device not found.";
		return EXIT_FAILURE;
	}
	if (device < 0)
	{
		kDebug() << "Trying to load saved device.";
		device = getSavedDevice();
		if (device < 0)
		{
			if (m_current_device < 0)
				device = 0;
			else
				device = m_current_device;
			kDebug() << "Saved device is not available, using default device:" << device;
		}
	}
	int isopen = EXIT_FAILURE;
	if ((device != m_current_device) || !isOpen())
	{
		if (isOpen())
		{
			if (EXIT_SUCCESS == m_videodevices[m_current_device]->close())
				m_clients--;
			else
				return EXIT_FAILURE;
		}
		isopen = m_videodevices[device]->open();
		if (isopen == EXIT_SUCCESS)
		{
			m_current_device = device;
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
    \fn int VideoDevicePool::size()
    \return The number of available video devices
    \brief Returns the number of available video devices
 */
int VideoDevicePool::size()
{
	return m_videodevices.size();
}

/*!
    \fn int VideoDevicePool::currentDevice()
    \return The index of the current device or -1 if no device available
    \brief Returns the index of the current device
 */
int VideoDevicePool::currentDevice()
{
	if (m_videodevices.size())
		return m_current_device;
	else	// to be sure...
		return -1;
}

/*!
    \fn QString Kopete::AV::VideoDevicePool::currentDeviceUdi()
    \return The unique device identifier (UDI) of the currently selected device
    \brief Returns the unique device identifier (UDI) of the currently selected device
 */
QString VideoDevicePool::currentDeviceUdi()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->udi();
	else
		return QString();
}

/*!
    \fn int VideoDevicePool::inputs()
    \return The number of inputs of the currently selected device
    \brief Returns the number of available inputs of the currently selected device
 */
int VideoDevicePool::inputs()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->inputs();
	else
		return 0;
}

/*!
    \fn int VideoDevicePool::currentInput()
    \return The index of the currently selected input or -1, if no input is available
    \brief Returns the index of the currently selected input
 */
int VideoDevicePool::currentInput()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->currentInput();
	else
		return -1;
}

/*!
    \fn int VideoDevicePool::selectInput( int input )
    \param input Index of input to be selected
    \return The result-code, EXIT_SUCCESS or EXIT_FAILURE
    \brief Selects the input of the current video device
 */
int VideoDevicePool::selectInput( int input )
{
	kDebug() << "called with input" << input;
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->selectInput(input);
	else
		return EXIT_FAILURE;
}



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
    \fn int VideoDevicePool::getControlValue( quint32 ctrl_id, qint32 * value )
    \param ctrl_id ID of the video-control
    \param value Pointer to the variable, which receives the value of the querried video-control.
                 For boolean controls, the value is 0 or 1.
                 For menu-controls, the value is the index of the currently selected option.
    \return The result-code, currently EXIT_SUCCESS or EXIT_FAILURE
    \brief Reads the value of a video-control
 */
int VideoDevicePool::getControlValue( quint32 ctrl_id, qint32 * value )
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getControlValue(ctrl_id, value);
	else
		return EXIT_FAILURE;
}

/*!
    \fn int VideoDevicePool::setControlValue( quint32 ctrl_id, qint32 value )
    \param ctrl_id ID of the video-control
    \param value The value that should be set.
                 For boolean controls, the value must be 0 or 1.
                 For menu-controls, the value must be the index of the option.
                 For action-controls, the value is ignored.
    \return The result-code, currently EXIT_SUCCESS or EXIT_FAILURE
    \brief Sets the value of a video-control
 */
int VideoDevicePool::setControlValue( quint32 ctrl_id, qint32 value )
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->setControlValue(ctrl_id, value);
	else
		return EXIT_FAILURE;
}



/*!
    \fn int VideoDevicePool::startCapturing()
    \return The result-code, EXIT_SUCCESS or EXIT_FAILURE
    \brief Starts capturing from the currently selected video device
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
    \fn int VideoDevicePool::stopCapturing()
    \return The result-code, EXIT_SUCCESS or EXIT_FAILURE
    \brief Starts capturing from the currently selected video device
 */
int VideoDevicePool::stopCapturing()
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->stopCapturing();
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
    \fn VideoDevicePool::getImage( QImage *qimage )
 */
int VideoDevicePool::getImage( QImage *qimage )
{
//	kDebug() << "VideoDevicePool::getImage() called.";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->getImage(qimage);
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

int VideoDevicePool::setImageSize( int newwidth, int newheight )
{
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
		return m_videodevices[m_current_device]->setSize(newwidth, newheight);
	else
		return EXIT_FAILURE;
}



/*!
    \fn void VideoDevicePool::fillDeviceKComboBox( KComboBox *combobox )
    \param combobox Pointer to a KComboBox object
    \brief Fills a combobox with the names of all available video devices
 */
void VideoDevicePool::fillDeviceKComboBox( KComboBox *combobox )
{
	kDebug() << "Called.";
	if (combobox == NULL)
		return;
	combobox->clear();
	if (m_videodevices.size())
	{
		for (int loop = 0; loop < m_videodevices.size(); loop++)
		{
			combobox->addItem(m_videodevices[loop]->m_name);
			kDebug() << "Added device" << loop << ":  " << m_videodevices[loop]->m_name;
		}
		combobox->setCurrentIndex(m_current_device);
	}
	combobox->setEnabled(m_videodevices.size());
}

/*!
    \fn void VideoDevicePool::fillInputKComboBox( KComboBox *combobox )
    \param combobox Pointer to a KComboBox object
    \brief Fills a combobox with the names of all available inputs for the currently selected device
 */
void VideoDevicePool::fillInputKComboBox( KComboBox *combobox )
{
	kDebug() << "Called.";
	if (combobox == NULL)
		return;
	combobox->clear();
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		if (m_videodevices[m_current_device]->inputs() > 0)
		{
			for (int loop = 0; loop < m_videodevices[m_current_device]->inputs(); loop++)
			{
				combobox->addItem(m_videodevices[m_current_device]->m_input[loop].name);
				kDebug() << "Added input" << loop << ":  " << m_videodevices[m_current_device]->m_input[loop].name
				         << " (tuner: " << m_videodevices[m_current_device]->m_input[loop].hastuner << ")";
			}
			combobox->setCurrentIndex(m_videodevices[m_current_device]->currentInput());
		}
	}
	combobox->setEnabled(combobox->count());
}

/*!
    \fn void VideoDevicePool::fillStandardKComboBox( KComboBox *combobox )
    \param combobox Pointer to a KComboBox object
    \brief Fills a combobox with the names of the available signal standards for the currently selected device
 */
void VideoDevicePool::fillStandardKComboBox( KComboBox *combobox )
{
	kDebug() << "Called.";
	if (combobox == NULL)
		return;
	combobox->clear();
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		if (m_videodevices[m_current_device]->inputs() > 0)
		{
			for (unsigned int loop = 0; loop < 25; loop++)
			{
				if (m_videodevices[m_current_device]->m_input[currentInput()].m_standards & (1 << loop))
				{
					combobox->addItem(m_videodevices[m_current_device]->signalStandardName(1 << loop));
					kDebug() << "Added signal standard" << loop << ":  " << m_videodevices[m_current_device]->signalStandardName(1 << loop);
				}
				  
			}
			combobox->setCurrentIndex(0);	// FIXME: set to actual signal standard
		}  
	}
	combobox->setEnabled(combobox->count());
}



/*!
    \fn int VideoDevicePool::getSavedDevice()
    \return The index of the saved device or -1, if the saved device is not available
    \brief Returns the index of the saved device
 */
int VideoDevicePool::getSavedDevice()
{
	kDebug() << "called";
	if (m_videodevices.size())
	{
		KConfigGroup config(KSharedConfig::openConfig(), "Video Device Settings");
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
					int devIndex = std::distance (m_videodevices.begin(), vditerator);
					kDebug() << "Saved device is available, device index is" << devIndex;
					return devIndex;
				}
			}
			kDebug() << "Saved device is not available.";
		}
		else
			kDebug() << "No device saved.";
	}
	return -1;
}

/*!
    \fn void VideoDevicePool::loadDeviceConfig()
    \brief Loads and applies the configuration for the currently selected device
    
    Loads the input and the values for all video-controls and applies them.
 */
void VideoDevicePool::loadDeviceConfig()
{
	kDebug() << "called";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		KConfigGroup config(KSharedConfig::openConfig(), "Video Device Settings");
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
    \fn void VideoDevicePool::saveCurrentDeviceConfig()
    \brief Saves the current device configuration
    
    Saves the current device, the current input and the current values for all supported video-controls.
 */
void VideoDevicePool::saveCurrentDeviceConfig()
{
	kDebug() << "called";
	if ((m_current_device >= 0) && (m_current_device < m_videodevices.size()))
	{
		KConfigGroup config(KSharedConfig::openConfig(), "Video Device Settings");
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



/*!
    \fn void VideoDevicePool::deviceAdded( const QString & udi )
    \param udi Unique device identifier (UDI) of the device that has been connected
    \brief Checks the device with the specified UDI and adds to the device pool, if it is a valid video device
 */
void VideoDevicePool::deviceAdded( const QString & udi )
{
	kDebug() << "called with UDI" << udi;
	Solid::Device dev( udi );
	/*if ( dev.is<Solid::Video>() )
	{
		kDebug() << "Device is a video device, trying to register it.";
		if ( registerDevice( dev ) )
			emit deviceRegistered( udi );
	}
	else*/
		kDebug() << "Device is not a video device";
}

/*!
    \fn void VideoDevicePool::deviceRemoved( const QString & udi )
    \param udi Unique device identifier (UDI) of the device that has been unplugged
    \brief Removes the device with the specified UDI from the device pool
 */
void VideoDevicePool::deviceRemoved( const QString & udi )
{
	kDebug() << "called with UDI" << udi;
	int i = 0;
	foreach ( VideoDevice* vd, m_videodevices )
	{
		if ( vd->udi() == udi )
		{
			kDebug() << "Video device with UDI" << udi << "has been removed!";
			delete m_videodevices[i]; // NOTE: device is closed in destructor
			m_videodevices.remove( i );
			if (m_current_device == i)
			{
				if (m_videodevices.size())
					m_current_device = 0;
				else
					m_current_device = -1;
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

/*!
    \fn void bool VideoDevicePool::registerDevice( Solid::Device & device )
    \param device The solid device that should be registered
    \return True, if the device has been registered; otherwise returns false
    \brief Checks if the given device is a valid video device and adds it do the device list
 */
bool VideoDevicePool::registerDevice( Solid::Device & device )
{
	kDebug() << "called, UDI is:\n   " << device.udi();
	const Solid::Device * vendorDevice = &device;
	while (vendorDevice->isValid() && vendorDevice->vendor().isEmpty())
		vendorDevice = new Solid::Device(vendorDevice->parentUdi());
	/* NOTE: The device we register has usually an empty vendor string and a less meaningfull product string.
	   So we go up to the first parent device that has a non-empty vendor string,
	   because we find the expected strings describing the device there.
	 */
	if (vendorDevice->isValid())
		kDebug() << "vendor:" << vendorDevice->vendor() << ", product:" << vendorDevice->product();
	else
		kDebug() << "vendor:" << device.vendor() << ", product:" << device.product();
 
	/*if (device.isValid())
	{
		Solid::Video * solidVideoDevice = device.as<Solid::Video>();
		if (solidVideoDevice)
		{
			QStringList protocols = solidVideoDevice->supportedProtocols();
			if (protocols.contains("video4linux"))
			{
				QStringList drivers = solidVideoDevice->supportedDrivers("video4linux");
				if (drivers.contains("video4linux"))
				{
					VideoDevice* videodevice = new VideoDevice;
					videodevice->setUdi( device.udi() );
					videodevice->setFileName(solidVideoDevice->driverHandle("video4linux").toString());
					kDebug() << "V4L device path is" << solidVideoDevice->driverHandle("video4linux").toString();
					if (EXIT_SUCCESS == videodevice->open())
					{
						bool cap = videodevice->canCapture();
						videodevice->close();
						if (cap)
						{
							if (m_videodevices.size() == 0)
								m_current_device = 0;
							m_videodevices.push_back(videodevice);
							kDebug() << "Device is a valid video device, adding it to video device pool.";
							return true;
						}
						else
							kDebug() << "Device does not support capturing.";
					}
					else
						kDebug() << "Device could not be opened.";
					delete videodevice;
				}
			}
		}
		else
			kDebug() << "Device is not a video device.";
	}
	else*/
		kDebug() << "Not a valid Solid device: device is not available in the system.";
	return false;
}

} // namespace AV

} // namespace Kopete
