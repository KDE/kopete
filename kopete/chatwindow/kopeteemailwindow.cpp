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
#include "kopeteprefs.h"
#include "kopetestdaction.h"
#include "kopeteviewmanager.h"

#include <kaction.h>
#include <kapplication.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kedittoolbar.h>
#include <kfontdialog.h>
#include <kglobalsettings.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <kwin.h>
#include <kgenericfactory.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvbox.h>

typedef KGenericFactory<EmailWindowPlugin> EmailWindowPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_emailwindow, EmailWindowPluginFactory( "kopete_emailwindow" )  )

EmailWindowPlugin::EmailWindowPlugin(QObject *parent, const char *name, const QStringList &) :
	Kopete::ViewPlugin( EmailWindowPluginFactory::instance(), parent, name )
{}

KopeteView* EmailWindowPlugin::createView( Kopete::ChatSession *manager )
{
	//TODO: foreignMessage, how will we do this cleanly?
	return (KopeteView*)new KopeteEmailWindow(manager,this, false);
}

class KopeteEmailWindow::Private
{
public:
	QValueList<Kopete::Message> messageQueue;
	bool showingMessage;
	bool sendInProgress;
	bool visible;
	uint queuePosition;
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
	QVBox *v = new QVBox( this );
	setCentralWidget( v );

	setMinimumSize( QSize( 75, 20 ) );

	d->split = new QSplitter( v );
	d->split->setOrientation( QSplitter::Vertical );

	d->messagePart = new ChatMessagePart( manager, d->split, "messagePart" );

	// FIXME: should this be in ChatView too? maybe move to ChatMessagePart?
	d->messagePart->view()->setMarginWidth( 4 );
	d->messagePart->view()->setMarginHeight( 4 );
	d->messagePart->view()->setMinimumSize( QSize( 75, 20 ) );

	d->editPart = new ChatTextEditPart( manager, d->split, "editPart" );

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
	connect( d->editPart, SIGNAL( messageSent( Kopete::Message & ) ),
	         this, SIGNAL( messageSent( Kopete::Message & ) ) );
	connect( d->editPart, SIGNAL( canSendChanged( bool ) ),
	         this, SLOT( slotUpdateReplySend() ) );
	connect( d->editPart, SIGNAL( typing(bool) ),
		 manager, SIGNAL( typing(bool) ) );

	//Connections to the manager and the ViewManager that every view should have
	connect( this, SIGNAL( closing( KopeteView * ) ),
		 KopeteViewManager::viewManager(), SLOT( slotViewDestroyed( KopeteView * ) ) );
	connect( this, SIGNAL( activated( KopeteView * ) ),
		 KopeteViewManager::viewManager(), SLOT( slotViewActivated( KopeteView * ) ) );
	connect( this, SIGNAL( messageSent(Kopete::Message &) ),
		 manager, SLOT( sendMessage(Kopete::Message &) ) );
	connect( manager, SIGNAL( messageSuccess() ),
		 this, SLOT( messageSentSuccessfully() ));

	QWidget *containerWidget = new QWidget( v );
	containerWidget->setSizePolicy( QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );

	QHBoxLayout *h = new QHBoxLayout( containerWidget, 4, 4 );
	h->addStretch();

	d->btnReadPrev = new KPushButton( i18n( "<< Prev" ), containerWidget );
	connect( d->btnReadPrev, SIGNAL( pressed() ), this, SLOT( slotReadPrev() ) );
	h->addWidget( d->btnReadPrev, 0, Qt::AlignRight | Qt::AlignVCenter );
	d->btnReadPrev->setEnabled( false );

	d->btnReadNext = new KPushButton( i18n( "(0) Next >>" ), containerWidget );
	connect( d->btnReadNext, SIGNAL( pressed() ), this, SLOT( slotReadNext() ) );
	h->addWidget( d->btnReadNext, 0, Qt::AlignRight | Qt::AlignVCenter );

	d->btnReplySend = new KPushButton( containerWidget );
	connect( d->btnReplySend, SIGNAL( pressed() ), this, SLOT( slotReplySend() ) );
	h->addWidget( d->btnReplySend, 0, Qt::AlignRight | Qt::AlignVCenter );

	initActions();
	setWFlags(Qt::WDestructiveClose);

	d->showingMessage = false;

	if( foreignMessage )
		toggleMode( Read );
	else
		toggleMode( Send );

	KConfig *config = KGlobal::config();
	applyMainWindowSettings( config, QString::fromLatin1( "KopeteEmailWindow" )  );

	d->sendInProgress = false;

	toolBar()->alignItemRight( 99 );

	d->visible = false;
	d->queuePosition = 0;

	setCaption( manager->displayName() );

	slotUpdateReplySend();
}

KopeteEmailWindow::~KopeteEmailWindow()
{
	emit( closing( this ) );

	// saves menubar, toolbar and statusbar setting
	KConfig *config = KGlobal::config();
	saveMainWindowSettings( config, QString::fromLatin1( "KopeteEmailWindow" ) );
	config->sync();

	delete d;
}

void KopeteEmailWindow::initActions(void)
{
	KActionCollection *coll = actionCollection();

	d->chatSend = new KAction( i18n( "&Send Message" ), QString::fromLatin1( "mail_send" ), 0,
		this, SLOT( slotReplySend() ), coll, "chat_send" );
	//Default to 'Return' for sending messages
	d->chatSend->setShortcut( QKeySequence( Key_Return ) );

	KStdAction::quit ( this, SLOT( slotCloseView() ), coll );

	KStdAction::cut( d->editPart->widget(), SLOT( cut() ), coll );
	KStdAction::copy( this, SLOT(slotCopy()), coll);
	KStdAction::paste( d->editPart->widget(), SLOT( paste() ), coll );

	new KAction( i18n( "&Set Font..." ), QString::fromLatin1( "charset" ), 0,
	             d->editPart, SLOT( setFont() ), coll, "format_font" );
	new KAction( i18n( "Set Text &Color..." ), QString::fromLatin1( "pencil" ), 0,
	             d->editPart, SLOT( setFgColor() ), coll, "format_color" );
	new KAction( i18n( "Set &Background Color..." ), QString::fromLatin1( "fill" ), 0,
	             d->editPart, SLOT( setBgColor() ), coll, "format_bgcolor" );

	KStdAction::showMenubar( this, SLOT( slotViewMenuBar() ), coll );
	setStandardToolBarMenuEnabled( true );

	d->actionSmileyMenu = new KopeteEmoticonAction( coll, "format_smiley" );
	d->actionSmileyMenu->setDelayed( false );
	connect(d->actionSmileyMenu, SIGNAL(activated(const QString &)), this, SLOT(slotSmileyActivated(const QString &)));

	// add configure key bindings menu item
	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), coll );
	KStdAction::configureToolbars(this, SLOT( slotConfToolbar() ), coll);
	//FIXME: no longer works?
	KopeteStdAction::preferences( coll , "settings_prefs" );

	// The animated toolbarbutton
	d->normalIcon = QPixmap( BarIcon( QString::fromLatin1( "kopete" ) ) );
	d->animIcon = KGlobal::iconLoader()->loadMovie( QString::fromLatin1( "newmessage" ), KIcon::Toolbar);
	d->animIcon.pause();

	d->anim = new QLabel( this, "kde toolbar widget" );
	d->anim->setMargin( 5 );
	d->anim->setPixmap( d->normalIcon );
	new KWidgetAction( d->anim, i18n("Toolbar Animation"), 0, 0, 0, coll, "toolbar_animation" );

	setXMLFile( QString::fromLatin1( "kopeteemailwindow.rc" ) );
	createGUI( d->editPart );
	//createGUI( QString::fromLatin1( "kopeteemailwindow.rc" ) );
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
	saveMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteEmailWindow" ));
	KEditToolbar *dlg = new KEditToolbar(actionCollection(), QString::fromLatin1("kopeteemailwindow.rc") );
	if (dlg->exec())
	{
		createGUI( d->editPart );
		applyMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteEmailWindow" ));
	}
	delete dlg;
}

void KopeteEmailWindow::slotCopy()
{
//	kdDebug(14010) << k_funcinfo << endl;

	if ( d->messagePart->hasSelection() )
		d->messagePart->copy();
	else
		d->editPart->widget()->copy();
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
			d->btnReadNext->setPaletteForegroundColor( QColor("red") );
			updateNextButton();
		}

		d->unreadMessageFrom = message.from()->metaContact() ?
			message.from()->metaContact()->displayName() : message.from()->contactId();
		QTimer::singleShot( 1000, this, SLOT(slotMarkMessageRead()) );
	}
}

void KopeteEmailWindow::slotMarkMessageRead()
{
	d->unreadMessageFrom = QString::null;
}

void KopeteEmailWindow::updateNextButton()
{
	if( d->queuePosition == d->messageQueue.count() )
	{
		d->btnReadNext->setEnabled( false );

		d->btnReadNext->setPaletteForegroundColor( KGlobalSettings::textColor() );
	}
	else
		d->btnReadNext->setEnabled( true );

	if( d->queuePosition == 1 )
		d->btnReadPrev->setEnabled( false );
	else
		d->btnReadPrev->setEnabled( true );

	d->btnReadNext->setText( i18n( "(%1) Next >>" ).arg( d->messageQueue.count() - d->queuePosition ) );
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
//	kdDebug(14010) << k_funcinfo << endl;

	d->showingMessage = true;

	d->queuePosition++;

	writeMessage( (*d->messageQueue.at( d->queuePosition - 1 )) );

	updateNextButton();
}

void KopeteEmailWindow::slotReadPrev()
{
//	kdDebug(14010) << k_funcinfo << endl;

	d->showingMessage = true;

	d->queuePosition--;

	writeMessage( (*d->messageQueue.at( d->queuePosition - 1 )) );

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
	d->anim->setMovie( d->animIcon );
	d->animIcon.unpause();
	d->editPart->widget()->setEnabled( false );
	d->editPart->sendMessage();
}

void KopeteEmailWindow::messageSentSuccessfully()
{
	d->sendInProgress = false;
	d->anim->setPixmap( d->normalIcon );
	d->animIcon.pause();
	closeView();
}

bool KopeteEmailWindow::closeView( bool force )
{
	int response = KMessageBox::Continue;

	if( !force )
	{
		if( m_manager->members().count() > 1 )
		{
			QString shortCaption = caption();
			if( shortCaption.length() > 40 )
				shortCaption = shortCaption.left( 40 ) + QString::fromLatin1("...");

			response = KMessageBox::warningContinueCancel(this, i18n("<qt>You are about to leave the group chat session <b>%1</b>.<br>"
				"You will not receive future messages from this conversation.</qt>").arg(shortCaption), i18n("Closing Group Chat"),
				i18n("Cl&ose Chat"), QString::fromLatin1("AskCloseGroupChat"));
		}

		if( !d->unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("<qt>You have received a message from <b>%1</b> in the last "
				"second. Are you sure you want to close this chat?</qt>").arg(d->unreadMessageFrom), i18n("Unread Message"),
				i18n("Cl&ose Chat"), QString::fromLatin1("AskCloseChatRecentMessage"));
		}

		if( d->sendInProgress  && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have a message send in progress, which will be "
				"aborted if this chat is closed. Are you sure you want to close this chat?"), i18n("Message in Transit"),
				i18n("Cl&ose Chat"), QString::fromLatin1("AskCloseChatMessageInProgress") );
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
			QValueList<int> splitPercent;
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

	if ( !KWin::windowInfo( winId(), NET::WMDesktop ).onAllDesktops() )
		KWin::setOnDesktop( winId(), KWin::currentDesktop() );

	KMainWindow::raise();

	/* Removed Nov 2003
	According to Zack, the user double-clicking a contact is not valid reason for a non-pager
	to grab window focus. While I don't agree with this, and it runs contradictory to every other
	IM out there, commenting this code out to agree with KWin policy.

	Redirect any bugs relating to the widnow now not grabbing focus on clicking a contact to KWin.
		- Jason K
	*/

	//Will not activate window if user was typing
	if(activate)
		KWin::activateWindow( winId() );
}

void KopeteEmailWindow::windowActivationChange( bool )
{
	if( isActiveWindow() )
		emit( activated( static_cast<KopeteView*>(this) ) );
}

void KopeteEmailWindow::makeVisible()
{
//	kdDebug(14010) << k_funcinfo << endl;
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

