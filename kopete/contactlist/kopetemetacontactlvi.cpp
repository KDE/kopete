/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact KListViewItem

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar P     <duncan@kde.org>

    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

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
#include <qcolor.h>
#include <qimage.h>
#include <qlistview.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <knotifyclient.h>
#include <kiconeffect.h>

#include "kopetecontactlistview.h"
#include "kopetemetacontactlvi.h"
#include "kopetegroupviewitem.h"
#include "kopetecontactlist.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "addcontactpage.h"
#include "kopeteprotocol.h"
#include "kopeteplugin.h"
#include "pluginloader.h"
#include "kopeteevent.h"
#include "kopeteprefs.h"
#include "kopeteaway.h"

#if KDE_VERSION >= 0x030006
#include "systemtray.h"
#include "kpassivepopup.h"
#endif

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, KopeteGroupViewItem *parent )
	: QObject( contact ), KListViewItem( parent )
{
	m_metaContact = contact;
	m_isTopLevel = false;
	m_parentGroup = parent;
	m_parentView = 0L;

	initLVI();
	parent->refreshDisplayName();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListViewItem *parent )
: QObject( contact ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = 0L;

	initLVI();
}

KopeteMetaContactLVI::KopeteMetaContactLVI( KopeteMetaContact *contact, QListView *parent )
: QObject( contact ), KListViewItem( parent )
{
	m_metaContact = contact;

	m_isTopLevel = true;
	m_parentGroup = 0L;
	m_parentView = parent;

	initLVI();
}

void KopeteMetaContactLVI::initLVI()
{
/*	m_actionRemove = 0L;
	m_actionRemoveFromGroup = 0L;
	m_actionChat = 0L;
	m_actionSendMessage = 0L;
	m_actionInfo = 0L;
	m_actionBlock = 0L;
	m_actionHistory = 0L;
	m_actionRename = 0L;
	m_actionAddTemporaryContact = 0l;*/
	m_actionMove = 0L;
	m_actionCopy = 0L;
	m_actionAddContact = 0L;

	m_addContactPage=0L;

	connect( m_metaContact, SIGNAL( displayNameChanged( const QString &, const QString & ) ),
		SLOT( slotDisplayNameChanged() ) );

	connect( m_metaContact, SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotContactStatusChanged() ) );

	connect( m_metaContact, SIGNAL( contactStatusChanged( KopeteContact *, const KopeteOnlineStatus & ) ),
		SLOT( slotContactStatusChanged() ) );

	connect( m_metaContact, SIGNAL( contactAdded( KopeteContact * ) ),
		SLOT( slotContactStatusChanged() ) );

	connect( m_metaContact, SIGNAL( contactRemoved( KopeteContact * ) ),
		SLOT( slotContactStatusChanged() ) );

	connect( m_metaContact, SIGNAL( idleStateChanged( KopeteMetaContact *, KopeteMetaContact::IdleState ) ),
		SLOT( slotIdleStateChanged() ) );

	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( slotConfigChanged() ) );

	mBlinkTimer = new QTimer(this, "mBlinkTimer" );
	QObject::connect(mBlinkTimer, SIGNAL(timeout()), this, SLOT(slotBlink()));
	mBlinkIcon = KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "newmsg" ), KIcon::User);
	mIsBlinkIcon=false;
	m_event=0L;

	slotContactStatusChanged();
	slotDisplayNameChanged();
}

KopeteMetaContactLVI::~KopeteMetaContactLVI()
{
//	if(m_parentGroup)
//		m_parentGroup->refreshDisplayName();
}

/*void KopeteMetaContactLVI::setup()
{
	KListViewItem::setup();
	// Do custom layouting here...
	//setHeight( 150 );
}*/

void KopeteMetaContactLVI::movedToGroup(KopeteGroup *to)
{
	KopeteContactListView *lv =
		dynamic_cast<KopeteContactListView *>( listView() );
	if( !lv )
		return;

	if(m_parentGroup)
		m_parentGroup->refreshDisplayName();

	KopeteGroupViewItem *group_item = lv->getGroup( to );
	if(group_item)
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
	if( newName.isEmpty() )
	{
		//m_metaContact->setDisplayName( somethinginhere ); // FIXME: restore last displayname
//		m_metaContact->setTrackChildNameChanges( true );
	}
	else // user changed name manually, disable tracking of contact nickname and update displayname
	{
		m_metaContact->setTrackChildNameChanges( false );
		m_metaContact->setDisplayName( newName );
	}

	kdDebug( 14000 ) << k_funcinfo << "newName=" << newName <<
		", TrackChildNameChanges=" << m_metaContact->trackChildNameChanges() << endl;
}

void KopeteMetaContactLVI::showContextMenu( const QPoint &point )
{
	KPopupMenu *popup = new KPopupMenu();

	popup->insertTitle( i18n( "Format: 'nickname (online status)'",
		"%1 (%2)" ).arg( text( 0 ) ).arg( m_metaContact->statusString() ) );

	KAction *actionSendMessage = KopeteStdAction::sendMessage( m_metaContact,
		SLOT( sendMessage() ), popup, "actionMessage" );

	KAction *actionChat = KopeteStdAction::chat( m_metaContact,
		SLOT( startChat() ), popup, "actionChat" );

	KAction *actionHistory = KopeteStdAction::viewHistory( m_metaContact,
		SLOT( viewHistory() ), popup, "actionHistory" );

	actionSendMessage->plug( popup );
	actionChat->plug( popup );
	actionHistory->plug( popup );

	/*
	 * If the contact can accept file transfers, add this to the top popup window
	 * It will use the highest priority protocol to send the file
	 */
	if( m_metaContact->canAcceptFiles() )
	{
		KAction *actionFile = KopeteStdAction::sendFile( m_metaContact,
			SLOT( sendFile() ), popup, "actionSendFile" );
		actionFile->plug( popup );
	}

	//-- MetaContact actions
	KAction *m_actionRemoveGroup = 0L;
	if(isGrouped())
	{
		if(!m_actionMove)
			m_actionMove = KopeteStdAction::moveContact( this, SLOT( slotMoveToGroup() ), this, "m_actionMove" );
		if(!m_actionCopy)
			m_actionCopy = KopeteStdAction::copyContact( this, SLOT( slotAddToGroup() ), this, "m_actionCopy" );

		m_actionRemoveGroup = KopeteStdAction::deleteContact( this, SLOT( slotRemoveFromGroup() ), popup, "m_actionRemove" );
		m_actionRemoveGroup->setText( i18n("Remove From Group") );
	}

	KAction *m_actionRemove = KopeteStdAction::deleteContact( this, SLOT( slotRemoveThisUser() ), popup, "m_actionRemove" );
	KAction *m_actionRename = new KAction( i18n( "Rename Contact" ), "filesaveas", 0,
			this, SLOT( slotRename() ), popup, "m_actionRename" );
	if(!m_actionAddContact)
		m_actionAddContact = KopeteStdAction::addContact( this, SLOT( slotAddContact() ), this, "m_actionAddContact" );
	KAction *m_actionAddTemporaryContact = new KAction( i18n( "Add to your contact list" ), "bookmark_add", 0,
		this, SLOT( slotAddTemoraryContact() ), popup, "m_actionAddTemporaryContact" );

	if( m_actionRemoveGroup )
	{
		m_actionCopy->setEnabled( !m_metaContact->isTemporary());
		m_actionMove->setEnabled( !m_metaContact->isTemporary() );
		m_actionRemoveGroup->setEnabled( !m_metaContact->isTemporary() );
	}
	m_actionAddContact->setEnabled( !m_metaContact->isTemporary() );

	//remove the checkMark
	if( m_actionRemoveGroup )
	{
		m_actionMove->setCurrentItem( -1 );
		m_actionCopy->setCurrentItem( -1 );
	}
	m_actionAddContact->setCurrentItem( -1 );

	// Plug all items in the menu
	popup->insertSeparator();
	if( m_actionRemoveGroup )
	{
		m_actionMove->plug( popup );
		m_actionCopy->plug( popup );
	}

	if(m_metaContact->isTemporary())
		m_actionAddTemporaryContact->plug(popup);
	m_actionAddContact->plug(popup);

	m_actionRename->plug( popup );

	if( m_actionRemoveGroup )
		m_actionRemoveGroup->plug( popup );

	m_actionRemove->plug( popup );

	//-- Specific plugins actions
	bool sep=true;
	QPtrList<KopetePlugin> ps = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = ps.first() ; p ; p = ps.next() )
	{
		KActionCollection *customActions = p->customContextMenuActions(m_metaContact);
		if(customActions)
		{
			if(sep)
			{
				popup->insertSeparator();
				sep=false;
			}
			for(unsigned int i = 0; i < customActions->count(); i++)
			{
				customActions->action(i)->plug( popup );
			}
		}
	}

	//-- Submenus for separate contacts actions
	sep=true;
	QPtrList<KopeteContact> it = m_metaContact->contacts();
	for( KopeteContact *c = it.first(); c; c = it.next() )
	{
		if( sep )
		{
			popup->insertSeparator();
			sep = false;
		}
		KPopupMenu *contactMenu = it.current()->popupMenu();
		popup->insertChild( contactMenu );
		popup->insertItem( c->onlineStatus().iconFor( c, 16 ),
			i18n( "Format: 'displayName (id)'", "%2 (%1)" ).arg(
			c->contactId() ).arg( c->displayName() ),
			contactMenu );
	}

	popup->exec( point );
	delete popup;
}

void KopeteMetaContactLVI::slotContactStatusChanged()
{
	//kdDebug( 14000 ) << k_funcinfo << m_metaContact->displayName() << ": new status " << m_metaContact->status() << endl;

	KopetePrefs *prefs = KopetePrefs::prefs();

	QPixmap statusIcon = UserIcon( m_metaContact->statusIcon() );
	if( prefs->greyIdleMetaContacts() && m_metaContact->idleState() == KopeteMetaContact::Idle )
		KIconEffect::semiTransparent( statusIcon );

	setPixmap( 0, statusIcon );

	updateVisibility();

	if( m_parentGroup )
		m_parentGroup->refreshDisplayName();

#if KDE_VERSION > 0x030100
	// FIXME: All this code should be in kopetemetacontact.cpp.. having it in the LVI makes it all fire
	// multiple times if the user is in multiple groups - Jason
	QString event = (m_metaContact->status() == KopeteOnlineStatus::Online) ? "kopete_online" : "kopete_status_change";
	int winId = KopeteSystemTray::systemTray() ? KopeteSystemTray::systemTray()->winId() : 0;
	KNotifyClient::event( winId, event,
		i18n( "%2 is now %1!" ).arg( m_metaContact->statusString() ).arg( m_metaContact->displayName() ) );
#else
// ### All this should go away ASAP.
  #if KDE_VERSION >= 0x030006
	// Show passive popup when requested and the user's KDE version supports it
	// FIXME: when a contact is in several groups, this popup is showed multiple times
	if( m_metaContact->status() == KopeteOnlineStatus::Online )
	{
		if( prefs->showTray() )
		{
			KPassivePopup::message( i18n( "%2 is now %1!" ).arg( m_metaContact->statusString() ).arg(
				m_metaContact->displayName() ), QString::null, statusIcon, KopeteSystemTray::systemTray() );
		}
	}
  #endif

	if( m_metaContact->status() != KopeteOnlineStatus::Away || prefs->soundIfAway() )
	{
		KNotifyClient::event( "kopete_online",
            i18n( "%2 is now %1!" ).arg( m_metaContact->statusString() ).arg( m_metaContact->displayName() ) );
	}

#endif
}

void KopeteMetaContactLVI::execute() const
{
	m_metaContact->execute();
}

void KopeteMetaContactLVI::slotDisplayNameChanged()
{
	setText( 0, m_metaContact->displayName() );
}

void KopeteMetaContactLVI::slotRemoveThisUser()
{

	kdDebug(14000) << "KopeteMetaContactLVI::slotRemoveThisUser - removing user" << endl;
//	m_metaContact->removeThisUser();

	if( KMessageBox::questionYesNo( qApp->mainWidget(), i18n("Are you sure you want to remove"
		" %1 from your contact list?").arg( m_metaContact->displayName() ), i18n("Remove Contact")) == KMessageBox::Yes )
	{
		KopeteContactList::contactList()->removeMetaContact( m_metaContact );
	}
}

void KopeteMetaContactLVI::slotRemoveFromGroup()
{
	if(m_metaContact->isTemporary())
		return;

	m_metaContact->removeFromGroup( group() );
}

void KopeteMetaContactLVI::slotRename()
{
	setRenameEnabled(0, true);
	startRename(0);
}

void KopeteMetaContactLVI::okRename(int col)
{
	KListViewItem::okRename(col);
	emit rename(text(col));
	setRenameEnabled(col, false); // Yeah, that's on purpose...
}

void KopeteMetaContactLVI::slotMoveToGroup()
{
	if( m_actionMove && !m_metaContact->isTemporary() )
	{
		if( m_actionMove->currentItem() == 0 )
		{
			// we are moving to top-level
			if( group() != KopeteGroup::toplevel )
				m_metaContact->moveToGroup( group(), KopeteGroup::toplevel );
		}
		else
		{
			KopeteGroup *to = KopeteContactList::contactList()->getGroup( m_actionMove->currentText() );
			if( !m_metaContact->groups().contains( to ) )
				m_metaContact->moveToGroup( group(), to );
		}
	}
}

/*void KopeteMetaContactLVI::slotViewHistory()
{
	KMessageBox::information( 0, i18n( "This function is available only from the within individual contacts." ),
				  i18n( "Kopete" ) );
} */

void KopeteMetaContactLVI::slotAddToGroup()
{
	if( m_actionCopy )
	{
		kdDebug(14000) << "KopeteMetaContactLVI::slotAddToGroup " << endl;
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

void KopeteMetaContactLVI::slotAddToNewGroup()
{
	if( m_metaContact->isTemporary() )
		return;

	bool ok;
	QString groupName = KLineEditDlg::getText(
		i18n( "New Group - Kopete" ),
		i18n( "Please enter the name for the new group:" ),
		QString::null, &ok );
	if( ok )
		m_metaContact->addToGroup( KopeteContactList::contactList()->getGroup(groupName) );
}

void KopeteMetaContactLVI::slotAddContact()
{
	if( m_actionAddContact && !m_metaContact->isTemporary())
	{
		KopetePlugin *tmpprot = LibraryLoader::pluginLoader()->searchByName( m_actionAddContact->currentText() );
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>( tmpprot );
		if(!prot)
			kdDebug(14000) << "KopeteMetaContactLVI::slotAddContact : WARNING protocol not found" <<endl;
		else
		{
			KDialogBase *addDialog= new KDialogBase( qApp->mainWidget(),
				"addDialog", true, i18n( "Add Contact" ),
				KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );
			addDialog->resize( 543, 379 );

			m_addContactPage = prot->createAddContactWidget(addDialog);
			if (!m_addContactPage)
				kdDebug(14000) << "KopeteMetaContactLVI::slotAddContact : error while creating addcontactpage" <<endl;
			else
			{
				addDialog->setMainWidget(m_addContactPage);
				connect( addDialog, SIGNAL( okClicked()) , this, SLOT( slotAddDialogOkClicked() ) );
				addDialog->show();
			}
		}
	}
}

void KopeteMetaContactLVI::slotAddTemoraryContact()
{
	m_metaContact->setTemporary( false, KopeteGroup::toplevel );
}

void KopeteMetaContactLVI::slotConfigChanged()
{
	updateVisibility();
	slotIdleStateChanged();
	repaint();
}

void KopeteMetaContactLVI::updateVisibility()
{
	if ( KopetePrefs::prefs()->showOffline() /*|| mEventCount */)
		setVisible(true);
	else if ( m_metaContact->status() == KopeteOnlineStatus::Offline )
		setVisible(false);
	else
		setVisible(true);
}

void KopeteMetaContactLVI::paintCell( QPainter *p, const QColorGroup &cg,
	int column, int width, int align )
{
	if( column == 0 )
	{
		QPtrList<KopeteContact> contacts = m_metaContact->contacts();
		int cellWidth = width - ( contacts.count() * 16 ) - 4;
		if( cellWidth < 0 )
			cellWidth = 0;

		QColorGroup modcg = cg;
		if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleState() == KopeteMetaContact::Idle ) )
		{
			//FIXME: HARD CODED COLOR!
			modcg.setColor(QColorGroup::Text, darkGray);
		}
		QListViewItem::paintCell( p, modcg, column, cellWidth, align );
		QFontMetrics fm( p->font() );
		int pixelsWide = fm.width( text( 0 ) ) + 16 + 4;
		if( pixelsWide > cellWidth )
			pixelsWide = cellWidth;
		m_pixelWide = pixelsWide;

		// Draw the rest of the background
		QListView *lv = listView();
		if( !lv )
			return;

		int marg = lv->itemMargin();
		int r = marg;
		const BackgroundMode bgmode = lv->viewport()->backgroundMode();
		const QColorGroup::ColorRole crole =
			QPalette::backgroundRoleFromMode( bgmode );
		p->fillRect( cellWidth, 0, width - cellWidth, height(),
			cg.brush( crole ) );

		if( isSelected() &&
			( column == 0 || listView()->allColumnsShowFocus() ) )
		{
			p->fillRect( QMAX( cellWidth, r - marg ), 0,
				width - cellWidth - r + marg, height(),
				cg.brush( QColorGroup::Highlight ) );
			if( isEnabled() || !lv )
				p->setPen( cg.highlightedText() );
			else if( !isEnabled() && lv)
				p->setPen( lv->palette().disabled().highlightedText() );
		}

		// And last, draw the online status icons
		int mc_x = 0;
		QPtrListIterator<KopeteContact> it( contacts );
		for( ; it.current(); ++it )
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
		QListViewItem::paintCell( p, cg, column, width, align );
	}
}

KopeteContact *KopeteMetaContactLVI::getContactFromIcon( const QPoint &p )
{
	QPtrList<KopeteContact> contacts = m_metaContact->contacts();
	QPtrListIterator<KopeteContact> it( contacts );
	int mc_x = 0;
	for( ; it.current(); ++it )
	{
		if( QRect( mc_x + m_pixelWide+ 4, 0, 12, height() ).contains( p ) )
		{
			return *it;
		}
		mc_x += 16;
	}
	return 0L;
}

void KopeteMetaContactLVI::slotAddDialogOkClicked()
{
	if( m_addContactPage )
	{
		if(m_addContactPage->validateData())
			m_addContactPage->slotFinish(m_metaContact);

		delete m_addContactPage;
		m_addContactPage=0L;
	}
}

KopeteGroup *KopeteMetaContactLVI::group()
{
	if(m_parentGroup)
		if(m_parentGroup->group() && m_parentGroup->group() != KopeteGroup::toplevel)
			return m_parentGroup->group();

	return KopeteGroup::toplevel;
}


// Duncan Experiment
/*void KopeteMetaContactLVI::setText(int column,const QString &text)
{
	if (column!=1)
	{
		QListViewItem::setText(column,text);
	}
}

void KopeteMetaContactLVI::setColor(const QString &color)
{
	columnColor=QColor(color);
	repaint();
}

void KopeteMetaContactLVI::setColor(const QColor &color)
{
	columnColor=color;
	repaint();
}

QString KopeteMetaContactLVI::colorName()
{
	return columnColor.name();
}

QColor KopeteMetaContactLVI::color()
{
	return columnColor;
} */

QString KopeteMetaContactLVI::key( int, bool ) const
{
	char importance_char;
	switch (m_metaContact->status())
	{
	 case KopeteOnlineStatus::Online:
		importance_char = 'A';
		break;
	 case KopeteOnlineStatus::Away:
		importance_char = 'B';
		break;
	 case KopeteOnlineStatus::Offline:
		importance_char = 'C';
		break;
	 case KopeteOnlineStatus::Unknown:
	 default:
		importance_char = 'D';
	}

	return importance_char + text( 0 ).lower();
}

bool KopeteMetaContactLVI::isTopLevel()
{
	return m_isTopLevel;
}

bool KopeteMetaContactLVI::isGrouped()
{
	if(m_parentView)
		return true;
	if(!m_parentGroup)
		return false;
	if(!m_parentGroup->group())
		return false;
	if(m_parentGroup->group()==KopeteGroup::temporary && !KopetePrefs::prefs()->sortByGroup())
		return false;
	return true;
}

void KopeteMetaContactLVI::slotIdleStateChanged()
{
	QPixmap theIcon = UserIcon( m_metaContact->statusIcon() );
	if ( KopetePrefs::prefs()->greyIdleMetaContacts() && ( m_metaContact->idleState() == KopeteMetaContact::Idle ) )
	{
		KIconEffect::semiTransparent(theIcon);
	}

	setPixmap( 0, theIcon );

	if ( m_parentGroup )
		m_parentGroup->refreshDisplayName();
}

void KopeteMetaContactLVI::catchEvent(KopeteEvent *event)
{
	if(m_event)
	{
		return; //ignore the new event.
		//TODO: add a queue
	}
	m_event=event;
	connect(event , SIGNAL(done(KopeteEvent*)) , this , SLOT( slotEventDone(KopeteEvent *) ));

	if (mBlinkTimer->isActive())
		mBlinkTimer->stop();

	mBlinkTimer->start(1000, false);
}

void KopeteMetaContactLVI::slotBlink()
{
	if (mIsBlinkIcon)
	{
		setPixmap(0,UserIcon(m_metaContact->statusIcon()));
		mIsBlinkIcon = false;
	}
	else
	{
		setPixmap(0,mBlinkIcon);
		mIsBlinkIcon = true;
	}
}

void KopeteMetaContactLVI::slotEventDone(KopeteEvent* )
{
	m_event=0L;
	if (mBlinkTimer->isActive())
	{
		mBlinkTimer->stop();
	}
	setPixmap(0,UserIcon(m_metaContact->statusIcon()));
	mIsBlinkIcon = false;
}



#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:


