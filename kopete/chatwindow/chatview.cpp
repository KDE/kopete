/*
    chatview.cpp - Chat View

    Copyright (c) 2002      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qclipboard.h>
#include <qheader.h>

#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_base.h>
#include <dom/html_inline.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kapplication.h>
#include <krun.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <krootpixmap.h>
#include <ktempfile.h>
#include <kiconeffect.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kcompletion.h>
#include <kpopupmenu.h>

#include "chatview.h"
#include "kopetechatwindow.h"
#include "kopeteprotocol.h"
#include "kopetemessagemanager.h"
#include "kopetemetacontact.h"
#include "kopeteprefs.h"
#include "kopetexsl.h"
#include "pluginloader.h"
#include "ktabwidget.h"



ChatView::ChatView( KopeteMessageManager *mgr, const char *name )
	 : KDockMainWindow( 0L, name, 0L ), KopeteView( mgr ), editpart(0)
{
	hide();

	//Create the view dock widget (KHTML Part), and set it to no docking (lock it in place)
	viewDock = createDockWidget(QString::fromLatin1( "viewDock" ), QPixmap(),
		0L,QString::fromLatin1("viewDock"), QString::fromLatin1(" "));
	chatView = new KHTMLPart( viewDock, "chatView" );

	//Security settings, we don't need this stuff
	chatView->setJScriptEnabled( false ) ;
	chatView->setJavaEnabled( false );
	chatView->setPluginsEnabled( false );
	chatView->setMetaRefreshEnabled( false );


	chatView->begin();
	chatView->write( QString::fromLatin1( "<html><head><style>") + styleHTML() +
		QString::fromLatin1("</style></head><body></body></html>") );
	chatView->end();

	htmlWidget = chatView->view();
	viewDock->setWidget(htmlWidget);
	viewDock->setDockSite(KDockWidget::DockBottom);
	viewDock->setEnableDocking(KDockWidget::DockNone);

	//Create the bottom dock widget, with the edit area, statusbar and send button
	editDock = createDockWidget( QString::fromLatin1( "editDock" ), QPixmap(),
		0L, QString::fromLatin1("editDock"), QString::fromLatin1(" ") );

	if(KopetePrefs::prefs()->richText())
	{
		KLibFactory *factory = KLibLoader::self()->factory("libkrichtexteditpart");
		if ( factory )
		{
			editpart = dynamic_cast<KParts::Part*> (
				factory->create( editDock, "krichtexteditpart",
					"KParts::ReadWritePart" ) );
		}
	}

	// FIXME: This can't be a sane way to customize a KPart, find something better
	if ( editpart )
	{
		QDomDocument doc = editpart->domDocument();
		QDomNode menu = doc.documentElement().firstChild();
		menu.removeChild( menu.firstChild() ); // Remove File
		menu.removeChild( menu.firstChild() ); // Remove Edit
		menu.removeChild( menu.firstChild() ); // Remove View
		menu.removeChild( menu.lastChild() ); //Remove Help

		doc.documentElement().removeChild( doc.documentElement().childNodes().item(1) ); //Remove MainToolbar
		doc.documentElement().removeChild( doc.documentElement().lastChild() ); // Remove Edit popup

		m_edit = static_cast<KTextEdit*>( editpart->widget() );
	}
	else
	{
		m_edit = new KTextEdit( editDock, "m_edit" );
	}

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

	// some signals and slots connections
	connect( m_edit, SIGNAL( textChanged()), this, SLOT( slotTextChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(transparencyChanged()),
		this, SLOT( slotTransparencyChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(messageAppearanceChanged()),
		this, SLOT( slotRefreshNodes() ) );
	connect( KopetePrefs::prefs(), SIGNAL(windowAppearanceChanged()),
		this, SLOT( slotRefreshView() ) );

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
	connect( mgr, SIGNAL( contactAdded(const KopeteContact*, bool) ),
		this, SLOT( slotContactAdded(const KopeteContact*, bool) ) );
	connect( mgr, SIGNAL( contactRemoved(const KopeteContact*, const QString&) ),
		this, SLOT( slotContactRemoved(const KopeteContact*, const QString&) ) );

	connect ( chatView->browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
		this, SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );

	connect( chatView, SIGNAL(popupMenu(const QString &, const QPoint &)),
		this, SLOT(slotRightClick(const QString &, const QPoint &)) );
	connect( htmlWidget, SIGNAL(contentsMoving(int,int)),
		this, SLOT(slotScrollingTo(int,int)) );

	htmlWidget->setFocusPolicy( NoFocus );
	setFocusProxy( m_edit );
	m_edit->setFocus();

	// Finalize
	historyPos = -1;
	m_type = KopeteMessage::Chat;
	m_mainWindow = 0L;
	membersDock = 0L;
	backgroundFile = QString::null;
	root = 0L;
	isActive = false;
	m_tabBar = 0L;
	messageId = 0;
	bgChanged = false;
	m_tabState=Normal;

//	m_icon = SmallIcon( mgr->protocol()->pluginIcon() );
//	m_iconLight = KIconEffect().apply( m_icon, KIconEffect::ToGamma, 0.5, Qt::white, true );
	m_sendInProgress = false;
	scrollPressed = false;
	mComplete = new KCompletion();
	mComplete->setIgnoreCase( true );
	mComplete->setOrder( KCompletion::Weighted );


	//initActions
	copyAction  = KStdAction::copy( this, SLOT(copy()), actionCollection());
	saveAction = KStdAction::save ( this, SLOT(save()), actionCollection());
	printAction = KStdAction::print ( this, SLOT(print()),actionCollection());
	closeAction = KStdAction::close ( this, SLOT(closeView()),actionCollection());
	copyURLAction = new KAction( i18n("Copy Link Location"),QString::fromLatin1("editcopy"),0,this, SLOT(slotCopyURL()),actionCollection());

	setCaption( m_manager->displayName(), false );

	//Restore docking positions
	readOptions();

	//Show chat members
	createMembersList();

	slotTransparencyChanged();
}

ChatView::~ChatView()
{
	emit( closing( static_cast<KopeteView*>(this) ) );

	saveOptions();

	delete mComplete;
}

void ChatView::raise(bool activate)
{
	//this shouldn't change the focus. When the window is reased when a new mesage arrive
	// if i am coding, or talking to someone else, i want to end my sentence before switch to
	// the other chat. i just want to KNOW and SEE the other chat to switch to it right later
	// (exepted if activate==true)

	if(!m_mainWindow || !m_mainWindow->isActiveWindow() || activate)
		makeVisible();

	if( !KWin::info( m_mainWindow->winId() ).onAllDesktops )
		KWin::setOnDesktop( m_mainWindow->winId(), KWin::currentDesktop() );

	m_mainWindow->show();
	//raise() and show() should normaly deIconify the window. but it doesn't do here due
	// to a bug in QT or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
	if(m_mainWindow->isMinimized())
		KWin::deIconifyWindow(m_mainWindow->winId() );
	m_mainWindow->raise();
	if(activate)
		m_mainWindow->setActiveWindow();  //this set the focus to the window
}

void ChatView::slotScrollingTo( int /*x*/, int y)
{
	int scrolledTo = y + htmlWidget->visibleHeight();
	if( scrolledTo >= ( htmlWidget->contentsHeight() - 10 ) )
		scrollPressed = false;
	else
		scrollPressed = true;
}

void ChatView::save()
{
	QString fileName = KFileDialog::getSaveFileName ( QString::null, QString::fromLatin1( "text/xml" ),
		this, i18n( "Save Conversation" ) );

	if ( fileName.isEmpty() )
		return;

	QFile file ( fileName );
	if ( file.open(IO_WriteOnly) )
	{
		QTextStream stream ( &file );
		QString xmlString;
		for( QValueList<KopeteMessage>::Iterator it = messageList.begin(); it != messageList.end(); ++it)
			xmlString += (*it).asXML().toString();
		stream << QString::fromLatin1("<document>") + xmlString + QString::fromLatin1("</document>");
		file.close(); // maybe unneeded but I like to close opened files ;)
	}
	else
	{
		KMessageBox::error( 0, //No parent
			i18n("Could not open %1 for writing").arg(fileName), // Message
			i18n("Error While Saving") ); //Caption
	}
}

void ChatView::makeVisible()
{
	if( !m_mainWindow )
	{
		m_mainWindow = KopeteChatWindow::window( m_manager->account() );
		if( root )
			root->repaint( true );
	}

	if( !m_mainWindow->isVisible() )
		m_mainWindow->show();

	m_mainWindow->setActiveView( this );
}

bool ChatView::isVisible()
{
	return ( m_mainWindow && m_mainWindow->isVisible() && ( isActive || docked() ) );
}

bool ChatView::closeView( bool force )
{
	int response = KMessageBox::Continue;

	if( !force )
	{
		if( m_manager->members().count() > 1 )
		{
			QString shortCaption = m_captionText;
			if( shortCaption.length() > 40 )
				shortCaption = shortCaption.left( 40 ) + QString::fromLatin1("...");

			response = KMessageBox::warningContinueCancel(this, i18n("You are about to leave the group chat session \"%1\"."
				"You will not receive future messages from this conversation").arg(shortCaption), i18n("Closing Group Chat"),
				i18n("Cl&ose Chat"), i18n("Do not ask me this again"));
		}

		if( !unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have received a message from \"%1\" in the last "
				"second, are you sure you want to close this chat?").arg(unreadMessageFrom), i18n("Unread Message"),
				i18n("Cl&ose Chat"), i18n("Do not ask me this again"));
		}

		if( m_sendInProgress  && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have a message send in progress, which will be "
				"aborted if this chat is closed. Are you sure you want to close this chat?"), i18n("Message in Transit"),
				i18n("Cl&ose Chat"), i18n("Do not ask me this again"));
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

void ChatView::setTabState( KopeteTabState newState  )
{
	if( m_tabBar )
	{
		if( newState == Undefined )
			newState = m_tabState;

		switch( newState )
		{
			case Highlighted:
				m_tabBar->setLabelTextColor( this, Qt::blue );
				break;

			case Message:
				if( m_tabState != Highlighted )
					m_tabBar->setLabelTextColor( this, Qt::red );
				break;

			case Changed:
				if( m_tabState != Highlighted && m_tabState != Message  )
					m_tabBar->setLabelTextColor( this, Qt::darkRed );
				break;

			case Typing:
				if( m_tabState != Highlighted && m_tabState != Message  )
					m_tabBar->setLabelTextColor( this, Qt::darkGreen );
				break;

			case Normal:
			default:
				m_tabBar->setLabelTextColor( this, KGlobalSettings::textColor() );
				break;
		}

		if( newState != Typing &&  (  newState!=Changed || (m_tabState != Message && m_tabState != Highlighted) ) && ( newState != Message ||  m_tabState != Highlighted ) )
			m_tabState = newState;
	}
	if( newState!= Typing )
		setStatus ( i18n( "%1 people in the chat" ).arg( memberContactMap.count()  ) );

}

void ChatView::setMainWindow( KopeteChatWindow* parent )
{
	m_mainWindow = parent;
	if( root )
	{
		disconnect(root, SIGNAL(backgroundUpdated(const QPixmap &)), this, SLOT(slotUpdateBackground(const QPixmap &)));
		delete root;
		root = 0L;
		slotTransparencyChanged();
	}
}

void ChatView::createMembersList(void)
{
	if( !membersDock )
	{
		//Create the chat members list
		membersDock = createDockWidget( QString::fromLatin1( "membersDock" ), QPixmap(), 0L,
			QString::fromLatin1( "membersDock" ), QString::fromLatin1( " " ) );
		membersList = new KListView(this);
		membersList->setAllColumnsShowFocus( true );
		membersList->addColumn( QString::null, 18);
		membersList->addColumn( i18n("Chat Members"), -1 );
		membersList->setSorting( 0, true );
		membersList->header()->setStretchEnabled( true, 1 );
		membersList->header()->hide();
		//Add the contacts that are in the message manager
		KopeteContact *contact;
		KopeteContactPtrList chatMembers = m_manager->members();

		for ( contact = chatMembers.first(); contact; contact = chatMembers.next() )
			slotContactAdded( contact, true );

		slotContactAdded( m_manager->user(), true);

		membersDock->setWidget(membersList);

		//Dock the chatmembers list. If this is a group chat, show it always initially
		// TODO: Respond to member list visibility hint here
		if ( false )
			visibleMembers = ( (m_manager->members()).count() > 1 );

		placeMembersList( membersDockPosition );

		//Connect the popup menu
		connect( membersList, SIGNAL( contextMenu( KListView*, QListViewItem *, const QPoint &) ),
			SLOT( slotContactsContextMenu(KListView*, QListViewItem *, const QPoint & ) ) );
	}
}

void ChatView::placeMembersList( KDockWidget::DockPosition dp )
{
	membersDockPosition = dp;
	showMembersList( visibleMembers );
	refreshView();
}

void ChatView::toggleMembersVisibility()
{
	bool newVisibility;
	if ( visibleMembers )
		newVisibility = false;
	else
		newVisibility = true;

	showMembersList( newVisibility );
	refreshView();
}

void ChatView::showMembersList( bool visible )
{
	if ( visible )
	{
		// look up the dock width
		int dockWidth;
		KGlobal::config()->setGroup( QString::fromLatin1("ChatViewDock") );
		if( membersDockPosition == KDockWidget::DockLeft )
			dockWidth = KGlobal::config()->readNumEntry( QString::fromLatin1("membersDock,viewDock:sepPos"), 30);
		else
			dockWidth = KGlobal::config()->readNumEntry( QString::fromLatin1("viewDock,membersDock:sepPos"), 70);

		// TODO: Why is it necessary to reset the dockings here?
		// Make sure it is shown then place it wherever
		membersDock->setEnableDocking( KDockWidget::DockLeft | KDockWidget::DockRight );
		membersDock->manualDock( viewDock, membersDockPosition, dockWidth );
		membersDock->show();
		membersDock->setEnableDocking( KDockWidget::DockNone );
		visibleMembers = true;
	}
	else
	{
		// Dock it to the desktop then hide it
		membersDock->undock();
		membersDock->hide();
		visibleMembers = false;
	}
}



void ChatView::slotContactsContextMenu( KListView*, QListViewItem *item, const QPoint &point )
{
	KopeteContactLVI *contactLVI = dynamic_cast<KopeteContactLVI*>( item );
	if (contactLVI) {
		KPopupMenu *p = ((KopeteContact*)contactLVI->contact())->popupMenu();
		p->exec( point );
		delete p;
	}
}

void ChatView::remoteTyping( const KopeteContact *c, bool isTyping )
{
	// Ensure this contact is in the typing map
	// Strictly speaking the below code does that, but contactAdded() does
	// some additional bookkeeping, hence this call
	if( !typingMap.contains( c ) )
	{
		kdDebug( 14000 ) << k_funcinfo << "WARNING: contact was not in the typing map" << endl;
		slotContactAdded( c, false );
	}

	// Set his typing status
	typingMap[ c ] = isTyping;

	// Make sure we (re-)add the timer at the end, because the slot will
	// remove the first timer
	// And yes, the const_cast is a bit ugly, but it's only used as key
	// value in this dictionary (no indirections) so it's basically
	// harmless. Unfortunately there's no QConstPtrDictionary in Qt...
	void *key = const_cast<KopeteContact *>( c );
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
	QMap<const KopeteContact*, bool>::Iterator it;
	for( it = typingMap.begin(); it != typingMap.end(); ++it )
	{
		// FIXME: is it really possible to have null pointers here? The check
		// seems unneeded to me - Martijn
		if( !it.data() )
			continue;

		typingList.append( it.key()->metaContact() ? it.key()->metaContact()->displayName() : it.key()->displayName() );
	}
	statusTyping = typingList.join( QString::fromLatin1( ", " ) );

	// Update the status area
	if( !typingList.isEmpty() )
	{
		setStatus ( i18n( "%1 is typing a message", "%1 are typing a message", typingList.count() ).arg( statusTyping ) );
		setTabState( Typing );
	}
	else
	{
		setTabState();
	}
}

void ChatView::setStatus( const QString &status )
{
	m_status = status;
	if( isActive )
		m_mainWindow->setStatus( status );
}

void ChatView::pageUp()
{
	htmlWidget->scrollBy( 0, -htmlWidget->visibleHeight() );
}

void ChatView::pageDown()
{
	htmlWidget->scrollBy( 0, htmlWidget->visibleHeight() );
}

void ChatView::nickComplete()
{
	int firstSpace, lastSpace, para = 1, parIdx = 1;
	m_edit->getCursorPosition( &para, &parIdx);
	QString txt = m_edit->text( para );

	if( parIdx > 0 )
	{
		firstSpace = txt.findRev( QRegExp( QString::fromLatin1("\\s\\S+") ), parIdx - 1 ) + 1;
		lastSpace = txt.find( QRegExp( QString::fromLatin1("[\\s\\W]") ), firstSpace );
		if( lastSpace == -1 )
			lastSpace = txt.length();

		QString word = txt.mid( firstSpace, lastSpace - firstSpace );
		QString m_Match;

//		kdDebug( 14000 ) <<  "Word is '" << word << "', last match is '" << m_lastMatch << "'" << endl;

		if( word != m_lastMatch )
		{
			m_Match = mComplete->makeCompletion( word );
			m_lastMatch = QString::null;
		}
		else
			m_Match = mComplete->nextMatch();

		if( !m_Match.isNull() && !m_Match.isEmpty() )
		{
			QString fullText = m_edit->text();
			QString rightText = fullText.right( fullText.length() - lastSpace );
			if( para == 0 && firstSpace == 0 )
			{
				rightText = m_Match + QString::fromLatin1(": ");
				parIdx += 2;
			}
			else
				rightText = m_Match + rightText;

			fullText += rightText;
			m_edit->setText( fullText.left(firstSpace) + rightText );
			m_edit->setCursorPosition( para, parIdx + (m_Match.length() - m_lastMatch.length()) );
			m_lastMatch = m_Match;
		}
	}
}

void ChatView::slotChatDisplayNameChanged()
{
	//This fires whenever a contact or MC changes displayName, so only
	//update the caption if it changed to avioud unneeded updates that
	//could cause flickering
	QString chatName = m_manager->displayName();
	if( chatName != m_captionText )
		setCaption( chatName, true );

	emit updateStatusIcon( this );
}

void ChatView::slotContactNameChanged( const QString &oldName, const QString &newName )
{
	if( KopetePrefs::prefs()->showEvents() )
		sendInternalMessage( i18n( "%1 changed his or her nickname to %2" ).
#if QT_VERSION < 0x030200
			arg( oldName ).arg( newName )
#else
			arg( oldName, newName )
#endif
		);
	mComplete->removeItem( oldName );
	mComplete->addItem( newName );
}

void ChatView::slotContactAdded(const KopeteContact *c, bool surpress)
{
	if( !memberContactMap.contains(c) )
	{
		QString contactName;
		contactName = c->displayName();
		connect( c, SIGNAL( displayNameChanged( const QString &,const QString & ) ),
			this, SLOT( slotContactNameChanged( const QString &,const QString & ) ) );

		mComplete->addItem( contactName );

		connect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus & , const KopeteOnlineStatus &) ),
			this, SLOT( slotContactStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ) );

		typingMap.insert( c, false );

		if( !surpress && memberContactMap.count() > 1 )
		{
			sendInternalMessage(  i18n("%1 has joined the chat.").arg(contactName) );
		}

		memberContactMap.insert(c, new KopeteContactLVI( this, c, membersList ) );
	}
	setTabState();
}

void ChatView::slotContactRemoved( const KopeteContact *c, const QString& reason )
{
	if( memberContactMap.contains(c) && (c != m_manager->user()) )
	{
		typingMap.remove( c );
		m_remoteTypingMap.remove( const_cast<KopeteContact *>( c ) );

		QString contactName;
		contactName = c->displayName();
		disconnect( c, SIGNAL( displayNameChanged( const QString &,const QString & ) ),
			this, SLOT( slotContactNameChanged( const QString &,const QString & ) ) );

		mComplete->removeItem( contactName );


		disconnect( c, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
			this, SLOT( slotContactStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ) );

		//FIXME: bugs if the nickname contains %1 again
		if(reason.isNull())
			sendInternalMessage( i18n( "%1 has left the chat." ).arg(contactName) ) ;
		else
			sendInternalMessage( i18n( "%1 has left the chat (%2).").
#if QT_VERSION < 0x030200
			arg( contactName ).arg( reason )
#else
			arg( contactName, reason )
#endif
		);

		delete memberContactMap[c];
		memberContactMap.remove(c);
	}
	setTabState();
}

void ChatView::setCaption( const QString &text, bool modified )
{
	QString newCaption = text;

	//Save this caption
	m_captionText = text;

	//Turncate if needed
	if( newCaption.length() > 20 )
		newCaption = newCaption.left( 20 ).append( QString::fromLatin1( "..." ) );

	//Call the original set caption
	KDockMainWindow::setCaption( newCaption, false );

	if( m_tabBar )
	{
		m_tabBar->setTabToolTip( this, QString::fromLatin1("<qt>%1</qt>").arg( m_captionText ) );
		m_tabBar->setTabLabel( this, newCaption  );

		//Blink icon if modified and not active
		if( !isActive && modified )
			setTabState( Changed );
		else
			setTabState();
	}

	//Tell the parent we changed our caption
	emit( captionChanged( isActive ) );
}

void ChatView::appendMessage(KopeteMessage &message)
{
	//Note: it may happend that message not from contact in the list are hapened.
	//  this is for example the case with the history
	//(calling remoteTyping in this case is not fine because it adds the user to the chat)

	if(typingMap.contains(message.from()))  
		remoteTyping( message.from(), false );

	//Need to copy this because it comes in as a const
	KopeteMessage m = message;
	addChatMessage(m);
	if( !isActive )
	{
		switch (m.importance())
		{
			case KopeteMessage::Highlight:
				setTabState( Highlighted );
#if KDE_IS_VERSION( 3, 1, 90 )
				KWin::setState( m_mainWindow->winId(), NET::DemandsAttention );
#endif
				break;
			case KopeteMessage::Normal:
				if(m.direction() == KopeteMessage::Inbound || m.direction() == KopeteMessage::Action)
				{
					setTabState( Message );
					break;
				}
			default:
				setTabState( Changed );
		}
	}

	if( !m_sendInProgress || message.from() != m_manager->user() )
	{
		unreadMessageFrom = message.from()->displayName();
		QTimer::singleShot( 1000, this, SLOT(slotMarkMessageRead()) );
	}
}

void ChatView::slotMarkMessageRead()
{
	unreadMessageFrom = QString::null;
}

void ChatView::slotContactStatusChanged( KopeteContact *contact, const KopeteOnlineStatus & /* newStatus */ , const KopeteOnlineStatus & /* oldstatus */)
{
	if(KopetePrefs::prefs()->showEvents())
	{
		if( contact->metaContact() )
		{
			sendInternalMessage( i18n( "%2 changed status to %1." )
#if QT_VERSION < 0x030200
				.arg(contact->onlineStatus().description() ).arg( contact->metaContact()->displayName() )
#else
				.arg(contact->onlineStatus().description(), contact->metaContact()->displayName() )
#endif
			);
		}
		else
		{
			sendInternalMessage( i18n( "%2 changed status to %1." )
#if QT_VERSION < 0x030200
				.arg( contact->onlineStatus().description() ).arg( contact->displayName() )
#else
				.arg( contact->onlineStatus().description(), contact->displayName() )
#endif
			);
		}
	}
}

void ChatView::slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/)
{
	kdDebug(14000) << k_funcinfo << "url=" << url.url() << endl;
	new KRun(url, 0, false); // false = non-local files
}

void ChatView::sendInternalMessage(const QString &msg)
{
	                               //when closing kopete, some internal message may be send because some contact are deleted
                                   //theses contact can already been deleted
	KopeteMessage m = KopeteMessage(/*m_manager->user(),  m_manager->members()*/ 0L,0L, msg, KopeteMessage::Internal);
	//(in many case, this is useless to set myself as contact
	//TODO: set the contact which initiate the internal messae, so we can later show a icon of it (for example, when he join a chat
	addChatMessage(m);
}

void ChatView::sendMessage()
{
	m_sendInProgress = true;
	m_mainWindow->setSendEnabled(false);

	QString txt = m_edit->text();
	if( m_lastMatch.isNull() && txt.contains( QRegExp( QString::fromLatin1("^\\w+:[\\s\\w]") ) ) )
	{
		QString search = txt.left( (int)txt.find(':') );
		if( !search.isEmpty() )
		{
			QString match = mComplete->makeCompletion( search );
			if( !match.isNull() )
				m_edit->setText( txt.replace(0,search.length(),match) );
		}
	}

	if( !m_lastMatch.isNull() )
	{
		mComplete->addItem( m_lastMatch );
		m_lastMatch = QString::null;
	}

	KopeteMessage sentMessage = currentMessage();
	emit messageSent( sentMessage );
	historyList.prepend(m_edit->text());
	historyPos = -1;
	m_edit->setText( QString::null );
	slotStopTimer();
}

void ChatView::messageSentSuccessfully()
{
	m_sendInProgress = false;
	emit ( messageSuccess( this ) );
}

void ChatView::slotTextChanged()
{
	QString txt = m_edit->text();
	if(editpart) //remove all <p><br> and other html tags
		txt.replace( QRegExp( QString::fromLatin1( "<[^>]*>" ) ), QString::null );

	bool typing=!txt.stripWhiteSpace().isEmpty();
	m_mainWindow->setSendEnabled( typing );

	if(typing)
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
}

void ChatView::historyUp()
{
	historyPos++;
	if(historyPos < (int)historyList.count())
		m_edit->setText(historyList[historyPos]);
	else
		historyPos = historyList.count()-1;
}

void ChatView::historyDown()
{
	if(historyPos > 0)
	{
		historyPos--;
		m_edit->setText(historyList[historyPos]);
	}
}

void ChatView::saveOptions()
{
	KConfig *config = KGlobal::config();

	writeDockConfig ( config, QString::fromLatin1("ChatViewDock") );
	config->setGroup( QString::fromLatin1("ChatViewDock") );
	config->writeEntry( QString::fromLatin1("membersDockPosition"), membersDockPosition );
	config->writeEntry( QString::fromLatin1("visibleMembers"), visibleMembers );
	config->setGroup( QString::fromLatin1("ChatViewSettings") );
	config->writeEntry ( QString::fromLatin1("BackgroundColor"), mBgColor );
	config->writeEntry ( QString::fromLatin1("Font"), mFont );
	config->writeEntry ( QString::fromLatin1("TextColor"), mFgColor );

	//config->writeEntry ( "SplitterWidth", editDock->parent()->seperatorPos() );

	config->sync();
}

void ChatView::readOptions()
{
	KConfig *config = KGlobal::config();

	/** THIS IS BROKEN !!! */
	//dockManager->readConfig ( config, "ChatViewDock" );

	//Work-around to restore dock widget positions
	config->setGroup( QString::fromLatin1("ChatViewDock") );
	int splitterPos = config->readNumEntry( QString::fromLatin1("viewDock,membersDock,editDock:sepPos"), 70);
	editDock->manualDock( viewDock, KDockWidget::DockBottom, splitterPos );
	viewDock->setDockSite(KDockWidget::DockLeft | KDockWidget::DockRight );
	editDock->setEnableDocking(KDockWidget::DockNone);
	membersDockPosition = static_cast<KDockWidget::DockPosition>(
		config->readNumEntry( QString::fromLatin1("membersDockPosition"), KDockWidget::DockNone ) );
	visibleMembers = config->readBoolEntry( QString::fromLatin1("visibleMembers"), false );

	config->setGroup( QString::fromLatin1("ChatViewSettings") );

	QFont tmpFont = KGlobalSettings::generalFont();
	setFont( config->readFontEntry( QString::fromLatin1("Font"), &tmpFont) );
	QColor tmpColor = KGlobalSettings::baseColor();
	setBgColor( config->readColorEntry ( QString::fromLatin1("BackgroundColor"), &tmpColor) );
	tmpColor = KGlobalSettings::textColor();
	setFgColor( config->readColorEntry ( QString::fromLatin1("TextColor"), &tmpColor ) );


	//editDock->parent()->setSeperatorPos( config->readNumEntry ( "SplitterWidth", 70 ) );
}

void ChatView::addText(const QString &text)
{
	m_edit->insert(text);
}

void ChatView::addChatMessage( KopeteMessage &m )
{
	uint bufferLen = (uint)KopetePrefs::prefs()->chatViewBufferSize();

	if( transparencyEnabled )
		m.setBgOverride( bgOverride );

	messageMap.insert( ++messageId, m );
 QDomDocument message = m.asXML();
	message.documentElement().setAttribute( QString::fromLatin1("id"), QString::number(messageId) );
	QString resultHTML = KopeteXSL::xsltTransform( message.toString(), KopetePrefs::prefs()->styleContents() );
	HTMLElement newNode = chatView->document().createElement( QString::fromLatin1("span") );
	newNode.setInnerHTML( resultHTML );

	chatView->htmlDocument().body().appendChild( newNode );
	messageList.append( m );

	if( messageList.count() >= bufferLen )
	{
		chatView->htmlDocument().body().removeChild( chatView->htmlDocument().body().firstChild() );
		messageMap.remove( messageMap.begin() );
		messageList.pop_front();
	}

	if( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

void ChatView::slotRefreshNodes()
{
	HTMLBodyElement bodyElement = chatView->htmlDocument().body();

	QString xmlString;
	for( QValueList<KopeteMessage>::Iterator it = messageList.begin(); it != messageList.end(); ++it)
		xmlString += (*it).asXML().toString();
	KopeteXSL::xsltTransformAsync( QString::fromLatin1("<document>") + xmlString + QString::fromLatin1("</document>"), KopetePrefs::prefs()->styleContents(), this, SLOT(slotTransformComplete( const QVariant &)) );
}

void ChatView::slotRefreshView()
{
	HTMLElement styleElement = chatView->document().documentElement().firstChild().firstChild();
	styleElement.setInnerText( styleHTML() );

	HTMLBodyElement bodyElement = chatView->htmlDocument().body();
	bodyElement.setBgColor( KopetePrefs::prefs()->bgColor().name() );
}

void ChatView::slotTransformComplete( const QVariant &result )
{
	chatView->htmlDocument().body().setInnerHTML( result.toString() );

	if( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

const QString ChatView::styleHTML() const
{
	KopetePrefs *p = KopetePrefs::prefs();

	QString style = QString::fromLatin1(
		"body{margin:4px;background-color:%1;font-family:%2;font-size:%3pt;color:%4;background-repeat:no-repeat;background-attachment:fixed}"
		"td{font-family:%5;font-size:%6pt;color:%7}"
		"a{color:%8}a.visited{color:%9}"
		"span.KopeteDisplayName{cursor:pointer;}span.KopeteDisplayName:hover{text-decoration:underline}"
		".KopeteLink{cursor:pointer;}.KopeteLink:hover{text-decoration:underline}" )
		.arg( p->bgColor().name() )
		.arg( p->fontFace().family() )
		.arg( p->fontFace().pointSize() )
		.arg( p->textColor().name() )
		.arg( p->fontFace().family() )
		.arg( p->fontFace().pointSize() )
		.arg( p->textColor().name() )
		.arg( p->linkColor().name() )
		.arg( p->linkColor().name() );

	//JASON, VA TE FAIRE FOUTRE AVEC TON *default* HIGHLIGHT!
	// that broke my highlight plugin
	// if the user has not Spetialy specified that it you theses 'putaint de' default color, WE DON'T USE THEM
	if(p->highlightEnabled())
	{
		style += QString::fromLatin1( ".highlight{color:%1;background-color:%2}" )
			.arg( p->highlightForeground().name() )
			.arg( p->highlightBackground().name() );
	}

	return style;
}

void ChatView::clear()
{
	HTMLElement body = chatView->htmlDocument().body();
	while( body.hasChildNodes() )
		body.removeChild( body.childNodes().item( body.childNodes().length() - 1 ) );

	messageList.clear();
}

void ChatView::slotRightClick( const QString &, const QPoint &point )
{
	KPopupMenu *chatWindowPopup = new KPopupMenu();
	Node n = chatView->nodeUnderMouse();
	if( n.nodeType() == Node::TEXT_NODE )
		n = n.parentNode();

	activeElement = n;
	KopeteMessage m = messageFromNode( activeElement );

	if( !activeElement.isNull() )
	{
		if( activeElement.className() == QString::fromLatin1("KopeteDisplayName") )
		{
			if( msgManager()->members().contains( m.from() ) )
			{
				KPopupMenu *p = ((KopeteContact*)m.from())->popupMenu();
				p->exec( point );
				delete p;
				delete chatWindowPopup;
				chatWindowPopup = 0L;
			}
			else
			{
				chatWindowPopup->insertItem( i18n("User Has Left"), 1 );
				chatWindowPopup->setItemEnabled( 1, false );
				chatWindowPopup->insertSeparator();
			}
		}
		else if( activeElement.tagName() == QString::fromLatin1("A") )
		{
			copyURLAction->plug(chatWindowPopup);
			chatWindowPopup->insertSeparator();
		}
	}

	if( chatWindowPopup )
	{
		if( !n.isNull() && msgManager()->members().contains( m.from() ) )
		{
			QPtrList<KopetePlugin> ps = LibraryLoader::pluginLoader()->plugins();
			bool actions = false;
			int j = 0;
			for( KopetePlugin *p = ps.first() ; p ; p = ps.next() )
			{
				KopeteProtocol *protocol = dynamic_cast<KopeteProtocol*>( p );
				if( !p || protocol == msgManager()->protocol() )
				{
					KActionCollection *customActions = p->customChatWindowPopupActions( m, n );
					if( customActions  && !customActions->isEmpty() )
					{
						actions = true;
						for(unsigned int i = 0; i < customActions->count(); i++)
							customActions->action(i)->plug( chatWindowPopup, i+j );
					}
					j++;
				}
			}

			if( actions )
				chatWindowPopup->insertSeparator();
		}

		copyAction->setEnabled( chatView->hasSelection() );
		copyAction->plug( chatWindowPopup );
		saveAction->plug( chatWindowPopup );
		printAction->plug( chatWindowPopup );
		chatWindowPopup->insertSeparator();
		closeAction->plug( chatWindowPopup );

		chatWindowPopup->exec( point );
		delete chatWindowPopup;
	}
}

void ChatView::slotCopyURL()
{
	HTMLAnchorElement a = activeElement;
	if( !a.isNull() )
	{
		QApplication::clipboard()->setText( a.href().string(), QClipboard::Clipboard );
		QApplication::clipboard()->setText( a.href().string(), QClipboard::Selection );
	}
}

KopeteMessage ChatView::messageFromNode( Node &node )
{
	HTMLElement e = node;
	while( !e.isNull() && e.className() != QString::fromLatin1("KopeteMessage") && e != chatView->htmlDocument().body() )
		 e = e.parentNode();

	KopeteMessage m;
	if( e.className().string() == QString::fromLatin1("KopeteMessage") )
	{
		unsigned long mId = e.id().string().toULong();
		if( messageMap.contains( mId ) )
			m = messageMap[ mId ];
	}
	return m;
}

const QString ChatView::viewsText()
{
	return chatView->htmlDocument().body().innerHTML().string();
}

void ChatView::setActive( bool value )
{
	isActive = value;
	if( isActive )
	{
		setTabState( Normal );
		emit( activated( static_cast<KopeteView*>(this) ) );
	}
}

void ChatView::setTabBar( KTabWidget *tabBar )
{
	m_tabBar = tabBar;
}

void ChatView::refreshView()
{
	//This doesn't work well using the DOM, so just use some JS
	if( bgChanged && !backgroundFile.isNull() )
	{
		chatView->setJScriptEnabled( true ) ;
		chatView->executeScript( QString::fromLatin1("document.body.background = \"%1\";").arg( backgroundFile ) );
		chatView->setJScriptEnabled( false ) ;
	}

	bgChanged = false;

	if( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

void ChatView::slotScrollView()
{
	htmlWidget->scrollBy( 0, htmlWidget->contentsHeight() );
}

void ChatView::setCurrentMessage( const KopeteMessage &message )
{
	m_edit->setText( message.plainBody() );

	setFont( message.font() );
	setFgColor( message.fg() );
	setBgColor( message.bg() );
}

KopeteMessage ChatView::currentMessage()
{
	KopeteMessage currentMsg = KopeteMessage( m_manager->user(), m_manager->members(), m_edit->text(), KopeteMessage::Outbound, editpart ? KopeteMessage::RichText : KopeteMessage::PlainText );

	currentMsg.setBg( mBgColor );
	currentMsg.setFg( mFgColor );
	currentMsg.setFont( mFont );

	return currentMsg;
}

void ChatView::cut()
{
	m_edit->cut();
}

void ChatView::copy()
{
	if ( chatView->hasSelection() )
	{
		QApplication::clipboard()->setText( chatView->selectedText(), QClipboard::Clipboard );
		QApplication::clipboard()->setText( chatView->selectedText(), QClipboard::Selection );
	}
	else
		m_edit->copy();
}

void ChatView::paste()
{
	m_edit->paste();
}

void ChatView::print()
{
	chatView->view()->print();
}

void ChatView::selectAll()
{
	chatView->selectAll();
}

void ChatView::setBgColor( const QColor &newColor )
{
	if( newColor == QColor() )
		KColorDialog::getColor( mBgColor, this );
	else
		mBgColor = newColor;

	QPalette pal = m_edit->palette();
	pal.setColor(QPalette::Active, QColorGroup::Base, mBgColor );
	pal.setColor(QPalette::Inactive, QColorGroup::Base, mBgColor );
	pal.setColor(QPalette::Disabled, QColorGroup::Base, mBgColor );

	// unsetPalette() so that color changes in kcontrol are honoured
	// if we ever have a subclass of KTextEdit, reimplement setPalette()
	// and check it there.
	if ( pal == QApplication::palette( m_edit ) )
		m_edit->unsetPalette();
	else
		m_edit->setPalette(pal);
}

void ChatView::setFont(  )
{
	KFontDialog::getFont(mFont, false, this);
	setFont(mFont);
}

void ChatView::setFont( const QFont &newFont )
{
	mFont=newFont;
	m_edit->setFont(mFont);
}

void ChatView::setFgColor( const QColor &newColor )
{
	if( newColor == QColor() )
		KColorDialog::getColor( mFgColor, this );
	else
		mFgColor = newColor;

	m_edit->setColor( mFgColor);

	QPalette pal = m_edit->palette();
	pal.setColor(QPalette::Active, QColorGroup::Text, mFgColor );
	pal.setColor(QPalette::Inactive, QColorGroup::Text, mFgColor );

	// unsetPalette() so that color changes in kcontrol are honoured
	// if we ever have a subclass of KTextEdit, reimplement setPalette()
	// and check it there.
	if ( pal == QApplication::palette( m_edit ) )
		m_edit->unsetPalette();
	else
		m_edit->setPalette(pal);
}


void ChatView::slotRepeatTimer()
{
	emit typing( true );
}

void ChatView::slotRemoteTypingTimeout()
{
	// Remove the topmost timer from the list. Why does QPtrDict use void* keys and not typed keys? *sigh*
	if( !m_remoteTypingMap.isEmpty() )
	{
		remoteTyping( reinterpret_cast<const KopeteContact *>( QPtrDictIterator<QTimer>(m_remoteTypingMap).currentKey() ), false );
	}
}

void ChatView::slotStopTimer()
{
	m_typingRepeatTimer->stop();
	emit typing( false );
}

void ChatView::slotTransparencyChanged()
{
	transparencyEnabled = KopetePrefs::prefs()->transparencyEnabled();
	bgOverride = KopetePrefs::prefs()->bgOverride();

//	kdDebug(14000) << k_funcinfo << "transparencyEnabled=" << transparencyEnabled << ", bgOverride=" << bgOverride << "." << endl;

	if( transparencyEnabled )
	{
		if( !root )
		{
//			kdDebug(14000) << k_funcinfo << "enabling transparency" << endl;
			root = new KRootPixmap( this );
			connect(root, SIGNAL(backgroundUpdated(const QPixmap &)), this, SLOT(slotUpdateBackground(const QPixmap &)));
			root->setCustomPainting ( true );
			root->setFadeEffect(KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->start();
		} else {
			root->setFadeEffect(KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->repaint( true );
		}
	}
	else
	{
		if ( root )
		{
//			kdDebug(14000) << k_funcinfo << "disabling transparency" << endl;
			disconnect(root, SIGNAL(backgroundUpdated(const QPixmap &)), this, SLOT(slotUpdateBackground(const QPixmap &)));
			delete root;
			root = 0L;
			backgroundFile = QString::null;
			chatView->executeScript( QString::fromLatin1("document.body.background = \"\";") );
		}
	}
}

void ChatView::slotUpdateBackground(const QPixmap &pm)
{
	if( m_mainWindow )
	{
		m_mainWindow->updateBackground( pm );

		if( m_mainWindow->backgroundFile )
			backgroundFile = m_mainWindow->backgroundFile->name();

		bgChanged = true;

		refreshView();
	}
}

//-----------------------
//  KopeteContactLVI

KopeteContactLVI::KopeteContactLVI( KopeteView *view, const KopeteContact *contact, KListView *parent ) : KListViewItem( parent )
{
	m_contact = contact;
	m_parentView = parent;
	m_view = view;

	setText( 1, QString::fromLatin1( " " ) + m_contact->displayName() );
	connect( m_contact, SIGNAL( displayNameChanged( const QString &, const QString & ) ),
		this, SLOT( slotDisplayNameChanged(const QString &, const QString &) ) );

	connect( m_contact, SIGNAL( destroyed() ), this, SLOT( deleteLater() ) );
	connect( m_contact, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
		this, SLOT( slotStatusChanged() ) );
	connect( m_parentView, SIGNAL( executed( QListViewItem* ) ),
		this, SLOT( slotExecute( QListViewItem * ) ) );

	slotStatusChanged();
}

void KopeteContactLVI::slotDisplayNameChanged(const QString &, const QString &newName)
{
	setText( 1, QString::fromLatin1( " " ) + newName );
	m_parentView->sort();
}

void KopeteContactLVI::slotStatusChanged()
{
	setText( 0, QChar( -m_view->msgManager()->contactOnlineStatus( m_contact ).weight() ) );
	setPixmap( 0, m_view->msgManager()->contactOnlineStatus( m_contact ).iconFor(m_contact) );
	m_parentView->sort();
}

void KopeteContactLVI::slotExecute( QListViewItem *item )
{
	if( static_cast<QListViewItem*>( this ) == item )
		((KopeteContact*)m_contact)->execute();
}


#include "chatview.moc"

// vim: set noet ts=4 sts=4 sw=4:
