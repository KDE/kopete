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

#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <qimage.h>
#include <qpixmap.h>

#include <qtabwidget.h>

//#include "videodevice.h"
typedef KGenericFactory<AVDeviceConfig, QWidget> KopeteAVDeviceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_avdeviceconfig, KopeteAVDeviceConfigFactory( "kcm_kopete_avdeviceconfig" ) )

AVDeviceConfig::AVDeviceConfig(QWidget *parent, const char *  name , const QStringList &args)
 : KCModule( KopeteAVDeviceConfigFactory::instance(), parent, args )
{
	kdDebug() << "kopete:config (avdevice): KopeteAVDeviceConfigFactory::instance() called. " << endl;
	(new QVBoxLayout(this))->setAutoAdd(true);
	mAVDeviceTabCtl = new QTabWidget(this, "mAVDeviceTabCtl");

// "Video" TAB ============================================================
	mPrfsVideoDevice = new AVDeviceConfig_VideoDevice(mAVDeviceTabCtl);
	connect(mPrfsVideoDevice->mDeviceKComboBox,                   SIGNAL(activated(int)),    this, SLOT(slotDeviceKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mInputKComboBox,                    SIGNAL(activated(int)),    this, SLOT(slotInputKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mStandardKComboBox,                 SIGNAL(activated(int)),    this, SLOT(slotStandardKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mBrightnessSlider,                  SIGNAL(valueChanged(int)), this, SLOT(slotBrightnessSliderChanged(int)));
	connect(mPrfsVideoDevice->mContrastSlider,                    SIGNAL(valueChanged(int)), this, SLOT(slotContrastSliderChanged(int)));
	connect(mPrfsVideoDevice->mSaturationSlider,                  SIGNAL(valueChanged(int)), this, SLOT(slotSaturationSliderChanged(int)));
	connect(mPrfsVideoDevice->mHueSlider,                         SIGNAL(valueChanged(int)), this, SLOT(slotHueSliderChanged(int)));
	connect(mPrfsVideoDevice->mImageAutoAdjustBrightnessContrast, SIGNAL(toggled(bool)),     this, SLOT(slotImageAutoAdjustBrightnessContrastChanged(bool)));
	connect(mPrfsVideoDevice->mImageAutoColorCorrection,          SIGNAL(toggled(bool)),     this, SLOT(slotImageAutoColorCorrectionChanged(bool)));

	// why is this here?
	// mPrfsVideoDevice->mVideoImageLabel->setPixmap(qpixmap);
	mAVDeviceTabCtl->addTab(mPrfsVideoDevice, i18n("&Video"));
	d = Kopete::AV::VideoDevicePool::self();
	d->scanDevices();
	d->open(0);
	d->setSize(320, 240);
	d->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	d->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	d->selectInput(0);
	d->startCapturing();
	d->getFrame();
	d->getImage(&qimage);
	if (qpixmap.convertFromImage(qimage,0) == true)
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(qpixmap);
	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	qtimer.start(500,FALSE);
}


AVDeviceConfig::~AVDeviceConfig()
{
//	d->stopCapturing();
	d->close();
//	delete d;
}




/*!
    \fn VideoDeviceConfig::save()
 */
void AVDeviceConfig::save()
{
    /// @todo implement me
	kdDebug() << "kopete:config (avdevice): save() called. " << endl;
	d->saveConfig();
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
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. " << endl;
	unsigned int newdevice = mPrfsVideoDevice->mDeviceKComboBox->currentItem();
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) Current device: " << d->currentDevice() << "New device: " << newdevice << endl;
	if ((newdevice < d->m_videodevice.size())&&(newdevice!=d->currentDevice()))
	{
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) should change device. " << endl;
		d->open(newdevice);
		d->setSize(320, 240);
		d->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		d->selectInput(0);
		d->startCapturing();
	}
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. " << endl;
	emit changed( true );
}

void AVDeviceConfig::slotInputKComboBoxChanged(int){
	d->selectInput(mPrfsVideoDevice->mInputKComboBox->currentItem());
	emit changed( true );
}

void AVDeviceConfig::slotStandardKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotBrightnessSliderChanged(int){
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

void AVDeviceConfig::slotImageAutoAdjustBrightnessContrastChanged(bool){
  emit changed( true );
}

void AVDeviceConfig::slotImageAutoColorCorrectionChanged(bool){
	kdDebug() << "kopete:config (avdevice): slotImageAutoColorCorrectionChanged(" << mPrfsVideoDevice->mImageAutoColorCorrection->isChecked() << ") called. " << endl;
	d->setAutoColorCorrection(mPrfsVideoDevice->mImageAutoColorCorrection->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotUpdateImage()
{
	d->getFrame();
	d->getImage(&qimage);
	bitBlt(mPrfsVideoDevice->mVideoImageLabel, 0, 0, &qimage, 0, Qt::CopyROP);
	kdDebug() << "kopete (avdeviceconfig_videoconfig): Image updated." << endl;
}
