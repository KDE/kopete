/*
    avdeviceconfig.h  -  Kopete Video Device Configuration Panel

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

#ifndef AVDEVICECONFIG_H
#define AVDEVICECONFIG_H

#include <kcmodule.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtimer.h>

#include "avdevice/videodevicepool.h"
#include "ui_avdeviceconfig_videodevice.h"

class QTabWidget;

//class Ui_AVDeviceConfig_VideoDevice;
//class Ui_AVDeviceConfig_AudioDevice;

/**
@author Cláudio da Silveira Pinheiro
*/
class AVDeviceConfig : public KCModule
{
Q_OBJECT
public:
	AVDeviceConfig(QWidget *parent, const QVariantList &args);
	~AVDeviceConfig();
	virtual void save();
	virtual void load();

private slots:
	void slotSettingsChanged(bool);
	void slotValueChanged(int);
	void slotDeviceKComboBoxChanged(int);
	void slotInputKComboBoxChanged(int);
	void slotStandardKComboBoxChanged(int);
	void slotUpdateImage();
	void changeVideoControlValue(unsigned int id, int value = 0);
	void resetControls();
	void deviceRegistered( const QString & );
	void deviceUnregistered( const QString & );
	void updateVideoDevicePool();

private:
	class VideoControlValue
	{
	public:
		VideoControlValue() : id(0), value(0) {};
		VideoControlValue(quint32 id, qint32 value) : id(id), value(value) {};
		quint32 id;
		qint32 value;
	};

//	QTabWidget* mAVDeviceTabCtl;
	Ui_AVDeviceConfig_VideoDevice  *mPrfsVideoDevice;
//	AVDeviceConfig_AudioDevice  *mPrfsAudioDevice;
	Kopete::AV::VideoDevicePool *mVideoDevicePool;
	QImage qimage;
	QPixmap qpixmap;
	QTimer qtimer;
	QString capturingDevice_udi;
	QList<QWidget*> ctrlWidgets;
	QList<VideoControlValue> ctrl_values_bak;

	void setupControls();
	void clearControlGUIElements();
	void addSliderControlElement(int cid, QString title, int min, int max, int step, int value);
	void addCheckBoxControlElement(int cid, QString title, bool value);
	void addPopupMenuControlElement(int cid, QString title, QStringList options, int menuindex);
	void addButtonControlElement(int cid, QString title);
	void startCapturing();
	void stopCapturing();

protected:
	virtual void showEvent(QShowEvent *event);
	virtual void hideEvent(QHideEvent *event);
};

#endif
