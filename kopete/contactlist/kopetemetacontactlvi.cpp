/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact KListViewItem

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
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
#include <kglobal.h>
#include <kconfig.h>

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

using namespace Kopete::UI;

class KopeteMetaContactLVI::Private
{
public:
	ListView::ImageComponent *metaContactIcon;
	ListView::TextComponent *nameText;
	ListView::TextComponent *extraText;
	ListView::BoxComponent *contactIconBox;
	ListView::ImageComponent *buddyIcon;
	int iconSize;
	QTimer opacityTimer;
	float opacityTarget;
};

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, KopeteGroupViewItem *parent )
: ListView::Item( parent, contact, "MetaContactLVI" )
//: QObject( contact, "MetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;
	m_isTopLevel = false;
	m_parentGroup = parent;
	m_parentView = 0L;

	initLVI();
	parent->refreshDisplayName();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListViewItem *parent )
: ListView::Item( parent, contact, "MetaContactLVI" )
//: QObject( contact, "MetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = 0L;

	initLVI();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListView *parent )
: ListView::Item( parent, contact, "MetaContactLVI" )
//: QObject( contact, "MetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = parent;

	initLVI();
}

void KopeteMetaContactLVI::initLVI()
{
	d = new Private;

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

	connect( m_metaContact, SIGNAL( iconAppearanceChanged() ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( useCustomIconChanged( bool ) ),
		SLOT( slotUpdateIcons() ) );

	connect( m_metaContact, SIGNAL( contactIdleStateChanged( KopeteContact * ) ),
		SLOT( slotIdleStateChanged() ) );

	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( slotConfigChanged() ) );

	connect( &d->opacityTimer, SIGNAL( timeout() ), SLOT( slotOpacityTimer() ) );

	setOpacity( 0.0 );

	mBlinkTimer = new QTimer( this, "mBlinkTimer" );
	connect( mBlinkTimer, SIGNAL( timeout() ), SLOT( slotBlink() ) );
	mIsBlinkIcon = false;
	m_event = 0L;

	//if ( !mBlinkIcon )
	//	mBlinkIcon = new QPixmap( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::Small ) );

	// generate our contents
	using namespace ListView;
	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->metaContactIcon = new ImageComponent( hbox );

	KGlobal::config()->setGroup( QString::fromLatin1("ContactList") );
	QString type = KGlobal::config()->readEntry( QString::fromLatin1("ViewStyle"), QString::fromLatin1("Default") );
	if( type == QString::fromLatin1("Detailed") )    // new funky contact
	{
		Component *vbox = new BoxComponent( hbox, BoxComponent::Vertical );
		d->nameText = new TextComponent( vbox, listView()->font() );

		// FIXME: this is a nasty hack
		QFont smallFont = listView()->font();
		int size = smallFont.pixelSize();
		if ( size != -1 )
			smallFont.setPixelSize( (size * 3) / 2 );
		else
			smallFont.setPointSizeFloat( smallFont.pointSizeFloat() * 0.667 );
		d->extraText = new TextComponent( vbox, smallFont );

		Component *box = new BoxComponent( vbox, BoxComponent::Horizontal );
		d->contactIconBox = new BoxComponent( box, BoxComponent::Horizontal );
		new HSpacerComponent( box );

		d->buddyIcon = new ImageComponent( hbox );
		d->iconSize = 32;
	}
	else if( type == QString::fromLatin1("RightAligned") ) // old right-aligned contact
	{
		d->nameText = new TextComponent( hbox, listView()->font() );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
		d->buddyIcon = 0;
		d->extraText = 0;
		d->iconSize = 16;
	}
	else                 // older left-aligned contact
	{
		d->nameText = new TextComponent( hbox, listView()->font() );
		d->nameText->setFixedWidth( true );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
		new HSpacerComponent( hbox );
		d->buddyIcon = 0;
		d->extraText = 0;
		d->iconSize = 16;
	}

	// FIXME: fill in d->extraText

	slotUpdateIcons();
	slotDisplayNameChanged();
}

KopeteMetaContactLVI::~KopeteMetaContactLVI()
{
	delete d;
	//if ( m_parentGroup )
	//	m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::setOpacityTarget( float target )
{
	d->opacityTarget = target;

	if ( target != 0.0 )
		setVisible( true );
	// if we're aiming at invisible, and we're already there,
	// hide now. this happens on startup, since showing our
	// parent group shows us too. :(
	else if ( opacity() == 0.0 )
		setVisible( false );

	if ( opacity() != target )
		d->opacityTimer.start( 50 );
}

void KopeteMetaContactLVI::slotOpacityTimer()
{
	float dist = d->opacityTarget - opacity();
	float absdist = dist;
	if ( absdist < 0.0 )
		absdist = -absdist;

	if ( absdist > 0.1 )
	{
		setOpacity( opacity() + 0.1 * dist / absdist );
	}
	else
	{
		d->opacityTimer.stop();
		setOpacity( d->opacityTarget );
		if ( d->opacityTarget == 0.0 )
			setVisible( false );
	}
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
		slotDisplayNameChanged();
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

	if ( d->extraText )
	{
		if ( m_metaContact->status() == KopeteOnlineStatus::Online )
			d->extraText->setText( QString::null );
		else if ( c->hasProperty( QString::fromLatin1("awayMessage") ) )
			d->extraText->setText( i18n( "Message: %1" ).arg( c->property( QString::fromLatin1("awayMessage") ).value().toString() ) );
		else if ( d->extraText->text() == QString::null )
			d->extraText->setText( i18n( "User is %1" ).arg( m_metaContact->statusString() ) );
	}

	// FIXME: All this code should be in kopetemetacontact.cpp.. having it in the LVI makes it all fire
	// multiple times if the user is in multiple groups - Jason
	if ( c->account()->suppressStatusNotification() )
		return;

	if ( c->account()->myself()->onlineStatus().status() == KopeteOnlineStatus::Connecting )
		return;

	if ( !c->account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
	{
		int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;

		QString text = i18n( "%2 is now %1!" ).arg( m_metaContact->statusString(), m_metaContact->displayName() );

		if ( m_metaContact->isOnline() && m_oldStatus == KopeteOnlineStatus::Offline )
			KNotifyClient::event( winId,  "kopete_online", text, i18n( "Chat" ), this, SLOT( execute() ) );
		else if ( !m_metaContact->isOnline() && m_oldStatus != KopeteOnlineStatus::Offline && m_oldStatus != KopeteOnlineStatus::Unknown )
			KNotifyClient::event( winId , "kopete_offline", text );
		else if ( m_oldStatus != KopeteOnlineStatus::Unknown )
			KNotifyClient::event( winId , "kopete_status_change", text, i18n( "Chat" ), this, SLOT( execute() ) );

		if ( !mBlinkTimer->isActive() && ( m_metaContact->statusIcon() != m_oldStatusIcon ) )
		{
			mIsBlinkIcon = false;
			m_blinkLeft = 5;
			mBlinkTimer->start( 400, false );
		}
	}
}

void KopeteMetaContactLVI::slotUpdateIcons()
{
	slotIdleStateChanged();
	updateVisibility();
	updateContactIcons();

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
	d->nameText->setText( m_metaContact->displayName() );
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
	updateContactIcons();
}

void KopeteMetaContactLVI::updateVisibility()
{
	if ( KopetePrefs::prefs()->showOffline() /*|| mEventCount */ )
		setOpacityTarget( 1.0 );
	else if ( m_metaContact->status() == KopeteOnlineStatus::Offline && !mBlinkTimer->isActive() )
		setOpacityTarget( 0.0 );
	else
		setOpacityTarget( 1.0 );
}

class ContactComponent : public ListView::ImageComponent
{
	KopeteContact *mContact;
public:
	ContactComponent( ListView::ComponentBase *parent, KopeteContact *contact )
	 : ListView::ImageComponent( parent )
	 , mContact( contact )
	{
		setPixmap( contact->onlineStatus().iconFor( contact, 12 ) );
	}
	KopeteContact *contact()
	{
		return mContact;
	}
};

void KopeteMetaContactLVI::updateContactIcons()
{
	KGlobal::config()->setGroup( QString::fromLatin1("ContactList") );
	bool bHideOffline = KGlobal::config()->readBoolEntry( QString::fromLatin1("HideOfflineContacts"), false );
	if ( KopetePrefs::prefs()->showOffline() )
		bHideOffline = false;

	while ( d->contactIconBox->components() )
		delete d->contactIconBox->component( 0 );

	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	for ( QPtrListIterator<KopeteContact> it( contacts ); it.current(); ++it )
	{
		if ( !bHideOffline || (*it)->onlineStatus().status() != KopeteOnlineStatus::Offline )
			(void)new ContactComponent( d->contactIconBox, *it );
//		new ListView::TextComponent( d->contactIconBox, listView()->font(), QString::fromLatin1("test") );
	}
//		new ListView::TextComponent( d->contactIconBox, listView()->font(), QString::fromLatin1("end") );
}

KopeteContact *KopeteMetaContactLVI::contactForPoint( const QPoint &p ) const
{
	for ( uint n = 0; n < d->contactIconBox->components(); ++n )
	{
		if ( ContactComponent *comp = dynamic_cast<ContactComponent*>( d->contactIconBox->component( n ) ) )
		{
			if ( comp->rect().contains( p ) )
				return comp->contact();
		}
	}
	return 0L;
}

QRect KopeteMetaContactLVI::contactRect( const KopeteContact *c ) const
{
	for ( uint n = 0; n < d->contactIconBox->components(); ++n )
	{
		if ( ContactComponent *comp = dynamic_cast<ContactComponent*>( d->contactIconBox->component( n ) ) )
		{
			if ( comp->contact() == c )
				return comp->rect();
		}
	}
	return QRect();
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

	return importanceChar + d->nameText->text().lower();
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
	QPixmap icon = SmallIcon( m_metaContact->statusIcon(), d->iconSize );
	if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 ) )
	{
		KIconEffect::semiTransparent( icon );
		d->nameText->setColor( KopetePrefs::prefs()->idleContactColor() );
		if ( d->extraText )
			d->extraText->setColor( KopetePrefs::prefs()->idleContactColor() );
	}
	else
	{
		d->nameText->setDefaultColor();
		if ( d->extraText )
			d->extraText->setDefaultColor();
	}

	d->metaContactIcon->setPixmap( icon );
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
		d->metaContactIcon->setPixmap( SmallIcon( m_metaContact->statusIcon(), d->iconSize ) );
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
			d->metaContactIcon->setPixmap( SmallIcon( "newmsg", d->iconSize ) );
		}
		else
		{
			d->metaContactIcon->setPixmap( SmallIcon( m_oldStatusIcon, d->iconSize ) );
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

	d->metaContactIcon->setPixmap( SmallIcon( m_metaContact->statusIcon(), d->iconSize ) );
	mIsBlinkIcon = false;
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:

