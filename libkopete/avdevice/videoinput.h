//
// C++ Interface: videoinput
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KOPETE_AVVIDEOINPUT_H
#define KOPETE_AVVIDEOINPUT_H

#include <qstring.h>
#include "kopete_export.h"

namespace Kopete {

namespace AV {

/**
@author Kopete Developers
*/
class KOPETE_EXPORT VideoInput{
public:
	VideoInput();
	~VideoInput();
	QString name;
	int  hastuner;

};

}

}

#endif
