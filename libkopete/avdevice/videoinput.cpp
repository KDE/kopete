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

#include "videoinput.h"

namespace Kopete {

namespace AV {

VideoInput::VideoInput()
{
	kDebug() << "Executing Video Input's constructor!!!";
	img_softcorr_autobrightnesscontrast = false;
	img_softcorr_autocolor = false;
	img_softcorr_vflip = false;
	img_softcorr_hflip = false;
	hastuner = 0;
	m_standards = 0;
}


VideoInput::~VideoInput()
{
}

}

}
