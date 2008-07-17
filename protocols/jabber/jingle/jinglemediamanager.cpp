#include "jinglemediamanager.h"
#include <KDebug>

JingleMediaManager::JingleMediaManager()
{
	findDevice();
}

JingleMediaManager::~JingleMediaManager()
{

}

/*
 * Find audio input and output devices.
 */
int JingleMediaManager::findDevice()
{
	m_inputDevice = 0L;
	m_outputDevice = 0L;
	QList<Solid::Device> devicesList = Solid::Device::listFromType(Solid::DeviceInterface::AudioInterface, QString());
        int i = 0, ret = 0;
        while (i <= devicesList.count())
        {
                Solid::AudioInterface *device = devicesList[i].as<Solid::AudioInterface>();
                if (device->deviceType() == Solid::AudioInterface::AudioInput)
                {
			if (m_inputDevice != 0L)
				delete m_inputDevice;
			kDebug() << "Microphone found. The driver used is " << device->driver();
			m_inputDevice = device;
                	i++;
			ret++;
			continue;
                }
                
		if (device->deviceType() == Solid::AudioInterface::AudioOutput)
                {
			if (m_outputDevice != 0L)
				delete m_outputDevice;
			kDebug() << "Sound card found. The driver used is " << device->driver();
			m_outputDevice = device;
                	i++;
			ret++;
			continue;
                }
                
		delete device;
                i++;
        }

	return ret;

}
