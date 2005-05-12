/*
    avdeviceconfig.h  -  Kopete Video Device Configuration Panel

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
