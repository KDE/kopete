/*
    motionawayplugin.h

    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

	Contains code from motion.c ( Detect changes in a video stream )
	Copyright 2000 by Jeroen Vreeken (pe1rxq@amsat.org)
	Distributed under the GNU public license version 2
	See also the file 'COPYING.motion'

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MOTIONAWAYPLUGIN_H
#define MOTIONAWAYPLUGIN_H

#include "kopeteplugin.h"

class QTimer;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 */

class MotionAwayPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	MotionAwayPlugin( QObject *parent, const char *name, const QStringList &args );
	~MotionAwayPlugin();

public slots:
	void loadSettings();
	void slotTimeout();
	void slotCapture();
	void slotActivity();

private:
	int awayTimeout;
	bool becomeAvailableWithActivity;
	QString videoDevice;
	
	QTimer *m_captureTimer;
	QTimer *m_awayTimer;

	int getImage(int, QByteArray& ,int ,int ,int ,int ,int );

	bool m_tookFirst;
	bool m_wentAway;

	int m_width;
	int m_height;

	int m_quality;
	int m_maxChanges;

	int m_deviceHandler;
	int shots;
	int m_gap;

	QByteArray m_imageRef;
	QByteArray m_imageNew;
	QByteArray m_imageOld;
	QByteArray m_imageOut;

    /*
	time_t currenttimep;
	time_t lasttime;
	struct tm *currenttime;
	
	char file[255];
	char filepath[255];
	char c;
	
    */
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

