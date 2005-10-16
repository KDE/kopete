/*
    chatview.cpp - Chat View

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart @ kde.org>
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

#include "chatmemberslistwidget.h"
#include "chatmessagepart.h"
#include "chattexteditpart.h"
#include "kopetechatwindow.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetecontactlist.h"
#include "kopeteviewmanager.h"

#include <kconfig.h>
#include <ktabwidget.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstringhandler.h>
#include <kwin.h>
#include <kurldrag.h>
#include <kglobalsettings.h>
#include <kgenericfactory.h>
#include <khtmlview.h>
#include <ksyntaxhighlighter.h>
#include <q3scrollview.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PtrList>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <kvbox.h>

typedef KGenericFactory<ChatWindowPlugin> ChatWindowPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_chatwindow, ChatWindowPluginFactory( "kopete_chatwindow" )  )

ChatWindowPlugin::ChatWindowPlugin(QObject *parent, const char *name, const QStringList &) :
	Kopete::ViewPlugin( ChatWindowPluginFactory::instance(), parent, name )
{}

KopeteView* ChatWindowPlugin::createView( Kopete::ChatSession *manager )
{
    return (KopeteView*)new ChatView(manager,this);
}

class KopeteChatViewPrivate
{
public:
	QString captionText;
	QString statusText;
	bool isActive;
	bool sendInProgress;
	bool visibleMembers;
};

ChatView::ChatView( Kopete::ChatSession *mgr, ChatWindowPlugin *parent )
	 : KVBox( 0l ), KopeteView( mgr, parent )
{
	unsigned int i;

	d = new KopeteChatViewPrivate;
	d->isActive = false;
	d->visibleMembers = false;
	d->sendInProgress = false;

	KVBox *vbox=this;

	m_mainWindow = 0L;
	membersDock = new KHBox(vbox);
	membersStatus = Smart;
	m_tabState = Normal;


	//FIXME: don't widgets start off hidden anyway?
	hide();

	//Create the view dock widget (KHTML Part), and set it to no docking (lock it in place)	
	m_messagePart = new ChatMessagePart( mgr, membersDock );

	//Create the bottom dock widget, with the edit area, statusbar and send button
	m_editPart = new ChatTextEditPart( mgr, vbox );

	// FIXME: is this used these days? it seems totally unnecessary
	connect( editPart(), SIGNAL( toggleToolbar(bool)), this, SLOT(slotToggleRtfToolbar(bool)) );

	connect( editPart(), SIGNAL( messageSent( Kopete::Message & ) ),
	         this, SIGNAL( messageSent( Kopete::Message & ) ) );
	connect( editPart(), SIGNAL( canSendChanged( bool ) ),
	         this, SIGNAL( canSendChanged(bool) ) );
	connect( editPart(), SIGNAL( typing(bool) ),
		 mgr, SLOT( typing(bool) ) );

	//Set the view as the main widget
//	setView(viewDock);

	//It is possible to drag and drop on this widget.
	// I had to disable the acceptDrop in the khtml widget to be able to intercept theses events.
	setAcceptDrops(true);
//	viewDock->setAcceptDrops(false);

	m_remoteTypingMap.setAutoDelete( true );

	//Manager signals
	connect( mgr, SIGNAL( displayNameChanged() ),
	         this, SLOT( slotChatDisplayNameChanged() ) );
	connect( mgr, SIGNAL( contactAdded(const Kopete::Contact*, bool) ),
	         this, SLOT( slotContactAdded(const Kopete::Contact*, bool) ) );
	connect( mgr, SIGNAL( contactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ),
	         this, SLOT( slotContactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ) );
	connect( mgr, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & , const Kopete::OnlineStatus &) ),
	         this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
	connect( mgr, SIGNAL( remoteTyping( const Kopete::Contact *, bool) ),
		 this, SLOT( remoteTyping(const Kopete::Contact *, bool) ) );
	connect( mgr, SIGNAL( eventNotification( const QString& ) ),
		 this, SLOT( setStatusText( const QString& ) ) );

	//Connections to the manager and the ViewManager that every view should have
	connect( this, SIGNAL( closing( KopeteView * ) ),
		 KopeteViewManager::viewManager(), SLOT( slotViewDestroyed( KopeteView * ) ) );
	connect( this, SIGNAL( activated( KopeteView * ) ),
		 KopeteViewManager::viewManager(), SLOT( slotViewActivated( KopeteView * ) ) );
	connect( this, SIGNAL( messageSent(Kopete::Message &) ),
		 mgr, SLOT( sendMessage(Kopete::Message &) ) );
	connect( mgr, SIGNAL( messageSuccess() ),
		 this, SLOT( messageSentSuccessfully() ));

	// add contacts
	slotContactAdded( mgr->myself(), true );
	for ( i = 0; i != mgr->members().size(); i++ )
		slotContactAdded( mgr->members()[i], true );

	setFocusProxy( editPart()->widget() );
	editPart()->widget()->setFocus();

	// init actions
#warning TODO
#if 0
	KStdAction::copy( this, SLOT(copy()), actionCollection() );
	KStdAction::close( this, SLOT(closeView()),actionCollection() );
#endif 
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

	delete d;
}

KTextEdit *ChatView::editWidget()
{
	return editPart()->widget();
}

QWidget *ChatView::mainWidget()
{
	return this;
}

KParts::Part *ChatView::part() const
{
	return editPart();
}

bool ChatView::canSend()
{
	return editPart()->canSend();
}

Kopete::Message ChatView::currentMessage()
{
	return editPart()->contents();
}

void ChatView::setCurrentMessage( const Kopete::Message &message )
{
	editPart()->setContents( message );
}

void ChatView::cut()
{
	editPart()->edit()->cut();
}

void ChatView::copy()
{
	if ( messagePart()->hasSelection() )
		messagePart()->copy();
	else
		editPart()->edit()->copy();
}

void ChatView::paste()
{
	editPart()->edit()->paste();
}

void ChatView::nickComplete()
{
	return editPart()->complete();
}

void ChatView::addText( const QString &text )
{
	editPart()->addText( text );
}

void ChatView::clear()
{
	messagePart()->clear();
}

void ChatView::setBgColor( const QColor &newColor )
{
	editPart()->setBgColor( newColor );
}

void ChatView::setFont()
{
	editPart()->setFont();
}

QFont ChatView::font()
{
	return editPart()->font();
}

void ChatView::setFont( const QFont &font )
{
	editPart()->setFont( font );
}

void ChatView::setFgColor( const QColor &newColor )
{
	editPart()->setFgColor( newColor );
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
		if( KopetePrefs::prefs()->trayflashNotifySetCurrentDesktopToChatView() && activate )
			KWin::setCurrentDesktop( KWin::windowInfo( m_mainWindow->winId(), NET::WMDesktop ).desktop() );
		else
			KWin::setOnDesktop( m_mainWindow->winId(), KWin::currentDesktop() );

	if(m_mainWindow->isMinimized())
	{
		m_mainWindow->showNormal();
	}


	m_mainWindow->raise();

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
	{
		m_mainWindow->show();
		// scroll down post show and layout, otherwise the geometry is wrong to scroll to the bottom.
		m_messagePart->keepScrolledDown();
	}



	m_mainWindow->setActiveView( this );
}

bool ChatView::isVisible()
{
	return ( m_mainWindow && m_mainWindow->isVisible() );
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

void ChatView::updateChatState( KopeteTabState newState )
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

	emit updateChatState( this, newState );

	if( newState != Typing )
	{
		setStatusText( i18n( "One other person in the chat",
			       "%n other people in the chat", m_manager->members().count() ) );
	}
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
		m_membersList = new ChatMembersListWidget( m_manager, membersDock );

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
			d->visibleMembers = ( m_manager->members().count() > 1 );
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

void ChatView::placeMembersList( K3DockWidget::DockPosition dp )
{
// 	kdDebug(14000) << k_funcinfo << "Members list policy " << membersStatus <<
// 			", visible " << d->visibleMembers << endl;

	
#if 0
	if ( d->visibleMembers )
	{
		membersDockPosition = dp;

		// look up the dock width
		int dockWidth;
		KGlobal::config()->setGroup( QString::fromLatin1( "ChatViewDock" ) );

		if( membersDockPosition == K3DockWidget::DockLeft )
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
		membersDock->setEnableDocking( K3DockWidget::DockLeft | K3DockWidget::DockRight );
		membersDock->manualDock( viewDock, membersDockPosition, dockWidth );
		membersDock->show();
		membersDock->setEnableDocking( K3DockWidget::DockNone );
	}
	else
	{
		// Dock it to the desktop then hide it
		membersDock->undock();
		membersDock->hide();
	}

#endif

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
	Q3PtrDictIterator<QTimer> it( m_remoteTypingMap );

	for( ; it.current(); ++it )
	{
		Kopete::Contact *c = static_cast<Kopete::Contact*>( it.currentKey() );
		QString nick = c->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
		typingList.append( c->metaContact() ? c->metaContact()->displayName() : ( nick.isEmpty() ? c->contactId() : nick ) );
	}

	// Update the status area
	if( !typingList.isEmpty() )
	{
		if ( typingList.count() == 1 )
			setStatusText( i18n( "%1 is typing a message" ).arg( typingList.first() ) );
		else
		{
			QString statusTyping = typingList.join( QString::fromLatin1( ", " ) );
			setStatusText( i18n( "%1 is a list of names", "%1 are typing a message" ).arg( statusTyping ) );
		}
		updateChatState( Typing );
	}
	else
	{
		updateChatState();
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

void ChatView::slotChatDisplayNameChanged()
{
	// This fires whenever a contact or MC changes displayName, so only
	// update the caption if it changed to avoid unneeded updates that
	// could cause flickering
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
	}
}

void ChatView::slotContactAdded(const Kopete::Contact *contact, bool suppress)
{
	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	connect( contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
		this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;

	if( !suppress && m_manager->members().count() > 1 )
		sendInternalMessage(  i18n("%1 has joined the chat.").arg(contactName) );

	if( membersStatus == Smart && membersDock )
	{
		bool shouldShowMembers = ( m_manager->members().count() > 1);
		if( shouldShowMembers != d->visibleMembers )
		{
			d->visibleMembers = shouldShowMembers;
			placeMembersList( membersDockPosition );
		}
	}

	updateChatState();
	emit updateStatusIcon( this );
}

void ChatView::slotContactRemoved( const Kopete::Contact *contact, const QString &reason, Kopete::Message::MessageFormat format, bool suppressNotification )
{
// 	kdDebug(14000) << k_funcinfo << endl;
	if ( contact != m_manager->myself() )
	{
		m_remoteTypingMap.remove( const_cast<Kopete::Contact *>( contact ) );

		QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();

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

	updateChatState();
	emit updateStatusIcon( this );
}

QString& ChatView::caption() const
{
	 return d->captionText;
}

void ChatView::setCaption( const QString &text, bool modified )
{
// 	kdDebug(14000) << k_funcinfo << endl;
	QString newCaption = text;

	//Save this caption
	d->captionText = text;

	//Turncate if needed
	newCaption = KStringHandler::rsqueeze( d->captionText, 20 );

	//Call the original set caption
	QWidget::setCaption( newCaption );

	emit updateChatTooltip( this, QString::fromLatin1("<qt>%1</qt>").arg( d->captionText ) );
	emit updateChatLabel( this, newCaption );
	//Blink icon if modified and not active
	if( !d->isActive && modified )
		updateChatState( Changed );
	else
		updateChatState();

	//Tell the parent we changed our caption
	emit( captionChanged( d->isActive ) );
}

void ChatView::appendMessage(Kopete::Message &message)
{
	remoteTyping( message.from(), false );

	messagePart()->appendMessage(message);
	if( !d->isActive )
	{
		switch ( message.importance() )
		{
			case Kopete::Message::Highlight:
				updateChatState( Highlighted );
				break;
			case Kopete::Message::Normal:
				if ( message.direction() == Kopete::Message::Inbound )
				{
					updateChatState( Message );
					break;
				}
			default:
				updateChatState( Changed );
		}
	}

	if( message.direction() == Kopete::Message::Inbound )
	{
		unreadMessageFrom = message.from()->metaContact() ?
			 message.from()->metaContact()->displayName() : message.from()->contactId();
		QTimer::singleShot( 1000, this, SLOT( slotMarkMessageRead() ) );
	}
	else
		unreadMessageFrom = QString::null;
}

void ChatView::slotMarkMessageRead()
{
	unreadMessageFrom = QString::null;
}

void ChatView::slotToggleRtfToolbar( bool enabled )
{
	emit rtfEnabled( this, enabled );
}

void ChatView::slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus )
{
 	kdDebug(14000) << k_funcinfo << contact << endl;
	bool inhibitNotification = ( newStatus.status() == Kopete::OnlineStatus::Unknown ||
	                             oldStatus.status() == Kopete::OnlineStatus::Unknown );
	if ( contact && KopetePrefs::prefs()->showEvents() && !inhibitNotification )
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

	// update the windows caption
	slotChatDisplayNameChanged();
	emit updateStatusIcon( this );
}

void ChatView::sendInternalMessage(const QString &msg, Kopete::Message::MessageFormat format )
{
	// When closing kopete, some internal message may be sent because some contact are deleted
	// these contacts can already be deleted
	Kopete::Message message = Kopete::Message( 0L /*m_manager->myself()*/ , 0L /*m_manager->members()*/, msg, Kopete::Message::Internal, format );
	// (in many case, this is useless to set myself as contact)
	// TODO: set the contact which initiate the internal message,
	// so we can later show a icon of it (for example, when he join a chat)
	messagePart()->appendMessage( message );
}

void ChatView::sendMessage()
{
	d->sendInProgress = true;
	editPart()->sendMessage();
}

void ChatView::messageSentSuccessfully()
{
	d->sendInProgress = false;
	emit messageSuccess( this );
}

void ChatView::saveOptions()
{
	KConfig *config = KGlobal::config();

//	writeDockConfig ( config, QString::fromLatin1( "ChatViewDock" ) );
	config->setGroup( QString::fromLatin1( "ChatViewDock" ) );
	config->writeEntry( QString::fromLatin1( "membersDockPosition" ), membersDockPosition );
	config->sync();
}

void ChatView::readOptions()
{
	KConfig *config = KGlobal::config();

	/** THIS IS BROKEN !!! */
	//dockManager->readConfig ( config, QString::fromLatin1("ChatViewDock") );
#if 0
	//Work-around to restore dock widget positions
	config->setGroup( QString::fromLatin1( "ChatViewDock" ) );

	membersDockPosition = static_cast<K3DockWidget::DockPosition>(
		config->readNumEntry( QString::fromLatin1( "membersDockPosition" ), K3DockWidget::DockRight ) );

	QString dockKey = QString::fromLatin1( "viewDock" );
	if ( d->visibleMembers )
	{
		if( membersDockPosition == K3DockWidget::DockLeft )
			dockKey.prepend( QString::fromLatin1( "membersDock," ) );
		else if( membersDockPosition == K3DockWidget::DockRight )
			dockKey.append( QString::fromLatin1( ",membersDock" ) );
	}

	dockKey.append( QString::fromLatin1( ",editDock:sepPos" ) );
	//kdDebug(14000) << k_funcinfo << "reading splitterpos from key: " << dockKey << endl;
	int splitterPos = config->readNumEntry( dockKey, 70 );
	editDock->manualDock( viewDock, K3DockWidget::DockBottom, splitterPos );
	viewDock->setDockSite( K3DockWidget::DockLeft | K3DockWidget::DockRight );
	editDock->setEnableDocking( K3DockWidget::DockNone );
#endif
}

void ChatView::setActive( bool value )
{
	d->isActive = value;
	if ( d->isActive )
	{
		updateChatState( Normal );
		emit( activated( static_cast<KopeteView*>(this) ) );
	}
}

void ChatView::slotRemoteTypingTimeout()
{
	// Remove the topmost timer from the list. Why does QPtrDict use void* keys and not typed keys? *sigh*
	if ( !m_remoteTypingMap.isEmpty() )
		remoteTyping( reinterpret_cast<const Kopete::Contact *>( Q3PtrDictIterator<QTimer>(m_remoteTypingMap).currentKey() ), false );
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
			foreach(Kopete::Contact * cts, m_manager->members())
			{
				if(cts->contactId() == contact)
				{
					found=true;
					break;
				}
			}

			if(!found && contact != m_manager->myself()->contactId())
				event->accept();
		}
	}
	else if( event->provides( "kopete/x-metacontact" ) )
	{
#warning commented to make it compile
#if 0
		QString metacontactID=QString::fromUtf8(event->encodedData ( "kopete/x-metacontact" ));
		Kopete::MetaContact *m=Kopete::ContactList::self()->metaContact(metacontactID);

		if( m && m_manager->mayInvite())
		{
			cts=m->contacts();
			for ( i = 0; i != cts.size(); i++)
			{
				if(cts[i] && cts[i]->account() == m_manager->account())
				{
					if( cts[i] != m_manager->myself() &&  !m_manager->members().contains(cts[i])  && cts[i]->isOnline())
						event->accept();
				}
			}
		}
#endif
	}
	// make sure it doesn't come from the current chat view - then it's an emoticon
	else if ( event->provides( "text/uri-list" ) && m_manager->members().count() == 1 &&
				 ( event->source() != (QWidget*)m_messagePart->view()->viewport() ) )
	{
		Kopete::ContactPtrList members = m_manager->members();
		Kopete::Contact *contact = members.first();
		if ( contact && contact->canAcceptFiles() );
			event->accept();
	}
	else
		QWidget::dragEnterEvent(event);
}

void ChatView::dropEvent ( QDropEvent * event )
{
	QList<Kopete::Contact*> cts;
	unsigned int i;

	if( event->provides( "kopete/x-contact" ) )
	{
		QStringList lst=QStringList::split( QChar( 0xE000 ) , QString::fromUtf8(event->encodedData ( "kopete/x-contact" )) );
		if(m_manager->mayInvite() && m_manager->protocol()->pluginId() == lst[0] && m_manager->account()->accountId() == lst[1])
		{
			QString contact=lst[2];

			bool found =false;
			cts=m_manager->members();
			for ( i = 0; i != cts.size(); i++ )
			{
				if(cts[i]->contactId() == contact)
				{
					found=true;
					break;
				}
			}
			if(!found && contact != m_manager->myself()->contactId())
				m_manager->inviteContact(contact);
		}
	}
	else if( event->provides( "kopete/x-metacontact" ) )
	{
		
#warning commented to make it compile
#if 0
		QString metacontactID=QString::fromUtf8(event->encodedData ( "kopete/x-metacontact" ));
		Kopete::MetaContact *m=Kopete::ContactList::self()->metaContact(metacontactID);
		if(m && m_manager->mayInvite())
		{
			cts=m->contacts();
			for ( i = 0; i != cts.size(); i++ )
			{
				if(cts[i] && cts[i]->account() == m_manager->account() && cts[i]->isOnline())
				{
					if( cts[i] != m_manager->myself() &&  !m_manager->members().contains(cts[i]) )
						m_manager->inviteContact(c->contactId());
				}
			}
		}
#endif
	}
	else if ( event->provides( "text/uri-list" ) && m_manager->members().count() == 1 )
	{
		Kopete::ContactPtrList members = m_manager->members();
		Kopete::Contact *contact = members.first();

		if ( !contact || !contact->canAcceptFiles() || !Q3UriDrag::canDecode( event ) )
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
				addText( (*it).url() );
			}
		}
		event->acceptAction();
		return;
	}
	else
		QWidget::dropEvent(event);

}

void ChatView::registerContextMenuHandler( QObject *target, const char* slot )
{
	connect( m_messagePart,
		SIGNAL( contextMenuEvent( Kopete::Message &, const QString &, KMenu * ) ),
		target,
		slot
	);
}

void ChatView::registerTooltipHandler( QObject *target, const char* slot )
{
	connect( m_messagePart,
		SIGNAL( tooltipEvent( Kopete::Message &, const QString &, QString & ) ),
		target,
		slot
	);
}

#include "chatview.moc"

// vim: set noet ts=4 sts=4 sw=4:

