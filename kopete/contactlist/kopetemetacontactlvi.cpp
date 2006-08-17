/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact KListViewItem

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2002-2004 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@ kde.org>
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

#include <qpainter.h>
#include <qtimer.h>
#include <qvariant.h>
#include <qmime.h>
#include <qstylesheet.h>

#include "knotification.h"
#include <kdebug.h>
#include <kiconeffect.h>
#include <kimageeffect.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kpopupmenu.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapplication.h>

#include <kabc/addressbook.h>
#include <kabc/addressee.h>

#include <kdeversion.h>
#include <kinputdialog.h>


#include "addcontactpage.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopeteemoticons.h"
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
#include "kopetecontact.h"
#include "kabcpersistence.h"

#include <memory>

using namespace Kopete::UI;

namespace Kopete {
namespace UI {
namespace ListView {

class MetaContactToolTipSource : public ToolTipSource
{
public:
	MetaContactToolTipSource( MetaContact *mc )
	 : metaContact( mc )
	{
	}
	QString operator()( ComponentBase *, const QPoint &, QRect & )
	{

		// We begin with the meta contact display name at the top of the tooltip
		QString toolTip = QString::fromLatin1("<qt><table cellpadding=\"0\" cellspacing=\"1\">");

        toolTip += QString::fromLatin1("<tr><td>");

		if ( ! metaContact->photo().isNull() )
        {
			QString photoName = QString::fromLatin1("kopete-metacontact-photo:%1").arg( KURL::encode_string( metaContact->metaContactId() ));
			//QMimeSourceFactory::defaultFactory()->setImage( "contactimg", metaContact->photo() );
			toolTip += QString::fromLatin1("<img src=\"%1\">").arg( photoName );
        }

		toolTip += QString::fromLatin1("</td><td>");

		QString displayName;
		Kopete::Emoticons *e = Kopete::Emoticons::self();
		QValueList<Emoticons::Token> t = e->tokenize( metaContact->displayName());
		QValueList<Emoticons::Token>::iterator it;
		for( it = t.begin(); it != t.end(); ++it )
		{
			if( (*it).type == Kopete::Emoticons::Image )
			{
				displayName += (*it).picHTMLCode;
			} else if( (*it).type == Kopete::Emoticons::Text )
			{
				displayName += QStyleSheet::escape( (*it).text );
			}
		}

		toolTip += QString::fromLatin1("<b><font size=\"+1\">%1</font></b><br><br>").arg( displayName );

		QPtrList<Contact> contacts = metaContact->contacts();
		if ( contacts.count() == 1 )
		{
			return toolTip + contacts.first()->toolTip() + QString::fromLatin1("</td></tr></table></qt>");
		}

		toolTip += QString::fromLatin1("<table>");

        // We are over a metacontact with > 1 child contacts, and not over a specific contact
        // Iterate through children and display a summary tooltip
        for(Contact *c = contacts.first(); c; c = contacts.next())
		{
			QString iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
			.arg( KURL::encode_string( c->protocol()->pluginId() ),
					KURL::encode_string( c->account()->accountId() ),
					KURL::encode_string( c->contactId() )
				);

			toolTip += i18n("<tr><td>STATUS ICON <b>PROTOCOL NAME</b> (ACCOUNT NAME)</td><td>STATUS DESCRIPTION</td></tr>",
							"<tr><td><img src=\"%1\">&nbsp;<nobr><b>%2</b></nobr>&nbsp;<nobr>(%3)</nobr></td><td align=\"right\"><nobr>%4</nobr></td></tr>")
						.arg( iconName, Kopete::Emoticons::parseEmoticons(c->property(Kopete::Global::Properties::self()->nickName()).value().toString()) , c->contactId(), c->onlineStatus().description() );
		}

		return toolTip + QString::fromLatin1("</table></td></tr></table></qt>");
	}
private:
	MetaContact *metaContact;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

class KopeteMetaContactLVI::Private
{
public:
	Private() : metaContactIcon( 0L ), nameText( 0L ), extraText( 0L ), contactIconBox( 0L ),
	            currentMode( -1 ), currentIconMode( -1 ) {}
	ListView::ImageComponent *metaContactIcon;
	ListView::DisplayNameComponent *nameText;
	ListView::DisplayNameComponent *extraText;
	ListView::BoxComponent *contactIconBox;
	ListView::BoxComponent *spacerBox;
	std::auto_ptr<ListView::ToolTipSource> toolTipSource;
	// metacontact icon size
	int iconSize;
	// protocol icon size
	int contactIconSize;
	int currentMode;
	int currentIconMode;

	QPtrList<Kopete::MessageEvent> events;
};

KopeteMetaContactLVI::KopeteMetaContactLVI( Kopete::MetaContact *contact, KopeteGroupViewItem *parent )
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

KopeteMetaContactLVI::KopeteMetaContactLVI( Kopete::MetaContact *contact, QListViewItem *parent )
: ListView::Item( parent, contact, "MetaContactLVI" )
//: QObject( contact, "MetaContactLVI" ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = 0L;

	initLVI();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( Kopete::MetaContact *contact, QListView *parent )
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
	
	connect( m_metaContact, SIGNAL( displayNameChanged( const QString &, const QString & ) ),
		SLOT( slotDisplayNameChanged() ) );

	connect( m_metaContact, SIGNAL( photoChanged() ),
		SLOT( slotPhotoChanged() ) );

	connect( m_metaContact, SIGNAL( onlineStatusChanged( Kopete::MetaContact *, Kopete::OnlineStatus::StatusType ) ),
		SLOT( slotPhotoChanged() ) );

	connect( m_metaContact, SIGNAL( onlineStatusChanged( Kopete::MetaContact *, Kopete::OnlineStatus::StatusType ) ),
		this, SLOT(slotIdleStateChanged(  ) ) );

	connect( m_metaContact, SIGNAL( contactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & ) ),
		SLOT( slotContactStatusChanged( Kopete::Contact * ) ) );

	connect( m_metaContact, SIGNAL( contactAdded( Kopete::Contact * ) ),
		SLOT( slotContactAdded( Kopete::Contact * ) ) );

	connect( m_metaContact, SIGNAL( contactRemoved( Kopete::Contact * ) ),
		SLOT( slotContactRemoved( Kopete::Contact * ) ) );

	connect( m_metaContact, SIGNAL( iconAppearanceChanged() ),
		SLOT( slotUpdateMetaContact() ) );

	connect( m_metaContact, SIGNAL( useCustomIconChanged( bool ) ),
		SLOT( slotUpdateMetaContact() ) );

	connect( m_metaContact, SIGNAL( contactIdleStateChanged( Kopete::Contact * ) ),
		SLOT( slotIdleStateChanged( Kopete::Contact * ) ) );

	connect( KopetePrefs::prefs(), SIGNAL( contactListAppearanceChanged() ),
			 SLOT( slotConfigChanged() ) );
	
	connect( kapp, SIGNAL( appearanceChanged() ),  SLOT( slotConfigChanged() ) );

	mBlinkTimer = new QTimer( this, "mBlinkTimer" );
	connect( mBlinkTimer, SIGNAL( timeout() ), SLOT( slotBlink() ) );
	mIsBlinkIcon = false;

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
		new ListView::SpacerComponent( d->spacerBox, 20, 0 );
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
	QString oldName = m_metaContact->displayName();
	KopeteContactListView *lv = dynamic_cast<KopeteContactListView *>( listView() );
	if ( lv )
	{
		KopeteContactListView::UndoItem *u=new KopeteContactListView::UndoItem(KopeteContactListView::UndoItem::MetaContactRename, m_metaContact);
		// HACK but args are strings not ints
		u->nameSource = m_metaContact->displayNameSource();
		// additional args
		if ( m_metaContact->displayNameSource() == Kopete::MetaContact::SourceCustom )
		{
			u->args << m_metaContact->customDisplayName();
		}
		else if ( m_metaContact->displayNameSource() == Kopete::MetaContact::SourceContact )
		{
			Kopete::Contact* c = m_metaContact->displayNameSourceContact();
			if(c)
				u->args << c->contactId() << c->protocol()->pluginId() << c->account()->accountId();
		}
		// source kabc requires no arguments

		lv->insertUndoItem(u);
	}

	if ( newName.isEmpty() )
	{
		// fallback to KABC
		if ( !m_metaContact->metaContactId().isEmpty() )
		{
			m_metaContact->setDisplayNameSource(Kopete::MetaContact::SourceKABC);
			if ( ! m_metaContact->displayName().isEmpty() )
			{
				slotDisplayNameChanged();
				return;
			}
		}
		// bad luck with KABC
		m_metaContact->setDisplayNameSource(Kopete::MetaContact::SourceContact);
		// TODO iterate though all subcontacts to check non empty nick
		m_metaContact->setDisplayNameSourceContact( m_metaContact->contacts().first() );
		slotDisplayNameChanged();
	}
	else // user changed name manually, set source to custom
	{
		m_metaContact->setDisplayNameSource( Kopete::MetaContact::SourceCustom );
		m_metaContact->setDisplayName( newName );
		slotDisplayNameChanged();
	}

	kdDebug( 14000 ) << k_funcinfo << "newName=" << newName << endl;
}

void KopeteMetaContactLVI::slotContactStatusChanged( Kopete::Contact *c )
{
	updateContactIcon( c );

	// FIXME: All this code should be in kopetemetacontact.cpp.. having it in the LVI makes it all fire
	// multiple times if the user is in multiple groups - Jason

	// comparing the status of the previous and new preferred contact is the determining factor in deciding to notify
	Kopete::OnlineStatus newStatus;
	if ( m_metaContact->preferredContact() )
		newStatus = m_metaContact->preferredContact()->onlineStatus();
	else
	{
		// the last child contact has gone offline or otherwise unreachable, so take the changed contact's online status
		newStatus = c->onlineStatus();
	}

	// ensure we are not suppressing notifications, because connecting or disconnected
	if ( !(c->account()->suppressStatusNotification()
		 || ( c->account()->myself()->onlineStatus().status() == Kopete::OnlineStatus::Connecting )
		 || !c->account()->isConnected() ) )
	{
		if ( !c->account()->isAway() || KopetePrefs::prefs()->soundIfAway() )
		{
			//int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;

			QString text = i18n( "<qt><i>%1</i> is now %2.</qt>" )
					.arg( Kopete::Emoticons::parseEmoticons( QStyleSheet::escape(m_metaContact->displayName()) ) ,
						  QStyleSheet::escape(c->onlineStatus().description()));
			
			// figure out what's happened
			enum ChangeType { noChange, noEvent, signedIn, changedStatus, signedOut };
			ChangeType t = noChange;
			//kdDebug( 14000 ) << k_funcinfo << m_metaContact->displayName() <<
			//" - Old MC Status: " << m_oldStatus.status() << ", New MC Status: " << newStatus.status() << endl;
			// first, exclude changes due to blocking or subscription changes at the protocol level
			if ( ( m_oldStatus.status() == Kopete::OnlineStatus::Unknown
						|| newStatus.status() == Kopete::OnlineStatus::Unknown ) )
				t = noEvent;	// This means the contact's changed from or to unknown - due to a protocol state change, not a contact state change
			else	// we're dealing with a genuine contact state change
			{
				if ( m_oldStatus.status() == Kopete::OnlineStatus::Offline )
				{
					if ( newStatus.status() != Kopete::OnlineStatus::Offline )
					{
						//kdDebug( 14000 ) << "signed in" << endl;
						t = signedIn;	// contact has gone from offline to something else, it's a sign-in
					}
				}
				else if ( m_oldStatus.status() == Kopete::OnlineStatus::Online
						  || m_oldStatus.status() == Kopete::OnlineStatus::Away
						  || m_oldStatus.status() == Kopete::OnlineStatus::Invisible)
				{
					if ( newStatus.status() == Kopete::OnlineStatus::Offline )
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
				connect(KNotification::event(m_metaContact, "kopete_contact_online", text, m_metaContact->photo(), KopeteSystemTray::systemTray(), i18n( "Chat" )) ,
						SIGNAL(activated(unsigned int )) , this, SLOT( execute() ) );
				break;
			case changedStatus:
				connect(KNotification::event(m_metaContact, "kopete_contact_status_change", text, m_metaContact->photo(), KopeteSystemTray::systemTray(), i18n( "Chat" )) ,
						SIGNAL(activated(unsigned int )) , this, SLOT( execute() ));
				break;
			case signedOut:
				KNotification::event(m_metaContact, "kopete_contact_offline", text, m_metaContact->photo(), KopeteSystemTray::systemTray());
				break;
			}
		}
		//blink if the metacontact icon has changed.
		if ( !mBlinkTimer->isActive() && d->metaContactIcon /*&& d->metaContactIcon->pixmap()  != m_oldStatusIcon */) 
		{
			mIsBlinkIcon = false;
			m_blinkLeft = 9;
			mBlinkTimer->start( 400, false );
		}
	}
	else
	{
		//the status icon probably changed, but we didn't blink.
		//So the olfStatusIcon will not be set to the real after the blink.
		//we set it now.
		if( !mBlinkTimer->isActive() )
			m_oldStatusIcon=d->metaContactIcon ? d->metaContactIcon->pixmap() : QPixmap();
	}

	// make a note of the current status for the next time we get a status change
	m_oldStatus = newStatus;
	
	if ( m_parentGroup )
		m_parentGroup->refreshDisplayName();
	updateVisibility();
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
	if ( d->events.first() )
		d->events.first()->apply();
	else
		m_metaContact->execute();
	
	//The selection is removed, but the contact still hihjlihted,  remove the selection in the contactlist (see bug 106090)
	Kopete::ContactList::self()->setSelectedItems( QPtrList<Kopete::MetaContact>() , QPtrList<Kopete::Group>() );
}

void KopeteMetaContactLVI::slotDisplayNameChanged()
{
	if ( d->nameText )
	{
		d->nameText->setText( m_metaContact->displayName() );

		// delay the sort if we can
		if ( ListView::ListView *lv = dynamic_cast<ListView::ListView *>( listView() ) )
			lv->delayedSort();
		else
			listView()->sort();
	}
}

void KopeteMetaContactLVI::slotPhotoChanged()
{
	if ( d->metaContactIcon && d->currentIconMode == KopetePrefs::PhotoPic ) 
	{
		m_oldStatusIcon= d->metaContactIcon->pixmap();
		QPixmap photoPixmap;
		//QPixmap defaultIcon( KGlobal::iconLoader()->loadIcon( "vcard", KIcon::Desktop ) );
		QImage photoImg = m_metaContact->photo();
		if ( !photoImg.isNull() && (photoImg.width() > 0) &&  (photoImg.height() > 0) )
		{
			int photoSize = d->iconSize;

			photoImg = photoImg.smoothScale( photoSize, photoSize, QImage::ScaleMin );
			
			KImageEffect *effect = 0L;
			switch ( m_metaContact->status() )
			{
				case Kopete::OnlineStatus::Online:
				break;
				case Kopete::OnlineStatus::Away:
					effect = new KImageEffect();
					effect->fade(photoImg, 0.5, Qt::white);
				break;
				case Kopete::OnlineStatus::Offline:
					effect = new KImageEffect();
					effect->fade(photoImg, 0.4, Qt::white);
					effect->toGray(photoImg);
				break;
				case Kopete::OnlineStatus::Unknown:
				default:
					effect = new KImageEffect();
					effect->fade(photoImg, 0.8, Qt::white);
			}
			delete effect;
			
			photoPixmap = photoImg;
		}
		else
		{
			photoPixmap=SmallIcon(m_metaContact->statusIcon(), d->iconSize);
		}
		d->metaContactIcon->setPixmap( photoPixmap, false);
		if(mBlinkTimer->isActive())
			m_originalBlinkIcon=photoPixmap;
	}
}

/*
void KopeteMetaContactLVI::slotRemoveThisUser()
{
	kdDebug( 14000 ) << k_funcinfo << " Removing user" << endl;
	//m_metaContact->removeThisUser();

	if ( KMessageBox::warningContinueCancel( Kopete::UI::Global::mainWidget(),
		i18n( "Are you sure you want to remove %1 from your contact list?" ).
		arg( m_metaContact->displayName() ), i18n( "Remove Contact" ), KGuiItem(i18n("Remove"),"delete_user") )
		== KMessageBox::Continue )
	{
		Kopete::ContactList::self()->removeMetaContact( m_metaContact );
	}
}

void KopeteMetaContactLVI::slotRemoveFromGroup()
{
	if ( m_metaContact->isTemporary() )
		return;

	m_metaContact->removeFromGroup( group() );
}
*/

void KopeteMetaContactLVI::startRename( int /*col*/ )
{
	KListViewItem::startRename( 0 );
}

void KopeteMetaContactLVI::okRename( int col )
{
	KListViewItem::okRename( col );
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
			if ( group() != Kopete::Group::toplevel )
				m_metaContact->moveToGroup( group(), Kopete::Group::toplevel );
		}
		else
		{
			Kopete::Group *to = Kopete::ContactList::self()->getGroup( m_actionMove->currentText() );
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
			m_metaContact->addToGroup( Kopete::Group::toplevel );
		}
		else
		{
			m_metaContact->addToGroup( Kopete::ContactList::self()->getGroup( m_actionCopy->currentText() ) );
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
		m_metaContact->addToGroup( Kopete::ContactList::self()->findGroup( groupName ) );
}

void KopeteMetaContactLVI::slotConfigChanged()
{
    setDisplayMode( KopetePrefs::prefs()->contactListDisplayMode(),
                    KopetePrefs::prefs()->contactListIconMode() );

	// create a spacer if wanted
	delete d->spacerBox->component( 0 );
	if ( KListViewItem::parent() && KopetePrefs::prefs()->contactListIndentContacts() &&
	                !KopetePrefs::prefs()->treeView() )
	{
		new ListView::SpacerComponent( d->spacerBox, 20, 0 );
	}

	if ( KopetePrefs::prefs()->contactListUseCustomFonts() )
	{
		d->nameText->setFont( KopetePrefs::prefs()->contactListCustomNormalFont() );
		if ( d->extraText )
			d->extraText->setFont( KopetePrefs::prefs()->contactListSmallFont() );
	}
	else
	{
		QFont font=listView()->font();
		d->nameText->setFont( font );
		if(d->extraText)
		{
			if ( font.pixelSize() != -1 )
				font.setPixelSize( (font.pixelSize() * 3) / 4 );
			else
				font.setPointSizeFloat( font.pointSizeFloat() * 0.75 );
			d->extraText->setFont( font );
		}
	}
	
	updateVisibility();
	updateContactIcons();
	slotIdleStateChanged( 0 );
	if(d->nameText)
		d->nameText->redraw();
	if(d->extraText)
		d->extraText->redraw();
}

void KopeteMetaContactLVI::setMetaContactToolTipSourceForComponent( ListView::Component *comp )
{
	if ( comp )
		comp->setToolTipSource( d->toolTipSource.get() );
}

void KopeteMetaContactLVI::setDisplayMode( int mode, int iconmode )
{
	if ( mode == d->currentMode && iconmode == d->currentIconMode )
		return;

	d->currentMode = mode;
	d->currentIconMode = iconmode;

	// empty...
	while ( component( 0 ) )
		delete component( 0 );

	d->nameText = 0L;
	d->extraText = 0L;
	d->metaContactIcon = 0L;
	d->contactIconSize = 12;
	if (mode == KopetePrefs::Detailed) {
		d->iconSize =  iconmode == KopetePrefs::IconPic ?  KIcon::SizeMedium : KIcon::SizeLarge;
	} else {
		d->iconSize = iconmode == KopetePrefs::IconPic ? IconSize( KIcon::Small ) :  KIcon::SizeMedium;
	}
	disconnect( Kopete::KABCPersistence::self()->addressBook() , 0 , this , 0);

	// generate our contents
	using namespace ListView;
	Component *hbox = new BoxComponent( this, BoxComponent::Horizontal );
	d->spacerBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	
	if (iconmode == KopetePrefs::PhotoPic) {
		Component *imageBox = new BoxComponent( hbox, BoxComponent::Vertical );
		new VSpacerComponent( imageBox );
		d->metaContactIcon = new ImageComponent( imageBox, d->iconSize + 2 , d->iconSize + 2 );
		new VSpacerComponent( imageBox );
		if(!metaContact()->photoSource() && !Kopete::KABCPersistence::self()->addressBook()->findByUid( metaContact()->metaContactId() ).isEmpty()   )
		{	//if the photo is the one of the kaddressbook,  track every change in the adressbook, it might be the photo of our contact.
			connect( Kopete::KABCPersistence::self()->addressBook() , SIGNAL(addressBookChanged (AddressBook *) ) ,
				 this , SLOT(slotPhotoChanged()));
		}
	} else {
		d->metaContactIcon = new ImageComponent( hbox );
	}
	 
	if( mode == KopetePrefs::Detailed )
	{
		d->contactIconSize = IconSize( KIcon::Small );
		Component *vbox = new BoxComponent( hbox, BoxComponent::Vertical );
		d->nameText = new DisplayNameComponent( vbox );
		d->extraText = new DisplayNameComponent( vbox );

		Component *box = new BoxComponent( vbox, BoxComponent::Horizontal );
		d->contactIconBox = new BoxComponent( box, BoxComponent::Horizontal );
	}
	else if( mode == KopetePrefs::RightAligned )       // old right-aligned contact
	{
		d->nameText = new DisplayNameComponent( hbox );
		new HSpacerComponent( hbox );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	}
	else					       // older left-aligned contact
	{
		d->nameText = new DisplayNameComponent( hbox );
		d->contactIconBox = new BoxComponent( hbox, BoxComponent::Horizontal );
	}

	// set some components to have the metacontact tooltip
	setMetaContactToolTipSourceForComponent( d->metaContactIcon );
	setMetaContactToolTipSourceForComponent( d->nameText );
	setMetaContactToolTipSourceForComponent( d->extraText );

	// update the display name
	slotDisplayNameChanged();
	slotPhotoChanged();
	slotIdleStateChanged( 0 );

	// finally, re-add all contacts so their icons appear. remove them first for consistency.
	QPtrList<Kopete::Contact> contacts = m_metaContact->contacts();
	for ( QPtrListIterator<Kopete::Contact> it( contacts ); it.current(); ++it )
	{
		slotContactRemoved( *it );
		slotContactAdded( *it );
	}
	m_oldStatusIcon=d->metaContactIcon ? d->metaContactIcon->pixmap() : QPixmap();
	if( mBlinkTimer->isActive() )
		m_originalBlinkIcon=m_oldStatusIcon;
}

void KopeteMetaContactLVI::updateVisibility()
{
	if ( KopetePrefs::prefs()->showOffline() || !d->events.isEmpty()  )
		setTargetVisibility( true );
	else if ( !m_metaContact->isOnline() && !mBlinkTimer->isActive() )
		setTargetVisibility( false );
	else
		setTargetVisibility( true );
}

void KopeteMetaContactLVI::slotContactPropertyChanged( Kopete::Contact *contact,
	const QString &key, const QVariant &old, const QVariant &newVal )
{
//	if ( key == QString::fromLatin1("awayMessage") )
//		kdDebug( 14000 ) << k_funcinfo << "contact=" << contact->contactId() << ", isonline=" << contact->isOnline() << ", alloffline=" << !m_metaContact->isOnline() << ", oldvalue=" << old.toString() << ", newvalue=" << newVal.toString() << endl;
	if ( key == QString::fromLatin1("awayMessage") && d->extraText && old != newVal )
	{
		bool allOffline = !m_metaContact->isOnline();
		if ( newVal.toString().isEmpty() || ( !contact->isOnline() && !allOffline ) )
		{
			// try to find a more suitable away message to be displayed when: 
			// -new away message is empty or
			// -contact who set it is offline and there are contacts online in the metacontact
			bool allAwayMessagesEmpty = true;
			QPtrList<Kopete::Contact> contacts = m_metaContact->contacts();
			for ( Kopete::Contact *c = contacts.first(); c; c = contacts.next() )
			{
//				kdDebug( 14000 ) << k_funcinfo << "ccontact=" << c->contactId() << ", isonline=" << c->isOnline() << ", awaymsg=" << c->property( key ).value().toString() << endl;
				QString awayMessage( c->property( key ).value().toString() );
				if ( ( allOffline || c->isOnline() ) && !awayMessage.isEmpty() )
				{
					// display this contact's away message when:
					// -this contact's away message is not empty and
					// -this contact is online or there are no contacts online at all
					allAwayMessagesEmpty = false;
					d->extraText->setText( awayMessage );
					break;
				}
			}
			if ( allAwayMessagesEmpty )
				d->extraText->setText( QString::null );
		}
		else
		{
			// just use new away message when:
			// -new away message is not empty and
			// -contact who set it is online or there are no contacts online at all
			d->extraText->setText( newVal.toString() );
		}
	} // wtf? KopeteMetaContact also connects this signals and emits photoChanged! why no connect photoChanged to slotPhotoChanged?
	/*else if ( key == QString::fromLatin1("photo") && (m_metaContact->photoSourceContact() == contact) && (m_metaContact->photoSource() == Kopete::MetaContact::SourceContact))
	{
		slotPhotoChanged();
	}*/
}

void KopeteMetaContactLVI::slotContactAdded( Kopete::Contact *c )
{
	connect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &,
			const QVariant &, const QVariant & ) ),
		this, SLOT( slotContactPropertyChanged( Kopete::Contact *, const QString &,
			const QVariant &, const QVariant & ) ) );
	connect( c->account() , SIGNAL( colorChanged(const QColor& ) ) , this, SLOT( updateContactIcons() ) );

	updateContactIcon( c );

	slotContactPropertyChanged( c, QString::fromLatin1("awayMessage"),
		QVariant(), c->property( QString::fromLatin1("awayMessage") ).value() );
}

void KopeteMetaContactLVI::slotContactRemoved( Kopete::Contact *c )
{
	disconnect( c, SIGNAL( propertyChanged( Kopete::Contact *, const QString &,
			const QVariant &, const QVariant & ) ),
		this, SLOT( slotContactPropertyChanged( Kopete::Contact *,
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
	QPtrList<Kopete::Contact> contacts = m_metaContact->contacts();
	for ( QPtrListIterator<Kopete::Contact> it( contacts ); it.current(); ++it )
		updateContactIcon( *it );
}

void KopeteMetaContactLVI::updateContactIcon( Kopete::Contact *c )
{
	KGlobal::config()->setGroup( QString::fromLatin1("ContactList") );
	bool bHideOffline = KGlobal::config()->readBoolEntry(
		QString::fromLatin1("HideOfflineContacts"), false );
	if ( KopetePrefs::prefs()->showOffline() )
		bHideOffline = false;

	ListView::ContactComponent *comp = contactComponent( c );
	bool bShow = !bHideOffline || c->isOnline();
	if ( bShow && !comp )
		(void)new ListView::ContactComponent( d->contactIconBox, c, d->contactIconSize );
	else if ( !bShow && comp )
		delete comp;
	else if ( comp )
		comp->updatePixmap();
}

Kopete::Contact *KopeteMetaContactLVI::contactForPoint( const QPoint &p ) const
{
	if ( ListView::ContactComponent *comp = dynamic_cast<ListView::ContactComponent*>( d->contactIconBox->componentAt( p ) ) )
		return comp->contact();
	return 0L;
}

ListView::ContactComponent *KopeteMetaContactLVI::contactComponent( const Kopete::Contact *c ) const
{
	for ( uint n = 0; n < d->contactIconBox->components(); ++n )
	{
		if ( ListView::ContactComponent *comp = dynamic_cast<ListView::ContactComponent*>( d->contactIconBox->component( n ) ) )
		{
			if ( comp->contact() == c )
				return comp;
		}
	}
	return 0;
}

QRect KopeteMetaContactLVI::contactRect( const Kopete::Contact *c ) const
{
	if ( ListView::Component *comp = contactComponent( c ) )
		return comp->rect();
	return QRect();
}

Kopete::Group *KopeteMetaContactLVI::group()
{
	if ( m_parentGroup && m_parentGroup->group() != Kopete::Group::topLevel() )
		return m_parentGroup->group();
	else
		return Kopete::Group::topLevel();
}

QString KopeteMetaContactLVI::key( int, bool ) const
{
	char importanceChar;
	switch ( m_metaContact->status() )
	{
	case Kopete::OnlineStatus::Online:
		importanceChar = 'A';
		break;
	case Kopete::OnlineStatus::Away:
		importanceChar = 'B';
		break;
	case Kopete::OnlineStatus::Offline:
		importanceChar = 'C';
		break;
	case Kopete::OnlineStatus::Unknown:
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

	if ( m_parentGroup->group() == Kopete::Group::temporary() && !KopetePrefs::prefs()->sortByGroup() )
		return false;
 
	return true;
}

void KopeteMetaContactLVI::slotIdleStateChanged( Kopete::Contact *c )
{
	bool doWeHaveToGrayThatContact = KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleTime() >= 10 * 60 );
	if ( doWeHaveToGrayThatContact )
	{
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

	if(d->metaContactIcon && d->currentIconMode==KopetePrefs::IconPic)
	{
		m_oldStatusIcon=d->metaContactIcon->pixmap();
		
		QPixmap icon = SmallIcon( m_metaContact->statusIcon(), d->iconSize );
		if ( doWeHaveToGrayThatContact )
		{
			// TODO: QPixmapCache this result
			KIconEffect::semiTransparent( icon );
		}

		d->metaContactIcon->setPixmap( icon );
		if(mBlinkTimer->isActive())
			m_originalBlinkIcon=icon;
	}
	// we only need to update the contact icon if one was supplied;
	// if none was supplied, we only need to update the MC appearance
	if ( c )
		updateContactIcon( c );
	else
		return;
}

void KopeteMetaContactLVI::catchEvent( Kopete::MessageEvent *event )
{
	d->events.append( event );

	connect( event, SIGNAL( done( Kopete::MessageEvent* ) ),
	         this, SLOT( slotEventDone( Kopete::MessageEvent * ) ) );

	if ( mBlinkTimer->isActive() )
		mBlinkTimer->stop();

	m_oldStatusIcon= d->metaContactIcon ? d->metaContactIcon->pixmap() : QPixmap();

	mBlinkTimer->start( 400, false );

	//show the contact if it was hidden because offline.
	updateVisibility();
 }

void KopeteMetaContactLVI::slotBlink()
{
	bool haveEvent = !d->events.isEmpty();
	if ( mIsBlinkIcon )
	{
		if(d->metaContactIcon)
			d->metaContactIcon->setPixmap( m_originalBlinkIcon );
		if ( !haveEvent && m_blinkLeft <= 0 )
		{
			mBlinkTimer->stop();
			m_oldStatusIcon=d->metaContactIcon ? d->metaContactIcon->pixmap() : QPixmap();
			updateVisibility();
			m_originalBlinkIcon=QPixmap(); //i hope this help to reduce memory consuption
		}
	}
	else
	{
		if(d->metaContactIcon)
			m_originalBlinkIcon=d->metaContactIcon->pixmap();
		if ( haveEvent )
		{
			if(d->metaContactIcon)
				d->metaContactIcon->setPixmap( SmallIcon( "newmsg", d->iconSize ) );
		}
		else
		{
			if(d->metaContactIcon)
				d->metaContactIcon->setPixmap( m_oldStatusIcon );
			m_blinkLeft--;
		}
	}

	mIsBlinkIcon = !mIsBlinkIcon;
}

void KopeteMetaContactLVI::slotEventDone( Kopete::MessageEvent *event )
{
	d->events.remove( event );

	if ( d->events.isEmpty() )
	{
		if ( mBlinkTimer->isActive() )
		{
			mBlinkTimer->stop();
			//If the contact gone offline while the timer was actif,
			//the visibility has not been correctly updated. so do it now
			updateVisibility();
		}

		if(d->metaContactIcon)
			d->metaContactIcon->setPixmap( m_originalBlinkIcon );
		m_originalBlinkIcon=QPixmap(); //i hope this help to reduce memory consuption
		mIsBlinkIcon = false;
	}
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
		rename( text );
	else
		KListViewItem::setText( column, text );
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:
