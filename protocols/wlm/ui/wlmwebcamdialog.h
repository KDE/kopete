/*
    Kopete Wlm Protocol

    Copyright (c) 2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef WLMWEBCAMDIALOG_H
#define WLMWEBCAMDIALOG_H

#include <qstring.h>
#include <qimage.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <kdialog.h>

/**
	@author Kopete Developers <kopete-devel@kde.org>
*/
class QPixmap;
class QWidget;

namespace Kopete { 
	namespace AV	{
		class VideoDevicePool;
	}
	class WebcamWidget;
}

class WlmWebcamDialog : public KDialog
{
Q_OBJECT
public:
	explicit WlmWebcamDialog( const QString &, QWidget* parent = 0 );
	~WlmWebcamDialog();
	
public slots:
	void slotUpdateImage();
//signals:
//	void closingWebcamDialog();
	
private:
	Kopete::WebcamWidget *mImageContainer;
	QImage mImage;
	QTimer qtimer;
	QPixmap mPixmap;
	Kopete::AV::VideoDevicePool *mVideoDevicePool;
};

#endif
