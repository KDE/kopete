/*
    MetaContactSelectorWidget

    Copyright (c) 2005 by Duncan Mac-Vicar Prett <duncan@kde.org>

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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <q3vbox.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3PtrList>
#include <QBoxLayout>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <kdeversion.h>
#include <kinputdialog.h>
#include <kpushbutton.h>
#include <kactivelabel.h>
#include <kdebug.h>
#include <klistview.h>
#include <klistviewsearchline.h>

#include "kopetelistview.h"
#include "kopetelistviewsearchline.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "metacontactselectorwidget_base.h"
#include "metacontactselectorwidget.h"

using namespace Kopete::UI::ListView;

namespace Kopete
{
namespace UI
{

class MetaContactSelectorWidgetLVI::Private
{
public:
	Kopete::MetaContact *metaContact;
	ImageComponent *metaContactPhoto;
	ImageComponent *metaContactIcon;
	DisplayNameComponent *nameText;
	TextComponent *extraText;
	BoxComponent *contactIconBox;
	BoxComponent *spacerBox;
	int photoSize;
	int contactIconSize;
};


MetaContactSelectorWidgetLVI::MetaContactSelectorWidgetLVI(Kopete::MetaContact *mc, Q3ListView *parent, QObject *owner, const char *name) : Kopete::UI::ListView::Item(parent, owner, name) , d( new Private() )
{
	d->metaContact = mc;
	d->photoSize = 60;
	
	connect( d->metaContact, SIGNAL( photoChanged() ),
		SLOT( slotPhotoChanged() ) );
	connect( d->metaContact, SIGNAL( displayNameChanged(const QString&, const QString&) ),
		SLOT( slotDisplayNameChanged() ) );
	buildVisualComponents();
}

Kopete::MetaContact* MetaContactSelectorWidgetLVI::metaContact()
{
	return d->metaContact;
}

void MetaContactSelectorWidgetLVI::slotDisplayNameChanged()
{
	if ( d->nameText )
	{
		d->nameText->setText( d->metaContact->displayName() );
	
		// delay the sort if we can
		if ( ListView::ListView *lv = dynamic_cast<ListView::ListView *>( listView() ) )
			lv->delayedSort();
		else
			listView()->sort();
	}
}

QString MetaContactSelectorWidgetLVI::text ( int /* column */ ) const
{
	return d->metaContact->displayName();
}

void MetaContactSelectorWidgetLVI::slotPhotoChanged()
{
	QPixmap photoPixmap;
	QImage photoImg = d->metaContact->photo();
	if ( !photoImg.isNull() && (photoImg.width() > 0) &&  (photoImg.height() > 0) )
	{
		int photoSize = d->photoSize;
		
		photoImg = photoImg.smoothScale( photoSize, photoSize, Qt::KeepAspectRatio ) ;
		
		// draw a 1 pixel black border
		photoPixmap = photoImg;
		QPainter p(&photoPixmap);
		p.setPen(Qt::black);
		p.drawLine(0, 0, photoPixmap.width()-1, 0);
		p.drawLine(0, photoPixmap.height()-1, photoPixmap.width()-1, photoPixmap.height()-1);
		p.drawLine(0, 0, 0, photoPixmap.height()-1);
		p.drawLine(photoPixmap.width()-1, 0, photoPixmap.width()-1, photoPixmap.height()-1);
	}
	else
	{
		// if no photo use the smilie icon
		photoPixmap=SmallIcon(d->metaContact->statusIcon(), d->photoSize);
	}
	d->metaContactPhoto->setPixmap( photoPixmap, false);
}

void MetaContactSelectorWidgetLVI::buildVisualComponents()
{
	// empty...
	while ( component( 0 ) )
		delete component( 0 );

	d->nameText = 0L;
	d->metaContactPhoto = 0L;
	d->extraText = 0L;
	d->contactIconSize = 16;
	d->photoSize = 48;

	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->spacerBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	
	d->contactIconSize = IconSize( KIcon::Small );
	Component *imageBox = new BoxComponent( hbox, BoxComponent::Vertical );
	new VSpacerComponent( imageBox );
	// include borders in size
	d->metaContactPhoto = new ImageComponent( imageBox, d->photoSize + 2 , d->photoSize + 2 );
	new VSpacerComponent( imageBox );
	Component *vbox = new BoxComponent( hbox, BoxComponent::Vertical );
	d->nameText = new DisplayNameComponent( vbox );
	d->extraText = new TextComponent( vbox );

	Component *box = new BoxComponent( vbox, BoxComponent::Horizontal );
	d->contactIconBox = new BoxComponent( box, BoxComponent::Horizontal );
	
	slotUpdateContactBox();
	slotDisplayNameChanged();
	slotPhotoChanged();
}

void MetaContactSelectorWidgetLVI::slotUpdateContactBox()
{
	QList<Kopete::Contact *> contacts = d->metaContact->contacts();
	QListIterator<Contact *> it(contacts);
	while (it.hasNext())
	{
		Kopete::Contact *c = it.next();
		new ContactComponent(d->contactIconBox, c, IconSize( KIcon::Small ));
	}
}

class MetaContactSelectorWidget::Private
{
public:
	MetaContactSelectorWidget_Base *widget;
	QList<Kopete::MetaContact *> excludedMetaContacts;
};


MetaContactSelectorWidget::MetaContactSelectorWidget( QWidget *parent, const char *name )
		: QWidget( parent, name ), d( new Private() )
{
	QBoxLayout *l = new QVBoxLayout(this);
	d->widget = new MetaContactSelectorWidget_Base(this);
	l->addWidget(d->widget);
	
	connect( d->widget->metaContactListView, SIGNAL( clicked(Q3ListViewItem * ) ),
			SIGNAL( metaContactListClicked( Q3ListViewItem * ) ) );
	connect( d->widget->metaContactListView, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
			SIGNAL( metaContactListClicked( Q3ListViewItem * ) ) );
	connect( d->widget->metaContactListView, SIGNAL( spacePressed( Q3ListViewItem * ) ),
			SIGNAL( metaContactListClicked( Q3ListViewItem * ) ) );
	
	connect( Kopete::ContactList::self(), SIGNAL( metaContactAdded( Kopete::MetaContact * ) ), this, SLOT( slotLoadMetaContacts() ) );
	
	d->widget->kListViewSearchLine->setListView(d->widget->metaContactListView);
	d->widget->metaContactListView->setFullWidth( true );
	d->widget->metaContactListView->header()->hide();
	d->widget->metaContactListView->setColumnWidthMode(0, Q3ListView::Maximum);
	slotLoadMetaContacts();
}


MetaContactSelectorWidget::~MetaContactSelectorWidget()
{
	disconnect( Kopete::ContactList::self(), SIGNAL( metaContactAdded( Kopete::MetaContact * ) ), this, SLOT( slotLoadMetaContacts() ) );
}


Kopete::MetaContact* MetaContactSelectorWidget::metaContact()
{
	MetaContactSelectorWidgetLVI *item = 0L;
	item = static_cast<MetaContactSelectorWidgetLVI *>( d->widget->metaContactListView->selectedItem() );

	if ( item )
		return item->metaContact();

	return 0L;
}

void MetaContactSelectorWidget::selectMetaContact( Kopete::MetaContact *mc )
{
	// iterate trough list view
	Q3ListViewItemIterator it( d->widget->metaContactListView );
	while( it.current() )
	{
		MetaContactSelectorWidgetLVI *item = (MetaContactSelectorWidgetLVI *) it.current();
		if (!item)
			continue;
	
		if ( mc == item->metaContact() )
		{
			// select the contact item
			d->widget->metaContactListView->setSelected( item, true );
			d->widget->metaContactListView->ensureItemVisible( item );
		}
		++it;
	}
}

void MetaContactSelectorWidget::excludeMetaContact( Kopete::MetaContact *mc )
{
	if( d->excludedMetaContacts.findIndex(mc) == -1 )
	{
		d->excludedMetaContacts.append(mc);
	}
	slotLoadMetaContacts();
}

bool MetaContactSelectorWidget::metaContactSelected()
{
	return d->widget->metaContactListView->selectedItem() ? true : false;
}

/**  Read in metacontacts from contactlist */
void MetaContactSelectorWidget::slotLoadMetaContacts()
{
	d->widget->metaContactListView->clear();

	QList<Kopete::MetaContact *> metaContacts = Kopete::ContactList::self()->metaContacts();
	QListIterator<Kopete::MetaContact *> it(metaContacts);
	while ( it.hasNext() )
	{
		Kopete::MetaContact *mc = it.next();
		if( !mc->isTemporary() && mc != metaContact() )
		if( !mc->isTemporary() && (d->excludedMetaContacts.findIndex(mc) == -1) )
		{
			new MetaContactSelectorWidgetLVI(mc, d->widget->metaContactListView);
		}
	}

	d->widget->metaContactListView->sort();
}

void MetaContactSelectorWidget::setLabelMessage( const QString &msg )
{
	d->widget->lblHeader->setText(msg);
}

} // namespace UI
} // namespace Kopete

#include "metacontactselectorwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

