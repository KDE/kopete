/*
    videoinput.cpp  -  Kopete Video Input Class

    Copyright (c) 2005 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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
	kDebug() << k_funcinfo << "Executing Video Input's constructor!!!" << endl;
	m_brightness = 0.5;
	m_contrast = 0.5;
	m_saturation = 0.5;
	m_hue = 0.0;
	m_autobrightnesscontrast = false;
	m_autocolorcorrection = false;
}


VideoInput::~VideoInput()
{
}

float VideoInput::getBrightness()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_brightness;
}

float VideoInput::setBrightness(float brightness)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	if ( brightness > 1 )
		brightness = 1;
	else
	if ( brightness < 0 )
		brightness = 0;
	m_brightness = brightness;
	return getBrightness();
}

float VideoInput::getContrast()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_contrast;
}

float VideoInput::setContrast(float contrast)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	if ( contrast > 1 )
		contrast = 1;
	else
	if ( contrast < 0 )
		contrast = 0;
	m_contrast = contrast;
	return getContrast();
}

float VideoInput::getSaturation()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_saturation;
}

float VideoInput::setSaturation(float saturation)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	if ( saturation > 1 )
		saturation = 1;
	else
	if ( saturation < 0 )
		saturation = 0;
	m_saturation = saturation;
	return getSaturation();
}

float VideoInput::getHue()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_hue;
}

float VideoInput::setHue(float hue)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	if ( hue > 1 )
		hue = 1;
	else
	if ( hue < 0 )
		hue = 0;
	m_hue = hue;
	return getHue();
}


bool VideoInput::getAutoBrightnessContrast()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_autobrightnesscontrast;
}

bool VideoInput::setAutoBrightnessContrast(bool brightnesscontrast)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	m_autobrightnesscontrast = brightnesscontrast;
	return getAutoBrightnessContrast();
}

bool VideoInput::getAutoColorCorrection()
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	return m_autocolorcorrection;
}

bool VideoInput::setAutoColorCorrection(bool colorcorrection)
{
//	kDebug() <<  k_funcinfo << " called." << endl;
	m_autocolorcorrection = colorcorrection;
	return getAutoColorCorrection();
}

}

}
