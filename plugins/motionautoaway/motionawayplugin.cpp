/*
    motionawayplugin.cpp

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

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include <qtimer.h>

#include "motionawayplugin.h"
#include "motionawaypreferences.h"
#include "kopete.h"
#include "kopeteaway.h"

// motion.c includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>

#define DEF_WIDTH	352
#define DEF_HEIGHT	288
#define DEF_QUALITY	50
#define DEF_CHANGES	5000

#define DEF_GAP		60*5 /* 5 minutes */

#define NORM_DEFAULT	0
#define IN_DEFAULT	8
#define VIDEO_DEVICE		"/dev/video0"

int deamon=0;


K_EXPORT_COMPONENT_FACTORY( kopete_motionaway, KGenericFactory<MotionAwayPlugin> );

MotionAwayPlugin::MotionAwayPlugin( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopetePlugin( parent, name )
{
    m_tookFirst = false;

	width=DEF_WIDTH;
	height=DEF_HEIGHT;
	quality=DEF_QUALITY;
	max_changes=DEF_CHANGES;
	shots=0;
	sms=0;
	mail=0;
	gap=DEF_GAP;
	lasttime=0;
	device=VIDEO_DEVICE;
	mail_address=NULL;
	sms_nr=NULL;

	m_captureTimer = new QTimer(this);
	m_awayTimer = new QTimer(this);

	mPrefs = new MotionAwayPreferences ( "camera_umount", this );
	connect( mPrefs, SIGNAL(saved()), this, SLOT(slotSettingsChanged()) );
	connect( m_captureTimer, SIGNAL(timeout()), this, SLOT(slotCapture()) );
    connect( m_awayTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()) );

	filepath[0]=0;
	signal(SIGCHLD, SIG_IGN);

	image_ref=malloc(width*height*3);
	image_new=malloc(width*height*3);
	image_out=malloc(width*height*3);
	image_old=malloc(width*height*3);

	kdDebug() << "[MotionAway Plugin] : Opening Video4Linux Device" << endl;
	dev = open(device, O_RDWR);
	if (dev < 0)
	{
		kdDebug() << "[MotionAway Plugin] : Can't open Video4Linux Device" << endl;
	}
	else
	{
        kdDebug() << "[MotionAway Plugin] : Worked! Setting Capture timers!" << endl;
		/* Capture first image, or we will get a alarm on start */
		this->getImage (this->dev, this->image_ref, DEF_WIDTH, DEF_HEIGHT, IN_DEFAULT, NORM_DEFAULT,
	    	VIDEO_PALETTE_RGB24);

        /* We have the first image now */
		m_tookFirst = true;
     
		m_captureTimer->start( 1000 );
		m_awayTimer->start( mPrefs->awayTimeout() * 60 * 1000 );
	}
}

MotionAwayPlugin::~MotionAwayPlugin()
{
    kdDebug() << "[MotionAway Plugin] : Closing Video4Linux Device" << endl;
	close (dev);
	kdDebug() << "[MotionAway Plugin] : Freeing memory" << endl;
	free(image_ref);
	free(image_new);
	free(image_out);
	free(image_old);
}

void MotionAwayPlugin::init()
{
}

bool MotionAwayPlugin::unload()
{
	return true;
}

int MotionAwayPlugin::getImage(int _dev, char *_image, int _width, int _height, int _input, int _norm,  int _fmt)
{
	struct video_capability vid_caps;
	struct video_channel vid_chnl;
	struct video_window vid_win;

	if (ioctl (_dev, VIDIOCGCAP, &vid_caps) == -1)
	{
		perror ("ioctl (VIDIOCGCAP)");
		return (-1);
	}
	/* Set channels and norms, NOT TESTED my philips cam doesn't have a
	 * tuner. */
	if (_input != IN_DEFAULT)
	{
		vid_chnl.channel = -1;
		if (ioctl (_dev, VIDIOCGCHAN, &vid_chnl) == -1)
		{
			perror ("ioctl (VIDIOCGCHAN)");
		}
		else
		{
			vid_chnl.channel = _input;
			vid_chnl.norm    = _norm;
	
			if (ioctl (_dev, VIDIOCSCHAN, &vid_chnl) == -1)
			{
				perror ("ioctl (VIDIOCSCHAN)");
				return (-1);
			}
		}
	}
	/* Set image size */
	if (ioctl (_dev, VIDIOCGWIN, &vid_win) == -1)
		return (-1);

	vid_win.width=_width;
	vid_win.height=_height;

	if (ioctl (_dev, VIDIOCSWIN, &vid_win) == -1)
		return (-1);

	/* Read an image */
	return read (_dev, _image, _width * _height * 3);
}

void MotionAwayPlugin::slotCapture()
{
    int i, diffs;

	/* Should go on forever... emphasis on 'should' */
	if (getImage (this->dev, this->image_new, this->width, this->height, IN_DEFAULT, NORM_DEFAULT,
	    VIDEO_PALETTE_RGB24) == width*height*3)
	{
        if ( m_tookFirst )
		{
			/* Make a differences picture in image_out */
			diffs=0;
			for (i=0; i<width*height*3; i++)
			{
				image_out[i]=image_old[i]-image_new[i];
				if ((signed char)image_out[i] > 32 || (signed char)image_out[i] < -32)
				{
					image_old[i]=image_new[i];
					diffs++;
				}
				else
				{
					image_out[i]=0;
				}
			}
		}
		else
		{
			/* First picture: new image is now the old */
			for (i=0; i<width*height*3; i++)
				image_old[i]=image_new[i];		
		}

		/* The cat just walked in :) */
		if (diffs > max_changes)
		{
            kdDebug() << "[MotionAway Plugin] : Motion Detected. [" << diffs << "] Reseting Timeout" << endl;

			/* If we were away, now we are available again */
			if ( mPrefs->goAvailable() && KopeteAway::globalAway() )
				slotActivity();

			/* We reset the away timer */
            m_awayTimer->stop();
			m_awayTimer->start( mPrefs->awayTimeout() * 60 * 1000 );	
		}

		/* Old image slowly decays, this will make it even harder on
			slow moving object to stay undetected */

        /*
		for (i=0; i<width*height*3; i++)
		{
			image_ref[i]=(image_ref[i]+image_new[i])/2;
		}
		*/
	}
	else
	{
		m_captureTimer->stop();
	}
}

void MotionAwayPlugin::slotActivity()
{
		kdDebug() << "[MotionAway Plugin] : User activity!, going available" << endl;
		kopeteapp->slotSetAvailableAll();
}

void MotionAwayPlugin::slotTimeout()
{
	if ( !KopeteAway::globalAway() )
	{
		kdDebug() << "[MotionAway Plugin] : Timeout and no user activity, going away" << endl;
		kopeteapp->setAwayAll();
	}
}

void MotionAwayPlugin::slotSettingsChanged()
{
	m_awayTimer->changeInterval( mPrefs->awayTimeout() * 60 * 1000);
}

#include "motionawayplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

