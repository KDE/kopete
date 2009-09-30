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
#include "avdevice/videodevicepool.h"

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
	Kopete::AV::VideoDevicePool *m_devicePool;
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

	d->m_devicePool = Kopete::AV::VideoDevicePool::self();
	d->m_devicePool->loadConfig();
	d->m_devicePool->open();
	d->m_devicePool->setSize(640, 480);
	d->m_devicePool->startCapturing();

	d->m_timer = new QTimer( this );
	connect( d->m_timer, SIGNAL(timeout()), this, SLOT(updateImage()));

	d->mainWidget = new Kopete::WebcamWidget(this);
	d->mainWidget->setMinimumSize(645, 485);
	d->m_timer->start(40);
	setMainWidget(d->mainWidget);
}

void AvatarWebcamDialog::updateImage()
{
	QImage image = QImage();
	d->m_devicePool->getFrame();
	d->m_devicePool->getImage(&image);
	d->lastPixmap = QPixmap::fromImage(image.mirrored(false,false));//There is a better way of do this?
	d->mainWidget->updatePixmap(d->lastPixmap);
}

QPixmap& AvatarWebcamDialog::getLastPixmap()
{
	return d->lastPixmap;
}

void AvatarWebcamDialog::slotButtonClicked(int button)
{
	d->m_devicePool->close();
	KDialog::slotButtonClicked(button);
}

AvatarWebcamDialog::~AvatarWebcamDialog()
{
	delete d;
}

} // namespace UI

} // namespace Kopete
