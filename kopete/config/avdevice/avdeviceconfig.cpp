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

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qslider.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kpluginfactory.h>
#include <ktrader.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <qimage.h>
#include <qpixmap.h>

#include <qtabwidget.h>

//#include "videodevice.h"
K_PLUGIN_FACTORY( KopeteAVDeviceConfigFactory,
		registerPlugin<AVDeviceConfig>(); )
K_EXPORT_PLUGIN( KopeteAVDeviceConfigFactory("kcm_kopete_avdeviceconfig") )

AVDeviceConfig::AVDeviceConfig(QWidget *parent, const QVariantList &args)
 : KCModule( KopeteAVDeviceConfigFactory::componentData(), parent, args )
{
	kDebug() << "kopete:config (avdevice): KopeteAVDeviceConfigFactory::componentData() called. ";
// "Video" TAB ============================================================
	mPrfsVideoDevice = new Ui_AVDeviceConfig_VideoDevice();
	mPrfsVideoDevice->setupUi(this);

	// set a default image for the webcam widget, in case the user does not have a video device
	mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));

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

	mVideoDevicePool = Kopete::AV::VideoDevicePool::self();

	startCapturing();

	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);

	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, mVideoDevicePool->size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, mVideoDevicePool->size());

	connect(mVideoDevicePool, SIGNAL(deviceRegistered(const QString &) ),
			SLOT(deviceRegistered(const QString &)) );
	connect(mVideoDevicePool, SIGNAL(deviceUnregistered(const QString &) ),
			SLOT(deviceUnregistered(const QString &)) );

	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
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
	kDebug() << "kopete:config (avdevice): save() called. ";
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
	kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. ";
	int newdevice = mPrfsVideoDevice->mDeviceKComboBox->currentIndex();
	kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) Current device: " << mVideoDevicePool->currentDevice() << "New device: " << newdevice;
	if ((newdevice>=0 && newdevice < mVideoDevicePool->m_videodevice.size())&&(newdevice!=mVideoDevicePool->currentDevice()))
	{
		kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) should change device. ";
		stopCapturing();
		mVideoDevicePool->open(newdevice);
		mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		startCapturing();
		kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. ";
		emit changed( true );
	}
}

void AVDeviceConfig::slotInputKComboBoxChanged(int){
	int newinput = mPrfsVideoDevice->mInputKComboBox->currentIndex();
	if ((newinput < mVideoDevicePool->inputs()) && (newinput != mVideoDevicePool->currentInput()))
	{
		stopCapturing();
 		mVideoDevicePool->selectInput(mPrfsVideoDevice->mInputKComboBox->currentIndex());
 		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
		startCapturing();
 		emit changed( true );
	}
}

// ATTENTION: The 65535.0 value must be used instead of 65535 because the trailing ".0" converts the resulting value to floating point number.
// Otherwise the resulting division operation would return 0 or 1 exclusively.

void AVDeviceConfig::slotStandardKComboBoxChanged(int){
  emit changed( true );
}

void AVDeviceConfig::slotBrightnessSliderChanged(int){
	kDebug() << "kopete:config (avdevice): slotBrightnessSliderChanged(int) called. " << mPrfsVideoDevice->mBrightnessSlider->value() / 65535.0;
	mVideoDevicePool->setBrightness( mPrfsVideoDevice->mBrightnessSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotContrastSliderChanged(int){
	kDebug() << "kopete:config (avdevice): slotContrastSliderChanged(int) called. " << mPrfsVideoDevice->mContrastSlider->value() / 65535.0;
	mVideoDevicePool->setContrast( mPrfsVideoDevice->mContrastSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotSaturationSliderChanged(int){
	kDebug() << "kopete:config (avdevice): slotSaturationSliderChanged(int) called. " << mPrfsVideoDevice->mSaturationSlider->value() / 65535.0;
	mVideoDevicePool->setSaturation( mPrfsVideoDevice->mSaturationSlider->value() / 65535.0);
  emit changed( true );
}

void AVDeviceConfig::slotWhitenessSliderChanged(int){
	kDebug() << "kopete:config (avdevice): slotWhitenessSliderChanged(int) called. " << mPrfsVideoDevice->mWhitenessSlider->value() / 65535.0;
	mVideoDevicePool->setWhiteness( mPrfsVideoDevice->mWhitenessSlider->value() / 65535.0);
  emit changed( true );
}

void AVDeviceConfig::slotHueSliderChanged(int){
	kDebug() << "kopete:config (avdevice): slotHueSliderChanged(int) called. " << mPrfsVideoDevice->mHueSlider->value() / 65535.0;
	mVideoDevicePool->setHue( mPrfsVideoDevice->mHueSlider->value() / 65535.0 );
  emit changed( true );
}

void AVDeviceConfig::slotImageAutoBrightnessContrastChanged(bool){
	kDebug() << "kopete:config (avdevice): slotImageAutoBrightnessContrastChanged(" << mPrfsVideoDevice->mImageAutoBrightnessContrast->isChecked() << ") called. ";
	mVideoDevicePool->setAutoBrightnessContrast(mPrfsVideoDevice->mImageAutoBrightnessContrast->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotImageAutoColorCorrectionChanged(bool){
	kDebug() << "kopete:config (avdevice): slotImageAutoColorCorrectionChanged(" << mPrfsVideoDevice->mImageAutoColorCorrection->isChecked() << ") called. ";
	mVideoDevicePool->setAutoColorCorrection(mPrfsVideoDevice->mImageAutoColorCorrection->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotImageAsMirrorChanged(bool){
	kDebug() << "kopete:config (avdevice): slotImageAsMirrorChanged(" << mPrfsVideoDevice->mImageAsMirror->isChecked() << ") called. ";
	mVideoDevicePool->setImageAsMirror(mPrfsVideoDevice->mImageAsMirror->isChecked());
	emit changed( true );
}

void AVDeviceConfig::slotUpdateImage()
{
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&qimage);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(QPixmap::fromImage(qimage.mirrored(mVideoDevicePool->getImageAsMirror(),false)));
	//kDebug() << "kopete (avdeviceconfig_videoconfig): Image updated.";
}

void AVDeviceConfig::deviceRegistered( const QString & udi )
{
	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);

	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, mVideoDevicePool->size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, mVideoDevicePool->size());

	if (mVideoDevicePool->size() < 2) // otherwise we are already capturing
		startCapturing();
}

void AVDeviceConfig::deviceUnregistered( const QString & udi )
{
	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);

	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, mVideoDevicePool->size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, mVideoDevicePool->size());

	if (capturingDevice_udi == udi)
	{
		qtimer.stop();
		mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
		capturingDevice_udi.clear();

		if (mVideoDevicePool->size())
			startCapturing();
	}
}

void AVDeviceConfig::startCapturing()
{
	if (EXIT_SUCCESS == mVideoDevicePool->open())
	{
		mVideoDevicePool->setSize(320, 240);
		mVideoDevicePool->startCapturing();
		setVideoInputParameters();
		capturingDevice_udi = mVideoDevicePool->currentDeviceUdi();
		qtimer.start(40);
		mPrfsVideoDevice->mVideoImageLabel->setScaledContents(true);
	}
}

void AVDeviceConfig::stopCapturing()
{
	qtimer.stop();
	mVideoDevicePool->stopCapturing();
	mVideoDevicePool->close();
	mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
	capturingDevice_udi.clear();
}

