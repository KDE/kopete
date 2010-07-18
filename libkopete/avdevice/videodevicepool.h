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
#include <kcombobox.h>
#include "videodevice.h"
#include "kopete_export.h"
#include <kconfig.h>
#include <kglobal.h>
#include <solid/device.h>


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
	~VideoDevicePool();
	bool isOpen();
	int open( int device = -1 );
	int close();

	int size();
	int currentDevice();
	QString currentDeviceUdi();
	int inputs();
	int currentInput();
	int selectInput( int newinput );

	QList<NumericVideoControl> getSupportedNumericControls();
	QList<BooleanVideoControl> getSupportedBooleanControls();
	QList<MenuVideoControl> getSupportedMenuControls();
	QList<ActionVideoControl> getSupportedActionControls();
	int getControlValue( quint32 ctrl_id, qint32 * value );
	int setControlValue( quint32 ctrl_id, qint32 value );

	int startCapturing();
	int stopCapturing();
	int getFrame();
	int getImage( QImage *qimage );

	int width();
	int minWidth();
	int maxWidth();
	int height();
	int minHeight();
	int maxHeight();
	int setImageSize( int newwidth, int newheight );

	void fillDeviceKComboBox( KComboBox *combobox );
	void fillInputKComboBox( KComboBox *combobox );
	void fillStandardKComboBox( KComboBox *combobox );

	void saveCurrentDeviceConfig();

protected:
	int m_current_device;
	QVector<VideoDevice*> m_videodevices;	/*!< Vector of pointers to the available video devices */

	bool registerDevice( Solid::Device & dev );
	int showDeviceCapabilities( int device = -1 );
	int getSavedDevice();
	void loadDeviceConfig();

private:
	static VideoDevicePool* s_self;
	static __u64 m_clients; // Number of instances

	VideoDevicePool();

protected slots:
	/**
	 * Slot called when a new device is added to the system
	 */
	void deviceAdded( const QString & udi );
	void deviceRemoved( const QString & udi );

signals:
	/**
	 * Provisional signatures, probably more useful to indicate which device was registered
	 */
	void deviceRegistered( const QString & udi );
	void deviceUnregistered( const QString & udi );

};

}

}

#endif // VIDEODEVICEPOOL_H
