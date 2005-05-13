/*
    avdeviceconfig.cpp  -  Kopete Video Device Configuration Panel

    Copyright (c) 2005 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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

#include "avdeviceconfig.h"
#include "avdeviceconfig_videoconfig.h"
#include "videodevice.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qslider.h>

#include <kdebug.h>
#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>
#include <kcombobox.h>

#include <qtabwidget.h>

//#include "videodevice.h"
typedef KGenericFactory<AVDeviceConfig, QWidget> KopeteAVDeviceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_avdeviceconfig, KopeteAVDeviceConfigFactory( "kcm_kopete_avdeviceconfig" ) )

AVDeviceConfig::AVDeviceConfig(QWidget *parent, const char *  name , const QStringList &args)
 : KCModule( KopeteAVDeviceConfigFactory::instance(), parent, args )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	mAVDeviceTabCtl = new QTabWidget(this, "mAVDeviceTabCtl");

// "Video" TAB ============================================================
	mPrfsVideoDevice = new AVDeviceConfig_VideoDevice(mAVDeviceTabCtl);
	connect(mPrfsVideoDevice->mDeviceKComboBox,               SIGNAL(activated(int)),    this, SLOT(slotDeviceKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mInputKComboBox,                SIGNAL(activated(int)),    this, SLOT(slotInputKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mStandardKComboBox,             SIGNAL(activated(int)),    this, SLOT(slotStandardKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mBrightSlider,                  SIGNAL(valueChanged(int)), this, SLOT(slotBrightSliderChanged(int)));
	connect(mPrfsVideoDevice->mContrastSlider,                SIGNAL(valueChanged(int)), this, SLOT(slotContrastSliderChanged(int)));
	connect(mPrfsVideoDevice->mSaturationSlider,              SIGNAL(valueChanged(int)), this, SLOT(slotSaturationSliderChanged(int)));
	connect(mPrfsVideoDevice->mHueSlider,                     SIGNAL(valueChanged(int)), this, SLOT(slotHueSliderChanged(int)));
	connect(mPrfsVideoDevice->mImageAutoAdjustBrightContrast, SIGNAL(toggled(bool)),     this, SLOT(slotImageAutoAdjustBrightContrastChanged(bool)));

	 mAVDeviceTabCtl->addTab(mPrfsVideoDevice, i18n("&Video"));
	d = Kopete::AV::VideoDevice::self();
	d->setDevice(0);
	d->open();
	d->close();
}


AVDeviceConfig::~AVDeviceConfig()
{
	delete d;
}




/*!
    \fn VideoDeviceConfig::save()
 */
void AVDeviceConfig::save()
{
    /// @todo implement me
}


/*!
    \fn VideoDeviceConfig::load()
 */
void AVDeviceConfig::load()
{
    /// @todo implement me
}

void AVDeviceConfig::slotSettingsChanged(bool){
  emit changed(true);
}

void AVDeviceConfig::slotValueChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotDeviceKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotInputKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotStandardKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotBrightSliderChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotContrastSliderChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotSaturationSliderChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotHueSliderChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotImageAutoAdjustBrightContrastChanged(bool){
  emit changed( true );
}
