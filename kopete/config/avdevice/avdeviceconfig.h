//
// C++ Interface: avdeviceconfig
//
// Description: 
//
//
// Author: Cláudio da Silveira Pinheiro <taupter@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef AVDEVICECONFIG_H
#define AVDEVICECONFIG_H

#include "kcmodule.h"

class QFrame;
class QTabWidget;

class AVDeviceConfig_VideoDevice;
class AVDeviceConfig_AudioDevice;

/**
@author Cláudio da Silveira Pinheiro
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

private:
    QTabWidget* mAVDeviceTabCtl;
    AVDeviceConfig_VideoDevice *mPrfsVideoDevice;
  AVDeviceConfig_AudioDevice *mPrfsAudioDevice;
};

#endif
