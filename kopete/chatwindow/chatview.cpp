/*
    chatview.cpp - Chat View

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

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

#include "chatview.h"

#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kcompletion.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstringhandler.h>
#include <kwin.h>
#include <kurldrag.h>

#include "chatmemberslistwidget.h"
#include "chatmessagepart.h"
#include "kopetechatwindow.h"
// FIXME: including a .cpp file !!!
#include "krichtexteditpart.cpp"
#include "kopetemessagemanager.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetexsl.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetecontactlist.h"

#include <ktabwidget.h>

class KopeteChatViewPrivate
{
public:
	QString captionText;
	QString statusText;
	bool isActive;
	bool sendInProgress;
	bool visibleMembers;
};

ChatView::ChatView( Kopete::ChatSession *mgr, const char *name )
	 : KDockMainWindow( 0L, name, 0L ), KopeteView( mgr ), editpart(0)
{
	d = new KopeteChatViewPrivate;

	historyPos = -1;
	m_type = Kopete::Message::Chat;
	m_mainWindow = 0L;
	membersDock = 0L;
	d->isActive = false;
	m_tabBar = 0L;
	membersStatus = Smart;
	m_tabState=Normal;
	d->visibleMembers = false;
	d->sendInProgress = false;
	mComplete = new KCompletion();
	mComplete->setIgnoreCase( true );
	mComplete->setOrder( KCompletion::Weighted );
	
	//FIXME: don't widgets start off hidden anyway?
	hide();
	
	//Create the view dock widget (KHTML Part), and set it to no docking (lock it in place)
	viewDock = createDockWidget(QString::fromLatin1( "viewDock" ), QPixmap(),
		0L,QString::fromLatin1("viewDock"), QString::fromLatin1(" "));
	m_messagePart = new ChatMessagePart( mgr, viewDock, "m_messagePart" );

	viewDock->setWidget(messagePart()->widget());
	viewDock->setDockSite(KDockWidget::DockBottom);
	viewDock->setEnableDocking(KDockWidget::DockNone);

	//Create the bottom dock widget, with the edit area, statusbar and send button
	editDock = createDockWidget( QString::fromLatin1( "editDock" ), QPixmap(),
		0L, QString::fromLatin1("editDock"), QString::fromLatin1(" ") );
	editpart = new KopeteRichTextEditPart( editDock, "kopeterichtexteditpart",
		mgr->protocol()->capabilities() );
	connect( editpart, SIGNAL( toggleToolbar(bool)), this, SLOT(slotToggleRtfToolbar(bool)) );

	m_edit = static_cast<KTextEdit*>( editpart->widget() );

	//Set params on the edit widget
	m_edit->setMinimumSize( QSize( 75, 20 ) );
	m_edit->setWordWrap( QTextEdit::WidgetWidth );
	m_edit->setWrapPolicy( QTextEdit::AtWhiteSpace );
	m_edit->setAutoFormatting( QTextEdit::AutoNone );

	//Make the edit area dockable for now
	editDock->setWidget( m_edit );
	editDock->setDockSite( KDockWidget::DockNone );
	editDock->setEnableDocking(KDockWidget::DockBottom);

	//Set the view as the main widget
	setMainDockWidget( viewDock );
	setView(viewDock);
	
	//It is possible to drag and drop on this widget.
	// I had to disable the acceptDrop in the khtml widget to be able to intercept theses events.
	setAcceptDrops(true);
	viewDock->setAcceptDrops(false);
	
	// some signals and slots connections
	connect( m_edit, SIGNAL( textChanged()), this, SLOT( slotTextChanged() ) );

	// Timers for typing notifications
	m_typingRepeatTimer = new QTimer(this, "m_typingRepeatTimer");
	m_typingStopTimer   = new QTimer(this, "m_typingStopTimer");

	m_remoteTypingMap.setAutoDelete( true );

	connect( m_typingRepeatTimer, SIGNAL( timeout() ),
	         this, SLOT( slotRepeatTimer() ) );
	connect( m_typingStopTimer,   SIGNAL( timeout() ),
	         this, SLOT( slotStopTimer() ) );

	connect( mgr, SIGNAL( displayNameChanged() ),
	         this, SLOT( slotChatDisplayNameChanged() ) );
	connect( mgr, SIGNAL( contactAdded(const Kopete::Contact*, bool) ),
	         this, SLOT( slotContactAdded(const Kopete::Contact*, bool) ) );
	connect( mgr, SIGNAL( contactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ),
	         this, SLOT( slotContactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ) );
	connect( mgr, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & , const Kopete::OnlineStatus &) ),
	         this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );

	setFocusProxy( m_edit );
	m_edit->setFocus();

	// init actions
	copyAction  = KStdAction::copy( this, SLOT(copy()), actionCollection() );

	setCaption( m_manager->displayName(), false );

	// restore docking positions
	readOptions();

	// maybe show chat members
	createMembersList();
}

ChatView::~ChatView()
{
	emit( closing( static_cast<KopeteView*>(this) ) );

	saveOptions();

	delete mComplete;

	delete d;
}

void ChatView::raise( bool activate )
{
	// this shouldn't change the focus. When the window is raised when a new message arrives
	// if i am coding, or talking to someone else, i want to end my sentence before switching to
	// the other chat. i just want to KNOW and SEE the other chat to switch to it later
	// (except if activate==true)

	if ( !m_mainWindow || !m_mainWindow->isActiveWindow() || activate )
		makeVisible();

	if ( !KWin::windowInfo( m_mainWindow->winId(), NET::WMDesktop ).onAllDesktops() )
		KWin::setOnDesktop( m_mainWindow->winId(), KWin::currentDesktop() );

	m_mainWindow->show();
	//raise() and show() should normally deIconify the window. but it doesn't do that here due
	// to a bug in QT or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
	if ( m_mainWindow->isMinimized() )
		KWin::deIconifyWindow( m_mainWindow->winId() );
	KWin::raiseWindow( m_mainWindow->winId() );

	/* Removed Nov 2003
	According to Zack, the user double-clicking a contact is not valid reason for a non-pager
	to grab window focus. While I don't agree with this, and it runs contradictory to every other
	IM out there, commenting this code out to agree with KWin policy.

	Redirect any bugs relating to the widnow now not grabbing focus on clicking a contact to KWin.
		- Jason K
	*/

	//Will not activate window if user was typing
	if ( activate )
		KWin::activateWindow( m_mainWindow->winId() );

}

void ChatView::makeVisible()
{
	if ( !m_mainWindow )
	{
		m_mainWindow = KopeteChatWindow::window( m_manager );
// 		if ( root )
// 			root->repaint( true );
		emit windowCreated();
	}

	if ( !m_mainWindow->isVisible() )
		m_mainWindow->show();

	m_mainWindow->setActiveView( this );
}

bool ChatView::isVisible()
{
	return ( m_mainWindow && m_mainWindow->isVisible() && ( d->isActive || docked() ) );
}

bool ChatView::visibleMembersList()
{
	return d->visibleMembers;
}

bool ChatView::sendInProgress()
{
	return d->sendInProgress;
}

bool ChatView::closeView( bool force )
{
	int response = KMessageBox::Continue;

	if ( !force )
	{
		if ( m_manager->members().count() > 1 )
		{
			QString shortCaption = d->captionText;
			shortCaption = KStringHandler::rsqueeze( shortCaption );

			response = KMessageBox::warningContinueCancel( this, i18n("<qt>You are about to leave the group chat session <b>%1</b>.<br>"
				"You will not receive future messages from this conversation.</qt>").arg( shortCaption ), i18n( "Closing Group Chat" ),
				i18n( "Cl&ose Chat" ), QString::fromLatin1( "AskCloseGroupChat" ) );
		}

		if ( !unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel( this, i18n("<qt>You have received a message from <b>%1</b> in the last "
				"second. Are you sure you want to close this chat?</qt>").arg( unreadMessageFrom ), i18n( "Unread Message" ),
				i18n( "Cl&ose Chat" ), QString::fromLatin1("AskCloseChatRecentMessage" ) );
		}

		if ( d->sendInProgress && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel( this, i18n( "You have a message send in progress, which will be "
				"aborted if this chat is closed. Are you sure you want to close this chat?" ), i18n( "Message in Transit" ),
				i18n( "Cl&ose Chat" ), QString::fromLatin1( "AskCloseChatMessageInProgress" ) );
		}
	}

	if( response == KMessageBox::Continue )
	{
		// Remove the widget from the window it's attached to
		// and schedule it for deletion
		if( m_mainWindow )
			m_mainWindow->detachChatView( this );
		deleteLater();

		return true;
	}

	return false;
}

void ChatView::setTabState( KopeteTabState newState )
{
	if ( newState == Undefined )
		newState = m_tabState;
	else if ( newState != Typing && ( newState != Changed || ( m_tabState != Message && m_tabState != Highlighted ) )
		&& ( newState != Message ||  m_tabState != Highlighted ) )
	{ //if the new state is not a typing state and we don't already have a message or a highlighted message
	  //change the tab state
		m_tabState = newState;
	}

	newState = m_remoteTypingMap.isEmpty() ? m_tabState : Typing ;

	if( m_tabBar )
	{
		switch( newState )
		{
			case Highlighted:
				m_tabBar->setTabColor( this, Qt::blue );
				break;

			case Message:
				m_tabBar->setTabColor( this, Qt::red );
				break;

			case Changed:
				m_tabBar->setTabColor( this, Qt::darkRed );
				break;

			case Typing:
				m_tabBar->setTabColor( this, Qt::darkGreen );
				break;

			case Normal:
			default:
				m_tabBar->setTabColor( this, KGlobalSettings::textColor() );
				break;
		}
	}

	if( newState != Typing )
		setStatusText( i18n( "One person in the chat", "%n people in the chat", m_manager->members().count() ) );
}

void ChatView::setMainWindow( KopeteChatWindow* parent )
{
	m_mainWindow = parent;
}

void ChatView::createMembersList()
{
	if ( !membersDock )
	{
		//Create the chat members list
		membersDock = createDockWidget( QString::fromLatin1( "membersDock" ), QPixmap(), 0L,
			QString::fromLatin1( "membersDock" ), QString::fromLatin1( " " ) );
		m_membersList = new ChatMembersListWidget( m_manager, this, "m_membersList" );

		membersDock->setWidget( m_membersList );

		Kopete::ContactPtrList members = m_manager->members();

		if ( members.first() && members.first()->metaContact() != 0 )
		{
			membersStatus = static_cast<MembersListPolicy>
			(
				members.first()->metaContact()->pluginData
				( m_manager->protocol(), QString::fromLatin1( "MembersListPolicy" ) ).toInt()
			);
		}
		else
		{
			membersStatus = Smart;
		}

		if( membersStatus == Smart )
			d->visibleMembers = ( m_manager->members().count() > 2 );
		else
			d->visibleMembers = ( membersStatus == Visible );

		placeMembersList( membersDockPosition );
	}
}

void ChatView::toggleMembersVisibility()
{
	if( membersDock )
	{
		d->visibleMembers = !d->visibleMembers;
		membersStatus = d->visibleMembers ? Visible : Hidden;
		placeMembersList( membersDockPosition );
		Kopete::ContactPtrList members = m_manager->members();
		if ( members.first()->metaContact() )
		{
			members.first()->metaContact()->setPluginData( m_manager->protocol(),
				QString::fromLatin1( "MembersListPolicy" ), QString::number(membersStatus) );
		}
		//refreshView();
	}
}

void ChatView::placeMembersList( KDockWidget::DockPosition dp )
{
	kdDebug(14000) << k_funcinfo << "Members list policy " << membersStatus <<
			", visible " << d->visibleMembers << endl;

	if ( d->visibleMembers )
	{
		membersDockPosition = dp;

		// look up the dock width
		int dockWidth;
		KGlobal::config()->setGroup( QString::fromLatin1( "ChatViewDock" ) );

		if( membersDockPosition == KDockWidget::DockLeft )
		{
			dockWidth = KGlobal::config()->readNumEntry(
				QString::fromLatin1( "membersDock,viewDock:sepPos" ), 30);
		}
		else
		{
			dockWidth = KGlobal::config()->readNumEntry(
				QString::fromLatin1( "viewDock,membersDock:sepPos" ), 70);
		}

		// Make sure it is shown then place it wherever
		membersDock->setEnableDocking( KDockWidget::DockLeft | KDockWidget::DockRight );
		membersDock->manualDock( viewDock, membersDockPosition, dockWidth );
		membersDock->show();
		membersDock->setEnableDocking( KDockWidget::DockNone );
	}
	else
	{
		// Dock it to the desktop then hide it
		membersDock->undock();
		membersDock->hide();
	}

	if( d->isActive )
		m_mainWindow->updateMembersActions();

	//refreshView();
}

void ChatView::remoteTyping( const Kopete::Contact *contact, bool isTyping )
{
	// Make sure we (re-)add the timer at the end, because the slot will
	// remove the first timer
	// And yes, the const_cast is a bit ugly, but it's only used as key
	// value in this dictionary (no indirections) so it's basically
	// harmless. Unfortunately there's no QConstPtrDictionary in Qt...
	void *key = const_cast<Kopete::Contact *>( contact );
	m_remoteTypingMap.remove( key );
	if( isTyping )
	{
		m_remoteTypingMap.insert( key, new QTimer(this) );
		connect( m_remoteTypingMap[ key ], SIGNAL( timeout() ), SLOT( slotRemoteTypingTimeout() ) );
		m_remoteTypingMap[ key ]->start( 6000, true );
	}

	// Loop through the map, constructing a string of people typing
	QStringList typingList;
	QString statusTyping;
	QPtrDictIterator<QTimer> it( m_remoteTypingMap );

	for( ; it.current(); ++it )
	{
		Kopete::Contact *c = static_cast<Kopete::Contact*>( it.currentKey() );
		QString nick = c->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
		typingList.append( c->metaContact() ? c->metaContact()->displayName() : ( nick.isEmpty() ? c->contactId() : nick ) );
	}

	statusTyping = typingList.join( QString::fromLatin1( ", " ) );

	// Update the status area
	if( !typingList.isEmpty() )
	{
		setStatusText( i18n( "%1 is typing a message", "%1 are typing a message", typingList.count() ).arg( statusTyping ) );
		setTabState( Typing );
	}
	else
	{
		setTabState();
	}
}

void ChatView::setStatusText( const QString &status )
{
	d->statusText = status;
	if ( d->isActive )
		m_mainWindow->setStatus( status );
}

const QString& ChatView::statusText()
{
	return d->statusText;
}

// NAUGHTY, BAD AND WRONG! (but needed to fix nick complete bugs)
#include <private/qrichtext_p.h>
class EvilTextEdit : public KTextEdit
{
public:
	// grab the paragraph as plain text - very very evil.
	QString plainText( int para )
	{
		QString str = document()->paragAt( para )->string()->toString();
		// str includes an extra space on the end (from the newline character?) - remove it
		return str.left( str.length() - 1 );
	}
};

void ChatView::nickComplete()
{
	int para = 1, parIdx = 1;
	m_edit->getCursorPosition( &para, &parIdx);

	// FIXME: strips out all formatting
	QString txt = static_cast<EvilTextEdit*>(m_edit)->plainText( para );

	if ( parIdx > 0 )
	{
		int firstSpace = txt.findRev( QRegExp( QString::fromLatin1("\\s\\S+") ), parIdx - 1 ) + 1;
		int lastSpace = txt.find( QRegExp( QString::fromLatin1("[\\s\\:]") ), firstSpace );
		if( lastSpace == -1 )
			lastSpace = txt.length();

		QString word = txt.mid( firstSpace, lastSpace - firstSpace );
		QString match;

		kdDebug(14000) << k_funcinfo << word << " from '" << txt << "'" << endl;

		if ( word != m_lastMatch )
		{
			match = mComplete->makeCompletion( word );
			m_lastMatch = QString::null;
			parIdx -= word.length();
		}
		else
		{
			match = mComplete->nextMatch();
			parIdx -= m_lastMatch.length();
		}

		if ( !match.isNull() && !match.isEmpty() )
		{
			QString rightText = txt.right( txt.length() - lastSpace );

			if ( para == 0 && firstSpace == 0 && rightText[0] != QChar(':') )
			{
				rightText = match + QString::fromLatin1(": ") + rightText;
				parIdx += 2;
			}
			else
				rightText = match + rightText;

			// insert *before* remove. this is becase Qt adds an extra blank line
			// if the rich text control becomes empty (if you remove the only para).
			// disable updates while we change the contents to eliminate flicker.
			m_edit->setUpdatesEnabled( false );
			m_edit->insertParagraph( txt.left(firstSpace) + rightText, para );
			m_edit->removeParagraph( para + 1 );
			m_edit->setCursorPosition( para, parIdx + match.length() );
			m_edit->setUpdatesEnabled( true );
			// must call this rather than update because QTextEdit is broken :(
			m_edit->updateContents();
			m_lastMatch = match;
		}
	}
}

void ChatView::slotChatDisplayNameChanged()
{
	//This fires whenever a contact or MC changes displayName, so only
	//update the caption if it changed to avoid unneeded updates that
	//could cause flickering
	QString chatName = m_manager->displayName();
	if ( chatName != d->captionText )
		setCaption( chatName, true );
}

void ChatView::slotPropertyChanged( Kopete::Contact*, const QString &key,
		const QVariant& oldValue, const QVariant &newValue  )
{
	if ( key == Kopete::Global::Properties::self()->nickName().key() )
	{
		QString newName=newValue.toString();
		QString oldName=oldValue.toString();

		if(KopetePrefs::prefs()->showEvents())
			if ( oldName != newName && !oldName.isEmpty())
				sendInternalMessage( i18n( "%1 is now known as %2" ). arg( oldName, newName ) );

		mComplete->removeItem( oldName );
		mComplete->addItem( newName );
	}
}

void ChatView::slotContactAdded(const Kopete::Contact *contact, bool suppress)
{
	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	connect( contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
		this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;

	mComplete->addItem( contactName );

	if( !suppress && m_manager->members().count() > 1 )
		sendInternalMessage(  i18n("%1 has joined the chat.").arg(contactName) );

	if( membersStatus == Smart && membersDock )
	{
		bool shouldShowMembers = ( m_manager->members().count() > 2 );
		if( shouldShowMembers != d->visibleMembers )
		{
			d->visibleMembers = shouldShowMembers;
			placeMembersList( membersDockPosition );
		}
	}

	setTabState();
	emit updateStatusIcon( this );
}

void ChatView::slotContactRemoved( const Kopete::Contact *contact, const QString &reason, Kopete::Message::MessageFormat format, bool suppressNotification )
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( contact != m_manager->user() )
	{
		m_remoteTypingMap.remove( const_cast<Kopete::Contact *>( contact ) );

		QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
		mComplete->removeItem( contactName );

		// When the last person leaves, don't disconnect the signals, since we're in a one-to-one chat
		if ( m_manager->members().count() > 0 )
		{
			disconnect(contact,SIGNAL(propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & )),
				this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;
		}

		if ( !suppressNotification )
		{
			if ( reason.isEmpty() )
				sendInternalMessage( i18n( "%1 has left the chat." ).arg( contactName ), format ) ;
			else
				sendInternalMessage( i18n( "%1 has left the chat (%2)." ).arg( contactName, reason ), format);
		}
	}

	setTabState();
	emit updateStatusIcon( this );
}

QString& ChatView::caption() const
{
	 return d->captionText;
}

void ChatView::setCaption( const QString &text, bool modified )
{
	kdDebug(14000) << k_funcinfo << endl;
	QString newCaption = text;

	//Save this caption
	d->captionText = text;

	//Turncate if needed
	newCaption = KStringHandler::rsqueeze( d->captionText, 20 );
	
	//Call the original set caption
	KDockMainWindow::setCaption( newCaption, false );

	if( m_tabBar )
	{
		m_tabBar->setTabToolTip( this, QString::fromLatin1("<qt>%1</qt>").arg( d->captionText ) );
		m_tabBar->setTabLabel( this, newCaption  );

		//Blink icon if modified and not active
		if( !d->isActive && modified )
			setTabState( Changed );
		else
			setTabState();
	}

	//Tell the parent we changed our caption
	emit( captionChanged( d->isActive ) );
}

void ChatView::appendMessage(Kopete::Message &message)
{
	remoteTyping( message.from(), false );

	messagePart()->appendMessage( message );
	if( !d->isActive )
	{
		switch ( message.importance() )
		{
			case Kopete::Message::Highlight:
				setTabState( Highlighted );
				break;
			case Kopete::Message::Normal:
				if ( message.direction() == Kopete::Message::Inbound )
				{
					setTabState( Message );
					break;
				}
			default:
				setTabState( Changed );
		}
	}

	if( !d->sendInProgress || message.from() != m_manager->user() )
	{
		unreadMessageFrom = message.from()->metaContact() ?
			 message.from()->metaContact()->displayName() : message.from()->contactId();
		QTimer::singleShot( 1000, this, SLOT( slotMarkMessageRead() ) );
	}
}

void ChatView::slotMarkMessageRead()
{
	unreadMessageFrom = QString::null;
}

void ChatView::slotToggleRtfToolbar( bool enabled )
{
	if ( enabled )
		m_mainWindow->toolBar( "formatToolBar" )->show();
	else
		m_mainWindow->toolBar( "formatToolBar" )->hide();
}

bool ChatView::canSend()
{
	if ( !m_manager ) return false;

	// can't send if there's nothing *to* send...
	if ( m_edit->text().isEmpty() )
		return false;

	Kopete::ContactPtrList members = m_manager->members();
	
	// if we can't send offline, make sure we have a reachable contact...
	if ( !( m_manager->protocol()->capabilities() & Kopete::Protocol::CanSendOffline ) )
	{
		bool reachableContactFound = false;

		//TODO: does this perform badly in large / busy IRC channels? - no, doesn't seem to
		QPtrListIterator<Kopete::Contact> it ( members );
		for( ; it.current(); ++it )
		{
			if ( (*it)->isReachable() )
			{
				reachableContactFound = true;
				break;
			}
		}

		// no online contact found and can't send offline? can't send.
		if ( !reachableContactFound )
			return false;
	}

	return true;
}

void ChatView::slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus )
{
	kdDebug(14000) << k_funcinfo << endl;
	if ( contact && KopetePrefs::prefs()->showEvents() )
	{
		if ( contact->account() && contact == contact->account()->myself() )
		{
			// Separate notification for the 'self' contact
			if ( newStatus.status() != Kopete::OnlineStatus::Connecting )
				sendInternalMessage( i18n( "You are now marked as %1." ).arg( newStatus.description() ) );
		}
		else if ( !contact->account() || !contact->account()->suppressStatusNotification() )
		{
			// Don't send notifications when we just connected ourselves, i.e. when suppressions are still active
			if ( contact->metaContact() )
			{
				sendInternalMessage( i18n( "%2 is now %1." )
					.arg( newStatus.description(), contact->metaContact()->displayName() ) );
			}
			else
			{
				QString nick=contact->property(Kopete::Global::Properties::self()->nickName().key()).value().toString();
				sendInternalMessage( i18n( "%2 is now %1." )
					.arg( newStatus.description(), nick.isEmpty() ? contact->contactId() : nick  ) );
			}
		}
	}

	if ( m_tabBar )
	{
		QPtrList<Kopete::Contact> chatMembers = m_manager->members();
		Kopete::Contact *tempContact = 0L;
		for ( QPtrListIterator<Kopete::Contact> it ( chatMembers ); it.current(); ++it )
		{
			if ( !tempContact || tempContact->onlineStatus() < (*it)->onlineStatus() )
				tempContact = (*it);
		}
		if ( tempContact )
			m_tabBar->setTabIconSet( this, m_manager->contactOnlineStatus( tempContact ).iconFor( tempContact ) );
	}

	// update the windows caption
	slotChatDisplayNameChanged();
	emit updateStatusIcon( this );


	if ( ( oldStatus.status() == Kopete::OnlineStatus::Offline )
	  != ( newStatus.status() == Kopete::OnlineStatus::Offline ) )
	{
		emit canSendChanged();
	}
}

void ChatView::sendInternalMessage(const QString &msg, Kopete::Message::MessageFormat format )
{
	// When closing kopete, some internal message may be sent because some contact are deleted
	// these contacts can already be deleted
	Kopete::Message message = Kopete::Message( 0L /*m_manager->user()*/ , 0L /*m_manager->members()*/, msg, Kopete::Message::Internal, format );
	// (in many case, this is useless to set myself as contact)
	// TODO: set the contact which initiate the internal message,
	// so we can later show a icon of it (for example, when he join a chat)
	messagePart()->appendMessage( message );
}

void ChatView::sendMessage()
{
	d->sendInProgress = true;

	QString txt = editpart->text( Qt::PlainText );
	if ( m_lastMatch.isNull() && ( txt.find( QRegExp( QString::fromLatin1("^\\w+:\\s") ) ) > -1 ) )
	{ //no last match and it finds something of the form of "word:" at the start of a line
		QString search = txt.left( txt.find(':') );
		if( !search.isEmpty() )
		{
			QString match = mComplete->makeCompletion( search );
			if( !match.isNull() )
				m_edit->setText( txt.replace(0,search.length(),match) );
		}
	}

	if ( !m_lastMatch.isNull() )
	{
		mComplete->addItem( m_lastMatch );
		m_lastMatch = QString::null;
	}

	Kopete::Message sentMessage = currentMessage();
	emit messageSent( sentMessage );
	historyList.prepend( m_edit->text() );
	historyPos = -1;
	editpart->clear();
	emit canSendChanged();
	slotStopTimer();
}

void ChatView::messageSentSuccessfully()
{
	d->sendInProgress = false;
	emit ( messageSuccess( this ) );
}

bool ChatView::isTyping()
{
	QString txt = editpart->text( Qt::PlainText );

	//Make sure the message is empty. QString::isEmpty()
	//returns false if a message contains just whitespace
	//which is the reason why we strip the whitespace	
	return !txt.stripWhiteSpace().isEmpty();
}

void ChatView::slotTextChanged()
{
	if ( isTyping() )
	{
		// And they were previously typing
		if( !m_typingRepeatTimer->isActive() )
		{
			m_typingRepeatTimer->start( 4000, false );
			slotRepeatTimer();
		}

		// Reset the stop timer again, regardless of status
		m_typingStopTimer->start( 4500, true );
	}

	canSendChanged();
}

void ChatView::historyUp()
{
	QString txt = editpart->text( Qt::PlainText );
	bool empty = txt.stripWhiteSpace().isEmpty();
	QString textToSave = m_edit->text();

	if ( historyPos == -1 )
	{
		if ( !empty ) //if we've typed something...
		{
			historyList.prepend( textToSave ); //...save it
			if ( historyList.count() > 1)
				historyPos = 1;
			else
				historyPos = 0;
		}
		else if ( historyList.count() != 0 )
			historyPos = 0;
	}
	else
	{
		if ( !empty )
			historyList[historyPos] = textToSave;
		if ( historyPos < historyList.count() - 1 )
			historyPos++;
	}
	if ( historyPos != -1 )
	{
		m_edit->setText( historyList[historyPos] );
		m_edit->moveCursor( QTextEdit::MoveEnd, false );
	}
}

void ChatView::historyDown()
{
	QString txt = m_edit->text( Qt::PlainText );
	bool empty = txt.stripWhiteSpace().isEmpty();
	QString textToSave = m_edit->text();

	if ( historyPos == -1 )
	{
		if ( !empty ) //if we've typed something...
		{
			historyList.prepend( textToSave ); //...save it
			m_edit->setText( QString::null );
		}
	}
	else
	{
		if ( !empty )
			historyList[historyPos] = textToSave;
		historyPos--;
		if ( historyPos >= 0 )
		{
			m_edit->setText( historyList[historyPos] );
			m_edit->moveCursor( QTextEdit::MoveEnd, false );
		}
		else
			m_edit->setText( QString::null );
	}
}

void ChatView::saveOptions()
{
	KConfig *config = KGlobal::config();

	writeDockConfig ( config, QString::fromLatin1( "ChatViewDock" ) );
	config->setGroup( QString::fromLatin1( "ChatViewDock" ) );
	config->writeEntry( QString::fromLatin1( "membersDockPosition" ), membersDockPosition );
	config->sync();
}

void ChatView::readOptions()
{
	KConfig *config = KGlobal::config();

	/** THIS IS BROKEN !!! */
	//dockManager->readConfig ( config, QString::fromLatin1("ChatViewDock") );

	//Work-around to restore dock widget positions
	config->setGroup( QString::fromLatin1( "ChatViewDock" ) );

	membersDockPosition = static_cast<KDockWidget::DockPosition>(
		config->readNumEntry( QString::fromLatin1( "membersDockPosition" ), KDockWidget::DockRight ) );

	QString dockKey = QString::fromLatin1( "viewDock" );
	if ( d->visibleMembers )
	{
		if( membersDockPosition == KDockWidget::DockLeft )
			dockKey.prepend( QString::fromLatin1( "membersDock," ) );
		else if( membersDockPosition == KDockWidget::DockRight )
			dockKey.append( QString::fromLatin1( ",membersDock" ) );
	}
	
	dockKey.append( QString::fromLatin1( ",editDock:sepPos" ) );
	//kdDebug(14000) << k_funcinfo << "reading splitterpos from key: " << dockKey << endl;
	int splitterPos = config->readNumEntry( dockKey, 70 );
	editDock->manualDock( viewDock, KDockWidget::DockBottom, splitterPos );
	viewDock->setDockSite( KDockWidget::DockLeft | KDockWidget::DockRight );
	editDock->setEnableDocking( KDockWidget::DockNone );
}

void ChatView::addText( const QString &text )
{
	m_edit->insert( text );
}

void ChatView::clear()
{
	messagePart()->clear();
}

void ChatView::setActive( bool value )
{
	d->isActive = value;
	if ( d->isActive )
	{
		setTabState( Normal );
		emit( activated( static_cast<KopeteView*>(this) ) );
	}
}

void ChatView::setTabBar( KTabWidget *tabBar )
{
	m_tabBar = tabBar;
}

void ChatView::setCurrentMessage( const Kopete::Message &message )
{
	m_edit->setText( message.plainBody() );

	setFont( message.font() );
	setFgColor( message.fg() );
	setBgColor( message.bg() );
}

Kopete::Message ChatView::currentMessage()
{
	Kopete::Message currentMsg = Kopete::Message( m_manager->user(), m_manager->members(),
		 				m_edit->text(), Kopete::Message::Outbound,
						editpart->richTextEnabled() ? Kopete::Message::RichText : Kopete::Message::PlainText );

	currentMsg.setBg( editpart->bgColor() );
	currentMsg.setFg( editpart->fgColor() );
	currentMsg.setFont( editpart->font() );

	return currentMsg;
}

void ChatView::cut()
{
	m_edit->cut();
}

void ChatView::copy()
{
	if ( messagePart()->hasSelection() )
		messagePart()->copy();
	else
		m_edit->copy();
}

void ChatView::paste()
{
	m_edit->paste();
}

void ChatView::setBgColor( const QColor &newColor )
{
	editpart->setBgColor( newColor );
}

void ChatView::setFont()
{
	editpart->setFont();
}

void ChatView::setFont( const QFont &font )
{
	editpart->setFont( font );
}

void ChatView::setFgColor( const QColor &newColor )
{
	editpart->setFgColor( newColor );
}


void ChatView::slotRepeatTimer()
{
	emit typing( true );
}

void ChatView::slotRemoteTypingTimeout()
{
	// Remove the topmost timer from the list. Why does QPtrDict use void* keys and not typed keys? *sigh*
	if ( !m_remoteTypingMap.isEmpty() )
		remoteTyping( reinterpret_cast<const Kopete::Contact *>( QPtrDictIterator<QTimer>(m_remoteTypingMap).currentKey() ), false );
}

void ChatView::slotStopTimer()
{
	m_typingRepeatTimer->stop();
	emit typing( false );
}

void ChatView::dragEnterEvent ( QDragEnterEvent * event )
{
	if( event->provides( "kopete/x-contact" ) )
	{
		QStringList lst=QStringList::split( QChar( 0xE000 ) , QString::fromUtf8(event->encodedData ( "kopete/x-contact" )) );
		if(m_manager->mayInvite() && m_manager->protocol()->pluginId() == lst[0] && m_manager->account()->accountId() == lst[1])
		{
			QString contact=lst[2];
						
			bool found =false;
			QPtrList<Kopete::Contact> cts=m_manager->members();
			for ( QPtrListIterator<Kopete::Contact> it( cts ); it.current(); ++it )
			{
				if(it.current()->contactId() == contact)
				{
					found=true;
					break;
				}
			}
		
			if(!found && contact != m_manager->user()->contactId())
				event->accept();
		}
	}
	else if( event->provides( "kopete/x-metacontact" ) )
	{
		QString metacontactID=QString::fromUtf8(event->encodedData ( "kopete/x-metacontact" ));
		Kopete::MetaContact *m=Kopete::ContactList::self()->metaContact(metacontactID);

		if( m && m_manager->mayInvite())
		{
			QPtrList<Kopete::Contact> cts=m->contacts();
			for ( QPtrListIterator<Kopete::Contact> it( cts ); it.current(); ++it )
			{
				Kopete::Contact *c=it.current();
				if(c && c->account() == m_manager->account())
				{
					if( c != m_manager->user() &&  !m_manager->members().contains(c)  && c->isOnline())
						event->accept();
				}
			}
		}
	}
	else if ( event->provides( "text/uri-list" ) && m_manager->members().count() == 1 )
	{
		Kopete::ContactPtrList members = m_manager->members();
		Kopete::Contact *contact = members.first();
		if ( contact && contact->canAcceptFiles() );
			event->accept();
	}
	else 
		KDockMainWindow::dragEnterEvent(event);
}

void ChatView::dropEvent ( QDropEvent * event )
{
	if( event->provides( "kopete/x-contact" ) )
	{
		QStringList lst=QStringList::split( QChar( 0xE000 ) , QString::fromUtf8(event->encodedData ( "kopete/x-contact" )) );
		if(m_manager->mayInvite() && m_manager->protocol()->pluginId() == lst[0] && m_manager->account()->accountId() == lst[1])
		{
			QString contact=lst[2];
			
			bool found =false;
			QPtrList<Kopete::Contact> cts=m_manager->members();
			for ( QPtrListIterator<Kopete::Contact> it( cts ); it.current(); ++it )
			{
				if(it.current()->contactId() == contact)
				{
					found=true;
					break;
				}
			}
			if(!found && contact != m_manager->user()->contactId())
				m_manager->inviteContact(contact);
		}
	}
	else if( event->provides( "kopete/x-metacontact" ) )
	{
		QString metacontactID=QString::fromUtf8(event->encodedData ( "kopete/x-metacontact" ));
		Kopete::MetaContact *m=Kopete::ContactList::self()->metaContact(metacontactID);
		if(m && m_manager->mayInvite())
		{
			QPtrList<Kopete::Contact> cts=m->contacts();
			for ( QPtrListIterator<Kopete::Contact> it( cts ); it.current(); ++it )
			{
				Kopete::Contact *c=it.current();
				if(c && c->account() == m_manager->account() && c->isOnline())
				{
					if( c != m_manager->user() &&  !m_manager->members().contains(c) )
						m_manager->inviteContact(c->contactId());
				}
			}
		}
	}
	else if ( event->provides( "text/uri-list" ) && m_manager->members().count() == 1 )
	{
		Kopete::ContactPtrList members = m_manager->members();
		Kopete::Contact *contact = members.first();
		
		if ( !contact || !contact->canAcceptFiles() || !QUriDrag::canDecode( event )  )
		{
			event->ignore();
			return;
		}

		KURL::List urlList;
		KURLDrag::decode( event, urlList );

		for ( KURL::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
		{
			if ( (*it).isLocalFile() )
			{ //send a file
				contact->sendFile( *it );
			}
			else
			{ //this is a URL, send the URL in a message
				m_edit->insert( (*it).url() );
			}
		}
		event->acceptAction();
		return;
	}
	else
		KDockMainWindow::dropEvent(event);

}

#include "chatview.moc"

// vim: set noet ts=4 sts=4 sw=4:

