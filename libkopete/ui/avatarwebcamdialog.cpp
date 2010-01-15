/*
    avatarfromwebcamdialog.cpp - Dialog to get a pixmap from a webcam

    Copyright (c) 2009      by Alex Fiestas <alex@eyeos.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "avatarwebcamdialog.h"
#include "webcamwidget.h"

// QT includes
#include <qtimer.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>

// Kopete includes
#ifndef Q_OS_WIN
#include "avdevice/videodevicepool.h"
#endif

namespace Kopete
{

namespace UI
{

class AvatarWebcamDialog::Private
{
public:
	Private()
	 : mainWidget(0)
	{}
	~Private()
	{
	}

	Kopete::WebcamWidget *mainWidget;
	QTimer *m_timer;
#ifndef Q_OS_WIN
	Kopete::AV::VideoDevicePool *m_devicePool;
#endif
	QPixmap lastPixmap;
	QString selectedPath;
	QString currentPath;
};

AvatarWebcamDialog::AvatarWebcamDialog(QWidget *parent)
 : KDialog(parent), d(new Private)
{
	showButtonSeparator(true);
	setCaption(i18n("Take a photo"));
	setButtons(KDialog::Ok | KDialog::Cancel);

#ifndef Q_OS_WIN
	d->m_devicePool = Kopete::AV::VideoDevicePool::self();
	// FIXME d->m_devicePool->loadConfig();
	d->m_devicePool->open();
	// NB: this may fail if the device is already in use
	d->m_devicePool->setSize(640, 480);
	d->m_devicePool->startCapturing();
#endif

	d->m_timer = new QTimer( this );
	connect( d->m_timer, SIGNAL(timeout()), this, SLOT(updateImage()));

	d->mainWidget = new Kopete::WebcamWidget(this);
#ifndef Q_OS_WIN
	d->mainWidget->setMinimumSize(d->m_devicePool->width() + 5,
	                              d->m_devicePool->height() + 5);
#endif
	d->m_timer->start(40);
	setMainWidget(d->mainWidget);
}

void AvatarWebcamDialog::updateImage()
{
	QImage image = QImage();
#ifndef Q_OS_WIN
	d->m_devicePool->getFrame();
	d->m_devicePool->getImage(&image);
#endif
	d->lastPixmap = QPixmap::fromImage(image.mirrored(false,false));//There is a better way of do this?
	d->mainWidget->updatePixmap(d->lastPixmap);
}

QPixmap& AvatarWebcamDialog::getLastPixmap()
{
	return d->lastPixmap;
}

void AvatarWebcamDialog::slotButtonClicked(int button)
{
#ifndef Q_OS_WIN
	d->m_devicePool->close();
#endif
	KDialog::slotButtonClicked(button);
}

AvatarWebcamDialog::~AvatarWebcamDialog()
{
	delete d;
}

} // namespace UI

} // namespace Kopete
