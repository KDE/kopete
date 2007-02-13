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
	VideoInput( const QString & name, const int hasTuner, const __u64 standards );
	~VideoInput();

	QString name() const;
	void setName( const QString & );
	int hasTuner() const;
	void setHasTuner( const int hasTuner );
	__u64 standards() const;
	void setStandards( __u64 standards );
	float brightness() const;
	void setBrightness(float brightness);
	float contrast() const;
	void setContrast(float contrast);
	float saturation() const;
	void setSaturation(float saturation);
	float whiteness() const;
	void setWhiteness(float whiteness);
	float hue() const;
	void setHue(float Hue);
	bool autoBrightnessContrast() const;
	void setAutoBrightnessContrast(bool brightnesscontrast);
	bool autoColorCorrection() const;
	void setAutoColorCorrection(bool colorcorrection);
	bool imageAsMirror() const;
	void setImageAsMirror(bool imageasmirror);

protected:
	float m_brightness;
	float m_contrast;
	float m_saturation;
	float m_whiteness;
	float m_hue;
	bool m_autobrightnesscontrast;
	bool m_autocolorcorrection;
	bool m_imageasmirror;
	QString m_name;
	int m_hasTuner;
	__u64 m_standards;
};

} // namespace AV

} // namespace Kopete

#endif
