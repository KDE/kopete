//
// C++ Interface: videodevicemodelpool
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KOPETE_AVVIDEODEVICEMODELPOOL_H
#define KOPETE_AVVIDEODEVICEMODELPOOL_H

#include <qstring.h>
#include <qvaluevector.h>
#include <kdebug.h>
#include "kopete_export.h"

namespace Kopete {

namespace AV {

/**
	@author Kopete Developers <kopete-devel@kde.org>
*/
class VideoDeviceModelPool{

	struct VideoDeviceModel
	{
		QString model;
		size_t count;
	};
	QValueVector<VideoDeviceModel> m_devicemodel;
public:
	VideoDeviceModelPool();
	~VideoDeviceModelPool();
	void clear();
	size_t size();
	size_t addModel(QString newmodel);
};

}

}

#endif
