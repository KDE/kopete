/*
    chatmemberslistwidget.cpp - Chat Members List Widget

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "chatmemberslistwidget.h"

#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteglobal.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <kdebug.h>
#include <kmultipledrag.h>
#include <kpopupmenu.h>

#include <qheader.h>
#include <qtooltip.h>

//BEGIN ChatMembersListWidget::ToolTip

class ChatMembersListWidget::ToolTip : public QToolTip
{
public:
	ToolTip( KListView *parent )
		: QToolTip( parent->viewport() ), m_listView ( parent )
	{
	}

	virtual ~ToolTip()
	{
		remove( m_listView->viewport() );
	}

	void maybeTip( const QPoint &pos )
	{
		if( QListViewItem *item = m_listView->itemAt( pos ) )
		{
			QRect itemRect = m_listView->itemRect( item );
			if( itemRect.contains( pos ) )
				tip( itemRect, static_cast<ContactItem*>( item )->contact()->toolTip() );
		}
	}

private:
	KListView *m_listView;
};

//END ChatMembersListWidget::ToolTip


//BEGIN ChatMembersListWidget::ContactItem

ChatMembersListWidget::ContactItem::ContactItem( ChatMembersListWidget *parent, Kopete::Contact *contact )
	: KListViewItem( parent ), m_contact( contact )
{
	QString nick = m_contact->property(Kopete::Global::Properties::self()->nickName().key()).value().toString();
	if ( nick.isEmpty() )
		nick = m_contact->contactId();
	setText( 0, nick );
	setDragEnabled(true);

	connect( m_contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
	         this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;

	setStatus( parent->session()->contactOnlineStatus(m_contact) );
	reposition();
}

void ChatMembersListWidget::ContactItem::slotPropertyChanged( Kopete::Contact*,
	const QString &key, const QVariant&, const QVariant &newValue  )
{
	if ( key == Kopete::Global::Properties::self()->nickName().key() )
	{
		setText( 0, newValue.toString() );
		reposition();
	}
}

void ChatMembersListWidget::ContactItem::setStatus( const Kopete::OnlineStatus &status )
{
	setPixmap( 0, status.iconFor( m_contact ) );
	reposition();
}

void ChatMembersListWidget::ContactItem::reposition()
{
	// Qt's listview sorting is pathetic - it's impossible to reposition a single item
	// when its key changes, without re-sorting the whole list. Plus, the whole list gets
	// re-sorted whenever an item is added/removed. So, we do manual sorting.
	// In particular, this makes adding N items O(N^2) not O(N^2 log N).
	Kopete::ChatSession *session = static_cast<ChatMembersListWidget*>( listView() )->session();
	int ourWeight = session->contactOnlineStatus(m_contact).weight();
	QListViewItem *after = 0;

	for ( QListViewItem *it = KListViewItem::listView()->firstChild(); it; it = it->nextSibling() )
	{
		ChatMembersListWidget::ContactItem *item = static_cast<ChatMembersListWidget::ContactItem*>(it);
		int theirWeight = session->contactOnlineStatus(item->m_contact).weight();

		if( theirWeight < ourWeight ||
			(theirWeight == ourWeight && item->text(0).localeAwareCompare( text(0) ) > 0 ) )
		{
			break;
		}

		after = it;
	}

	moveItem( after );
}

//END ChatMembersListWidget::ContactItem


//BEGIN ChatMembersListWidget

ChatMembersListWidget::ChatMembersListWidget( Kopete::ChatSession *session, QWidget *parent, const char *name )
	 : KListView( parent, name ), m_session( session )
{
	// use our own custom tooltips
	setShowToolTips( false );
	m_toolTip = new ToolTip( this );

	// set up display: no header
	setAllColumnsShowFocus( true );
	addColumn( QString::null, -1 );
	header()->setStretchEnabled( true, 0 );
	header()->hide();

	// list is sorted by us, not by Qt
	setSorting( -1 );

	// add chat members
	slotContactAdded( session->myself() );
	for ( QPtrListIterator<Kopete::Contact> it( session->members() ); it.current(); ++it )
		slotContactAdded( *it );

	connect( this, SIGNAL( contextMenu( KListView*, QListViewItem *, const QPoint &) ),
	         SLOT( slotContextMenu(KListView*, QListViewItem *, const QPoint & ) ) );
	connect( this, SIGNAL( executed( QListViewItem* ) ),
	         SLOT( slotExecute( QListViewItem * ) ) );

	connect( session, SIGNAL( contactAdded(const Kopete::Contact*, bool) ),
	         this, SLOT( slotContactAdded(const Kopete::Contact*) ) );
	connect( session, SIGNAL( contactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ),
	         this, SLOT( slotContactRemoved(const Kopete::Contact*) ) );
	connect( session, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & , const Kopete::OnlineStatus &) ),
	         this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & ) ) );
}

ChatMembersListWidget::~ChatMembersListWidget()
{
}

void ChatMembersListWidget::slotContextMenu( KListView*, QListViewItem *item, const QPoint &point )
{
	if ( ContactItem *contactItem = dynamic_cast<ContactItem*>(item) )
	{
		KPopupMenu *p = contactItem->contact()->popupMenu( session() );
		connect( p, SIGNAL( aboutToHide() ), p, SLOT( deleteLater() ) );
		p->popup( point );
	}
}

void ChatMembersListWidget::slotContactAdded( const Kopete::Contact *contact )
{
	if ( !m_members.contains( contact ) )
		m_members.insert( contact, new ContactItem( this, const_cast<Kopete::Contact*>( contact ) ) );
}

void ChatMembersListWidget::slotContactRemoved( const Kopete::Contact *contact )
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( m_members.contains( contact ) && contact != session()->myself() )
	{
		delete m_members[ contact ];
		m_members.remove( contact );
	}
}

void ChatMembersListWidget::slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &status )
{
	if ( m_members.contains( contact ) )
		m_members[contact]->setStatus( status );
}

void ChatMembersListWidget::slotExecute( QListViewItem *item )
{
	if ( ContactItem *contactItem = dynamic_cast<ContactItem*>(item ) )
	{
		Kopete::Contact *contact=contactItem->contact();

		if(!contact || contact == contact->account()->myself())
			return;
				
		contact->execute();
	}
}

QDragObject *ChatMembersListWidget::dragObject()
{
	QListViewItem *currentLVI = currentItem();
	if( !currentLVI )
		return 0L;

	ContactItem *lvi = dynamic_cast<ContactItem*>( currentLVI );
	if( !lvi )
		return 0L;

	Kopete::Contact *c = lvi->contact();
	KMultipleDrag *drag = new KMultipleDrag( this );
	drag->addDragObject( new QStoredDrag("application/x-qlistviewitem", 0L ) );

	QStoredDrag *d = new QStoredDrag("kopete/x-contact", 0L );
	d->setEncodedData( QString( c->protocol()->pluginId()+QChar( 0xE000 )+c->account()->accountId()+QChar( 0xE000 )+ c->contactId() ).utf8() );
	drag->addDragObject( d );

	KABC::Addressee address = KABC::StdAddressBook::self()->findByUid(c->metaContact()->metaContactId());

	if( !address.isEmpty() )
	{
		drag->addDragObject( new QTextDrag( address.fullEmail(), 0L ) );
		KABC::VCardConverter converter;
		QString vcard = converter.createVCard( address );
		if( !vcard.isNull() )
		{
			QStoredDrag *vcardDrag = new QStoredDrag("text/x-vcard", 0L );
			vcardDrag->setEncodedData( vcard.utf8() );
			drag->addDragObject( vcardDrag );
		}
	}

	drag->setPixmap( c->onlineStatus().iconFor(c, 12) );

	return drag;
}


//END ChatMembersListWidget

#include "chatmemberslistwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

