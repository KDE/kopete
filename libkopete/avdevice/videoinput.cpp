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
	kdDebug( 14010 ) << k_funcinfo << "Executing Video Input's constructor!!!" << endl;
	m_brightness = 0.5;
	m_contrast = 0.5;
	m_saturation = 0.5;
	m_hue = 0.5;
	m_autobrightnesscontrast = false;
	m_autocolorcorrection = false;
	m_imageasmirror = true;
}

VideoInput::VideoInput( const QString & name, const int hasTuner, const __u64 standards )
{
	kdDebug( 14010 ) << k_funcinfo << "Executing Video Input's constructor!!!" << endl;
	m_name = name;
	m_hasTuner = hasTuner;
	m_standards = standards;
	m_brightness = 0.5;
	m_contrast = 0.5;
	m_saturation = 0.5;
	m_hue = 0.5;
	m_autobrightnesscontrast = false;
	m_autocolorcorrection = false;
	m_imageasmirror = true;
}


VideoInput::~VideoInput()
{
}

QString VideoInput::name() const
{
	return m_name;
}

void VideoInput::setName( const QString & name )
{
	m_name = name;
}

int VideoInput::hasTuner() const
{
	return m_hasTuner;
}

void VideoInput::setHasTuner( const int hasTuner )
{
	m_hasTuner = hasTuner;
}

__u64 VideoInput::standards() const
{
	return m_standards;
}

void VideoInput::setStandards( __u64 standards )
{
	m_standards = standards;
}

float VideoInput::brightness() const
{
	return m_brightness;
}

void VideoInput::setBrightness(float brightness)
{
	brightness = QMIN( brightness, 1.0 );
	brightness = QMAX( brightness, 0.0 );
	m_brightness = brightness;
}

float VideoInput::contrast() const
{
	return m_contrast;
}

void VideoInput::setContrast(float contrast)
{
	contrast = QMIN( contrast, 1.0 );
	contrast = QMAX( contrast, 0.0 );
	m_contrast = contrast;
}

float VideoInput::saturation() const
{
	return m_saturation;
}

void VideoInput::setSaturation(float saturation)
{
	saturation = QMIN( saturation, 1.0 );
	saturation = QMAX( saturation, 0.0 );
	m_saturation = saturation;
}

float VideoInput::whiteness() const
{
	return m_whiteness;
}

void VideoInput::setWhiteness(float whiteness)
{
	whiteness = QMIN( whiteness, 1.0 );
	whiteness = QMAX( whiteness, 0.0 );
	m_whiteness = whiteness;
}

float VideoInput::hue() const
{
	return m_hue;
}

void VideoInput::setHue(float hue)
{
	hue = QMIN( hue, 1.0 );
	hue = QMAX( hue, 0.0 );
	m_hue = hue;
}


bool VideoInput::autoBrightnessContrast() const
{
	return m_autobrightnesscontrast;
}

void VideoInput::setAutoBrightnessContrast(bool brightnesscontrast)
{
	m_autobrightnesscontrast = brightnesscontrast;
}

bool VideoInput::autoColorCorrection() const
{
	return m_autocolorcorrection;
}

void VideoInput::setAutoColorCorrection(bool colorcorrection)
{
	m_autocolorcorrection = colorcorrection;
}

bool VideoInput::imageAsMirror() const
{
	return m_imageasmirror;
}

void VideoInput::setImageAsMirror(bool imageasmirror)
{
	m_imageasmirror = imageasmirror;
}

} // namespace AV

} // namespace kopete
