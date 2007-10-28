/*
    videoinput.cpp  -  Kopete Video Input Class

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

#ifndef KOPETE_AVVIDEOCONTROL_H
#define KOPETE_AVVIDEOCONTROL_H

#include <asm/types.h>
#undef __STRICT_ANSI__
#ifndef __u64 //required by videodev.h
#define __u64 unsigned long long
#endif // __u64

#ifndef __s64 //required by videodev.h
#define __s64 long long
#endif // __s64

#include <qstring.h>
#include <kdebug.h>
#include <qvaluevector.h>
#include "kopete_export.h"

namespace Kopete {

namespace AV {

typedef enum
{
	CONTROLTYPE_INTEGER	= 0,
	CONTROLTYPE_BOOLEAN	= 1,
	CONTROLTYPE_MENU	= 2,
	CONTROLTYPE_BUTTON	= 3
} control_type;

typedef enum
{
	CONTROLFLAG_DISABLED	= (1 << 0), // This control is permanently disabled and should be ignored by the application.
	CONTROLFLAG_GRABBED	= (1 << 1), // This control is temporarily unchangeable,
	CONTROLFLAG_READONLY	= (1 << 2), // This control is permanently readable only.
	CONTROLFLAG__UPDATE	= (1 << 3), // Changing this control may affect the value of other controls within the same control class.
	CONTROLFLAG_INACTIVE	= (1 << 4), // This control is not applicable to the current configuration.
	CONTROLFLAG_SLIDER	= (1 << 5)  // This control is best represented as a slider.
} control_flag;
/**
	@author Kopete Developers <kopete-devel@kde.org>
*/
class VideoControl{
public:
	VideoControl();
	~VideoControl();

protected:
	__u32 m_id;
	control_type m_type;
	QString m_name;
	__s32 m_minimum;
	__s32 m_maximum;
	__s32 m_step;
	__s32 m_default;
	__u32 m_flags;
};

}

}

#endif
