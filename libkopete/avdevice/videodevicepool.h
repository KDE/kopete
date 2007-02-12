/*
    videodevicepool.h  -  Kopete Multiple Video Device handler Class

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

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

#ifndef KOPETE_AVVIDEODEVICE_H
#define KOPETE_AVVIDEODEVICE_H

#include <qvaluevector.h>
#include <iostream>


#include "videoinput.h"
#include "videodevicemodelpool.h"
#include <qstring.h>
#include <qimage.h>
#include <qvaluevector.h>
#include <qmutex.h>
#include <kcombobox.h>
#include "videodevice.h"
#include "kopete_export.h"
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

namespace Kopete {

namespace AV {

/**
This class allows kopete to check for the existence, open, configure, test, set parameters, grab frames from and close a given video capture card using the Video4Linux API.

@author Cláudio da Silveira Pinheiro
*/

typedef QValueList<Kopete::AV::VideoDevice *> VideoDeviceList;

class VideoDevicePoolPrivate;

class KOPETE_EXPORT VideoDevicePool
{
public:
	static VideoDevicePool* self();
	/**
	 * @return the current VideoDevice or 0 if there is none
	 */
	VideoDevice * current();
	int open();
	int open(unsigned int device);
	int setSize( int newwidth, int newheight);
	int close();
	int startCapturing();
	int stopCapturing();
	/**
	 * Gets a single frame from the current device, or a dummy frame if there is 
	 * no device. TODO what happens to the got frame?
	 * @return EXIT_SUCCESS or EXIT_FAILURE
	 */
	int getFrame();
	int readFrame();
	int getImage(QImage *qimage);
	int getPreviewImage(QImage *qimage);
	int selectInput(int newinput);
	int setInputParameters();
	int scanDevices();
	bool hasDevices();
	size_t size();
	~VideoDevicePool();
	VideoDeviceList m_videodevices; // Vector to be filled with found devices
	VideoDeviceModelPool m_modelvector;  // Vector to be filled with unique device models
	int fillDeviceKComboBox(KComboBox *combobox);
	int fillInputKComboBox(KComboBox *combobox);
	int fillStandardKComboBox(KComboBox *combobox);
	unsigned int currentDeviceIndex();
	int currentInput();
	unsigned int inputs();

	float getBrightness();
	float setBrightness(float brightness);
	float getContrast();
	float setContrast(float contrast);
	float getSaturation();
	float setSaturation(float saturation);
	float getWhiteness();
	float setWhiteness(float whiteness);
	float getHue();
	float setHue(float hue);

	bool getAutoBrightnessContrast();
	bool setAutoBrightnessContrast(bool brightnesscontrast);
	bool getAutoColorCorrection();
	bool setAutoColorCorrection(bool colorcorrection);
	bool getImageAsMirror();
	bool setImageAsMirror(bool imageasmirror);

	bool getDisableMMap();
	bool setDisableMMap(bool disablemmap);
	bool getWorkaroundBrokenDriver();
	bool setWorkaroundBrokenDriver(bool workaroundbrokendriver);

	void loadConfig(); // Load configuration parameters;
	void saveConfig(); // Save configuretion parameters;

protected:
	/**
	 * Get a list of video devices in the given dir
	 * @param dirPath the directory to look in
	 * @param filter the directory filter, eg 'video*'
	 * @return list of absolute paths
	 */
	QStringList videoDevicePaths( const QString & dirPath, const QString & filter );
	int xioctl(int request, void *arg);
	int errnoReturn(const char* s);
	int showDeviceCapabilities(unsigned int device);
	void guessDriver();
	unsigned int m_current_device;
	struct imagebuffer m_buffer; // only used when no devices were found

	QMutex m_ready;
private:
	VideoDevicePool();
	static VideoDevicePool* s_self;
	static __u64 m_clients; // Number of instances
};

}

}

#endif
