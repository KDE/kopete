/*
    kopeteeditglobalidentitywidget.cpp  -  Kopete Edit Global Identity widget

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteeditglobalidentitywidget.h"

// Qt include
#include <qlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>

// KDE include
#include <klineedit.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kpixmapregionselectordialog.h>

// Kopete include
#include "kopeteglobal.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetepicture.h"


ClickableLabel::ClickableLabel(QWidget *parent)
	: QLabel(parent)
{
	setCursor(QCursor(Qt::PointingHandCursor));
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
	{
		emit clicked();
		event->accept();
	}
}

class KopeteEditGlobalIdentityWidget::Private
{
public:
	Private() : myself(0L), labelPicture(0L), lineNickname(0L), lineStatusMessage(0L), mainLayout(0L), iconSize(22, 22)
	{}
		
	Kopete::MetaContact *myself;
	ClickableLabel *labelPicture;
	KLineEdit *lineNickname;
	KLineEdit *lineStatusMessage;
	QHBoxLayout *mainLayout;
	QSize iconSize;
	QString lastNickname;
};

KopeteEditGlobalIdentityWidget::KopeteEditGlobalIdentityWidget(QWidget *parent) 
	: QWidget(parent)
{
	d = new Private;

	d->myself = Kopete::ContactList::self()->myself();

	createGUI();

	// Update the GUI when a global identity key change.
	connect(Kopete::ContactList::self(), SIGNAL(globalIdentityChanged(const QString&, const QVariant& )), this, SLOT(updateGUI(const QString&, const QVariant&)));
}

KopeteEditGlobalIdentityWidget::~KopeteEditGlobalIdentityWidget()
{
	delete d;
}

void KopeteEditGlobalIdentityWidget::setIconSize(const QSize &size)
{
	kDebug(14000) << k_funcinfo << "Manually changing the icon size." << endl;

	// Update the picture (change the size of it)
	d->iconSize = size;
	d->labelPicture->setMinimumSize(d->iconSize);
	d->labelPicture->setMaximumSize(d->iconSize);
	if( !d->myself->picture().isNull() )
		d->labelPicture->setPixmap(QPixmap::fromImage(d->myself->picture().image().scaled(d->iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void KopeteEditGlobalIdentityWidget::createGUI()
{
	d->mainLayout = new QHBoxLayout(this);
	
	// The picture label
	d->labelPicture = new ClickableLabel(this);
	d->labelPicture->setMinimumSize(d->iconSize);
	d->labelPicture->setMaximumSize(d->iconSize);
	d->labelPicture->setFrameShape(QFrame::Box);
	d->mainLayout->addWidget(d->labelPicture);
	connect(d->labelPicture, SIGNAL(clicked()), this, SLOT(photoClicked()));
	
	// The nickname lineEdit
	d->lineNickname = new KLineEdit(this);
	d->mainLayout->addWidget(d->lineNickname);
	 // Update the nickname when the user press return.
	connect(d->lineNickname, SIGNAL(returnPressed()), this, SLOT(changeNickname()));
	// Show the nickname text in red when they are change.
	connect(d->lineNickname, SIGNAL(textChanged(const QString&)), this, SLOT(lineNicknameTextChanged(const QString& )));
}

void KopeteEditGlobalIdentityWidget::updateGUI(const QString &key, const QVariant &value)
{
	kDebug(14000) << k_funcinfo << "Updating the GUI reflecting the global identity change." << endl;
	
	if(key == Kopete::Global::Properties::self()->photo().key())
	{
		// Update the picture and the tooltip
		if( !d->myself->picture().isNull() )
		{
			QImage myselfImage = d->myself->picture().image();
			d->labelPicture->setPixmap(QPixmap::fromImage(myselfImage.scaled(d->iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
			
			d->labelPicture->setToolTip( QString::fromUtf8("<qt><img src=\"%1\" width=\"%2\" height=\"%3\"></qt>").arg(value.toString()).arg(myselfImage.width()).arg(myselfImage.height()) );
		}
	}
	else if(key == Kopete::Global::Properties::self()->nickName().key())
	{
		// Update the nickname
		d->lastNickname = value.toString();
		d->lineNickname->setText(value.toString());
	}
}

void KopeteEditGlobalIdentityWidget::photoClicked()
{
	KUrl photoURL = KFileDialog::getImageOpenUrl( KUrl(), this, i18n("Global Photo"));
	if(photoURL.isEmpty())
		return;

	// Only accept local file.
	if(!photoURL.isLocalFile())
	{
		KMessageBox::sorry(this, i18n("Remote photos are not allowed."), i18n("Global Photo"));
		return;
	}

	QString saveLocation(KStandardDirs::locateLocal("appdata", "global-photo.png"));
	QImage photo(photoURL.path());
	photo = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap::fromImage(photo), 96, 96, this );

	if(!photo.isNull())
	{
		if(photo.width() > 96 || photo.height() > 96)
		{
			// Scale and crop the picture.
			photo = photo.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
			// crop image if not square
			if(photo.width() < photo.height()) 
				photo = photo.copy((photo.width()-photo.height())/2, 0, 96, 96);
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, 96, 96);

		}
		else if (photo.width() < 32 || photo.height() < 32)
		{
			// Scale and crop the picture.
			photo = photo.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
			// crop image if not square
			if(photo.width() < photo.height())
				photo = photo.copy((photo.width()-photo.height())/2, 0, 32, 32);
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, 32, 32);
	
		}
		else if (photo.width() != photo.height())
		{
			if(photo.width() < photo.height())
				photo = photo.copy((photo.width()-photo.height())/2, 0, photo.height(), photo.height());
			else if (photo.width() > photo.height())
				photo = photo.copy(0, (photo.height()-photo.width())/2, photo.height(), photo.height());
		}

		if(!photo.save(saveLocation, "PNG"))
		{
			KMessageBox::sorry(this, 
					i18n("An error occurred when trying to save the global photo."),
					i18n("Global Photo"));
		}
	}

	d->myself->setPhotoSource(Kopete::MetaContact::SourceCustom);
	d->myself->setPhoto(KUrl(saveLocation));
}

void KopeteEditGlobalIdentityWidget::lineNicknameTextChanged(const QString &text)
{
	// Display the nickname in red if they are any change. 
	if(text != d->lastNickname)
	{
		QPalette palette;
    	palette.setColor(d->lineNickname->foregroundRole(), Qt::red);
		d->lineNickname->setPalette(palette);
	}
	// The nickname re-become like it was before, reset the palette.
	else
	{
		d->lineNickname->setPalette(QPalette());
	}
}

void KopeteEditGlobalIdentityWidget::changeNickname()
{
	if( !d->lineNickname->text().isEmpty() && d->lineNickname->text() != d->myself->displayName() )
	{
		kDebug(14000) << k_funcinfo << "Updating global nickname..." << endl;

		// Reset the text color since the nickname is now updated.
		d->lineNickname->setPalette(QPalette());

		// Set the new nickname and set the DisplayName source Custom.
		d->lastNickname = d->lineNickname->text();
		d->myself->setDisplayName(d->lineNickname->text());
		d->myself->setDisplayNameSource(Kopete::MetaContact::SourceCustom);
	}
}

#include "kopeteeditglobalidentitywidget.moc"
