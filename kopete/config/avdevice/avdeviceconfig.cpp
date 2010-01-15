/*
    avdeviceconfig.cpp  -  Kopete Video Device Configuration Panel

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
    Copyright (c) 2010      by Frank Schaefer                 <fschaefer.oss@googlemail.com>

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
#include "IdGuiElements.h"


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

	mVideoDevicePool = Kopete::AV::VideoDevicePool::self();

	startCapturing();

	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);

	connect(mVideoDevicePool, SIGNAL(deviceRegistered(const QString &) ),
			SLOT(deviceRegistered(const QString &)) );
	connect(mVideoDevicePool, SIGNAL(deviceUnregistered(const QString &) ),
			SLOT(deviceUnregistered(const QString &)) );

	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
}


AVDeviceConfig::~AVDeviceConfig()
{
	mVideoDevicePool->close();
	clearControlGUIElements();
}


void AVDeviceConfig::setupControls()
{
	int k = 0;
	qint32 cval = 0;
	clearControlGUIElements();
	
	QList<Kopete::AV::NumericVideoControl> numericCtrls;
	QList<Kopete::AV::BooleanVideoControl> booleanCtrls;
	QList<Kopete::AV::MenuVideoControl> menuCtrls;
	QList<Kopete::AV::ActionVideoControl> actionCtrls;
	numericCtrls = mVideoDevicePool->getSupportedNumericControls();
	booleanCtrls = mVideoDevicePool->getSupportedBooleanControls();
	menuCtrls = mVideoDevicePool->getSupportedMenuControls();
	actionCtrls = mVideoDevicePool->getSupportedActionControls();

	kDebug() << "Supported controls:" << numericCtrls.size() << "numeric," << booleanCtrls.size()
		<< "boolean," << menuCtrls.size() << "menus," << actionCtrls.size() << "actions.";

	/* SETUP GUI-elements */
	// Numeric Controls: => Slider
	for (k=0; k<numericCtrls.size(); k++)
	{
		mVideoDevicePool->getControlValue(numericCtrls.at(k).id, &cval);
		addSliderControlElement(numericCtrls.at(k).id, numericCtrls.at(k).name, numericCtrls.at(k).value_min, numericCtrls.at(k).value_max, numericCtrls.at(k).value_step, cval);
	}
	// Boolean Controls: => Checkbox
	for (k=0; k<booleanCtrls.size(); k++)
	{
		mVideoDevicePool->getControlValue(booleanCtrls.at(k).id, &cval);
		addCheckBoxControlElement(booleanCtrls.at(k).id, booleanCtrls.at(k).name, cval);
	}
	// Menu Controls: => Combobox
	for (k=0; k<menuCtrls.size(); k++)
	{
		mVideoDevicePool->getControlValue(menuCtrls.at(k).id, &cval);
		addPopupMenuControlElement(menuCtrls.at(k).id, menuCtrls.at(k).name, menuCtrls.at(k).options, cval);
	}
	// Action Controls: => Button
	for (k=0; k<actionCtrls.size(); k++)
		addButtonControlElement(actionCtrls.at(k).id, actionCtrls.at(k).name);
	/* TODO: check success of mVideoDevicePool->getControlValue() */
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, numericCtrls.size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, booleanCtrls.size() + menuCtrls.size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(3, actionCtrls.size());
}


void AVDeviceConfig::clearControlGUIElements()
{
	for (int k=0; k<ctrlWidgets.size(); k++)
		delete ctrlWidgets.at(k);
	ctrlWidgets.clear();
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, false);
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, false);
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(3, false);
}


void AVDeviceConfig::addSliderControlElement(int cid, QString title, int min, int max, int step, int value)
{
	int insert_row = mPrfsVideoDevice->sliders_gridLayout->rowCount();
	QLabel *label = new QLabel( title + ":", mPrfsVideoDevice->VideoTabWidget );
	mPrfsVideoDevice->sliders_gridLayout->addWidget( label, insert_row, 0 );
	IdSlider *slider = new IdSlider(cid, Qt::Horizontal, mPrfsVideoDevice->VideoTabWidget);
	mPrfsVideoDevice->sliders_gridLayout->addWidget( slider, insert_row, 1 );
	slider->setMinimum( min );
	slider->setMaximum( max );
	slider->setSliderPosition( value );
	slider->setTickInterval( step );
	connect( slider, SIGNAL( valueChanged(uint, int) ), this, SLOT( changeVideoControlValue(uint, int) ) );
	ctrlWidgets.push_back(label);
	ctrlWidgets.push_back(slider);
}


void AVDeviceConfig::addCheckBoxControlElement(int cid, QString title, bool value)
{
	IdCheckBox *checkbox = new IdCheckBox( cid, mPrfsVideoDevice->VideoTabWidget );
	checkbox->setText( title );
	mPrfsVideoDevice->checkboxOptions_verticalLayout->addWidget( checkbox );
	checkbox->setChecked( value );
	connect( checkbox, SIGNAL( stateChanged(uint, int) ), this, SLOT( changeVideoControlValue(uint, int) ) );
	ctrlWidgets.push_back(checkbox);
}


void AVDeviceConfig::addPopupMenuControlElement(int cid, QString title, QStringList options, int menuindex)
{
	int insert_row = mPrfsVideoDevice->menuOptions_gridLayout->rowCount();
	QLabel *label = new QLabel( title + ":", mPrfsVideoDevice->VideoTabWidget );
	mPrfsVideoDevice->menuOptions_gridLayout->addWidget( label, insert_row, 0 );
	IdComboBox *combobox = new IdComboBox( cid, mPrfsVideoDevice->VideoTabWidget );
	mPrfsVideoDevice->menuOptions_gridLayout->addWidget( combobox, insert_row, 1 );
	combobox->addItems( options );
	combobox->setCurrentIndex( menuindex );
	connect( combobox, SIGNAL( currentIndexChanged(uint, int) ), this, SLOT( changeVideoControlValue(uint, int) ) );
	ctrlWidgets.push_back(label);
	ctrlWidgets.push_back(combobox);
}


void AVDeviceConfig::addButtonControlElement(int cid, QString title)
{
	int insert_row = mPrfsVideoDevice->actions_gridLayout->rowCount();
	QLabel *label = new QLabel( title + ":", mPrfsVideoDevice->VideoTabWidget );
	mPrfsVideoDevice->actions_gridLayout->addWidget( label, insert_row, 0 );
	IdPushButton *button = new IdPushButton( cid, mPrfsVideoDevice->VideoTabWidget );
	button->setText( i18n("Execute") );
	mPrfsVideoDevice->actions_gridLayout->addWidget( button, insert_row, 1 );
	connect( button, SIGNAL( pressed(uint) ), this, SLOT( changeVideoControlValue(uint) ) );
	ctrlWidgets.push_back(label);
	ctrlWidgets.push_back(button);
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

void AVDeviceConfig::slotSettingsChanged(bool)
{
	emit changed(true);
}

void AVDeviceConfig::slotValueChanged(int)
{
	emit changed( true );
}

void AVDeviceConfig::slotDeviceKComboBoxChanged(int)
{
	kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) called. ";
	int newdevice = mPrfsVideoDevice->mDeviceKComboBox->currentIndex();
	kDebug() << "kopete:config (avdevice): slotDeviceKComboBoxChanged(int) Current device: " << mVideoDevicePool->currentDevice() << "New device: " << newdevice;
	if ((newdevice >= 0 && newdevice < mVideoDevicePool->size()) && (newdevice != mVideoDevicePool->currentDevice()))
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

void AVDeviceConfig::slotInputKComboBoxChanged(int)
{
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

void AVDeviceConfig::slotStandardKComboBoxChanged(int)
{
	emit changed( true );
}

void AVDeviceConfig::changeVideoControlValue(unsigned int id, int value)
{
	mVideoDevicePool->setControlValue(id, value);
	emit changed( true );
	/* TODO: Check success, fallback */
}

void AVDeviceConfig::slotUpdateImage()
{
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&qimage);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(QPixmap::fromImage(qimage));
	//kDebug() << "kopete (avdeviceconfig_videoconfig): Image updated.";
}

void AVDeviceConfig::deviceRegistered( const QString & udi )
{
	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
	if (mVideoDevicePool->size() < 2) // otherwise we are already capturing
		startCapturing();
}

void AVDeviceConfig::deviceUnregistered( const QString & udi )
{
	if (capturingDevice_udi == udi)
	{
		qtimer.stop();
		mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
		capturingDevice_udi.clear();
		mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
		mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
		clearControlGUIElements();
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
		setupControls();
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
