/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact KListViewItem

    Copyright (c) 2002-2004 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Copyright (c) 2002      by Duncan Mac-Vicar P     <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>

#include "kopetenotifyclient.h"
#include <kdebug.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kpopupmenu.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include "addcontactpage.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "kopetegroupviewitem.h"
#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopetestdaction.h"
#include "systemtray.h"

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, KopeteGroupViewItem *parent )
: QObject( contact, "KopeteMetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;
	m_isTopLevel = false;
	m_parentGroup = parent;
	m_parentView = 0L;

	initLVI();
	parent->refreshDisplayName();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListViewItem *parent )
: QObject( contact, "KopeteMetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = 0L;

	initLVI();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListView *parent )
: QObject( contact, "KopeteMetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = parent;

	initLVI();
}

void KopeteMetaContactLVI::initLVI()
{
	m_oldStatus = m_metaContact->status();
	m_oldStatusIcon = m_metaContact->statusIcon();

	connect( m_metaContact, SIGNAL( displayNameChanged( const QString &, const QString & ) ),
		SLOT( slotDisplayNameChanged() ) );

	connect( m_metaContact, SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( contactStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ),
		SLOT( slotContactStatusChanged( KopeteContact * ) ) );

	connect( m_metaContact, SIGNAL( contactAdded( KopeteContact * ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( contactRemoved( KopeteContact * ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( iconChanged( KopetePluginDataObject::IconState, const QString & ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( useCustomIconChanged( bool ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( contactIdleStateChanged( KopeteContact * ) ),
		SLOT( slotIdleStateChanged() ) );

	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( slotConfigChanged() ) );

	mBlinkTimer = new QTimer( this, "mBlinkTimer" );
	connect( mBlinkTimer, SIGNAL( timeout() ), SLOT( slotBlink() ) );
	mIsBlinkIcon = false;
	m_event = 0L;

	//if ( !mBlinkIcon )
	//	mBlinkIcon = new QPixmap( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::Small ) );

	slotUpdateIcons();
	slotDisplayNameChanged();
}

KopeteMetaContactLVI::~KopeteMetaContactLVI()
{
	//if ( m_parentGroup )
	//	m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::movedToGroup( KopeteGroup *to )
{
	KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() );
	if ( !lv )
		return;

	if ( m_parentGroup )
		m_parentGroup->refreshDisplayName();

	KopeteGroupViewItem *group_item = lv->getGroup( to );
	if ( group_item )
	{
		m_isTopLevel = false;
		m_parentGroup = group_item;
		m_parentView = 0L;
		group_item->refreshDisplayName();
	}
	else
	{
		m_isTopLevel = true;
		m_parentGroup = 0L;
		m_parentView = lv;
	}
}

void KopeteMetaContactLVI::rename( const QString& newName )
{
	if ( newName.isEmpty() )
	{
		// Reset the last display name
		QListViewItem::setText( 0, m_metaContact->displayName() );
		m_metaContact->setTrackChildNameChanges( true );
	}
	else // user changed name manually, disable tracking of contact nickname and update displayname
	{
		m_metaContact->setTrackChildNameChanges( false );
		m_metaContact->setDisplayName( newName );
	}

	kdDebug( 14000 ) << k_funcinfo << "newName=" << newName << ", TrackChildNameChanges=" << m_metaContact->trackChildNameChanges() << endl;
}

void KopeteMetaContactLVI::slotContactStatusChanged( KopeteContact *c )
{
	m_oldStatus = m_metaContact->status();
	slotUpdateIcons();

	// FIXME: All this code should be in kopetemetacontact.cpp.. having it in the LVI makes it all fire
	// multiple times if the user is in multiple groups - Jason
	if ( c->account()->suppressStatusNotification() )
		return;

	if ( c->account()->myself()->onlineStatus().status() == KopeteOnlineStatus::Connecting )
		return;

	if ( !c->account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
	{
		int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;

		QString text = i18n( "%2 is now %1!" ).arg( m_metaContact->statusString() ).arg( m_metaContact->displayName() );

		if ( m_metaContact->isOnline() && m_oldStatus == KopeteOnlineStatus::Offline )
			KNotifyClient::event( winId,  "kopete_online", text, i18n( "Chat" ), this, SLOT( execute() ) );
		else if ( !m_metaContact->isOnline() && m_oldStatus != KopeteOnlineStatus::Offline && m_oldStatus != KopeteOnlineStatus::Unknown )
#if KDE_IS_VERSION( 3, 1, 1 )
			KNotifyClient::event( winId , "kopete_offline", text );
#else
			KNotifyClient::event( "kopete_offline", text );
#endif
		else if ( m_oldStatus != KopeteOnlineStatus::Unknown )
			KNotifyClient::event( winId , "kopete_status_change", text, i18n( "Chat" ), this, SLOT( execute() ) );

		if ( !mBlinkTimer->isActive() &&
			( m_metaContact->statusIcon() != m_oldStatusIcon ) )
		{
			mIsBlinkIcon = false;
			m_blinkLeft = 5;
			mBlinkTimer->start( 400, false );
		}
	}
}

void KopeteMetaContactLVI::slotUpdateIcons()
{
	QPixmap statusIcon = SmallIcon( m_metaContact->statusIcon() );
	if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 ) )
		KIconEffect::semiTransparent( statusIcon );

	setPixmap( 0, statusIcon );

	updateVisibility();

	if ( m_parentGroup )
		m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::execute() const
{
	if ( m_event )
		m_event->apply();
	else
		m_metaContact->execute();
}

void KopeteMetaContactLVI::slotDisplayNameChanged()
{
	setText( 0, m_metaContact->displayName() );
}

/*
void KopeteMetaContactLVI::slotRemoveThisUser()
{
	kdDebug( 14000 ) << k_funcinfo << " Removing user" << endl;
	//m_metaContact->removeThisUser();

	if ( KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n( "Are you sure you want to remove %1 from your contact list?" ).
		arg( m_metaContact->displayName() ), i18n( "Remove Contact - Kopete" ) ) == KMessageBox::Yes )
	{
		KopeteContactList::contactList()->removeMetaContact( m_metaContact );
	}
}

void KopeteMetaContactLVI::slotRemoveFromGroup()
{
	if ( m_metaContact->isTemporary() )
		return;

	m_metaContact->removeFromGroup( group() );
}
*/

void KopeteMetaContactLVI::slotRename()
{
	setRenameEnabled( 0, true );

	// Use KListView's inline renaming when available, as it is more powerful
	// and allows clicking outside the edit box to actually save the changes.
	KListView *lv = dynamic_cast<KListView *>( listView() );
	if ( lv )
		lv->rename( this, 0 );
	else
		startRename( 0 );
}

void KopeteMetaContactLVI::okRename( int col )
{
	KListViewItem::okRename( col );
	setRenameEnabled( col, false ); // Yeah, that's on purpose...
}

/*
void KopeteMetaContactLVI::slotMoveToGroup()
{
	if ( m_actionMove && !m_metaContact->isTemporary() )
	{
		if ( m_actionMove->currentItem() == 0 )
		{
			// we are moving to top-level
			if ( group() != KopeteGroup::toplevel )
				m_metaContact->moveToGroup( group(), KopeteGroup::toplevel );
		}
		else
		{
			KopeteGroup *to = KopeteContactList::contactList()->getGroup( m_actionMove->currentText() );
			if ( !m_metaContact->groups().contains( to ) )
				m_metaContact->moveToGroup( group(), to );
		}
	}
}

void KopeteMetaContactLVI::slotAddToGroup()
{
	if ( m_actionCopy )
	{
		kdDebug( 14000 ) << "KopeteMetaContactLVI::slotAddToGroup " << endl;
		if ( m_actionCopy->currentItem() == 0 )
		{
			// we are adding to top-level
			m_metaContact->addToGroup( KopeteGroup::toplevel );
		}
		else
		{
			m_metaContact->addToGroup( KopeteContactList::contactList()->getGroup( m_actionCopy->currentText() ) );
		}
	}
}
*/

//FIXME: this is not used... remove?
void KopeteMetaContactLVI::slotAddToNewGroup()
{
	if ( m_metaContact->isTemporary() )
		return;

#if KDE_IS_VERSION( 3, 1, 90 )
	QString groupName = KInputDialog::getText(
		i18n( "New Group" ), i18n( "Please enter the name for the new group:" ) );
#else
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group" ), i18n( "Please enter the name for the new group:" ) );
#endif
	if ( !groupName.isEmpty() )
		m_metaContact->addToGroup( KopeteContactList::contactList()->getGroup( groupName ) );
}

void KopeteMetaContactLVI::slotConfigChanged()
{
	updateVisibility();
	slotIdleStateChanged();
	//repaint();
}

void KopeteMetaContactLVI::updateVisibility()
{
	if ( KopetePrefs::prefs()->showOffline() /*|| mEventCount */ )
		setVisible( true );
	else if ( m_metaContact->status() == KopeteOnlineStatus::Offline && !mBlinkTimer->isActive() )
		setVisible( false );
	else
		setVisible( true );
}

void KopeteMetaContactLVI::paintCell( QPainter *p, const QColorGroup &cg,
	int column, int width, int align )
{
	if ( column == 0 )
	{
		QPtrList<KopeteContact> contacts = m_metaContact->contacts();
		int cellWidth = width - ( contacts.count() * 16 ) - 4;
		if ( cellWidth < 0 )
			cellWidth = 0;

		QColorGroup modcg = cg;
		KopetePrefs *prefs = KopetePrefs::prefs();
		if ( prefs->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 ) )
			modcg.setColor( QColorGroup::Text, prefs->idleContactColor() );

		KListViewItem::paintCell( p, modcg, column, cellWidth, align );
		QFontMetrics fm( p->font() );
		int pixelsWide = fm.width( text( 0 ) ) + 16 + 4;
		if ( pixelsWide > cellWidth )
			pixelsWide = cellWidth;
		m_pixelWide = pixelsWide;

		// Draw the rest of the background
		QListView *lv = listView();
		if ( !lv )
			return;


			/*

  QColorGroup _cg = cg;
  const QPixmap *pm = listView()->viewport()->backgroundPixmap();
  if (pm && !pm->isNull())
  {
        _cg.setBrush(QColorGroup::Base, QBrush(backgroundColor(), *pm));
        QPoint o = p->brushOrigin();
        p->setBrushOrigin( o.x()-listView()->contentsX(), o.y()-listView()->contentsY() );
  }
  else if (isAlternate())
       if (listView()->viewport()->backgroundMode()==Qt::FixedColor)
            _cg.setColor(QColorGroup::Background, static_cast< KListView* >(listView())->alternateBackground());
       else
        _cg.setColor(QColorGroup::Base, static_cast< KListView* >(listView())->alternateBackground());

  QListViewItem::paintCell(p, _cg, column, width, alignment);


			*/

		int marg = lv->itemMargin();
		int r = marg;

		p->fillRect( cellWidth, 0, width - cellWidth, height(),
			QBrush( backgroundColor() ) );

		/*
			const BackgroundMode bgmode = lv->viewport()->backgroundMode();
			const QColorGroup::ColorRole crole =
				QPalette::backgroundRoleFromMode( bgmode );
			p->fillRect( cellWidth, 0, width - cellWidth, height(),
				cg.brush( crole ) );
		*/


		if ( isSelected() && ( column == 0 || listView()->allColumnsShowFocus() ) )
		{
			p->fillRect( QMAX( cellWidth, r - marg ), 0,
				width - cellWidth - r + marg, height(),
				cg.brush( QColorGroup::Highlight ) );
			if ( isEnabled() || !lv )
				p->setPen( cg.highlightedText() );
			else if ( !isEnabled() && lv )
				p->setPen( lv->palette().disabled().highlightedText() );
		}

		// And last, draw the online status icons
		int mc_x = 0;
		QPtrListIterator<KopeteContact> it( contacts );
		for ( ; it.current(); ++it )
		{
			QPixmap icon = ( *it )->onlineStatus().iconFor( *it, 12 );
			p->drawPixmap( mc_x + pixelsWide + 4, height() - 16,
				icon );
			mc_x += 16;
		}
	}
	else
	{
		// Use Qt's own drawing
		KListViewItem::paintCell( p, cg, column, width, align );
	}
}

KopeteContact *KopeteMetaContactLVI::contactForPoint( const QPoint &p ) const
{
	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	QPtrListIterator<KopeteContact> it( contacts );
	int mc_x = 0;
	for ( ; it.current(); ++it )
	{
		if ( QRect( mc_x + m_pixelWide + 4, 0, 12, height() ).contains( p ) )
		{
			return *it;
		}
		mc_x += 16;
	}
	return 0L;
}

QRect KopeteMetaContactLVI::contactRect( const KopeteContact *c ) const
{
	if ( !c )
		return QRect();

	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	QPtrListIterator<KopeteContact> it( contacts );
	int mc_x = 0;
	for ( ; it.current(); ++it )
	{
		if ( it.current() == c )
			return QRect( mc_x + m_pixelWide + 4, 0, 12, height() );

		mc_x += 16;
	}

	return QRect();
}

uint KopeteMetaContactLVI::firstContactIconX() const
{
	return m_pixelWide;
}

uint KopeteMetaContactLVI::lastContactIconX() const
{
	QPtrList<KopeteContact> contacts = m_metaContact->contacts();

	if ( contacts.isEmpty() )
		return m_pixelWide;

	QPtrListIterator<KopeteContact> it( contacts );
	int mc_x = 0;
	for ( ; it.current(); ++it )
		mc_x += 16;

	return mc_x + m_pixelWide + 4;
}

KopeteGroup *KopeteMetaContactLVI::group()
{
	if ( m_parentGroup && m_parentGroup->group() != KopeteGroup::topLevel() )
		return m_parentGroup->group();
	else
		return KopeteGroup::topLevel();
}

QString KopeteMetaContactLVI::key( int, bool ) const
{
	char importanceChar;
	switch ( m_metaContact->status() )
	{
	case KopeteOnlineStatus::Online:
		importanceChar = 'A';
		break;
	case KopeteOnlineStatus::Away:
		importanceChar = 'B';
		break;
	case KopeteOnlineStatus::Offline:
		importanceChar = 'C';
		break;
	case KopeteOnlineStatus::Unknown:
	default:
		importanceChar = 'D';
	}

	return importanceChar + text( 0 ).lower();
}

bool KopeteMetaContactLVI::isTopLevel() const
{
	return m_isTopLevel;
}

bool KopeteMetaContactLVI::isGrouped() const
{
	if ( m_parentView )
		return true;

	if ( !m_parentGroup || !m_parentGroup->group() )
		return false;

	if ( m_parentGroup->group() == KopeteGroup::temporary() && !KopetePrefs::prefs()->sortByGroup() )
		return false;

	return true;
}

void KopeteMetaContactLVI::slotIdleStateChanged()
{
	QPixmap icon = SmallIcon( m_metaContact->statusIcon() );
	if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 ) )
		KIconEffect::semiTransparent( icon );

	setPixmap( 0, icon );
	//if ( m_parentGroup )
	//	m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::catchEvent( KopeteEvent *event )
{
	if ( m_event )
	{
		//ignore the new event.
		return;
		//TODO: add a queue
	}

	m_event = event;
	connect( event, SIGNAL( done( KopeteEvent* ) ),
		this, SLOT( slotEventDone( KopeteEvent * ) ) );

	if ( mBlinkTimer->isActive() )
		mBlinkTimer->stop();

	m_oldStatusIcon = m_metaContact->statusIcon();

	mBlinkTimer->start( 500, false );
}

void KopeteMetaContactLVI::slotBlink()
{
	if ( mIsBlinkIcon )
	{
		setPixmap( 0, SmallIcon( m_metaContact->statusIcon() ) );
		if ( !m_event && m_blinkLeft <= 0 )
		{
			mBlinkTimer->stop();
			m_oldStatusIcon = m_metaContact->statusIcon();
			updateVisibility();
		}
	}
	else
	{
		if ( m_event )
		{
			setPixmap( 0, SmallIcon( "newmsg" ) );
		}
		else
		{
			setPixmap( 0, SmallIcon( m_oldStatusIcon ) );
			m_blinkLeft--;
		}
	}

	mIsBlinkIcon = !mIsBlinkIcon;
}

void KopeteMetaContactLVI::slotEventDone( KopeteEvent * /* event */ )
{
	m_event = 0L;
	if ( mBlinkTimer->isActive() )
	{
		mBlinkTimer->stop();
		//If the contact gone offline while the timer was actif,
		//the visibility has not been correctly updated. so do it now
		updateVisibility();
	}

	setPixmap( 0, SmallIcon( m_metaContact->statusIcon() ) );
	mIsBlinkIcon = false;
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:

