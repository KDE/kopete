#include <kdialog.h>
#include <klocale.h>

/********************************************************************************
** Form generated from reading ui file 'avdeviceconfig_videodevice.ui'
**
** Created: Wed Aug 15 02:14:56 2007
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_AVDEVICECONFIG_VIDEODEVICE_H
#define UI_AVDEVICECONFIG_VIDEODEVICE_H

#include <Qt3Support/Q3ButtonGroup>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "kcombobox.h"

class Ui_AVDeviceConfig_VideoDevice
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout1;
    QHBoxLayout *hboxLayout;
    QLabel *mVideoImageLabel;
    QTabWidget *VideoTabWidget;
    QWidget *tab;
    QGridLayout *gridLayout2;
    Q3ButtonGroup *videoDeviceConfigurationGroup;
    QGridLayout *gridLayout3;
    QHBoxLayout *hboxLayout1;
    QVBoxLayout *vboxLayout;
    QLabel *deviceLabel;
    QLabel *inputLabel;
    QLabel *standardLabel;
    QVBoxLayout *vboxLayout1;
    KComboBox *mDeviceKComboBox;
    KComboBox *mInputKComboBox;
    KComboBox *mStandardKComboBox;
    QWidget *tab1;
    QGridLayout *gridLayout4;
    Q3ButtonGroup *imageAdjustmentGroup;
    QGridLayout *gridLayout5;
    QHBoxLayout *hboxLayout2;
    QVBoxLayout *vboxLayout2;
    QLabel *brightnessLabel;
    QLabel *contrastLabel;
    QLabel *SaturationLabel;
    QLabel *whitenessLabel;
    QLabel *hueLabel;
    QVBoxLayout *vboxLayout3;
    QSlider *mBrightnessSlider;
    QSlider *mContrastSlider;
    QSlider *mSaturationSlider;
    QSlider *mWhitenessSlider;
    QSlider *mHueSlider;
    QWidget *TabPage;
    QVBoxLayout *vboxLayout4;
    Q3ButtonGroup *imageOptionsGroup;
    QVBoxLayout *vboxLayout5;
    QCheckBox *mImageAutoBrightnessContrast;
    QCheckBox *mImageAutoColorCorrection;
    QCheckBox *mImageAsMirror;
    Q3ButtonGroup *deviceOptionsGroup;
    QVBoxLayout *vboxLayout6;
    QCheckBox *mDeviceDisableMMap;
    QCheckBox *mDeviceWorkaroundBrokenDriver;

    void setupUi(QWidget *AVDeviceConfig_VideoDevice)
    {
    if (AVDeviceConfig_VideoDevice->objectName().isEmpty())
        AVDeviceConfig_VideoDevice->setObjectName(QString::fromUtf8("AVDeviceConfig_VideoDevice"));
    QSize size(342, 520);
    size = size.expandedTo(AVDeviceConfig_VideoDevice->minimumSizeHint());
    AVDeviceConfig_VideoDevice->resize(size);
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(AVDeviceConfig_VideoDevice->sizePolicy().hasHeightForWidth());
    AVDeviceConfig_VideoDevice->setSizePolicy(sizePolicy);
    gridLayout = new QGridLayout(AVDeviceConfig_VideoDevice);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(11);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(6);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    mVideoImageLabel = new QLabel(AVDeviceConfig_VideoDevice);
    mVideoImageLabel->setObjectName(QString::fromUtf8("mVideoImageLabel"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(mVideoImageLabel->sizePolicy().hasHeightForWidth());
    mVideoImageLabel->setSizePolicy(sizePolicy1);
    mVideoImageLabel->setMinimumSize(QSize(320, 240));
    mVideoImageLabel->setPixmap(QPixmap(QString::fromUtf8("../../../../../../smpte2.png")));
    mVideoImageLabel->setScaledContents(true);
    mVideoImageLabel->setWordWrap(false);

    hboxLayout->addWidget(mVideoImageLabel);


    gridLayout1->addLayout(hboxLayout, 0, 0, 1, 1);

    VideoTabWidget = new QTabWidget(AVDeviceConfig_VideoDevice);
    VideoTabWidget->setObjectName(QString::fromUtf8("VideoTabWidget"));
    sizePolicy.setHeightForWidth(VideoTabWidget->sizePolicy().hasHeightForWidth());
    VideoTabWidget->setSizePolicy(sizePolicy);
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    gridLayout2 = new QGridLayout(tab);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(11);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    videoDeviceConfigurationGroup = new Q3ButtonGroup(tab);
    videoDeviceConfigurationGroup->setObjectName(QString::fromUtf8("videoDeviceConfigurationGroup"));
    videoDeviceConfigurationGroup->setColumnLayout(0, Qt::Vertical);
    videoDeviceConfigurationGroup->layout()->setSpacing(6);
    videoDeviceConfigurationGroup->layout()->setMargin(11);
    gridLayout3 = new QGridLayout();
    QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(videoDeviceConfigurationGroup->layout());
    if (boxlayout)
        boxlayout->addLayout(gridLayout3);
    gridLayout3->setAlignment(Qt::AlignTop);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    deviceLabel = new QLabel(videoDeviceConfigurationGroup);
    deviceLabel->setObjectName(QString::fromUtf8("deviceLabel"));
    deviceLabel->setWordWrap(false);

    vboxLayout->addWidget(deviceLabel);

    inputLabel = new QLabel(videoDeviceConfigurationGroup);
    inputLabel->setObjectName(QString::fromUtf8("inputLabel"));
    inputLabel->setWordWrap(false);

    vboxLayout->addWidget(inputLabel);

    standardLabel = new QLabel(videoDeviceConfigurationGroup);
    standardLabel->setObjectName(QString::fromUtf8("standardLabel"));
    standardLabel->setWordWrap(false);

    vboxLayout->addWidget(standardLabel);


    hboxLayout1->addLayout(vboxLayout);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    mDeviceKComboBox = new KComboBox(videoDeviceConfigurationGroup);
    mDeviceKComboBox->setObjectName(QString::fromUtf8("mDeviceKComboBox"));
    QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(mDeviceKComboBox->sizePolicy().hasHeightForWidth());
    mDeviceKComboBox->setSizePolicy(sizePolicy2);

    vboxLayout1->addWidget(mDeviceKComboBox);

    mInputKComboBox = new KComboBox(videoDeviceConfigurationGroup);
    mInputKComboBox->setObjectName(QString::fromUtf8("mInputKComboBox"));
    sizePolicy2.setHeightForWidth(mInputKComboBox->sizePolicy().hasHeightForWidth());
    mInputKComboBox->setSizePolicy(sizePolicy2);

    vboxLayout1->addWidget(mInputKComboBox);

    mStandardKComboBox = new KComboBox(videoDeviceConfigurationGroup);
    mStandardKComboBox->setObjectName(QString::fromUtf8("mStandardKComboBox"));
    sizePolicy2.setHeightForWidth(mStandardKComboBox->sizePolicy().hasHeightForWidth());
    mStandardKComboBox->setSizePolicy(sizePolicy2);

    vboxLayout1->addWidget(mStandardKComboBox);


    hboxLayout1->addLayout(vboxLayout1);


    gridLayout3->addLayout(hboxLayout1, 0, 0, 1, 1);


    gridLayout2->addWidget(videoDeviceConfigurationGroup, 0, 0, 1, 1);

    VideoTabWidget->addTab(tab, QString());
    tab1 = new QWidget();
    tab1->setObjectName(QString::fromUtf8("tab1"));
    gridLayout4 = new QGridLayout(tab1);
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(11);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    imageAdjustmentGroup = new Q3ButtonGroup(tab1);
    imageAdjustmentGroup->setObjectName(QString::fromUtf8("imageAdjustmentGroup"));
    imageAdjustmentGroup->setColumnLayout(0, Qt::Vertical);
    imageAdjustmentGroup->layout()->setSpacing(6);
    imageAdjustmentGroup->layout()->setMargin(11);
    gridLayout5 = new QGridLayout();
    QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(imageAdjustmentGroup->layout());
    if (boxlayout1)
        boxlayout1->addLayout(gridLayout5);
    gridLayout5->setAlignment(Qt::AlignTop);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    brightnessLabel = new QLabel(imageAdjustmentGroup);
    brightnessLabel->setObjectName(QString::fromUtf8("brightnessLabel"));
    brightnessLabel->setWordWrap(false);

    vboxLayout2->addWidget(brightnessLabel);

    contrastLabel = new QLabel(imageAdjustmentGroup);
    contrastLabel->setObjectName(QString::fromUtf8("contrastLabel"));
    contrastLabel->setWordWrap(false);

    vboxLayout2->addWidget(contrastLabel);

    SaturationLabel = new QLabel(imageAdjustmentGroup);
    SaturationLabel->setObjectName(QString::fromUtf8("SaturationLabel"));
    SaturationLabel->setWordWrap(false);

    vboxLayout2->addWidget(SaturationLabel);

    whitenessLabel = new QLabel(imageAdjustmentGroup);
    whitenessLabel->setObjectName(QString::fromUtf8("whitenessLabel"));
    whitenessLabel->setWordWrap(false);

    vboxLayout2->addWidget(whitenessLabel);

    hueLabel = new QLabel(imageAdjustmentGroup);
    hueLabel->setObjectName(QString::fromUtf8("hueLabel"));
    hueLabel->setWordWrap(false);

    vboxLayout2->addWidget(hueLabel);


    hboxLayout2->addLayout(vboxLayout2);

    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setSpacing(6);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    mBrightnessSlider = new QSlider(imageAdjustmentGroup);
    mBrightnessSlider->setObjectName(QString::fromUtf8("mBrightnessSlider"));
    sizePolicy2.setHeightForWidth(mBrightnessSlider->sizePolicy().hasHeightForWidth());
    mBrightnessSlider->setSizePolicy(sizePolicy2);
    mBrightnessSlider->setOrientation(Qt::Horizontal);

    vboxLayout3->addWidget(mBrightnessSlider);

    mContrastSlider = new QSlider(imageAdjustmentGroup);
    mContrastSlider->setObjectName(QString::fromUtf8("mContrastSlider"));
    sizePolicy2.setHeightForWidth(mContrastSlider->sizePolicy().hasHeightForWidth());
    mContrastSlider->setSizePolicy(sizePolicy2);
    mContrastSlider->setOrientation(Qt::Horizontal);

    vboxLayout3->addWidget(mContrastSlider);

    mSaturationSlider = new QSlider(imageAdjustmentGroup);
    mSaturationSlider->setObjectName(QString::fromUtf8("mSaturationSlider"));
    sizePolicy2.setHeightForWidth(mSaturationSlider->sizePolicy().hasHeightForWidth());
    mSaturationSlider->setSizePolicy(sizePolicy2);
    mSaturationSlider->setOrientation(Qt::Horizontal);

    vboxLayout3->addWidget(mSaturationSlider);

    mWhitenessSlider = new QSlider(imageAdjustmentGroup);
    mWhitenessSlider->setObjectName(QString::fromUtf8("mWhitenessSlider"));
    sizePolicy2.setHeightForWidth(mWhitenessSlider->sizePolicy().hasHeightForWidth());
    mWhitenessSlider->setSizePolicy(sizePolicy2);
    mWhitenessSlider->setOrientation(Qt::Horizontal);

    vboxLayout3->addWidget(mWhitenessSlider);

    mHueSlider = new QSlider(imageAdjustmentGroup);
    mHueSlider->setObjectName(QString::fromUtf8("mHueSlider"));
    sizePolicy2.setHeightForWidth(mHueSlider->sizePolicy().hasHeightForWidth());
    mHueSlider->setSizePolicy(sizePolicy2);
    mHueSlider->setOrientation(Qt::Horizontal);

    vboxLayout3->addWidget(mHueSlider);


    hboxLayout2->addLayout(vboxLayout3);


    gridLayout5->addLayout(hboxLayout2, 0, 0, 1, 1);


    gridLayout4->addWidget(imageAdjustmentGroup, 0, 0, 1, 1);

    VideoTabWidget->addTab(tab1, QString());
    TabPage = new QWidget();
    TabPage->setObjectName(QString::fromUtf8("TabPage"));
    vboxLayout4 = new QVBoxLayout(TabPage);
    vboxLayout4->setSpacing(6);
    vboxLayout4->setMargin(11);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    imageOptionsGroup = new Q3ButtonGroup(TabPage);
    imageOptionsGroup->setObjectName(QString::fromUtf8("imageOptionsGroup"));
    imageOptionsGroup->setColumnLayout(0, Qt::Vertical);
    imageOptionsGroup->layout()->setSpacing(6);
    imageOptionsGroup->layout()->setMargin(11);
    vboxLayout5 = new QVBoxLayout();
    QBoxLayout *boxlayout2 = qobject_cast<QBoxLayout *>(imageOptionsGroup->layout());
    if (boxlayout2)
        boxlayout2->addLayout(vboxLayout5);
    vboxLayout5->setAlignment(Qt::AlignTop);
    vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
    mImageAutoBrightnessContrast = new QCheckBox(imageOptionsGroup);
    mImageAutoBrightnessContrast->setObjectName(QString::fromUtf8("mImageAutoBrightnessContrast"));
    QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(mImageAutoBrightnessContrast->sizePolicy().hasHeightForWidth());
    mImageAutoBrightnessContrast->setSizePolicy(sizePolicy3);

    vboxLayout5->addWidget(mImageAutoBrightnessContrast);

    mImageAutoColorCorrection = new QCheckBox(imageOptionsGroup);
    mImageAutoColorCorrection->setObjectName(QString::fromUtf8("mImageAutoColorCorrection"));
    sizePolicy3.setHeightForWidth(mImageAutoColorCorrection->sizePolicy().hasHeightForWidth());
    mImageAutoColorCorrection->setSizePolicy(sizePolicy3);

    vboxLayout5->addWidget(mImageAutoColorCorrection);

    mImageAsMirror = new QCheckBox(imageOptionsGroup);
    mImageAsMirror->setObjectName(QString::fromUtf8("mImageAsMirror"));
    sizePolicy3.setHeightForWidth(mImageAsMirror->sizePolicy().hasHeightForWidth());
    mImageAsMirror->setSizePolicy(sizePolicy3);

    vboxLayout5->addWidget(mImageAsMirror);


    vboxLayout4->addWidget(imageOptionsGroup);

    deviceOptionsGroup = new Q3ButtonGroup(TabPage);
    deviceOptionsGroup->setObjectName(QString::fromUtf8("deviceOptionsGroup"));
    deviceOptionsGroup->setColumnLayout(0, Qt::Vertical);
    deviceOptionsGroup->layout()->setSpacing(6);
    deviceOptionsGroup->layout()->setMargin(11);
    vboxLayout6 = new QVBoxLayout();
    QBoxLayout *boxlayout3 = qobject_cast<QBoxLayout *>(deviceOptionsGroup->layout());
    if (boxlayout3)
        boxlayout3->addLayout(vboxLayout6);
    vboxLayout6->setAlignment(Qt::AlignTop);
    vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
    mDeviceDisableMMap = new QCheckBox(deviceOptionsGroup);
    mDeviceDisableMMap->setObjectName(QString::fromUtf8("mDeviceDisableMMap"));
    sizePolicy3.setHeightForWidth(mDeviceDisableMMap->sizePolicy().hasHeightForWidth());
    mDeviceDisableMMap->setSizePolicy(sizePolicy3);

    vboxLayout6->addWidget(mDeviceDisableMMap);

    mDeviceWorkaroundBrokenDriver = new QCheckBox(deviceOptionsGroup);
    mDeviceWorkaroundBrokenDriver->setObjectName(QString::fromUtf8("mDeviceWorkaroundBrokenDriver"));
    sizePolicy3.setHeightForWidth(mDeviceWorkaroundBrokenDriver->sizePolicy().hasHeightForWidth());
    mDeviceWorkaroundBrokenDriver->setSizePolicy(sizePolicy3);

    vboxLayout6->addWidget(mDeviceWorkaroundBrokenDriver);


    vboxLayout4->addWidget(deviceOptionsGroup);

    VideoTabWidget->addTab(TabPage, QString());

    gridLayout1->addWidget(VideoTabWidget, 1, 0, 1, 1);


    gridLayout->addLayout(gridLayout1, 0, 0, 1, 1);


    retranslateUi(AVDeviceConfig_VideoDevice);

    VideoTabWidget->setCurrentIndex(0);


    QMetaObject::connectSlotsByName(AVDeviceConfig_VideoDevice);
    } // setupUi

    void retranslateUi(QWidget *AVDeviceConfig_VideoDevice)
    {
    AVDeviceConfig_VideoDevice->setWindowTitle(tr2i18n("Video", 0));
    videoDeviceConfigurationGroup->setTitle(tr2i18n("Video Device Configuration", 0));
    deviceLabel->setText(tr2i18n("Device:", 0));
    inputLabel->setText(tr2i18n("Input:", 0));
    standardLabel->setText(tr2i18n("Standard:", 0));
    VideoTabWidget->setTabText(VideoTabWidget->indexOf(tab), tr2i18n("Device", 0));
    imageAdjustmentGroup->setTitle(tr2i18n("Image Adjustment", 0));
    brightnessLabel->setText(tr2i18n("Brightness:", 0));
    contrastLabel->setText(tr2i18n("Contrast:", 0));
    SaturationLabel->setText(tr2i18n("Saturation:", 0));
    whitenessLabel->setText(tr2i18n("Whiteness:", 0));
    hueLabel->setText(tr2i18n("Hue:", 0));
    VideoTabWidget->setTabText(VideoTabWidget->indexOf(tab1), tr2i18n("Con&trols", 0));
    imageOptionsGroup->setTitle(tr2i18n("Image options", 0));
    mImageAutoBrightnessContrast->setText(tr2i18n("Au&tomatic brightness/contrast adjustment", 0));
    mImageAutoBrightnessContrast->setShortcut(tr2i18n("Alt+T", 0));
    mImageAutoColorCorrection->setText(tr2i18n("Automatic color correction", 0));
    mImageAutoColorCorrection->setShortcut(QString());
    mImageAsMirror->setText(tr2i18n("See preview mirrored", 0));
    mImageAsMirror->setShortcut(QString());
    deviceOptionsGroup->setTitle(tr2i18n("Device options", 0));
    mDeviceDisableMMap->setText(tr2i18n("Disable memor&y mapping", 0));
    mDeviceDisableMMap->setShortcut(tr2i18n("Alt+Y", 0));
    mDeviceWorkaroundBrokenDriver->setText(tr2i18n("&Workaround broken driver", 0));
    mDeviceWorkaroundBrokenDriver->setShortcut(tr2i18n("Alt+W", 0));
    VideoTabWidget->setTabText(VideoTabWidget->indexOf(TabPage), tr2i18n("Optio&ns", 0));
    Q_UNUSED(AVDeviceConfig_VideoDevice);
    } // retranslateUi

};

namespace Ui {
    class AVDeviceConfig_VideoDevice: public Ui_AVDeviceConfig_VideoDevice {};
} // namespace Ui

#endif // AVDEVICECONFIG_VIDEODEVICE_H

