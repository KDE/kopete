/*
    kopetechatwindow.cpp - Chat Window

    Copyright (c) 2002-2006 by Olivier Goffart       <ogoffart@ kde.org>
    Copyright (c) 2003-2004 by Richard Smith         <kde@metafoo.co.uk>
    Copyright (C) 2002      by James Grant
    Copyright (c) 2002      by Stefan Gehn           <metz AT gehn.net>
    Copyright (c) 2002-2004 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtimer.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kcursor.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kwin.h>
#include <ktempfile.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kpushbutton.h>
#include <ktabwidget.h>
#include <kstandarddirs.h>
#include <kdialog.h>
#include <kstringhandler.h>
#include <ksqueezedtextlabel.h>
#include <kstdaccel.h>
#include <kglobalsettings.h>

#include "chatmessagepart.h"
#include "chattexteditpart.h"
#include "chatview.h"
#include "kopeteapplication.h"
#include "kopetechatwindow.h"
#include "kopeteemoticonaction.h"
#include "kopetegroup.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteviewmanager.h"

#include <qtoolbutton.h>
#include <kactionclasses.h>

typedef QMap<Kopete::Account*,KopeteChatWindow*> AccountMap;
typedef QMap<Kopete::Group*,KopeteChatWindow*> GroupMap;
typedef QMap<Kopete::MetaContact*,KopeteChatWindow*> MetaContactMap;
typedef QPtrList<KopeteChatWindow> WindowList;

namespace
{
	AccountMap accountMap;
	GroupMap groupMap;
	MetaContactMap mcMap;
	WindowList windows;
}

KopeteChatWindow *KopeteChatWindow::window( Kopete::ChatSession *manager )
{
	bool windowCreated = false;
	KopeteChatWindow *myWindow;

	//Take the first and the first? What else?
	Kopete::Group *group = 0L;
	Kopete::ContactPtrList members = manager->members();
	Kopete::MetaContact *metaContact = members.first()->metaContact();

	if ( metaContact )
	{
		Kopete::GroupList gList = metaContact->groups();
		group = gList.first();
	}

	switch( KopetePrefs::prefs()->chatWindowPolicy() )
	{
		case GROUP_BY_ACCOUNT: //Open chats from the same protocol in the same window
			if( accountMap.contains( manager->account() ) )
				myWindow = accountMap[ manager->account() ];
			else
				windowCreated = true;
			break;

		case GROUP_BY_GROUP: //Open chats from the same group in the same window
			if( group && groupMap.contains( group ) )
				myWindow = groupMap[ group ];
			else
				windowCreated = true;
			break;

		case GROUP_BY_METACONTACT: //Open chats from the same metacontact in the same window
			if( mcMap.contains( metaContact ) )
				myWindow = mcMap[ metaContact ];
			else
				windowCreated = true;
			break;

		case GROUP_ALL: //Open all chats in the same window
			if( windows.isEmpty() )
				windowCreated = true;
			else
			{
				//Here we are finding the window with the most tabs and
				//putting it there. Need this for the cases where config changes
				//midstream

				int viewCount = -1;
				for ( KopeteChatWindow *thisWindow = windows.first(); thisWindow; thisWindow = windows.next() )
				{
					if( thisWindow->chatViewCount() > viewCount )
					{
						myWindow = thisWindow;
						viewCount = thisWindow->chatViewCount();
					}
				}
			}
			break;

		case NEW_WINDOW: //Open every chat in a new window
		default:
			windowCreated = true;
			break;
	}

	if ( windowCreated )
	{
		myWindow = new KopeteChatWindow();

		if ( !accountMap.contains( manager->account() ) )
			accountMap.insert( manager->account(), myWindow );

		if ( !mcMap.contains( metaContact ) )
			mcMap.insert( metaContact, myWindow );

		if ( group && !groupMap.contains( group ) )
			groupMap.insert( group, myWindow );
	}

//	kdDebug( 14010 ) << k_funcinfo << "Open Windows: " << windows.count() << endl;

	return myWindow;
}

KopeteChatWindow::KopeteChatWindow( QWidget *parent, const char* name )
	: KParts::MainWindow( parent, name )
{
	m_activeView = 0L;
	m_popupView = 0L;
	backgroundFile = 0L;
	updateBg = true;
	m_tabBar = 0L;

	initActions();

	QVBox *vBox = new QVBox( this );
	vBox->setLineWidth( 0 );
	vBox->setSpacing( 0 );
	vBox->setFrameStyle( QFrame::NoFrame );
	// set default window size.  This could be removed by fixing the size hints of the contents
	resize( 500, 500 );
	setCentralWidget( vBox );

	mainArea = new QFrame( vBox );
	mainArea->setLineWidth( 0 );
	mainArea->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	mainLayout = new QVBoxLayout( mainArea );

	if ( KopetePrefs::prefs()->chatWShowSend() )
	{
		//Send Button
		m_button_send = new KPushButton( i18n("Send"), statusBar() );
		m_button_send->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
		m_button_send->setEnabled( false );
		m_button_send->setFont( statusBar()->font() );
		m_button_send->setFixedHeight( statusBar()->sizeHint().height() );
		connect( m_button_send, SIGNAL( clicked() ), this, SLOT( slotSendMessage() ) );
		statusBar()->addWidget( m_button_send, 0, true );
	}
	else
		m_button_send = 0L;

	m_status_text = new KSqueezedTextLabel( i18n("Ready."), statusBar(), "m_status_text" );
	m_status_text->setAlignment( AlignLeft | AlignVCenter );
	m_status_text->setFont( statusBar()->font() );
	m_status_text->setFixedHeight( statusBar()->sizeHint().height() );
	statusBar()->addWidget( m_status_text, 1 );

	readOptions();
	setWFlags( Qt::WDestructiveClose );

	windows.append( this );
	windowListChanged();

	KGlobal::config()->setGroup( QString::fromLatin1("ChatWindowSettings") );
	m_alwaysShowTabs = KGlobal::config()->readBoolEntry( QString::fromLatin1("AlwaysShowTabs"), false );
	m_showFormatToolbar = KGlobal::config()->readBoolEntry( QString::fromLatin1("Show Format Toolbar"), true );
	adjustingFormatToolbar = false;
//	kdDebug( 14010 ) << k_funcinfo << "Open Windows: " << windows.count() << endl;
	kapp->ref();
}

KopeteChatWindow::~KopeteChatWindow()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	emit( closing( this ) );

	for( AccountMap::Iterator it = accountMap.begin(); it != accountMap.end(); )
	{
		AccountMap::Iterator mayDeleteIt = it;
		++it;
		if( mayDeleteIt.data() == this )
			accountMap.remove( mayDeleteIt.key() );
	}

	for( GroupMap::Iterator it = groupMap.begin(); it != groupMap.end(); )
	{
		GroupMap::Iterator mayDeleteIt = it;
		++it;
		if( mayDeleteIt.data() == this )
			groupMap.remove( mayDeleteIt.key() );
	}

	for( MetaContactMap::Iterator it = mcMap.begin(); it != mcMap.end(); )
	{
		MetaContactMap::Iterator mayDeleteIt = it;
		++it;
		if( mayDeleteIt.data() == this )
			mcMap.remove( mayDeleteIt.key() );
	}

	windows.remove( this );
	windowListChanged();

//	kdDebug( 14010 ) << "Open Windows: " << windows.count() << endl;

	saveOptions();

	if( backgroundFile )
	{
		backgroundFile->close();
		backgroundFile->unlink();
		delete backgroundFile;
	}

	delete anim;
	kapp->deref();
}

void KopeteChatWindow::windowListChanged()
{
	// update all windows' Move Tab to Window action
	for ( QPtrListIterator<KopeteChatWindow> it( windows ); *it; ++it )
		(*it)->checkDetachEnable();
}

void KopeteChatWindow::slotNickComplete()
{
	if( m_activeView )
		m_activeView->nickComplete();
}

void KopeteChatWindow::slotTabContextMenu( QWidget *tab, const QPoint &pos )
{
	m_popupView = static_cast<ChatView*>( tab );

	KPopupMenu *popup = new KPopupMenu;
	popup->insertTitle( KStringHandler::rsqueeze( m_popupView->caption() ) );

	actionContactMenu->plug( popup );
	popup->insertSeparator();
	actionTabPlacementMenu->plug( popup );
	tabDetach->plug( popup );
	actionDetachMenu->plug( popup );
	tabClose->plug( popup );
	popup->exec( pos );

	delete popup;
	m_popupView = 0;
}

ChatView *KopeteChatWindow::activeView()
{
	return m_activeView;
}

void KopeteChatWindow::initActions(void)
{
	KActionCollection *coll = actionCollection();

	createStandardStatusBarAction();

	chatSend = new KAction( i18n( "&Send Message" ), QString::fromLatin1( "mail_send" ), QKeySequence(Key_Return) ,
		this, SLOT( slotSendMessage() ), coll, "chat_send" );
	chatSend->setEnabled( false );

 	KStdAction::save ( this, SLOT(slotChatSave()), coll );
 	KStdAction::print ( this, SLOT(slotChatPrint()), coll );
	KAction* quitAction = KStdAction::quit ( this, SLOT(close()), coll );
	quitAction->setText( i18n("Close All Chats") );

	tabClose = KStdAction::close ( this, SLOT(slotChatClosed()), coll, "tabs_close" );

	tabRight=new KAction( i18n( "&Activate Next Tab" ), 0, KStdAccel::tabNext(),
		this, SLOT( slotNextTab() ), coll, "tabs_right" );
	tabLeft=new KAction( i18n( "&Activate Previous Tab" ), 0, KStdAccel::tabPrev(),
		this, SLOT( slotPreviousTab() ), coll, "tabs_left" );
	tabLeft->setEnabled( false );
	tabRight->setEnabled( false );

	nickComplete = new KAction( i18n( "Nic&k Completion" ), QString::null, 0, this, SLOT( slotNickComplete() ), coll , "nick_compete");
	nickComplete->setShortcut( QKeySequence( Key_Tab ) );

	tabDetach = new KAction( i18n( "&Detach Chat" ), QString::fromLatin1( "tab_breakoff" ), 0,
		this, SLOT( slotDetachChat() ), coll, "tabs_detach" );
	tabDetach->setEnabled( false );

	actionDetachMenu = new KActionMenu( i18n( "&Move Tab to Window" ), QString::fromLatin1( "tab_breakoff" ), coll, "tabs_detachmove" );
	actionDetachMenu->setDelayed( false );

	connect ( actionDetachMenu->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotPrepareDetachMenu()) );
	connect ( actionDetachMenu->popupMenu(), SIGNAL(activated(int)), this, SLOT(slotDetachChat(int)) );

	actionTabPlacementMenu = new KActionMenu( i18n( "&Tab Placement" ), coll, "tabs_placement" );
	connect ( actionTabPlacementMenu->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotPreparePlacementMenu()) );
	connect ( actionTabPlacementMenu->popupMenu(), SIGNAL(activated(int)), this, SLOT(slotPlaceTabs(int)) );

	tabDetach->setShortcut( QKeySequence(CTRL + SHIFT + Key_B) );

	KStdAction::cut( this, SLOT(slotCut()), coll);
	KStdAction::copy( this, SLOT(slotCopy()), coll);
	KStdAction::paste( this, SLOT(slotPaste()), coll);

	new KAction( i18n( "Set Default &Font..." ), QString::fromLatin1( "charset" ), 0, this, SLOT( slotSetFont() ), coll, "format_font" );
	new KAction( i18n( "Set Default Text &Color..." ), QString::fromLatin1( "pencil" ), 0, this, SLOT( slotSetFgColor() ), coll, "format_fgcolor" );
	new KAction( i18n( "Set &Background Color..." ), QString::fromLatin1( "fill" ), 0, this, SLOT( slotSetBgColor() ), coll, "format_bgcolor" );

	historyUp = new KAction( i18n( "Previous History" ), QString::null, 0,
		this, SLOT( slotHistoryUp() ), coll, "history_up" );
	historyUp->setShortcut( QKeySequence(CTRL + Key_Up) );

	historyDown = new KAction( i18n( "Next History" ), QString::null, 0,
		this, SLOT( slotHistoryDown() ), coll, "history_down" );
	historyDown->setShortcut( QKeySequence(CTRL + Key_Down) );

	KStdAction::prior( this, SLOT( slotPageUp() ), coll, "scroll_up" );
	KStdAction::next( this, SLOT( slotPageDown() ), coll, "scroll_down" );

	KStdAction::showMenubar( this, SLOT(slotViewMenuBar()), coll );

	membersLeft = new KToggleAction( i18n( "Place to Left of Chat Area" ), QString::null, 0,
		this, SLOT( slotViewMembersLeft() ), coll, "options_membersleft" );
	membersRight = new KToggleAction( i18n( "Place to Right of Chat Area" ), QString::null, 0,
		this, SLOT( slotViewMembersRight() ), coll, "options_membersright" );
	toggleMembers = new KToggleAction( i18n( "Show" ), QString::null, 0,
		this, SLOT( slotToggleViewMembers() ), coll, "options_togglemembers" );
	toggleMembers->setCheckedState(i18n("Hide"));
	toggleAutoSpellCheck = new KToggleAction( i18n( "Automatic Spell Checking" ), QString::null, 0,
		this, SLOT( toggleAutoSpellChecking() ), coll, "enable_auto_spell_check" );
	toggleAutoSpellCheck->setChecked( true );

	actionSmileyMenu = new KopeteEmoticonAction( coll, "format_smiley" );
	actionSmileyMenu->setDelayed( false );
	connect(actionSmileyMenu, SIGNAL(activated(const QString &)), this, SLOT(slotSmileyActivated(const QString &)));

	actionContactMenu = new KActionMenu(i18n("Co&ntacts"), coll, "contacts_menu" );
	actionContactMenu->setDelayed( false );
	connect ( actionContactMenu->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotPrepareContactMenu()) );

	// add configure key bindings menu item
	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), coll );

	KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), coll);
	KopeteStdAction::preferences( coll , "settings_prefs" );

	//The Sending movie
	normalIcon = QPixmap( BarIcon( QString::fromLatin1( "kopete" ) ) );
	animIcon = KGlobal::iconLoader()->loadMovie( QString::fromLatin1( "newmessage" ), KIcon::Toolbar);

	// Pause the animation because otherwise it's running even when we're not
	// showing it. This eats resources, and also triggers a pixmap leak in
	// QMovie in at least Qt 3.1, Qt 3.2 and the current Qt 3.3 beta
	if( !animIcon.isNull() )  //and another QT bug:  it crash if we pause a null movie
		animIcon.pause();

	// we can't set the tool bar as parent, if we do, it will be deleted when we configure toolbars
	anim = new QLabel( QString::null, 0L ,"kde toolbar widget" );
	anim->setMargin(5);
	anim->setPixmap( normalIcon );


	new KWidgetAction( anim , i18n("Toolbar Animation") , 0, 0 , 0 , coll , "toolbar_animation");

	//toolBar()->insertWidget( 99, anim->width(), anim );
	//toolBar()->alignItemRight( 99 );
	setStandardToolBarMenuEnabled( true );

	setXMLFile( QString::fromLatin1( "kopetechatwindow.rc" ) );
	createGUI( 0L );

	// Special handling for remembering whether the format toolbar is visible or not
	connect ( toolBar("formatToolBar"), SIGNAL(visibilityChanged(bool)), this, SLOT(slotToggleFormatToolbar(bool)) );
}

const QString KopeteChatWindow::fileContents( const QString &path ) const
{
 	QString contents;
	QFile file( path );
	if ( file.open( IO_ReadOnly ) )
	{
		QTextStream stream( &file );
		contents = stream.read();
		file.close();
	}

	return contents;
}

void KopeteChatWindow::slotStopAnimation( ChatView* view )
{
	if( view == m_activeView )
		anim->setPixmap( normalIcon );
}

void KopeteChatWindow::slotUpdateSendEnabled()
{
	if ( !m_activeView ) return;

	bool enabled = m_activeView->canSend();
	chatSend->setEnabled( enabled );
	if(m_button_send)
		m_button_send->setEnabled( enabled );
}

void KopeteChatWindow::updateMembersActions()
{
	if( m_activeView )
	{
		const KDockWidget::DockPosition pos = m_activeView->membersListPosition();
		bool visibleMembers = m_activeView->visibleMembersList();
		membersLeft->setChecked( pos == KDockWidget::DockLeft  );
		membersLeft->setEnabled( visibleMembers );
		membersRight->setChecked( pos == KDockWidget::DockRight );
		membersRight->setEnabled( visibleMembers );
		toggleMembers->setChecked( visibleMembers );
	}
}

void KopeteChatWindow::slotViewMembersLeft()
{
	m_activeView->placeMembersList( KDockWidget::DockLeft );
	updateMembersActions();
}

void KopeteChatWindow::slotViewMembersRight()
{
	m_activeView->placeMembersList( KDockWidget::DockRight );
	updateMembersActions();
}

void KopeteChatWindow::slotToggleViewMembers()
{
	m_activeView->toggleMembersVisibility();
	updateMembersActions();
}

void KopeteChatWindow::toggleAutoSpellChecking()
{
	if ( !m_activeView )
		return;

	bool currentSetting = m_activeView->editPart()->autoSpellCheckEnabled();
	m_activeView->editPart()->toggleAutoSpellCheck( !currentSetting );
	updateSpellCheckAction();
}

void KopeteChatWindow::updateSpellCheckAction()
{
	if ( !m_activeView )
		return;

	if ( m_activeView->editPart()->richTextEnabled() )
	{
		toggleAutoSpellCheck->setEnabled( false );
		toggleAutoSpellCheck->setChecked( false );
		m_activeView->editPart()->toggleAutoSpellCheck( false );
	}
	else
	{
		toggleAutoSpellCheck->setEnabled( true );
		if ( KopetePrefs::prefs()->spellCheck() )
		{
			kdDebug(14000) << k_funcinfo << "spell check enabled" << endl;
			toggleAutoSpellCheck->setChecked( true );
			m_activeView->editPart()->toggleAutoSpellCheck(true);
		}
		else
		{
			kdDebug(14000) << k_funcinfo << "spell check disabled" << endl;
			toggleAutoSpellCheck->setChecked( false );
			m_activeView->editPart()->toggleAutoSpellCheck(false);
		}
	}
}

void KopeteChatWindow::slotHistoryUp()
{
	if( m_activeView )
		m_activeView->editPart()->historyUp();
}

void KopeteChatWindow::slotHistoryDown()
{
	if( m_activeView )
		m_activeView->editPart()->historyDown();
}

void KopeteChatWindow::slotPageUp()
{
	if( m_activeView )
		m_activeView->messagePart()->pageUp();
}

void KopeteChatWindow::slotPageDown()
{
	if( m_activeView )
		m_activeView->messagePart()->pageDown();
}

void KopeteChatWindow::slotCut()
{
	m_activeView->cut();
}

void KopeteChatWindow::slotCopy()
{
	m_activeView->copy();
}

void KopeteChatWindow::slotPaste()
{
	m_activeView->paste();
}


void KopeteChatWindow::slotSetFont()
{
	m_activeView->setFont();
}

void KopeteChatWindow::slotSetFgColor()
{
	m_activeView->setFgColor();
}

void KopeteChatWindow::slotSetBgColor()
{
	m_activeView->setBgColor();
}

void KopeteChatWindow::setStatus(const QString &text)
{
	m_status_text->setText(text);
}

void KopeteChatWindow::createTabBar()
{
	if( !m_tabBar )
	{
		KGlobal::config()->setGroup( QString::fromLatin1("ChatWindowSettings") );

		m_tabBar = new KTabWidget( mainArea );
		m_tabBar->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
		m_tabBar->setHoverCloseButton(KGlobal::config()->readBoolEntry( QString::fromLatin1("HoverClose"), false ));
		m_tabBar->setTabReorderingEnabled(true);
#if KDE_IS_VERSION(3,4,0)
		m_tabBar->setAutomaticResizeTabs(true);
#endif
		connect( m_tabBar, SIGNAL( closeRequest( QWidget* )), this, SLOT( slotCloseChat( QWidget* ) ) );

		QToolButton* m_rightWidget = new QToolButton( m_tabBar );
		connect( m_rightWidget, SIGNAL( clicked() ), this, SLOT( slotChatClosed() ) );
		m_rightWidget->setIconSet( SmallIcon( "tab_remove" ) );
		m_rightWidget->adjustSize();
		QToolTip::add( m_rightWidget, i18n("Close the current tab"));
		m_tabBar->setCornerWidget( m_rightWidget, QWidget::TopRight );

		mainLayout->addWidget( m_tabBar );
		m_tabBar->show();
		connect ( m_tabBar, SIGNAL(currentChanged(QWidget *)), this, SLOT(setActiveView(QWidget *)) );
		connect ( m_tabBar, SIGNAL(contextMenu(QWidget *, const QPoint & )), this, SLOT(slotTabContextMenu( QWidget *, const QPoint & )) );

		for( ChatView *view = chatViewList.first(); view; view = chatViewList.next() )
			addTab( view );

		if( m_activeView )
			m_tabBar->showPage( m_activeView );
		else
			setActiveView( chatViewList.first() );

		int tabPosition = KGlobal::config()->readNumEntry( QString::fromLatin1("Tab Placement") , 0 );
		slotPlaceTabs( tabPosition );
	}
}

void KopeteChatWindow::slotCloseChat( QWidget *chatView )
{
	static_cast<ChatView*>( chatView )->closeView();
}

void KopeteChatWindow::addTab( ChatView *view )
{
	QPtrList<Kopete::Contact> chatMembers=view->msgManager()->members();
	Kopete::Contact *c=0L;
	for ( Kopete::Contact *contact = chatMembers.first(); contact; contact = chatMembers.next() )
	{
		if(!c || c->onlineStatus() < contact->onlineStatus())
			c=contact;
	}
	QPixmap pluginIcon = c ? view->msgManager()->contactOnlineStatus( c ).iconFor( c) : SmallIcon( view->msgManager()->protocol()->pluginIcon() );

	view->reparent( m_tabBar, 0, QPoint(), true );
	m_tabBar->addTab( view, pluginIcon, view->caption() );
	if( view == m_activeView )
		view->show();
	else
		view->hide();
	connect( view, SIGNAL( captionChanged( bool ) ), this, SLOT( updateChatLabel() ) );
	connect( view, SIGNAL( updateStatusIcon( ChatView* ) ), this, SLOT( slotUpdateCaptionIcons( ChatView* ) ) );
	view->setCaption( view->caption(), false );
}

void KopeteChatWindow::setPrimaryChatView( ChatView *view )
{
	//TODO figure out what else we have to save here besides the font
	//reparent clears a lot of stuff out
	QFont savedFont = view->font();
	view->reparent( mainArea, 0, QPoint(), true );
	view->setFont( savedFont );
	view->show();

	mainLayout->addWidget( view );
	setActiveView( view );
}

void KopeteChatWindow::deleteTabBar()
{
	if( m_tabBar )
	{
		disconnect ( m_tabBar, SIGNAL(currentChanged(QWidget *)), this, SLOT(setActiveView(QWidget *)) );
		disconnect ( m_tabBar, SIGNAL(contextMenu(QWidget *, const QPoint & )), this, SLOT(slotTabContextMenu( QWidget *, const QPoint & )) );

		if( !chatViewList.isEmpty() )
			setPrimaryChatView( chatViewList.first() );

		m_tabBar->deleteLater();
		m_tabBar = 0L;
	}
}

void KopeteChatWindow::attachChatView( ChatView* newView )
{
	chatViewList.append( newView );

	if ( !m_alwaysShowTabs && chatViewList.count() == 1 )
		setPrimaryChatView( newView );
	else
	{
		if ( !m_tabBar )
			createTabBar();
		else
			addTab( newView );
		newView->setActive( false );
	}

	newView->setMainWindow( this );
	newView->editWidget()->installEventFilter( this );

	KCursor::setAutoHideCursor( newView->editWidget(), true, true );
	connect( newView, SIGNAL(captionChanged( bool)), this, SLOT(slotSetCaption(bool)) );
	connect( newView, SIGNAL(messageSuccess( ChatView* )), this, SLOT(slotStopAnimation( ChatView* )) );
	connect( newView, SIGNAL(rtfEnabled( ChatView*, bool ) ), this, SLOT( slotRTFEnabled( ChatView*, bool ) ) );
	connect( newView, SIGNAL(updateStatusIcon( ChatView* ) ), this, SLOT(slotUpdateCaptionIcons( ChatView* ) ) );
	connect( newView, SIGNAL(updateChatState( ChatView*, int ) ), this, SLOT( updateChatState( ChatView*, int ) ) );

	updateSpellCheckAction();
	checkDetachEnable();
	newView->loadChatSettings();
	connect( newView, SIGNAL(autoSpellCheckEnabled( ChatView*, bool ) ),
	         this, SLOT( slotAutoSpellCheckEnabled( ChatView*, bool ) ) );
}

void KopeteChatWindow::checkDetachEnable()
{
	bool haveTabs = (chatViewList.count() > 1);
	tabDetach->setEnabled( haveTabs );
	tabLeft->setEnabled( haveTabs );
	tabRight->setEnabled( haveTabs );
	actionTabPlacementMenu->setEnabled( m_tabBar != 0 );

	bool otherWindows = (windows.count() > 1);
	actionDetachMenu->setEnabled( otherWindows );
}

void KopeteChatWindow::detachChatView( ChatView *view )
{
	if( !chatViewList.removeRef( view ) )
		return;

	disconnect( view, SIGNAL(captionChanged( bool)), this, SLOT(slotSetCaption(bool)) );
	disconnect( view, SIGNAL( updateStatusIcon( ChatView* ) ), this, SLOT( slotUpdateCaptionIcons( ChatView* ) ) );
	disconnect( view, SIGNAL( updateChatState( ChatView*, int ) ), this, SLOT( updateChatState( ChatView*, int ) ) );
	view->editWidget()->removeEventFilter( this );

	if( m_tabBar )
	{
		int curPage = m_tabBar->currentPageIndex();
		QWidget *page = m_tabBar->page( curPage );

		// if the current view is to be detached, switch to a different one
		if( page == view )
		{
			if( curPage > 0 )
				m_tabBar->setCurrentPage( curPage - 1 );
			else
				m_tabBar->setCurrentPage( curPage + 1 );
		}

		m_tabBar->removePage( view );

		if( m_tabBar->currentPage() )
			setActiveView( static_cast<ChatView*>(m_tabBar->currentPage()) );
	}

	if( chatViewList.isEmpty() )
		close();
	else if( !m_alwaysShowTabs && chatViewList.count() == 1)
		deleteTabBar();

	checkDetachEnable();
}

void KopeteChatWindow::slotDetachChat( int newWindowIndex )
{
	KopeteChatWindow *newWindow = 0L;
	ChatView *detachedView;

	if( m_popupView )
		detachedView = m_popupView;
	else
		detachedView = m_activeView;

	if( !detachedView )
		return;

	//if we don't do this, we might crash
	createGUI(0L);
	guiFactory()->removeClient(detachedView->msgManager());

	if( newWindowIndex == -1 )
		newWindow = new KopeteChatWindow();
	else
		newWindow = windows.at( newWindowIndex );

	newWindow->show();
	newWindow->raise();

	detachChatView( detachedView );
	newWindow->attachChatView( detachedView );
}

void KopeteChatWindow::slotPreviousTab()
{
	int curPage = m_tabBar->currentPageIndex();
	if( curPage > 0 )
		m_tabBar->setCurrentPage( curPage - 1 );
	else
		m_tabBar->setCurrentPage( m_tabBar->count() - 1 );
}

void KopeteChatWindow::slotNextTab()
{
	int curPage = m_tabBar->currentPageIndex();
	if( curPage == ( m_tabBar->count() - 1 ) )
		m_tabBar->setCurrentPage( 0 );
	else
		m_tabBar->setCurrentPage( curPage + 1 );
}

void KopeteChatWindow::slotSetCaption( bool active )
{
	if( active && m_activeView )
	{
		setCaption( m_activeView->caption(), false );
	}
}

void KopeteChatWindow::updateBackground( const QPixmap &pm )
{
	if( updateBg )
	{
		updateBg = false;
		if( backgroundFile != 0L )
		{
			backgroundFile->close();
			backgroundFile->unlink();
		}

		backgroundFile = new KTempFile( QString::null, QString::fromLatin1( ".bmp" ) );
		pm.save( backgroundFile->name(), "BMP" );
		QTimer::singleShot( 100, this, SLOT( slotEnableUpdateBg() ) );
	}
}

void KopeteChatWindow::setActiveView( QWidget *widget )
{
	ChatView *view = static_cast<ChatView*>(widget);

	if( m_activeView == view )
		return;

	if(m_activeView)
	{
		disconnect( m_activeView, SIGNAL( canSendChanged(bool) ), this, SLOT( slotUpdateSendEnabled() ) );
		guiFactory()->removeClient(m_activeView->msgManager());
		m_activeView->saveChatSettings();
	}

	guiFactory()->addClient(view->msgManager());
	createGUI( view->editPart() );

	if( m_activeView )
		m_activeView->setActive( false );

	m_activeView = view;

	if( !chatViewList.contains( view ) )
		attachChatView( view );

	connect( m_activeView, SIGNAL( canSendChanged(bool) ), this, SLOT( slotUpdateSendEnabled() ) );

	//Tell it it is active
	m_activeView->setActive( true );

	//Update icons to match
	slotUpdateCaptionIcons( m_activeView );

	//Update chat members actions
	updateMembersActions();

	if ( m_activeView->sendInProgress() && !animIcon.isNull() )
	{
		anim->setMovie( animIcon );
		animIcon.unpause();
	}
	else
	{
		anim->setPixmap( normalIcon );
		if( !animIcon.isNull() )
			animIcon.pause();
	}

	if ( m_alwaysShowTabs || chatViewList.count() > 1 )
	{
		if( !m_tabBar )
			createTabBar();

		m_tabBar->showPage( m_activeView );
	}

	setCaption( m_activeView->caption() );
	setStatus( m_activeView->statusText() );
	m_activeView->setFocus();
	updateSpellCheckAction();
	slotUpdateSendEnabled();
	m_activeView->editPart()->reloadConfig();
	m_activeView->loadChatSettings();
}

void KopeteChatWindow::slotUpdateCaptionIcons( ChatView *view )
{
	if ( !view )
		return; //(pas de charité)

	QPtrList<Kopete::Contact> chatMembers=view->msgManager()->members();
	Kopete::Contact *c=0L;
	for ( Kopete::Contact *contact = chatMembers.first(); contact; contact = chatMembers.next() )
	{
		if(!c || c->onlineStatus() < contact->onlineStatus())
			c=contact;
	}

	if ( view == m_activeView )
 	{
		QPixmap icon16 = c ? view->msgManager()->contactOnlineStatus( c ).iconFor( c , 16) :
		                     SmallIcon( view->msgManager()->protocol()->pluginIcon() );
		QPixmap icon32 = c ? view->msgManager()->contactOnlineStatus( c ).iconFor( c , 32) :
		                     SmallIcon( view->msgManager()->protocol()->pluginIcon() );
		KWin::setIcons( winId(), icon32, icon16 );
	}

	if ( m_tabBar )
		m_tabBar->setTabIconSet( view, c ? view->msgManager()->contactOnlineStatus( c ).iconFor( c ) :
		                                   SmallIcon( view->msgManager()->protocol()->pluginIcon() ) );
}

void KopeteChatWindow::slotChatClosed()
{
	if( m_popupView )
		m_popupView->closeView();
	else
		m_activeView->closeView();
}

void KopeteChatWindow::slotPrepareDetachMenu(void)
{
	QPopupMenu *detachMenu = actionDetachMenu->popupMenu();
	detachMenu->clear();

	for ( unsigned id=0; id < windows.count(); id++ )
	{
		KopeteChatWindow *win = windows.at( id );
		if( win != this )
			detachMenu->insertItem( win->caption(), id );
	}
}

void KopeteChatWindow::slotSendMessage()
{
	if ( m_activeView && m_activeView->canSend() )
	{
		if( !animIcon.isNull() )
		{
			anim->setMovie( animIcon );
			animIcon.unpause();
		}
		m_activeView->sendMessage();
	}
}

void KopeteChatWindow::slotPrepareContactMenu(void)
{
	QPopupMenu *contactsMenu = actionContactMenu->popupMenu();
	contactsMenu->clear();

	Kopete::Contact *contact;
	Kopete::ContactPtrList m_them;

	if( m_popupView )
		m_them = m_popupView->msgManager()->members();
	else
		m_them = m_activeView->msgManager()->members();

	//TODO: don't display a menu with one contact in it, display that
	// contact's menu instead. Will require changing text and icon of
	// 'Contacts' action, or something cleverer.
	uint contactCount = 0;

	for ( contact = m_them.first(); contact; contact = m_them.next() )
	{
		KPopupMenu *p = contact->popupMenu();
		connect ( actionContactMenu->popupMenu(), SIGNAL(aboutToHide()),
			p, SLOT(deleteLater() ) );

		if( contact->metaContact() )
			contactsMenu->insertItem( contact->onlineStatus().iconFor( contact ) , contact->metaContact()->displayName(), p );
		else
			contactsMenu->insertItem( contact->onlineStatus().iconFor( contact ) , contact->contactId(), p );

		//FIXME: This number should be a config option
		if( ++contactCount == 15 && contact != m_them.getLast() )
		{
			KActionMenu *moreMenu = new KActionMenu( i18n("More..."),
				 QString::fromLatin1("folder_open"), contactsMenu );
			connect ( actionContactMenu->popupMenu(), SIGNAL(aboutToHide()),
				moreMenu, SLOT(deleteLater() ) );
			moreMenu->plug( contactsMenu );
			contactsMenu = moreMenu->popupMenu();
			contactCount = 0;
		}
	}
}

void KopeteChatWindow::slotPreparePlacementMenu()
{
	QPopupMenu *placementMenu = actionTabPlacementMenu->popupMenu();
	placementMenu->clear();

	placementMenu->insertItem( i18n("Top"), 0 );
	placementMenu->insertItem( i18n("Bottom"), 1 );
}

void KopeteChatWindow::slotPlaceTabs( int placement )
{
	if( m_tabBar )
	{

		if( placement == 0 )
			m_tabBar->setTabPosition( QTabWidget::Top );
		else
			m_tabBar->setTabPosition( QTabWidget::Bottom );

		saveOptions();
	}
}

void KopeteChatWindow::readOptions()
{
	// load and apply config file settings affecting the appearance of the UI
//	kdDebug(14010) << k_funcinfo << endl;
	KConfig *config = KGlobal::config();
	applyMainWindowSettings( config, QString::fromLatin1( "KopeteChatWindow" ) );
	config->setGroup( QString::fromLatin1("ChatWindowSettings") );
	m_showFormatToolbar = config->readBoolEntry( QString::fromLatin1("Show Format Toolbar"), true );
}

void KopeteChatWindow::saveOptions()
{
//	kdDebug(14010) << k_funcinfo << endl;

	KConfig *config = KGlobal::config();

	// saves menubar,toolbar and statusbar setting
	saveMainWindowSettings( config, QString::fromLatin1( "KopeteChatWindow" ) );
	config->setGroup( QString::fromLatin1("ChatWindowSettings") );
	if( m_tabBar )
		config->writeEntry ( QString::fromLatin1("Tab Placement"), m_tabBar->tabPosition() );

	config->writeEntry( QString::fromLatin1("Show Format Toolbar"), m_showFormatToolbar );
	config->sync();
}

void KopeteChatWindow::slotChatSave()
{
//	kdDebug(14010) << "KopeteChatWindow::slotChatSave()" << endl;
	if( isActiveWindow() && m_activeView )
		m_activeView->messagePart()->save();
}

void KopeteChatWindow::windowActivationChange( bool )
{
	if( isActiveWindow() && m_activeView )
		m_activeView->setActive( true );
}

void KopeteChatWindow::slotChatPrint()
{
	m_activeView->messagePart()->print();
}

void KopeteChatWindow::slotToggleStatusBar()
{
	if (statusBar()->isVisible())
		statusBar()->hide();
	else
		statusBar()->show();
}

void KopeteChatWindow::slotToggleFormatToolbar(bool visible)
{
	if ( adjustingFormatToolbar )
		return;
	m_showFormatToolbar = visible;
}

void KopeteChatWindow::slotViewMenuBar()
{
	if( !menuBar()->isHidden() )
		menuBar()->hide();
	else
		menuBar()->show();
}

void KopeteChatWindow::slotSmileyActivated(const QString &sm)
{
	if ( !sm.isNull() )
		m_activeView->addText( " " + sm + " " );
	//we are adding space around the emoticon becasue our parser only display emoticons not in a word.
}

void KopeteChatWindow::slotRTFEnabled( ChatView* cv, bool enabled)
{
	if ( cv != m_activeView )
		return;

	adjustingFormatToolbar = true;
	if ( enabled && m_showFormatToolbar )
		toolBar( "formatToolBar" )->show();
	else
		toolBar( "formatToolBar" )->hide();
	adjustingFormatToolbar = false;
	updateSpellCheckAction();
}

void KopeteChatWindow::slotAutoSpellCheckEnabled( ChatView* view, bool isEnabled )
{
	if ( view != m_activeView )
		return;

	toggleAutoSpellCheck->setEnabled( isEnabled );
	toggleAutoSpellCheck->setChecked( isEnabled );
	m_activeView->editPart()->toggleAutoSpellCheck( isEnabled );
}

bool KopeteChatWindow::queryClose()
{
	bool canClose = true;

//	kdDebug( 14010 ) << " Windows left open:" << endl;
//	for( QPtrListIterator<ChatView> it( chatViewList ); it; ++it)
//		kdDebug( 14010 ) << "  " << *it << " (" << (*it)->caption() << ")" << endl;

	for( QPtrListIterator<ChatView> it( chatViewList ); it; )
	{
		ChatView *view = *it;
		// move out of the way before view is removed
		++it;

		// FIXME: This should only check if it *can* close
		// and not start closing if the close can be aborted halfway, it would
		// leave us with half the chats open and half of them closed. - Martijn

		// if the view is closed, it is removed from chatViewList for us
		if ( !view->closeView() )
		{
			kdDebug() << k_funcinfo << "Closing view failed!" << endl;
			canClose = false;
		}
	}
	return canClose;
}

bool KopeteChatWindow::queryExit()
{
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
 	if ( app->sessionSaving()
		|| app->isShuttingDown() /* only set if KopeteApplication::quitKopete() or
									KopeteApplication::commitData() called */
		|| !KopetePrefs::prefs()->showTray() /* also close if our tray icon is hidden! */
		|| !isShown() )
	{
		Kopete::PluginManager::self()->shutdown();
		return true;
	}
	else
		return false;
}

void KopeteChatWindow::closeEvent( QCloseEvent * e )
{
	// if there's a system tray applet and we are not shutting down then just do what needs to be done if a
	// window is closed.
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
	if ( KopetePrefs::prefs()->showTray() && !app->isShuttingDown() && !app->sessionSaving() ) {
//		hide();
		// BEGIN of code borrowed from KMainWindow::closeEvent
		// Save settings if auto-save is enabled, and settings have changed
		if ( settingsDirty() && autoSaveSettings() )
			saveAutoSaveSettings();

		if ( queryClose() ) {
			e->accept();
		}
		// END of code borrowed from KMainWindow::closeEvent
	}
	else
		KMainWindow::closeEvent( e );
}

void KopeteChatWindow::slotConfKeys()
{
	KKeyDialog dlg( false, this );
	dlg.insert( actionCollection() );
	if( m_activeView )
	{
		dlg.insert(m_activeView->msgManager()->actionCollection() , i18n("Plugin Actions") );
		QPtrListIterator<KXMLGUIClient> it( *m_activeView->msgManager()->childClients() );
		KXMLGUIClient *c = 0;
		while( (c = it.current()) != 0 )
		{
			dlg.insert( c->actionCollection() /*, i18n("Plugin Actions")*/ );
			++it;
		}

		if( m_activeView->editPart() )
			dlg.insert( m_activeView->editPart()->actionCollection(), m_activeView->editPart()->name() );
	}

	dlg.configure();
}

void KopeteChatWindow::slotConfToolbar()
{
	saveMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteChatWindow" ));
	KEditToolbar *dlg = new KEditToolbar(factory(), this );
	if (dlg->exec())
	{
		if( m_activeView )
			createGUI( m_activeView->editPart() );
		else
			createGUI( 0L );
		applyMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteChatWindow" ));
	}
	delete dlg;
}

void KopeteChatWindow::updateChatState( ChatView* cv, int newState )
{
	if ( m_tabBar )
	{
		switch( newState )
		{
			case ChatView::Highlighted:
				m_tabBar->setTabColor( cv, Qt::blue );
				break;
			case ChatView::Message:
				m_tabBar->setTabColor( cv, Qt::red );
				break;
			case ChatView::Changed:
				m_tabBar->setTabColor( cv, Qt::darkRed );
				break;
			case ChatView::Typing:
				m_tabBar->setTabColor( cv, Qt::darkGreen );
				break;
			case ChatView::Normal:
			default:
				m_tabBar->setTabColor( cv, KGlobalSettings::textColor() );
				break;
		}
	}
}

void KopeteChatWindow::updateChatTooltip( ChatView* cv )
{
	if ( m_tabBar )
		m_tabBar->setTabToolTip( cv, QString::fromLatin1("<qt>%1</qt>").arg( cv->caption() ) );
}

void KopeteChatWindow::updateChatLabel()
{
	const ChatView* cv = dynamic_cast<const ChatView*>( sender() );
	if ( !cv || !m_tabBar )
		return;

	ChatView* chat  = const_cast<ChatView*>( cv );
	if ( m_tabBar )
	{
		m_tabBar->setTabLabel( chat, chat->caption() );
		if ( m_tabBar->count() < 2 || m_tabBar->currentPage() == static_cast<const QWidget *>(cv) )
			setCaption( chat->caption() );
	}
}

#include "kopetechatwindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

