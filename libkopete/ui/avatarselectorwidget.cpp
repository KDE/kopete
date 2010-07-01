#ifndef LIBKOPETE_UI_AVATARSELECTORWIDGET_CPP
#define LIBKOPETE_UI_AVATARSELECTORWIDGET_CPP
/*
    avatarselectorwidget.cpp - Widget to manage and select user avatar

    Copyright (c) 2007      by Michaël Larouche       <larouche@kde.org>
    Copyright (c) 2007         Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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
#include "avatarwebcamdialog.h"

// Qt includes
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QPainter>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kpixmapregionselectordialog.h>

#include "ui_avatarselectorwidget.h"
#ifndef VIDEOSUPPORT_DISABLED
#include "avdevice/videodevicepool.h"

using namespace Kopete::AV;
#endif

namespace Kopete
{
namespace UI
{

class AvatarSelectorWidgetItem : public QListWidgetItem
{
public:
	AvatarSelectorWidgetItem(QListWidget *parent)
	 : QListWidgetItem(parent, QListWidgetItem::UserType)
	{}

	void setAvatarEntry(Kopete::AvatarManager::AvatarEntry entry)
	{
		m_entry = entry;
		
		QSize s(96,96);

		if (listWidget())
		    s = listWidget()->iconSize();

		QPixmap pix;
		if (entry.path.isEmpty())
		{
			// draw a fake image telling there is no avatar
			pix = QPixmap(s);
			QPainter p(&pix);
			p.fillRect(pix.rect(), Qt::white);
			p.drawText(pix.rect(), Qt::TextWordWrap | Qt::AlignCenter, i18n("No Avatar"));
		}
		else
		{
			pix = QPixmap(entry.path).scaled(s);
		}

		// draw a border around the avatar
		QPainter p(&pix);
		p.setBrush(Qt::NoBrush);
		p.drawRect(0,0,pix.width()-1,pix.height()-1);

		setIcon(pix);
	}

	Kopete::AvatarManager::AvatarEntry avatarEntry() const
	{
		return m_entry;
	}

private:
	Kopete::AvatarManager::AvatarEntry m_entry;
};

class AvatarSelectorWidget::Private
{
public:
	Private()
	 : selectedItem(0), noAvatarItem(0)
	{}

	Ui::AvatarSelectorWidget mainWidget;
	QListWidgetItem *selectedItem;
	QString currentAvatar;
	AvatarSelectorWidgetItem * noAvatarItem;
	AvatarSelectorWidgetItem * addItem(Kopete::AvatarManager::AvatarEntry entry);
};

AvatarSelectorWidget::AvatarSelectorWidget(QWidget *parent)
 : QWidget(parent), d(new Private)
{
	d->mainWidget.setupUi(this);

	// use icons on buttons
	d->mainWidget.buttonAddAvatar->setIcon( KIcon("list-add") );
	d->mainWidget.buttonRemoveAvatar->setIcon( KIcon("edit-delete") );
	d->mainWidget.buttonFromWebcam->setIcon( KIcon("camera-web") );

#ifndef VIDEOSUPPORT_DISABLED
	VideoDevicePool* devicePool = VideoDevicePool::self();
	if( devicePool->size() == 0 ){
		d->mainWidget.buttonFromWebcam->hide();
	}
#else
	//If windows, just hide it
	d->mainWidget.buttonFromWebcam->hide();
#endif

	// Connect signals/slots
	connect(d->mainWidget.buttonAddAvatar, SIGNAL(clicked()), this, SLOT(buttonAddAvatarClicked()));
	connect(d->mainWidget.buttonRemoveAvatar, SIGNAL(clicked()), this, SLOT(buttonRemoveAvatarClicked()));
	connect(d->mainWidget.buttonFromWebcam, SIGNAL(clicked()), this, SLOT(buttonFromWebcamClicked()));
	connect(d->mainWidget.listUserAvatar, SIGNAL(itemClicked(QListWidgetItem*)),
	        this, SLOT(listSelectionChanged(QListWidgetItem*)));
	connect(Kopete::AvatarManager::self(), SIGNAL(avatarAdded(Kopete::AvatarManager::AvatarEntry)),
	        this, SLOT(avatarAdded(Kopete::AvatarManager::AvatarEntry)));
	connect(Kopete::AvatarManager::self(), SIGNAL(avatarRemoved(Kopete::AvatarManager::AvatarEntry)),
	        this, SLOT(avatarRemoved(Kopete::AvatarManager::AvatarEntry)));

	// Add a "No Avatar" option
	Kopete::AvatarManager::AvatarEntry empty;
	empty.name = i18n("No Avatar");
	empty.contact = 0;
	empty.category = Kopete::AvatarManager::User;
	d->noAvatarItem = d->addItem(empty);

	// List avatars of User category
	Kopete::AvatarQueryJob *queryJob = new Kopete::AvatarQueryJob(this);
	connect(queryJob, SIGNAL(result(KJob*)), this, SLOT(queryJobFinished(KJob*)));
	queryJob->setQueryFilter( Kopete::AvatarManager::User );

	queryJob->start();
}

AvatarSelectorWidget::~AvatarSelectorWidget()
{
	delete d;
}

Kopete::AvatarManager::AvatarEntry AvatarSelectorWidget::selectedEntry() const
{
	Kopete::AvatarManager::AvatarEntry result;

	if( d->selectedItem )
	{
		result = static_cast<AvatarSelectorWidgetItem*>(d->selectedItem)->avatarEntry();
	}

	return result;
}

void AvatarSelectorWidget::setCurrentAvatar(const QString &path)
{
	d->currentAvatar = path;

	//try to find the avatar in the list
	QList<QListWidgetItem*> itemList = d->mainWidget.listUserAvatar->findItems("", Qt::MatchContains);
	QList<QListWidgetItem*>::iterator it = itemList.begin();
	
	while (it != itemList.end())
	{
		AvatarSelectorWidgetItem *item = static_cast<AvatarSelectorWidgetItem*>(*it);
		if (item->avatarEntry().path == path)
		{
			item->setSelected(true);
			listSelectionChanged( item );
			return;
		}
		++it;
	}

}

void AvatarSelectorWidget::buttonAddAvatarClicked()
{
	KUrl imageUrl = KFileDialog::getImageOpenUrl( KUrl(), this );
	if( !imageUrl.isEmpty() )
	{
		// TODO: Download image
		if( !imageUrl.isLocalFile() )
			return;

		QPixmap pixmap( imageUrl.toLocalFile() );
		if ( pixmap.isNull() )
			return;
		QString imageName = imageUrl.fileName();
		cropAndSaveAvatar(pixmap,imageName);
	}
}

void AvatarSelectorWidget::buttonRemoveAvatarClicked()
{
	// if no item was selected, just exit
	if ( !d->mainWidget.listUserAvatar->selectedItems().count() )
		return;

	AvatarSelectorWidgetItem *selectedItem = dynamic_cast<AvatarSelectorWidgetItem*>( d->mainWidget.listUserAvatar->selectedItems().first() );
	if( selectedItem )
	{
		if ( selectedItem != d->noAvatarItem )
		{
			if( !Kopete::AvatarManager::self()->remove( selectedItem->avatarEntry() ) )
			{
				kDebug(14010) << "Removing of avatar failed for unknown reason.";
			}
		}
	}
}

void AvatarSelectorWidget::cropAndSaveAvatar(QPixmap& pixmap, const QString& imageName){
	// Crop the image
	QImage avatar = KPixmapRegionSelectorDialog::getSelectedImage( pixmap, 96, 96, this );

	Kopete::AvatarManager::AvatarEntry newEntry;
	// Remove extension from filename
	const int extIndex = imageName.lastIndexOf('.');
	newEntry.name = ( extIndex > 0 ) ? imageName.left( extIndex ) : imageName;
	newEntry.image = avatar;
	newEntry.category = Kopete::AvatarManager::User;

	Kopete::AvatarManager::AvatarEntry addedEntry = Kopete::AvatarManager::self()->add( newEntry );
	if( addedEntry.path.isEmpty() )
	{
		//TODO add a real error message
		//d->mainWidget.labelErrorMessage->setText( i18n("Kopete cannot add this new avatar because it could not save the avatar image in user directory.") );
		return;
	}

	// select the added entry and show the user tab
	QList<QListWidgetItem *> foundItems = d->mainWidget.listUserAvatar->findItems( addedEntry.name, Qt::MatchContains );
	if( !foundItems.isEmpty() )
	{
		AvatarSelectorWidgetItem *item = dynamic_cast<AvatarSelectorWidgetItem*>( foundItems.first() );
		if ( !item )
			return;
		item->setSelected( true );
	}
}

void AvatarSelectorWidget::buttonFromWebcamClicked()
{
	Kopete::UI::AvatarWebcamDialog *dialog = new Kopete::UI::AvatarWebcamDialog();
	int result = dialog->exec();
	if(result == KDialog::Accepted){
		QString avatarName("Webcam");
		int increment = 1;
		kDebug(14010) << "Trying with: " << avatarName;
		while((Kopete::AvatarManager::self()->exists(avatarName))) {
			avatarName = "Webcam_"+QString::number(increment);
			++increment;
			kDebug(14010) << "Trying with: " << avatarName;
		}
		cropAndSaveAvatar(dialog->getLastPixmap(),avatarName);
	}
	dialog->close();
	delete dialog;
}

void AvatarSelectorWidget::queryJobFinished(KJob *job)
{
	Kopete::AvatarQueryJob *queryJob = static_cast<Kopete::AvatarQueryJob*>(job);
	if( !queryJob->error() )
	{
		QList<Kopete::AvatarManager::AvatarEntry> avatarList = queryJob->avatarList();
		foreach(const Kopete::AvatarManager::AvatarEntry &entry, avatarList)
		{
			d->addItem(entry);
		}
	}
	else
	{
		//TODO add a real error message
		//d->mainWidget.labelErrorMessage->setText( queryJob->errorText() );
	}
}

void AvatarSelectorWidget::avatarAdded(Kopete::AvatarManager::AvatarEntry newEntry)
{
	d->addItem(newEntry);
	setCurrentAvatar(newEntry.path);
}

void AvatarSelectorWidget::avatarRemoved(Kopete::AvatarManager::AvatarEntry entryRemoved)
{
	// Same here, avatar can be only removed from listUserAvatar
	foreach(QListWidgetItem *item, d->mainWidget.listUserAvatar->findItems("",Qt::MatchContains))
	{
		// checks if this is the right item
		AvatarSelectorWidgetItem *avatar = dynamic_cast<AvatarSelectorWidgetItem*>(item);
		if (!avatar || avatar->avatarEntry().name != entryRemoved.name)
		    continue;

		kDebug(14010) << "Removing " << entryRemoved.name << " from list.";

		int deletedRow = d->mainWidget.listUserAvatar->row( item );
		QListWidgetItem *removedItem = d->mainWidget.listUserAvatar->takeItem( deletedRow );
		delete removedItem;

		int newRow = --deletedRow;
		if( newRow < 0 )
			newRow = 0;

		// Select the previous avatar in the list, thus selecting a new avatar
		// and deselecting the avatar being removed.
		d->mainWidget.listUserAvatar->setCurrentRow( newRow );
		// Force update
		listSelectionChanged( d->mainWidget.listUserAvatar->item(newRow) );
	}
}

void AvatarSelectorWidget::listSelectionChanged(QListWidgetItem *item)
{
	d->mainWidget.buttonRemoveAvatar->setEnabled( item != d->noAvatarItem );
	d->selectedItem = item;
}

AvatarSelectorWidgetItem * AvatarSelectorWidget::Private::addItem(Kopete::AvatarManager::AvatarEntry entry)
{
	kDebug(14010) << "Entry(" << entry.name << "): " << entry.category;

	// only use User avatars
	if( !(entry.category & Kopete::AvatarManager::User) )
	    return 0;

	AvatarSelectorWidgetItem *item = new AvatarSelectorWidgetItem(mainWidget.listUserAvatar);
	item->setAvatarEntry(entry);
	if (entry.path == currentAvatar)
		item->setSelected(true);
	return item;
}

} // Namespace Kopete::UI

} // Namespace Kopete

#include "avatarselectorwidget.moc"
#endif // LIBKOPETE_UI/AVATARSELECTORWIDGET_CPP
