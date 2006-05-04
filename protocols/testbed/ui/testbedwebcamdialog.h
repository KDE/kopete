/*
    Kopete Testbed Protocol

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

#ifndef TESTBEDWEBCAMDIALOG_H
#define TESTBEDWEBCAMDIALOG_H

#include <qstring.h>
#include <kdialogbase.h>

/**
	@author Kopete Developers <kopete-devel@kde.org>
*/
class QPixmap;
class QWidget;
class TestbedContact;

namespace Kopete { 
	namespace AV	{
		class VideoDevicePool;
	}
}

class TestbedWebcamDialog : public KDialogBase
{
Q_OBJECT
public:
	TestbedWebcamDialog( const QString &, QWidget* parent = 0, const char* name = 0 );
	~TestbedWebcamDialog();
	
public slots:
	void newImage( const QPixmap &image );
signals:
	void closingWebcamDialog();
	
private:
	QLabel *m_imageContainer;
	QLabel *m_Viewer;
	QString contactName;
	Kopete::AV::VideoDevicePool *m_videoDevicePool;
};

#endif
