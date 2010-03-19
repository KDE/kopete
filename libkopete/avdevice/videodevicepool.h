/*
    videodevicepool.h  -  Kopete Multiple Video Device handler Class

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
    Copyright (c) 2010      by Frank Schaefer                 <fschaefer.oss@googlemail.com>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef VIDEODEVICEPOOL_H
#define VIDEODEVICEPOOL_H

#include <iostream>


#include "videoinput.h"
#include <qstring.h>
#include <qimage.h>
#include <q3valuevector.h>
#include <qmutex.h>
#include <kcombobox.h>
#include "videodevice.h"
#include "kopete_export.h"
#include <kconfig.h>
#include <kglobal.h>
#include <solid/device.h>

// Uncomment this line and comment out the include and using lines to 
// revert to standard vector if you hit issues. It should all be fine though.
// typedef QVector<Kopete::AV::VideoDevice> VideoDeviceVector;
#include "videodevicevector.h"
using Kopete::AV::VideoDeviceVector;

namespace Kopete {

namespace AV {

/**
This class allows kopete to check for the existence, open, configure, test, set parameters, grab frames from and close a given video capture card using the Video4Linux API.

@author Cláudio da Silveira Pinheiro
*/

class KOPETE_EXPORT VideoDevicePool : public QObject
{
Q_OBJECT
public:
	static VideoDevicePool* self();
	int open(int device = -1);
	bool isOpen();
	int getFrame();
	int width();
	int minWidth();
	int maxWidth();
	int height();
	int minHeight();
	int maxHeight();
	int setSize( int newwidth, int newheight);
	int close();
	int startCapturing();
	int stopCapturing();
	int readFrame();
	int getImage(QImage *qimage);
	int selectInput(int newinput);
	int scanDevices();
	void registerDevice( Solid::Device & dev );
	bool hasDevices();
	int size();
	~VideoDevicePool();
	int fillDeviceKComboBox(KComboBox *combobox);
	int fillInputKComboBox(KComboBox *combobox);
	int fillStandardKComboBox(KComboBox *combobox);
	QString currentDeviceUdi();
	int currentDevice();
	int currentInput();
	int inputs();

	QList<NumericVideoControl> getSupportedNumericControls();
	QList<BooleanVideoControl> getSupportedBooleanControls();
	QList<MenuVideoControl> getSupportedMenuControls();
	QList<ActionVideoControl> getSupportedActionControls();

	int getControlValue(quint32 ctrl_id, qint32 * value);
	int setControlValue(quint32 ctrl_id, qint32 value);

	void saveCurrentDeviceConfig();

signals:
	/**
	 * Provisional signatures, probably more useful to indicate which device was registered
	 */
	void deviceRegistered( const QString & udi );
	void deviceUnregistered( const QString & udi );

protected slots:
	/**
	 * Slot called when a new device is added to the system
	 */
	void deviceAdded( const QString & udi );
	void deviceRemoved( const QString & udi );

protected:
	int xioctl(int request, void *arg);
	int errnoReturn(const char* s);
	int showDeviceCapabilities(unsigned int device);
	void loadSelectedDevice();
	void loadDeviceConfig(); // Load configuration parameters;

	int m_current_device;
	VideoDeviceVector m_videodevice; // Vector to be filled with found devices
	struct imagebuffer m_buffer; // only used when no devices were found
	QMutex m_ready;

private:
	VideoDevicePool();
	static VideoDevicePool* s_self;
	static __u64 m_clients; // Number of instances

};

}

}

#endif // VIDEODEVICEPOOL_H
