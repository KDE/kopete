/*
    avatarselectorwidget.cpp - Widget to manage and select user avatar

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "avatarselectorwidget.h"

// Qt includes
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QIcon>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kpixmapregionselectordialog.h>

#include "ui_avatarselectorwidget.h"

namespace Kopete
{
namespace UI
{

class AvatarSelectorWidget::Private
{
public:
	Ui::AvatarSelectorWidget mainWidget;

	void addItem(Kopete::AvatarManager::AvatarEntry entry);
};

AvatarSelectorWidget::AvatarSelectorWidget(QWidget *parent)
 : QWidget(parent), d(new Private)
{
	d->mainWidget.setupUi(this);

	// Connect signals/slots
	connect(d->mainWidget.buttonAddAvatar, SIGNAL(clicked()), this, SLOT(buttonAddAvatarClicked()));
	connect(Kopete::AvatarManager::self(), SIGNAL(avatarAdded(Kopete::AvatarManager::AvatarEntry)), this, SLOT(avatarAdded(Kopete::AvatarManager::AvatarEntry)));

	// List avatars in lists
	Kopete::AvatarQueryJob *queryJob = new Kopete::AvatarQueryJob(this);
	connect(queryJob, SIGNAL(result(KJob*)), this, SLOT(queryJobFinished(KJob*)));
	queryJob->setQueryFilter( Kopete::AvatarManager::All );

	queryJob->start();
}

AvatarSelectorWidget::~AvatarSelectorWidget()
{
	delete d;
}

void AvatarSelectorWidget::applyAvatar()
{
	//TODO
}

void AvatarSelectorWidget::buttonAddAvatarClicked()
{
	KUrl imageUrl = KFileDialog::getImageOpenUrl( KUrl(), this );
	if( !imageUrl.isEmpty() )
	{
		// TODO: Download image
		if( !imageUrl.isLocalFile() )
		{
			return;
		}

		// Crop the image
		QImage avatar = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap(imageUrl.path()), 96, 96, this );

		QString imageName = imageUrl.fileName();

		Kopete::AvatarManager::AvatarEntry newEntry;
		// Remove extension from filename
		newEntry.name = imageName.left( imageName.lastIndexOf('.') );
		newEntry.image = avatar;
		newEntry.category = Kopete::AvatarManager::User;

		Kopete::AvatarManager::self()->add( newEntry );
	}
	else
	{
		// TODO
	}
}

void AvatarSelectorWidget::queryJobFinished(KJob *job)
{
	Kopete::AvatarQueryJob *queryJob = static_cast<Kopete::AvatarQueryJob*>(job);
	if( !queryJob->error() )
	{
		QList<Kopete::AvatarManager::AvatarEntry> avatarList = queryJob->avatarList();
		Kopete::AvatarManager::AvatarEntry entry;
		foreach(entry, avatarList)
		{
			d->addItem(entry);
		}
	}
	else
	{
		d->mainWidget.labelErrorMessage->setText( queryJob->errorText() );
	}
}

void AvatarSelectorWidget::avatarAdded(Kopete::AvatarManager::AvatarEntry newEntry)
{
	d->addItem(newEntry);
}

void AvatarSelectorWidget::Private::addItem(Kopete::AvatarManager::AvatarEntry entry)
{
	kDebug(14010) << k_funcinfo << "Entry(" << entry.name << "): " << entry.category << endl;

	QListWidget *listWidget  = 0;
	if( entry.category & Kopete::AvatarManager::User )
	{
		listWidget = mainWidget.listUserAvatar;
	}
	else if( entry.category & Kopete::AvatarManager::Contact )
	{
		listWidget = mainWidget.listUserContact;
	}
	else
	{
		return;
	}

	QListWidgetItem *item = new QListWidgetItem(listWidget);
	item->setText( entry.name );
	item->setIcon( QIcon(entry.path) );
}

} // Namespace Kopete::UI

} // Namespace Kopete

#include "avatarselectorwidget.moc"
