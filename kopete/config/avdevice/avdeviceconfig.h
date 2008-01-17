/*
    avdeviceconfig.h  -  Kopete Video Device Configuration Panel

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef AVDEVICECONFIG_H
#define AVDEVICECONFIG_H

#include "kcmodule.h"
#include "videodevicepool.h"
#include <qimage.h>
#include <qpixmap.h>
#include <qtimer.h>

#ifdef HAVE_GL
# include <qgl.h>
#endif

class QFrame;
class QTabWidget;

class AVDeviceConfig_VideoDevice;
class AVDeviceConfig_AudioDevice;

/**
@author Cl�dio da Silveira Pinheiro
*/
class AVDeviceConfig : public KCModule
{
Q_OBJECT
public:
	AVDeviceConfig(QWidget *parent, const char *  name , const QStringList &args);

	~AVDeviceConfig();
	virtual void save();
	virtual void load();

private slots:
	void slotSettingsChanged(bool);
	void slotValueChanged(int);
	void slotDeviceKComboBoxChanged(int);
	void slotInputKComboBoxChanged(int);
	void slotStandardKComboBoxChanged(int);
	void slotBrightnessSliderChanged(int);
	void slotContrastSliderChanged(int);
	void slotSaturationSliderChanged(int);
	void slotWhitenessSliderChanged(int);
	void slotHueSliderChanged(int);
	void slotImageAutoBrightnessContrastChanged(bool);
	void slotImageAutoColorCorrectionChanged(bool);
	void slotImageAsMirrorChanged(bool);
	void slotUpdateImage();
private:
	QTabWidget* mAVDeviceTabCtl;
	AVDeviceConfig_VideoDevice  *mPrfsVideoDevice;
	AVDeviceConfig_AudioDevice  *mPrfsAudioDevice;
	Kopete::AV::VideoDevicePool *mVideoDevicePool ;
	QImage qimage;
	QPixmap qpixmap;
	QTimer qtimer;
	void setVideoInputParameters();
#ifdef HAVE_GL
	QGLWidget m_video_gl;
#endif
};

#endif
