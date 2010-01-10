/*
    videodevice.h  -  Kopete Video Input Class

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

#ifndef KOPETE_AVVIDEOINPUT_H
#define KOPETE_AVVIDEOINPUT_H

#ifdef __linux__
#include <asm/types.h>
#undef __STRICT_ANSI__
#endif // __linux__
#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif // __u64*/

#include <qstring.h>
#include <kdebug.h>
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
	__u64 m_standards;
	bool img_softcorr_autobrightnesscontrast;
	bool img_softcorr_autocolor;
	bool img_softcorr_vflip;
	bool img_softcorr_hflip;
};

}

}

#endif
