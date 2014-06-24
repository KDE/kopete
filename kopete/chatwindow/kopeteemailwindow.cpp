/*
    kopeteemailwindow.cpp - Kopete "email window" for single-shot messages

    Copyright (c) 2002      by Daniel Stone          <dstone@kde.org>
    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

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

#include "kopeteemailwindow.h"

#include "chatmessagepart.h"
#include "chattexteditpart.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteemoticonaction.h"
#include "kopetechatsession.h"
#include "kopeteplugin.h"
#include "kopetepluginmanager.h"
#include "kopetestdaction.h"
#include "kopeteviewmanager.h"

#include <kaction.h>
#include <ktoolbarspaceraction.h>
#include <kstandardaction.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kedittoolbar.h>
#include <kfontdialog.h>
#include <kcolorscheme.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <kwindowsystem.h>
#include <kgenericfactory.h>
#include <kxmlguifactory.h>
#include <kvbox.h>
#include <ktoolbar.h>
#include <kicon.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <QPixmap>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QList>
#include <QMovie>
#include <QSplitter>
#include <kactioncollection.h>

K_PLUGIN_FACTORY( EmailWindowPluginFactory, registerPlugin<EmailWindowPlugin>(); )
K_EXPORT_PLUGIN( EmailWindowPluginFactory( "kopete_emailwindow" ) )

EmailWindowPlugin::EmailWindowPlugin(QObject *parent, const QVariantList &) :
	Kopete::ViewPlugin( EmailWindowPluginFactory::componentData(), parent )
{}

KopeteView* EmailWindowPlugin::createView( Kopete::ChatSession *manager )
{
	//TODO: foreignMessage, how will we do this cleanly?
	return new KopeteEmailWindow(manager,this, false);
}

class KopeteEmailWindow::Private
{
public:
	QList<Kopete::Message> messageQueue;
	bool showingMessage;
	bool sendInProgress;
	bool visible;
	int queuePosition;
	KPushButton *btnReplySend;
	KPushButton *btnReadNext;
	KPushButton *btnReadPrev;
	QSplitter *split;
	ChatMessagePart *messagePart;
	KopeteEmailWindow::WindowMode mode;
	KAction *chatSend;
	QLabel *anim;
	QMovie animIcon;
	QPixmap normalIcon;
	QString unreadMessageFrom;
	ChatTextEditPart *editPart;

	KActionMenu *actionActionMenu;
	KopeteEmoticonAction *actionSmileyMenu;
};

KopeteEmailWindow::KopeteEmailWindow( Kopete::ChatSession *manager, EmailWindowPlugin *parent, bool foreignMessage )
	:  KParts::MainWindow( ), KopeteView( manager, parent ), d( new Private )
{
	KVBox *v = new KVBox( this );
	setCentralWidget( v );

	setMinimumSize( QSize( 75, 20 ) );

	d->split = new QSplitter( v );
	d->split->setOrientation( Qt::Vertical );

	d->messagePart = new ChatMessagePart( manager, d->split);

	// FIXME: should this be in ChatView too? maybe move to ChatMessagePart?
	d->messagePart->view()->setMarginWidth( 4 );
	d->messagePart->view()->setMarginHeight( 4 );
	d->messagePart->view()->setMinimumSize( QSize( 75, 20 ) );

	d->editPart = new ChatTextEditPart( manager, d->split );

	/*
	FIXME: dude, wtf?
	QDomDocument doc = d->editPart->domDocument();
	QDomNode menu = doc.documentElement().firstChild();
	menu.removeChild( menu.firstChild() ); // Remove File
	menu.removeChild( menu.firstChild() ); // Remove Edit
	menu.removeChild( menu.firstChild() ); // Remove View
	menu.removeChild( menu.lastChild() ); //Remove Help

	doc.documentElement().removeChild( doc.documentElement().childNodes().item(1) ); //Remove MainToolbar
	doc.documentElement().removeChild( doc.documentElement().lastChild() ); // Remove Edit popup
	*/
	connect( d->editPart, SIGNAL(messageSent(Kopete::Message&)),
	         this, SIGNAL(messageSent(Kopete::Message&)) );
	connect( d->editPart, SIGNAL(canSendChanged(bool)),
	         this, SLOT(slotUpdateReplySend()) );
	connect( d->editPart, SIGNAL(typing(bool)),
		 manager, SLOT(typing(bool)) );

	//Connections to the manager and the ViewManager that every view should have
	connect( this, SIGNAL(closing(KopeteView*)),
		 KopeteViewManager::viewManager(), SLOT(slotViewDestroyed(KopeteView*)) );
	connect( this, SIGNAL(activated(KopeteView*)),
		 KopeteViewManager::viewManager(), SLOT(slotViewActivated(KopeteView*)) );
	connect( this, SIGNAL(messageSent(Kopete::Message&)),
		 manager, SLOT(sendMessage(Kopete::Message&)) );
	connect( manager, SIGNAL(messageSuccess()),
		 this, SLOT(messageSentSuccessfully()));

	QWidget *containerWidget = new QWidget( v );
	containerWidget->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );

	QHBoxLayout *h = new QHBoxLayout( containerWidget );
	h->setMargin( 4 );
	h->setSpacing( 4 );
	h->addStretch();

	d->btnReadPrev = new KPushButton( i18n( "<< Prev" ), containerWidget );
	connect( d->btnReadPrev, SIGNAL(pressed()), this, SLOT(slotReadPrev()) );
	h->addWidget( d->btnReadPrev, 0, Qt::AlignRight | Qt::AlignVCenter );
	d->btnReadPrev->setEnabled( false );

	d->btnReadNext = new KPushButton( i18n( "(0) Next >>" ), containerWidget );
	connect( d->btnReadNext, SIGNAL(pressed()), this, SLOT(slotReadNext()) );
	h->addWidget( d->btnReadNext, 0, Qt::AlignRight | Qt::AlignVCenter );

	d->btnReplySend = new KPushButton( containerWidget );
	connect( d->btnReplySend, SIGNAL(pressed()), this, SLOT(slotReplySend()) );
	h->addWidget( d->btnReplySend, 0, Qt::AlignRight | Qt::AlignVCenter );

	initActions();
//	setWFlags(Qt::WDestructiveClose);

	d->showingMessage = false;

	if( foreignMessage )
		toggleMode( Read );
	else
		toggleMode( Send );

	KSharedConfig::Ptr config = KGlobal::config();
	applyMainWindowSettings( config->group( QLatin1String( "KopeteEmailWindow" )  ) );

	d->sendInProgress = false;

	d->visible = false;
	d->queuePosition = 0;

	setCaption( manager->displayName() );

	slotUpdateReplySend();
}

KopeteEmailWindow::~KopeteEmailWindow()
{
	emit( closing( this ) );

	// saves menubar, toolbar and statusbar setting
	KConfigGroup cg( KGlobal::config(), QLatin1String( "KopeteEmailWindow" ) );
        saveMainWindowSettings( cg );
	cg.sync();

	delete d;
}

void KopeteEmailWindow::initActions(void)
{
	KActionCollection *coll = actionCollection();

	d->chatSend = new KAction( KIcon("mail-send"), i18n( "&Send Message" ), this );
        coll->addAction( "chat_send", d->chatSend );
	//Default to 'Return' for sending messages
	d->chatSend->setShortcut( QKeySequence( Qt::Key_Return ) );
	connect( d->chatSend, SIGNAL(triggered()), this, SLOT(slotReplySend()) );

	KStandardAction::quit ( this, SLOT(slotCloseView()), coll );

	KStandardAction::cut( d->editPart->widget(), SLOT(cut()), coll );
	KStandardAction::copy( this, SLOT(slotCopy()), coll);
	KStandardAction::paste( d->editPart->widget(), SLOT(paste()), coll );

	// FIXME: This code is not working. Slots setFont(), setForegroundColorColor() and setBackgroundColorColor do not exist
	// Disable it for now
#if 0
	KAction* action;
	action = new KAction( KIcon("preferences-desktop-font"), i18n( "&Set Font..." ), coll );
        coll->addAction( "format_font", action );
	connect( action, SIGNAL(triggered(bool)), d->editPart, SLOT(setFont()) );

	action = new KAction( KIcon("format-stroke-color"), i18n( "Set Text &Color..." ), coll );
        coll->addAction( "format_color", action );
	connect( action, SIGNAL(triggered()), d->editPart, SLOT(setForegroundColorColor()) );

	action = new KAction( KIcon("format-fill-color"), i18n( "Set &Background Color..." ), coll );
        coll->addAction( "format_bgcolor", action );
	connect( action, SIGNAL(triggered()), d->editPart, SLOT(setBackgroundColorColor()) );
#endif

	KStandardAction::showMenubar( this, SLOT(slotViewMenuBar()), coll );
	setStandardToolBarMenuEnabled( true );

	d->actionSmileyMenu = new KopeteEmoticonAction( coll );
        coll->addAction( "format_smiley", d->actionSmileyMenu );
	d->actionSmileyMenu->setDelayed( false );
	connect(d->actionSmileyMenu, SIGNAL(activated(QString)), this, SLOT(slotSmileyActivated(QString)));

	// add configure key bindings menu item
	KStandardAction::keyBindings( guiFactory(), SLOT(configureShortcuts()), coll );
	KStandardAction::configureToolbars(this, SLOT(slotConfToolbar()), coll);
	//FIXME: no longer works?
	KopeteStdAction::preferences( coll , "settings_prefs" );

	// The animated toolbarbutton
	d->normalIcon = QPixmap( BarIcon( QLatin1String( "kopete" ) ) );
//	d->animIcon = KGlobal::iconLoader()->loadMovie( QLatin1String( "newmessage" ), KIconLoader::Toolbar);
	d->animIcon.setPaused(true);

	d->anim = new QLabel( this );
	d->anim->setObjectName( QLatin1String("kde toolbar widget") );
	d->anim->setMargin( 5 );
	d->anim->setPixmap( d->normalIcon );

	KAction *spacerAction = new KToolBarSpacerAction( this );
	spacerAction->setText( i18n( "Spacer for Animation" ) );
	coll->addAction( "toolbar_spacer", spacerAction );

	KAction *animAction = new KAction( i18n("Toolbar Animation"), coll );
	coll->addAction( "toolbar_animation", animAction );
	animAction->setDefaultWidget( d->anim );

	setXMLFile( QLatin1String( "kopeteemailwindow.rc" ) );
	createGUI( d->editPart );
	//createGUI( QLatin1String( "kopeteemailwindow.rc" ) );
	guiFactory()->addClient(m_manager);
}

void KopeteEmailWindow::closeEvent( QCloseEvent *e )
{
	// DO NOT call base class's closeEvent - see comment in KopeteApplication constructor for reason

	// Save settings if auto-save is enabled, and settings have changed
	if ( settingsDirty() && autoSaveSettings() )
		saveAutoSaveSettings();

	e->accept();
}

void KopeteEmailWindow::slotViewMenuBar()
{
	if( !menuBar()->isHidden() )
		menuBar()->hide();
	else
		menuBar()->show();
}

void KopeteEmailWindow::slotSmileyActivated(const QString &sm )
{
	if ( !sm.isNull() )
		d->editPart->addText( sm );
}

void KopeteEmailWindow::slotConfToolbar()
{
        KConfigGroup cg( KGlobal::config(), QLatin1String( "KopeteEmailWindow" ) );
	saveMainWindowSettings( cg );
	QPointer <KEditToolBar> dlg = new KEditToolBar(actionCollection());
	dlg->setResourceFile("kopeteemailwindow.rc");
	if (dlg->exec())
	{
		createGUI( d->editPart );
		applyMainWindowSettings( cg );
	}
	delete dlg;
}

void KopeteEmailWindow::slotCopy()
{
//	kDebug(14010) ;

	if ( d->messagePart->hasSelection() )
		d->messagePart->copy();
	else
		d->editPart->textEdit()->copy();
}

void KopeteEmailWindow::appendMessage(Kopete::Message &message)
{
	if( message.from() != m_manager->myself() )
	{
		if( d->mode == Send )
			toggleMode( Reply );

		d->messageQueue.append( message );

		if( !d->showingMessage )
			slotReadNext();
		else
		{
			QPalette palette;
			palette.setColor(d->btnReadNext->foregroundRole(), QColor("red") );
			d->btnReadNext->setPalette(palette);

			updateNextButton();
		}

		d->unreadMessageFrom = message.from()->metaContact() ?
			message.from()->metaContact()->displayName() : message.from()->contactId();
		QTimer::singleShot( 1000, this, SLOT(slotMarkMessageRead()) );
	}
}

void KopeteEmailWindow::slotMarkMessageRead()
{
	d->unreadMessageFrom.clear();
}

void KopeteEmailWindow::updateNextButton()
{
	if( d->queuePosition == d->messageQueue.count() )
	{
		d->btnReadNext->setEnabled( false );

		QPalette palette;
		palette.setColor(d->btnReadNext->foregroundRole(), KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() );
		d->btnReadNext->setPalette(palette);
	}
	else
		d->btnReadNext->setEnabled( true );

	if( d->queuePosition == 1 )
		d->btnReadPrev->setEnabled( false );
	else
		d->btnReadPrev->setEnabled( true );

	d->btnReadNext->setText( i18n( "(%1) Next >>", d->messageQueue.count() - d->queuePosition ) );
}

void KopeteEmailWindow::slotUpdateReplySend()
{
	bool canSend;
	if( d->mode == Read )
		canSend = true;
	else
		canSend = d->editPart->canSend();

	d->btnReplySend->setEnabled( canSend );
	d->chatSend->setEnabled( canSend );
}

void KopeteEmailWindow::slotReadNext()
{
//	kDebug(14010) ;

	d->showingMessage = true;

	d->queuePosition++;

	writeMessage( (d->messageQueue[ d->queuePosition - 1 ]) );

	updateNextButton();
}

void KopeteEmailWindow::slotReadPrev()
{
//	kDebug(14010) ;

	d->showingMessage = true;

	d->queuePosition--;

	writeMessage( (d->messageQueue[ d->queuePosition - 1 ]) );

	updateNextButton();
}

void KopeteEmailWindow::writeMessage( Kopete::Message &msg )
{
	d->messagePart->clear();
	d->messagePart->appendMessage( msg );
}

void KopeteEmailWindow::sendMessage()
{
	if ( !d->editPart->canSend() )
		return;
	d->sendInProgress = true;
	d->anim->setMovie( &d->animIcon );
	d->animIcon.setPaused(false);
	d->editPart->widget()->setEnabled( false );
	d->editPart->sendMessage();
}

void KopeteEmailWindow::messageSentSuccessfully()
{
	d->sendInProgress = false;
	d->anim->setPixmap( d->normalIcon );
	d->animIcon.setPaused(true);
	closeView();
}

bool KopeteEmailWindow::closeView( bool force )
{
	int response = KMessageBox::Continue;

	if( !force )
	{
		if( m_manager->members().count() > 1 )
		{
			QString shortCaption = windowTitle();
			if( shortCaption.length() > 40 )
				shortCaption = shortCaption.left( 40 ) + QLatin1String("...");

			response = KMessageBox::warningContinueCancel(this, i18n("<qt>You are about to leave the groupchat session <b>%1</b>.<br />"
				"You will not receive future messages from this conversation.</qt>", shortCaption), i18n("Closing Group Chat"),
				KGuiItem( i18n("Cl&ose Chat") ), KStandardGuiItem::cancel(), QLatin1String("AskCloseGroupChat"));
		}

		if( !d->unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("<qt>You have received a message from <b>%1</b> in the last "
				"second. Are you sure you want to close this chat?</qt>", d->unreadMessageFrom), i18n("Unread Message"),
				KGuiItem( i18n("Cl&ose Chat") ), KStandardGuiItem::cancel(), QLatin1String("AskCloseChatRecentMessage"));
		}

		if( d->sendInProgress  && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have a message send in progress, which will be "
				"aborted if this chat is closed. Are you sure you want to close this chat?"), i18n("Message in Transit"),
				KGuiItem( i18n("Cl&ose Chat") ), KStandardGuiItem::cancel(), QLatin1String("AskCloseChatMessageInProgress") );
		}
	}

	if( response == KMessageBox::Continue )
	{
		d->visible = false;
		deleteLater();
		return true;
	}
	else
	{
		d->editPart->widget()->setEnabled( true );
	}

	return false;
}

void KopeteEmailWindow::toggleMode( WindowMode newMode )
{
	d->mode = newMode;

	switch( d->mode )
	{
		case Send:
			d->btnReplySend->setText( i18n( "Send" ) );
			d->editPart->widget()->show();
			d->messagePart->view()->hide();
			d->btnReadNext->hide();
			d->btnReadPrev->hide();
			break;
		case Read:
			d->btnReplySend->setText( i18n( "Reply" ) );
			d->editPart->widget()->hide();
			d->messagePart->view()->show();
			d->btnReadNext->show();
			d->btnReadPrev->show();
			break;
		case Reply:
			QList<int> splitPercent;
			// FIXME: should be saved and restored
			splitPercent.append(50);
			splitPercent.append(50);
			d->btnReplySend->setText( i18n( "Send" ) );
			d->editPart->widget()->show();
			d->messagePart->view()->show();
			d->btnReadNext->show();
			d->btnReadPrev->show();
			d->split->setSizes( splitPercent );
			d->editPart->widget()->setFocus();
			break;
	}
	slotUpdateReplySend();
}

void KopeteEmailWindow::slotReplySend()
{
	if( d->mode == Read )
		toggleMode( Reply );
	else
		sendMessage();
}

//FIXME: Activate bool no longer needed due to setActiveWindow not being allowed
void KopeteEmailWindow::raise(bool activate)
{
	makeVisible();
#ifdef Q_WS_X11
	if ( !KWindowSystem::windowInfo( winId(), NET::WMDesktop ).onAllDesktops() )
		KWindowSystem::setOnDesktop( winId(), KWindowSystem::currentDesktop() );
#endif
	KXmlGuiWindow::raise();

	/* Removed Nov 2003
	According to Zack, the user double-clicking a contact is not valid reason for a non-pager
	to grab window focus. While I don't agree with this, and it runs contradictory to every other
	IM out there, commenting this code out to agree with KWin policy.

	Redirect any bugs relating to the widnow now not grabbing focus on clicking a contact to KWin.
		- Jason K
	*/
	//Will not activate window if user was typing
	if(activate)
		KWindowSystem::activateWindow( winId() );
}

void KopeteEmailWindow::changeEvent( QEvent *e )
{
	if( e->type() == QEvent::ActivationChange && isActiveWindow() )
		emit( activated( static_cast<KopeteView*>(this) ) );
}

void KopeteEmailWindow::makeVisible()
{
//	kDebug(14010) ;
	d->visible = true;
	show();
}

bool KopeteEmailWindow::isVisible()
{
	return d->visible;
}

Kopete::Message KopeteEmailWindow::currentMessage()
{
	return d->editPart->contents();
}

void KopeteEmailWindow::setCurrentMessage( const Kopete::Message &newMessage )
{
	d->editPart->setContents( newMessage );
}

void KopeteEmailWindow::slotCloseView()
{
	closeView();
}


#include "kopeteemailwindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

