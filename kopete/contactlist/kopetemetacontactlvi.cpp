/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact KListViewItem

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2002-2004 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
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
#include <qvariant.h>

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
#include <kinputdialog.h>


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
#include "kopeteglobal.h"

#include <memory>

using namespace Kopete::UI;

namespace Kopete {
namespace UI {
namespace ListView {

class MetaContactToolTipSource : public ToolTipSource
{
public:
	MetaContactToolTipSource( KopeteMetaContact *mc )
	 : metaContact( mc )
	{
	}
	QString operator()( ComponentBase *, const QPoint &, QRect & )
	{
		QPtrList<KopeteContact> contacts = metaContact->contacts();
		if( contacts.count() == 1 )
			return contacts.first()->toolTip();

		// We are over a metacontact with > 1 child contacts, and not over a specific contact
		// Iterate through children and display a summary tooltip
		QString toolTip = QString::fromLatin1("<qt><table>");
		for(KopeteContact *c = contacts.first(); c; c = contacts.next())
		{
			QString iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
			.arg( KURL::encode_string( c->protocol()->pluginId() ),
					KURL::encode_string( c->account()->accountId() ),
					KURL::encode_string( c->contactId() )
				);

			toolTip += i18n("<tr><td>STATUS ICON <b>PROTOCOL NAME</b> (ACCOUNT NAME)</td><td>STATUS DESCRIPTION</td></tr>",
								"<tr><td><img src=\"%1\">&nbsp;<b>%2</b>&nbsp;(%3)</td><td align=\"right\">%4</td></tr>")
						.arg( iconName, c->property(Kopete::Global::Properties::self()->nickName()).value().toString() , c->contactId(), c->onlineStatus().description() );
		}

		return toolTip + QString::fromLatin1("</table></qt>");
	}
private:
	KopeteMetaContact *metaContact;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

class KopeteMetaContactLVI::Private
{
public:
	Private() : metaContactIcon( 0 ), nameText( 0 ), extraText( 0 ), contactIconBox( 0 ),
	            buddyIcon( 0 ), currentMode( -1 ) {}
	ListView::ImageComponent *metaContactIcon;
	ListView::TextComponent *nameText;
	ListView::TextComponent *extraText;
	ListView::BoxComponent *contactIconBox;
	ListView::BoxComponent *spacerBox;
	ListView::ImageComponent *buddyIcon;
	std::auto_ptr<ListView::ToolTipSource> toolTipSource;
	int iconSize;
	int currentMode;
};

class ContactComponent : public ListView::ImageComponent
{
	KopeteContact *mContact;
public:
	ContactComponent( ListView::ComponentBase *parent, KopeteContact *contact )
	 : ListView::ImageComponent( parent )
	 , mContact( contact )
	{
		updatePixmap();
	}
	void updatePixmap()
	{
		setPixmap( contact()->onlineStatus().iconFor( contact(), 12 ) );
	}
	KopeteContact *contact()
	{
		return mContact;
	}
	// we don't need to use a tooltip source here - this way is simpler
	std::pair<QString,QRect> toolTip( const QPoint &relativePos )
	{
		return std::make_pair(mContact->toolTip(),rect());
	}
};

// FIXME: move to kopetelistviewitem.cpp
class SpacerComponent : public ListView::Component
{
public:
	SpacerComponent( ListView::ComponentBase *parent, int w, int h )
	 : ListView::Component( parent )
	{
		setMinWidth(w);
		setMinHeight(h);
	}
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
	d->toolTipSource.reset( new ListView::MetaContactToolTipSource( m_metaContact ) );

	m_oldStatus = m_metaContact->status();
	m_oldStatusIcon = m_metaContact->statusIcon();

	connect( m_metaContact, SIGNAL( displayNameChanged( const QString &, const QString & ) ),
		SLOT( slotDisplayNameChanged() ) );

	connect( m_metaContact, SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotUpdateMetaContact() ) );

	connect( m_metaContact, SIGNAL( contactStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ),
		SLOT( slotContactStatusChanged( KopeteContact * ) ) );

	connect( m_metaContact, SIGNAL( contactAdded( KopeteContact * ) ),
		SLOT( slotContactAdded( KopeteContact * ) ) );

	connect( m_metaContact, SIGNAL( contactRemoved( KopeteContact * ) ),
		SLOT( slotContactRemoved( KopeteContact * ) ) );

	connect( m_metaContact, SIGNAL( iconAppearanceChanged() ),
		SLOT( slotUpdateMetaContact() ) );

	connect( m_metaContact, SIGNAL( useCustomIconChanged( bool ) ),
		SLOT( slotUpdateMetaContact() ) );

	connect( m_metaContact, SIGNAL( contactIdleStateChanged( KopeteContact * ) ),
		SLOT( slotIdleStateChanged( KopeteContact * ) ) );

	connect( KopetePrefs::prefs(), SIGNAL( contactListAppearanceChanged() ),
		SLOT( slotConfigChanged() ) );

	mBlinkTimer = new QTimer( this, "mBlinkTimer" );
	connect( mBlinkTimer, SIGNAL( timeout() ), SLOT( slotBlink() ) );
	mIsBlinkIcon = false;
	m_event = 0L;

	//if ( !mBlinkIcon )
	//	mBlinkIcon = new QPixmap( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::Small ) );

	slotConfigChanged();  // this calls slotIdleStateChanged(), which sets up the constituent components, spacing, fonts and indirectly, the contact icon
	slotDisplayNameChanged();
	updateContactIcons();
}

KopeteMetaContactLVI::~KopeteMetaContactLVI()
{
	delete d;
	//if ( m_parentGroup )
	//	m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::movedToDifferentGroup()
{
	KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() );
	if ( !lv )
		return;

	if ( m_parentGroup )
		m_parentGroup->refreshDisplayName();

	// create a spacer if wanted
	// I assume that the safety property that allows the delete in slotConfigChanged holds here - Will
	delete d->spacerBox->component( 0 );
	if ( KListViewItem::parent() && KopetePrefs::prefs()->contactListIndentContacts() &&
	                !KopetePrefs::prefs()->treeView() )
	{
		new SpacerComponent( d->spacerBox, 20, 0 );
	}

	KopeteGroupViewItem *group_item = dynamic_cast<KopeteGroupViewItem*>(KListViewItem::parent());
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
	updateContactIcon( c );

	// FIXME: All this code should be in kopetemetacontact.cpp.. having it in the LVI makes it all fire
	// multiple times if the user is in multiple groups - Jason

	// comparing the status of the previous and new preferred contact is the determining factor in deciding to notify
	KopeteOnlineStatus newStatus;
	if ( m_metaContact->preferredContact() )
		newStatus = m_metaContact->preferredContact()->onlineStatus();
	else
	{
		// the last child contact has gone offline or otherwise unreachable, so take the changed contact's online status
		newStatus = c->onlineStatus();
	}

	// ensure we are not suppressing notifications, because connecting or disconnected
	if ( !(c->account()->suppressStatusNotification()
		 || ( c->account()->myself()->onlineStatus().status() == KopeteOnlineStatus::Connecting )
		 || !c->account()->isConnected() ) )
	{
		if ( !c->account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		{
			int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;

			//QString text = i18n( "%2 is now %1." ).arg( m_metaContact->statusString(), m_metaContact->displayName() );
			QString text = i18n( "%2 is now %1." ).arg( c->onlineStatus().description(), m_metaContact->displayName() );

			// figure out what's happened
			enum ChangeType { noChange, noEvent, signedIn, changedStatus, signedOut };
			ChangeType t = noChange;
			//kdDebug( 14000 ) << k_funcinfo << m_metaContact->displayName() <<
			//" - Old MC Status: " << m_oldStatus.status() << ", New MC Status: " << newStatus.status() << endl;
			// first, exclude changes due to blocking or subscription changes at the protocol level
			if ( ( m_oldStatus.status() == KopeteOnlineStatus::Unknown
						|| newStatus.status() == KopeteOnlineStatus::Unknown ) )
				t = noEvent;	// This means the contact's changed from or to unknown - due to a protocol state change, not a contact state change
			else	// we're dealing with a genuine contact state change
			{
				if ( m_oldStatus.status() == KopeteOnlineStatus::Offline )
				{
					if ( newStatus.status() != KopeteOnlineStatus::Offline )
					{
						//kdDebug( 14000 ) << "signed in" << endl;
						t = signedIn;	// contact has gone from offline to something else, it's a sign-in
					}
				}
				else if ( m_oldStatus.status() == KopeteOnlineStatus::Online
						  || m_oldStatus.status() == KopeteOnlineStatus::Away
						  || m_oldStatus.status() == KopeteOnlineStatus::Invisible)
				{
					if ( newStatus.status() == KopeteOnlineStatus::Offline )
					{
						//kdDebug( 14000 ) << "signed OUT" << endl;
						t = signedOut;	// contact has gone from an online state to an offline state, it's a sign out
					}
					else if ( m_oldStatus > newStatus || m_oldStatus < newStatus ) // operator!= is useless because it's an identity operator, not an equivalence operator
					{
						// contact has changed online states, it's a status change,
						// and the preferredContact changed status, or there is a new preferredContacat
						// so it's worth notifying
						//kdDebug( 14000 ) << "changed status" << endl;
						t = changedStatus;
					}
				}
				else if ( m_oldStatus != newStatus )
				{
					//kdDebug( 14000 ) << "non-event" << endl;
					// catch-all for any other status change we don't know about
					t = noEvent;
				}
				// if none of the above were true, t will still be noChange
			}

			// now issue the appropriate notification
			switch ( t )
			{
			case noEvent:
			case noChange:
				break;
			case signedIn:
				KNotifyClient::event( winId,  "kopete_contact_online", text, m_metaContact, i18n( "Chat" ), this, SLOT( execute() ) );
				break;
			case changedStatus:
				KNotifyClient::event( winId , "kopete_contact_status_change", text, m_metaContact, i18n( "Chat" ), this, SLOT( execute() ) );
				break;
			case signedOut:
				KNotifyClient::event( winId , "kopete_contact_offline", text, m_metaContact, i18n( "Offline" ), 0, 0 );
				break;
			}
		}
		//blink if the metacontact icon has changed.
		if ( !mBlinkTimer->isActive() && ( m_metaContact->statusIcon() != m_oldStatusIcon ) )
		{
			mIsBlinkIcon = false;
			m_blinkLeft = 5;
			mBlinkTimer->start( 400, false );
		}
	}
	else
	{
		//the status icon probably changed, but we didn't blink.
		//So the olfStatusIcon will not be set to the real after the blink.
		//we set it now.
		if( !mBlinkTimer->isActive() )
			m_oldStatusIcon=m_metaContact->statusIcon();
	}

	// make a note of the current status for the next time we get a status change
	m_oldStatus = newStatus;

}

void KopeteMetaContactLVI::slotUpdateMetaContact()
{
	slotIdleStateChanged( 0 );
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
	d->nameText->setText( m_metaContact->displayName() );
}

/*
void KopeteMetaContactLVI::slotRemoveThisUser()
{
	kdDebug( 14000 ) << k_funcinfo << " Removing user" << endl;
	//m_metaContact->removeThisUser();

	if ( KMessageBox::warningContinueCancel( Kopete::UI::Global::mainWidget(),
		i18n( "Are you sure you want to remove %1 from your contact list?" ).
		arg( m_metaContact->displayName() ), i18n( "Remove Contact" ), KGuiItem(i18n("Remove"),"editdelete") )
		== KMessageBox::Continue )
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

void KopeteMetaContactLVI::startRename( int col )
{
	KListViewItem::startRename( 0 );
}

void KopeteMetaContactLVI::okRename( int col )
{
	KListViewItem::okRename( col );
	rename( text( 0 ) );
	setRenameEnabled( 0, false );
}

void KopeteMetaContactLVI::cancelRename( int col )
{
	KListViewItem::cancelRename( col );
	setRenameEnabled( 0, false );
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

	QString groupName = KInputDialog::getText(
		i18n( "New Group" ), i18n( "Please enter the name for the new group:" ) );

	if ( !groupName.isEmpty() )
		m_metaContact->addToGroup( KopeteContactList::contactList()->getGroup( groupName ) );
}

void KopeteMetaContactLVI::slotConfigChanged()
{
	setDisplayMode( KopetePrefs::prefs()->contactListDisplayMode() );

	// create a spacer if wanted
	delete d->spacerBox->component( 0 );
	if ( KListViewItem::parent() && KopetePrefs::prefs()->contactListIndentContacts() &&
	                !KopetePrefs::prefs()->treeView() )
	{
		new SpacerComponent( d->spacerBox, 20, 0 );
	}

	if ( KopetePrefs::prefs()->contactListUseCustomFonts() )
		d->nameText->setFont( KopetePrefs::prefs()->contactListCustomNormalFont() );
	else
		d->nameText->setFont( listView()->font() );
	if ( d->extraText )
		d->extraText->setFont( KopetePrefs::prefs()->contactListSmallFont() );

	updateVisibility();
	updateContactIcons();
	slotIdleStateChanged( 0 );
}

void KopeteMetaContactLVI::setMetaContactToolTipSourceForComponent( ListView::Component *comp )
{
	if ( comp )
		comp->setToolTipSource( d->toolTipSource.get() );
}

void KopeteMetaContactLVI::setDisplayMode( int mode )
{
	if ( mode == d->currentMode )
		return;
	d->currentMode = mode;

	// empty...
	while ( component( 0 ) )
		delete component( 0 );
	d->extraText = 0;
	d->buddyIcon = 0;
	d->iconSize = IconSize( KIcon::Small );

	// generate our contents
	using namespace ListView;
	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->spacerBox = new BoxComponent( hbox, BoxComponent::Horizontal );

	if( mode == KopetePrefs::Detailed )                // new funky contact
	{
		d->metaContactIcon = new ImageComponent( hbox );
		Component *vbox = new BoxComponent( hbox, BoxComponent::Vertical );
		d->nameText = new TextComponent( vbox );
		d->extraText = new TextComponent( vbox );

		Component *box = new BoxComponent( vbox, BoxComponent::Horizontal );
		d->contactIconBox = new BoxComponent( box, BoxComponent::Horizontal );

		new HSpacerComponent( hbox );
		d->buddyIcon = new ImageComponent( hbox );
		d->iconSize = IconSize( KIcon::Toolbar );
	}
	else if( mode == KopetePrefs::Yagami )             // just for Yagami :)
	{
		Component *vbox = new BoxComponent( hbox, BoxComponent::Vertical );
		d->nameText = new TextComponent( vbox );
		d->extraText = new TextComponent( vbox );

		Component *box = new BoxComponent( vbox, BoxComponent::Horizontal );
		d->contactIconBox = new BoxComponent( box, BoxComponent::Horizontal );

		new HSpacerComponent( hbox );
		d->buddyIcon = new ImageComponent( hbox );
		d->iconSize = IconSize( KIcon::Toolbar );
		d->metaContactIcon = new ImageComponent( hbox );
	}
	else if( mode == KopetePrefs::RightAligned )       // old right-aligned contact
	{
		d->metaContactIcon = new ImageComponent( hbox );
		d->nameText = new TextComponent( hbox );
		new HSpacerComponent( hbox );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	}
	else                                               // older left-aligned contact
	{
		d->metaContactIcon = new ImageComponent( hbox );
		d->nameText = new TextComponent( hbox );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	}

	// set some components to have the metacontact tooltip
	setMetaContactToolTipSourceForComponent( d->metaContactIcon );
	setMetaContactToolTipSourceForComponent( d->nameText );
	setMetaContactToolTipSourceForComponent( d->extraText );

	// update the display name
	slotDisplayNameChanged();

	// finally, re-add all contacts so their icons appear. remove them first for consistency.
	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	for ( QPtrListIterator<KopeteContact> it( contacts ); it.current(); ++it )
	{
		slotContactRemoved( *it );
		slotContactAdded( *it );
	}
}

void KopeteMetaContactLVI::updateVisibility()
{
	if ( KopetePrefs::prefs()->showOffline() || m_event  )
		setTargetVisibility( true );
	else if ( !m_metaContact->isOnline() && !mBlinkTimer->isActive() )
		setTargetVisibility( false );
	else
		setTargetVisibility( true );
}

void KopeteMetaContactLVI::slotContactPropertyChanged( KopeteContact *,
	const QString &key, const QVariant &old, const QVariant &newVal )
{
	if ( key == QString::fromLatin1("awayMessage") && d->extraText && old != newVal )
	{
		if ( newVal.toString().isEmpty() )
			d->extraText->setText( QString::null );
		else
			d->extraText->setText( newVal.toString() );
	}
}

void KopeteMetaContactLVI::slotContactAdded( KopeteContact *c )
{
	connect( c, SIGNAL( propertyChanged( KopeteContact *, const QString &,
			const QVariant &, const QVariant & ) ),
		this, SLOT( slotContactPropertyChanged( KopeteContact *, const QString &,
			const QVariant &, const QVariant & ) ) );
	connect( c->account() , SIGNAL( colorChanged(const QColor& ) ) , this, SLOT( updateContactIcons() ) );

	updateContactIcon( c );

	slotContactPropertyChanged( c, QString::fromLatin1("awayMessage"),
		QVariant(), c->property( QString::fromLatin1("awayMessage") ).value() );
}

void KopeteMetaContactLVI::slotContactRemoved( KopeteContact *c )
{
	disconnect( c, SIGNAL( propertyChanged( KopeteContact *, const QString &,
			const QVariant &, const QVariant & ) ),
		this, SLOT( slotContactPropertyChanged( KopeteContact *,
			const QString &, const QVariant &, const QVariant & ) ) );
	disconnect( c->account() , SIGNAL( colorChanged(const QColor& ) ) , this, SLOT( updateContactIcons() ) );

	if ( ListView::Component *comp = contactComponent( c ) )
		delete comp;

	slotContactPropertyChanged( c, QString::fromLatin1("awayMessage"),
		c->property( QString::fromLatin1("awayMessage") ).value(), QVariant() );
}

void KopeteMetaContactLVI::updateContactIcons()
{
	// show offline contacts setting may have changed
	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	for ( QPtrListIterator<KopeteContact> it( contacts ); it.current(); ++it )
		updateContactIcon( *it );
}

void KopeteMetaContactLVI::updateContactIcon( KopeteContact *c )
{
	KGlobal::config()->setGroup( QString::fromLatin1("ContactList") );
	bool bHideOffline = KGlobal::config()->readBoolEntry(
		QString::fromLatin1("HideOfflineContacts"), false );
	if ( KopetePrefs::prefs()->showOffline() )
		bHideOffline = false;

	ContactComponent *comp = contactComponent( c );
	bool bShow = !bHideOffline || c->isOnline();
	if ( bShow && !comp )
		(void)new ContactComponent( d->contactIconBox, c );
	else if ( !bShow && comp )
		delete comp;
	else if ( comp )
		comp->updatePixmap();
}

KopeteContact *KopeteMetaContactLVI::contactForPoint( const QPoint &p ) const
{
	if ( ContactComponent *comp = dynamic_cast<ContactComponent*>( d->contactIconBox->componentAt( p ) ) )
		return comp->contact();
	return 0L;
}

ContactComponent *KopeteMetaContactLVI::contactComponent( const KopeteContact *c ) const
{
	for ( uint n = 0; n < d->contactIconBox->components(); ++n )
	{
		if ( ContactComponent *comp = dynamic_cast<ContactComponent*>( d->contactIconBox->component( n ) ) )
		{
			if ( comp->contact() == c )
				return comp;
		}
	}
	return 0;
}

QRect KopeteMetaContactLVI::contactRect( const KopeteContact *c ) const
{
	if ( ListView::Component *comp = contactComponent( c ) )
		return comp->rect();
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

void KopeteMetaContactLVI::slotIdleStateChanged( KopeteContact *c )
{
	QPixmap icon = SmallIcon( m_metaContact->statusIcon(), d->iconSize );
	if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 ) )
	{
		// TODO: QPixmapCache this result
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
	// we only need to update the contact icon if one was supplied;
	// if none was supplied, we only need to update the MC appearance
	if ( c )
		updateContactIcon( c );
	else
		return;
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

	//show the contact if it was hidden because offline.
	updateVisibility();
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

QString KopeteMetaContactLVI::text( int column ) const
{
	if ( column == 0 )
		return d->nameText->text();
	else
		return KListViewItem::text( column );
}

void KopeteMetaContactLVI::setText( int column, const QString &text )
{
	if ( column == 0 )
		d->nameText->setText( text );
	else
		KListViewItem::setText( column, text );
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:




