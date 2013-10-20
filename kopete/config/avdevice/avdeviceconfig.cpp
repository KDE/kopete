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
#include <QVBoxLayout>
#include <QShowEvent>
#include <QHideEvent>

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

	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, false);
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, false);
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(3, false);

	// set a default image for the webcam widget, in case the user does not have a video device
	mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));

	mVideoDevicePool = NULL;
}


AVDeviceConfig::~AVDeviceConfig()
{
	if (mVideoDevicePool)
	{
		for (int k=0; k<ctrl_values_bak.size(); k++)
			mVideoDevicePool->setControlValue(ctrl_values_bak.at(k).id, ctrl_values_bak.at(k).value);
		mVideoDevicePool->close();
	}
	clearControlGUIElements();
	delete mPrfsVideoDevice;
}


void AVDeviceConfig::updateVideoDevicePool()
{
	bool visible = isVisible();

	if ((mVideoDevicePool && visible) || (!mVideoDevicePool && !visible))
		return;

	if (visible)
	{
		connect(mPrfsVideoDevice->mDeviceKComboBox,              SIGNAL(activated(int)),    this, SLOT(slotDeviceKComboBoxChanged(int)));
		connect(mPrfsVideoDevice->mInputKComboBox,               SIGNAL(activated(int)),    this, SLOT(slotInputKComboBoxChanged(int)));
		connect(mPrfsVideoDevice->mStandardKComboBox,            SIGNAL(activated(int)),    this, SLOT(slotStandardKComboBoxChanged(int)));

		mVideoDevicePool = Kopete::AV::VideoDevicePool::self();

		if (EXIT_SUCCESS == mVideoDevicePool->open())
		{
			setupControls();
			startCapturing();
		}

		mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
		mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);

		connect(mVideoDevicePool, SIGNAL(deviceRegistered(QString)), this,
				SLOT(deviceRegistered(QString)) );
		connect(mVideoDevicePool, SIGNAL(deviceUnregistered(QString)), this,
				SLOT(deviceUnregistered(QString)) );

		connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	}
	else
	{
		for (int k=0; k<ctrl_values_bak.size(); k++)
			mVideoDevicePool->setControlValue(ctrl_values_bak.at(k).id, ctrl_values_bak.at(k).value);

		disconnect(mVideoDevicePool, SIGNAL(deviceRegistered(QString)), this,
				SLOT(deviceRegistered(QString)) );
		disconnect(mVideoDevicePool, SIGNAL(deviceUnregistered(QString)), this,
				SLOT(deviceUnregistered(QString)) );

		disconnect(mPrfsVideoDevice->mDeviceKComboBox,              SIGNAL(activated(int)),    this, SLOT(slotDeviceKComboBoxChanged(int)));
		disconnect(mPrfsVideoDevice->mInputKComboBox,               SIGNAL(activated(int)),    this, SLOT(slotInputKComboBoxChanged(int)));
		disconnect(mPrfsVideoDevice->mStandardKComboBox,            SIGNAL(activated(int)),    this, SLOT(slotStandardKComboBoxChanged(int)));

		disconnect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );

		stopCapturing();
		mVideoDevicePool->close();

		mVideoDevicePool = NULL;

		// set a default image for the webcam widget, in case the user does not have a video device
		mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
	}
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
		ctrl_values_bak.push_back(VideoControlValue(numericCtrls.at(k).id, cval));
	}
	// Boolean Controls: => Checkbox
	for (k=0; k<booleanCtrls.size(); k++)
	{
		mVideoDevicePool->getControlValue(booleanCtrls.at(k).id, &cval);
		addCheckBoxControlElement(booleanCtrls.at(k).id, booleanCtrls.at(k).name, cval);
		ctrl_values_bak.push_back(VideoControlValue(booleanCtrls.at(k).id, cval));
	}
	// Menu Controls: => Combobox
	for (k=0; k<menuCtrls.size(); k++)
	{
		mVideoDevicePool->getControlValue(menuCtrls.at(k).id, &cval);
		addPopupMenuControlElement(menuCtrls.at(k).id, menuCtrls.at(k).name, menuCtrls.at(k).options, cval);
		ctrl_values_bak.push_back(VideoControlValue(booleanCtrls.at(k).id, cval));
	}
	// Action Controls: => Button
	for (k=0; k<actionCtrls.size(); k++)
		addButtonControlElement(actionCtrls.at(k).id, actionCtrls.at(k).name);
	/* TODO: check success of mVideoDevicePool->getControlValue() */

	// Button for resetting the control values:
	if (numericCtrls.size() || booleanCtrls.size() || menuCtrls.size())
	{
		int insert_row = mPrfsVideoDevice->actions_gridLayout->rowCount();
		QLabel *label = new QLabel( i18n("Reset sliders & options to default values") + ":", mPrfsVideoDevice->VideoTabWidget ); // "Reset sliders and options"
		mPrfsVideoDevice->actions_gridLayout->addWidget( label, insert_row, 0 );
		KPushButton *button = new KPushButton( mPrfsVideoDevice->VideoTabWidget );
		button->setText( i18n("Execute") );
		mPrfsVideoDevice->actions_gridLayout->addWidget( button, insert_row, 1 );
		connect( button, SIGNAL(pressed()), this, SLOT(resetControls()) );
		ctrlWidgets.push_back(label);
		ctrlWidgets.push_back(button);
	}

	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(1, numericCtrls.size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(2, booleanCtrls.size() + menuCtrls.size());
	mPrfsVideoDevice->VideoTabWidget->setTabEnabled(3, numericCtrls.size() || booleanCtrls.size() || menuCtrls.size() || actionCtrls.size());
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
	connect( slider, SIGNAL(valueChanged(uint,int)), this, SLOT(changeVideoControlValue(uint,int)) );
	ctrlWidgets.push_back(label);
	ctrlWidgets.push_back(slider);
}


void AVDeviceConfig::addCheckBoxControlElement(int cid, QString title, bool value)
{
	IdCheckBox *checkbox = new IdCheckBox( cid, mPrfsVideoDevice->VideoTabWidget );
	checkbox->setText( title );
	mPrfsVideoDevice->checkboxOptions_verticalLayout->addWidget( checkbox );
	checkbox->setChecked( value );
	connect( checkbox, SIGNAL(stateChanged(uint,int)), this, SLOT(changeVideoControlValue(uint,int)) );
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
	connect( combobox, SIGNAL(currentIndexChanged(uint,int)), this, SLOT(changeVideoControlValue(uint,int)) );
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
	connect( button, SIGNAL(pressed(uint)), this, SLOT(changeVideoControlValue(uint)) );
	ctrlWidgets.push_back(label);
	ctrlWidgets.push_back(button);
}



/*!
    \fn VideoDeviceConfig::save()
 */
void AVDeviceConfig::save()
{
	mVideoDevicePool->saveCurrentDeviceConfig();
	ctrl_values_bak.clear();
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
		mVideoDevicePool->close();
		mVideoDevicePool->open(newdevice);
		/* NOTE: input and signal standard are set automatically */
		mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
		setupControls();
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
		/* FIXME: select signal standard ! */
		mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
		setupControls();
		startCapturing();
		emit changed( true );
	}
}

void AVDeviceConfig::slotStandardKComboBoxChanged(int)
{
	/* FIXME: select signal standard ! */
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
	if (EXIT_SUCCESS == mVideoDevicePool->getFrame())
	{
		mVideoDevicePool->getImage(&qimage);
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(QPixmap::fromImage(qimage));
		//kDebug() << "kopete (avdeviceconfig_videoconfig): Image updated.";
	}
}

void AVDeviceConfig::deviceRegistered( const QString & udi )
{
	(void) udi; /* avoid compiler warning about unused parameter */
	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
	if (mVideoDevicePool->size() < 2) // otherwise we are already capturing
	{
		if (EXIT_SUCCESS == mVideoDevicePool->open())
		{
			setupControls();
			startCapturing();
		}
	}
}

void AVDeviceConfig::deviceUnregistered( const QString & udi )
{
	mVideoDevicePool->fillDeviceKComboBox(mPrfsVideoDevice->mDeviceKComboBox);
	mVideoDevicePool->fillInputKComboBox(mPrfsVideoDevice->mInputKComboBox);
	mVideoDevicePool->fillStandardKComboBox(mPrfsVideoDevice->mStandardKComboBox);
	if (capturingDevice_udi == udi)
	{
		qtimer.stop();
		mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
		mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
		capturingDevice_udi.clear();
		clearControlGUIElements();
		if (mVideoDevicePool->size())
		{
			if (EXIT_SUCCESS == mVideoDevicePool->open())
			{
				setupControls();
				startCapturing();
			}
		}
	}
}

void AVDeviceConfig::startCapturing()
{
	mVideoDevicePool->setImageSize(320, 240);
	mVideoDevicePool->startCapturing();
	capturingDevice_udi = mVideoDevicePool->currentDeviceUdi();
	qtimer.start(40);
	mPrfsVideoDevice->mVideoImageLabel->setScaledContents(true);
}

void AVDeviceConfig::stopCapturing()
{
	qtimer.stop();
	mVideoDevicePool->stopCapturing();
	mPrfsVideoDevice->mVideoImageLabel->setScaledContents(false);
	mPrfsVideoDevice->mVideoImageLabel->setPixmap(KIcon("camera-web").pixmap(128,128));
	capturingDevice_udi.clear();
}

void AVDeviceConfig::resetControls()
{
	int k = 0;
	// Numeric controls:
	QList<Kopete::AV::NumericVideoControl> numericCtrls;
	numericCtrls = mVideoDevicePool->getSupportedNumericControls();
	for (k=0; k<numericCtrls.size(); k++)
		mVideoDevicePool->setControlValue(numericCtrls.at(k).id, numericCtrls.at(k).value_default);
	// Boolean controls:
	QList<Kopete::AV::BooleanVideoControl> booleanCtrls;
	booleanCtrls = mVideoDevicePool->getSupportedBooleanControls();
	for (k=0; k<booleanCtrls.size(); k++)
		mVideoDevicePool->setControlValue(booleanCtrls.at(k).id, booleanCtrls.at(k).value_default);
	// Menu controls:
	QList<Kopete::AV::MenuVideoControl> menuCtrls;
	menuCtrls = mVideoDevicePool->getSupportedMenuControls();
	for (k=0; k<menuCtrls.size(); k++)
		mVideoDevicePool->setControlValue(menuCtrls.at(k).id, menuCtrls.at(k).index_default);
	// NOTE: action video controls can not be reset
	emit changed( true );
	// Adjust GUI-elements:
	setupControls();
	if (ctrlWidgets.size())
		mPrfsVideoDevice->VideoTabWidget->setCurrentIndex(3);
	// NOTE: TO BE IMPROVED
}

void AVDeviceConfig::showEvent(QShowEvent *event)
{
	// wait 1s so duplicate show/hide events will be skipped
	QTimer::singleShot(1000, this, SLOT(updateVideoDevicePool()));
	KCModule::showEvent(event);
}

void AVDeviceConfig::hideEvent(QHideEvent *event)
{
	// wait 1s so duplicate show/hide events will be skipped
	QTimer::singleShot(1000, this, SLOT(updateVideoDevicePool()));
	KCModule::hideEvent(event);
}
