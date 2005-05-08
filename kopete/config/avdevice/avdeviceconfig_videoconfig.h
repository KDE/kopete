/****************************************************************************
** Form interface generated from reading ui file './avdeviceconfig_videoconfig.ui'
**
** Created: Dom Mai 8 07:26:43 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef AVDEVICECONFIG_VIDEODEVICE_H
#define AVDEVICECONFIG_VIDEODEVICE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QButtonGroup;
class KComboBox;
class QSlider;
class QCheckBox;

class AVDeviceConfig_VideoDevice : public QWidget
{
    Q_OBJECT

public:
    AVDeviceConfig_VideoDevice( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~AVDeviceConfig_VideoDevice();

    QLabel* imagePixMap;
    QButtonGroup* buttonGroup7;
    QLabel* deviceLabel;
    QLabel* inputLabel;
    QLabel* standardLabel;
    KComboBox* devicekComboBox;
    KComboBox* inputkComboBox;
    KComboBox* normkComboBox;
    QButtonGroup* buttonGroup12;
    QLabel* brightLabel;
    QLabel* contrastLabel;
    QLabel* saturationLabel;
    QLabel* hueLabel;
    QSlider* brightSlider;
    QSlider* contrastSlider;
    QSlider* saturationSlider;
    QSlider* hueSlider;
    QCheckBox* imageAutoAdjustBrightContrast;

protected:
    QGridLayout* AVDeviceConfig_VideoDeviceLayout;
    QVBoxLayout* layout21;
    QHBoxLayout* layout19;
    QSpacerItem* imageLeftSpacer;
    QSpacerItem* imageRightSpacer;
    QGridLayout* buttonGroup7Layout;
    QHBoxLayout* videodevice_selection_layout;
    QVBoxLayout* videodevice_selection_labels;
    QVBoxLayout* videodevice_selection_combos;
    QGridLayout* buttonGroup12Layout;
    QVBoxLayout* layout25;
    QHBoxLayout* layout24;
    QVBoxLayout* layout22;
    QVBoxLayout* layout23;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // AVDEVICECONFIG_VIDEODEVICE_H
