//
// C++ Implementation: videodevicemodelpool
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "videodevicemodelpool.h"

namespace Kopete {

namespace AV {

VideoDeviceModelPool::VideoDeviceModelPool()
{
}


VideoDeviceModelPool::~VideoDeviceModelPool()
{
}

void VideoDeviceModelPool::clear()
{
	m_devicemodel.clear();
}

size_t VideoDeviceModelPool::size()
{
	return m_devicemodel.size();
}

size_t VideoDeviceModelPool::addModel( QString newmodel )
{
	VideoDeviceModel newdevicemodel;
	newdevicemodel.model=newmodel;
	newdevicemodel.count=0;

	if(m_devicemodel.size())
	{
		for ( size_t loop = 0 ; loop < m_devicemodel.size(); loop++)
		if (newmodel == m_devicemodel[loop].model)
		{
			kDebug() << k_funcinfo << "Model " << newmodel << " already exists." << endl;
			m_devicemodel[loop].count++;
			return m_devicemodel[loop].count;
		}
	}
	m_devicemodel.push_back(newdevicemodel);
	m_devicemodel[m_devicemodel.size()-1].model = newmodel;
	m_devicemodel[m_devicemodel.size()-1].count = 0;
	return 0;
}


}

}
