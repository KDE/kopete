/*
    avdeviceconfig.cpp  -  Kopete Video Device Configuration Panel

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
	connect(mPrfsVideoDevice->mDeviceKComboBox,              SIGNAL(activated(int)),    this, SLOT(slotDeviceKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mInputKComboBox,               SIGNAL(activated(int)),    this, SLOT(slotInputKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mStandardKComboBox,            SIGNAL(activated(int)),    this, SLOT(slotStandardKComboBoxChanged(int)));
	connect(mPrfsVideoDevice->mBrightnessSlider,             SIGNAL(valueChanged(int)), this, SLOT(slotBrightnessSliderChanged(int)));
	connect(mPrfsVideoDevice->mContrastSlider,               SIGNAL(valueChanged(int)), this, SLOT(slotContrastSliderChanged(int)));
	connect(mPrfsVideoDevice->mSaturationSlider,             SIGNAL(valueChanged(int)), this, SLOT(slotSaturationSliderChanged(int)));
	connect(mPrfsVideoDevice->mWhitenessSlider,              SIGNAL(valueChanged(int)), this, SLOT(slotWhitenessSliderChanged(int)));
	connect(mPrfsVideoDevice->mHueSlider,                    SIGNAL(valueChanged(int)), this, SLOT(slotHueSliderChanged(int)));
	connect(mPrfsVideoDevice->mImageAutoBrightnessContrast,  SIGNAL(toggled(bool)),     this, SLOT(slotImageAutoBrightnessContrastChanged(bool)));
	connect(mPrfsVideoDevice->mImageAutoColorCorrection,     SIGNAL(toggled(bool)),     this, SLOT(slotImageAutoColorCorrectionChanged(bool)));
	connect(mPrfsVideoDevice->mImageAsMirror,                SIGNAL(toggled(bool)),     this, SLOT(slotImageAsMirrorChanged(bool)));

	// why is this here?
	// mPrfsVideoDevice->mVideoImageLabel->setPixmap(qpixmap);
	mAVDeviceTabCtl->addTab(mPrfsVideoDevice, i18n("&Video"));
	mVideoDevicePool = Kopete::AV::VideoDevicePool::self();
	mVideoDevicePool->open();
	mVideoDevicePool->setSize(320, 240);

	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
	setVideoInputParameters();

	mVideoDevicePool->startCapturing();
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&qimage);
	if (qpixmap.convertFromImage(qimage,0) == true)
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(qpixmap);
	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	qtimer.start(0,FALSE);
}


AVDeviceConfig::~AVDeviceConfig()
{
	mVideoDevicePool->close();
}




/*!
    \fn VideoDeviceConfig::save()
 */
void AVDeviceConfig::save()
{
    /// @todo implement me
	kdDebug() << "kopete:config (avdevice): save() called. " << endl;
	mVideoDevicePool->saveConfig();
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

void AVDeviceConfig::setVideoInputParameters()
{
	if(mVideoDevicePool->size())
	{
		mPrfsVideoDevice->mBrightnessSlider->setValue((int)(mVideoDevicePool->getBrightness()*65535));
		mPrfsVideoDevice->mContrastSlider->setValue((int)(mVideoDevicePool->getContrast()*65535));
		mPrfsVideoDevice->mSaturationSlider->setValue((int)(mVideoDevicePool->getSaturation()*65535));
		mPrfsVideoDevice->mWhitenessSlider->setValue((int)(mVideoDevicePool->getWhiteness()*65535));
		mPrfsVideoDevice->mHueSlider->setValue((int)(mVideoDevicePool->getHue()*65535));
		mPrfsVideoDevice->mImageAutoBrightnessContrast->setChecked(mVideoDevicePool->getAutoBrightnessContrast());
		mPrfsVideoDevice->mImageAutoColorCorrection->setChecked(mVideoDevicePool->getAutoColorCorrection());
		mPrfsVideoDevice->mImageAsMirror->setChecked(mVideoDevicePool->getImageAsMirror());
	}
}

void AVDeviceConfig::slotDeviceKComboBoxChanged(int){
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. " << endl;
	unsigned int newdevice = mPrfsVideoDevice->mDeviceKComboBox->currentItem();
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) Current device: " << mVideoDevicePool->currentDevice() << "New device: " << newdevice << endl;
	if ((newdevice < mVideoDevicePool->m_videodevice.size())&&(newdevice!=mVideoDevicePool->currentDevice()))
	{
	kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) should change device. " << endl;
		mVideoDevicePool->open(newdevice);
		mVideoDevicePool->setSize(320, 240);
		mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		mVideoDevicePool->startCapturing();
		setVideoInputParameters();
		kdDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. " << endl;
		emit changed( true );
	}

}

void AVDeviceConfig::slotInputKComboBoxChanged(int){
	unsigned int newinput = mPrfsVideoDevice->mInputKComboBox->currentItem();
	if((newinput < mVideoDevicePool->inputs()) && ( newinput !=mVideoDevicePool->currentInput()))
	{
		mVideoDevicePool->selectInput(mPrfsVideoDevice->mInputKComboBox->currentItem());
		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
		setVideoInputParameters();
		emit changed( true );
	}
}

// ATTENTION: The 65535.0 value must be used instead of 65535 because the trailing ".0" converts the resulting value to floating point number.
// Otherwise the resulting division operation would return 0 or 1 exclusively.

void AVDeviceConfig::slotStandardKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotBrightnessSliderChanged(int){
	kdDebug() << "kopete:config (avdevice): slotBrightnessSliderChanged(int) called. " << mPrfsVideoDevice->mBrightnessSlider->value() / 65535.0 << endl;
	mVideoDevicePool->setBrightness( mPrfsVideoDevice->mBrightnessSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotContrastSliderChanged(int){
	kdDebug() << "kopete:config (avdevice): slotContrastSliderChanged(int) called. " << mPrfsVideoDevice->mContrastSlider->value() / 65535.0 << endl;
	mVideoDevicePool->setContrast( mPrfsVideoDevice->mContrastSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotSaturationSliderChanged(int){
	kdDebug() << "kopete:config (avdevice): slotSaturationSliderChanged(int) called. " << mPrfsVideoDevice->mSaturationSlider->value() / 65535.0 << endl;
	mVideoDevicePool->setSaturation( mPrfsVideoDevice->mSaturationSlider->value() / 65535.0);
  emit changed( true );
}

void AVDeviceConfig::slotWhitenessSliderChanged(int){
	kdDebug() << "kopete:config (avdevice): slotWhitenessSliderChanged(int) called. " << mPrfsVideoDevice->mWhitenessSlider->value() / 65535.0 << endl;
	mVideoDevicePool->setWhiteness( mPrfsVideoDevice->mWhitenessSlider->value() / 65535.0);
  emit changed( true );
}

void AVDeviceConfig::slotHueSliderChanged(int){
	kdDebug() << "kopete:config (avdevice): slotHueSliderChanged(int) called. " << mPrfsVideoDevice->mHueSlider->value() / 65535.0 << endl;
	mVideoDevicePool->setHue( mPrfsVideoDevice->mHueSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotImageAutoBrightnessContrastChanged(bool){
	kdDebug() << "kopete:config (avdevice): slotImageAutoBrightnessContrastChanged(" << mPrfsVideoDevice->mImageAutoBrightnessContrast->isChecked() << ") called. " << endl;
	mVideoDevicePool->setAutoBrightnessContrast(mPrfsVideoDevice->mImageAutoBrightnessContrast->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotImageAutoColorCorrectionChanged(bool){
	kdDebug() << "kopete:config (avdevice): slotImageAutoColorCorrectionChanged(" << mPrfsVideoDevice->mImageAutoColorCorrection->isChecked() << ") called. " << endl;
	mVideoDevicePool->setAutoColorCorrection(mPrfsVideoDevice->mImageAutoColorCorrection->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotImageAsMirrorChanged(bool){
	kdDebug() << "kopete:config (avdevice): slotImageAsMirrorChanged(" << mPrfsVideoDevice->mImageAsMirror->isChecked() << ") called. " << endl;
	mVideoDevicePool->setImageAsMirror(mPrfsVideoDevice->mImageAsMirror->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotUpdateImage()
{
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&qimage);
	bitBlt(mPrfsVideoDevice->mVideoImageLabel, 0, 0, &qimage, 0, Qt::CopyROP);
//	kdDebug() << "kopete (avdeviceconfig_videoconfig): Image updated." << endl;
}
