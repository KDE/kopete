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

class MotionAwayPreferences;
class QTimer;

/**
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 */

class MotionAwayPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	MotionAwayPlugin( QObject *parent, const char *name, const QStringList &args );
	~MotionAwayPlugin();
	virtual void init();
	virtual bool unload();

public slots:
	void slotTimeout();
	void slotCapture();
	void slotActivity();
	void slotSettingsChanged();
private:
	MotionAwayPreferences *mPrefs;
	QTimer *m_captureTimer;
	QTimer *m_awayTimer;

	int getImage(int, char *,int ,int ,int ,int ,int );

	bool m_tookFirst;
	bool m_wentAway;

	int width;
	int height;
	int quality;
	int max_changes;
	int dev;
	int shots;
	int sms;
	int mail;
	int gap;
	char *image_ref;
	char *image_new;
	char *image_old;
	char *image_out;

	time_t currenttimep;
	time_t lasttime;
	struct tm *currenttime;
	char *device;
	char *mail_address;
	char *sms_nr;
	
	char file[255];
	char filepath[255];
	char c;
	FILE *picture;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

