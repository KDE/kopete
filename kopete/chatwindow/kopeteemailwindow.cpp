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

#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvbox.h>

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

#include "chatmessagepart.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteemoticonaction.h"
#include "kopetemessagemanager.h"
#include "kopeteplugin.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopetestdaction.h"
#include "kopetexsl.h"


class KopeteEmailWindowPrivate
{
public:
	QValueList<Kopete::Message> messageQueue;
	bool blnShowingMessage;
	bool sendInProgress;
	bool visible;
	uint queuePosition;
	QColor fgColor;
	QColor bgColor;
	QFont font;
	KPushButton *btnReplySend;
	KPushButton *btnReadNext;
	KPushButton *btnReadPrev;
	QTextEdit *txtEntry;
	QSplitter *split;
	ChatMessagePart *messagePart;
	KopeteEmailWindow::WindowMode mode;
	KAction *chatSend;
	QLabel *anim;
	QMovie animIcon;
	QPixmap normalIcon;
	QString unreadMessageFrom;
	KParts::Part *editpart;

	KActionMenu *actionActionMenu;
	KopeteEmoticonAction *actionSmileyMenu;

	Kopete::XSLT *xsltParser;
};

KopeteEmailWindow::KopeteEmailWindow( Kopete::ChatSession *manager, bool foreignMessage )
:  KParts::MainWindow( ), KopeteView( manager )
{
	d = new KopeteEmailWindowPrivate;

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

	d->editpart = 0L;
	//FIXME: use KopeteRichTextEditPart here!
	if(KopetePrefs::prefs()->richText())
	{
		KLibFactory *factory = KLibLoader::self()->factory("libkrichtexteditpart");
		if ( factory )
		{
			d->editpart = dynamic_cast<KParts::Part*> (factory->create( d->split, "krichtexteditpart", "KParts::ReadWritePart" ) );
		}
	}

	if ( d->editpart )
	{
		QDomDocument doc = d->editpart->domDocument();
		QDomNode menu = doc.documentElement().firstChild();
		menu.removeChild( menu.firstChild() ); // Remove File
		menu.removeChild( menu.firstChild() ); // Remove Edit
		menu.removeChild( menu.firstChild() ); // Remove View
		menu.removeChild( menu.lastChild() ); //Remove Help

		doc.documentElement().removeChild( doc.documentElement().childNodes().item(1) ); //Remove MainToolbar
		doc.documentElement().removeChild( doc.documentElement().lastChild() ); // Remove Edit popup
		d->txtEntry = static_cast<KTextEdit*>( d->editpart->widget() );
	}
	else
	{
		d->txtEntry = new KTextEdit( d->split );
	}

	d->txtEntry->setMinimumSize( QSize( 75, 20 ) );
	d->txtEntry->setWordWrap( QTextEdit::WidgetWidth );
	d->txtEntry->setTextFormat( Qt::PlainText );
	d->txtEntry->setWrapPolicy( QTextEdit::AtWhiteSpace );
	connect( d->txtEntry, SIGNAL( textChanged()), this, SLOT( slotTextChanged() ) );

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
	connect( d->btnReplySend, SIGNAL( pressed() ), this, SLOT( slotReplySendClicked() ) );
	h->addWidget( d->btnReplySend, 0, Qt::AlignRight | Qt::AlignVCenter );

	initActions();
	setWFlags(Qt::WDestructiveClose);

	d->blnShowingMessage = false;
	
	if( foreignMessage )
		toggleMode( Read );
	else
		toggleMode( Send );

	KConfig *config = KGlobal::config();
	applyMainWindowSettings( config, QString::fromLatin1( "KopeteEmailWindow" )  );

	config->setGroup( QString::fromLatin1("KopeteEmailWindowSettings") );

	QFont tmpFont = KGlobalSettings::generalFont();
	slotSetFont( config->readFontEntry( QString::fromLatin1("Font"), &tmpFont) );

	QColor tmpColor = KGlobalSettings::textColor();
	slotSetFgColor( config->readColorEntry ( QString::fromLatin1("TextColor"), &tmpColor ) );

	tmpColor = KGlobalSettings::baseColor();
	slotSetBgColor( config->readColorEntry ( QString::fromLatin1("BackgroundColor"), &tmpColor) );

	d->sendInProgress = false;

	toolBar()->alignItemRight( 99 );

	d->visible = false;
	d->queuePosition = 0;

	setCaption( manager->displayName() );

	m_type = Kopete::Message::Email;

	d->txtEntry->installEventFilter( this );
	KCursor::setAutoHideCursor( d->txtEntry, true, true );
}

KopeteEmailWindow::~KopeteEmailWindow()
{
	emit( closing( this ) );

	KConfig *config = KGlobal::config();

	// saves menubar,toolbar and statusbar setting
	saveMainWindowSettings( config, QString::fromLatin1( "KopeteEmailWindow" ) );

	config->setGroup( QString::fromLatin1("KopeteEmailWindowSettings") );

	config->writeEntry( QString::fromLatin1( "Font" ), d->font );
	config->writeEntry( QString::fromLatin1( "TextColor" ), d->fgColor );
	config->writeEntry( QString::fromLatin1( "BackgroundColor" ), d->bgColor );

	config->sync();

	delete d;
}

void KopeteEmailWindow::initActions(void)
{
	KActionCollection *coll = actionCollection();

	d->chatSend = new KAction( i18n( "&Send Message" ), QString::fromLatin1( "mail_send" ), 0,
		this, SLOT( sendMessage() ), coll, "chat_send" );
	//Default to 'Return' for sending messages
	d->chatSend->setShortcut( QKeySequence( Key_Return ) );
	d->chatSend->setEnabled( false );

	KStdAction::quit ( this, SLOT( slotCloseView() ), coll );

	KStdAction::cut( d->txtEntry, SLOT( cut() ), coll );
	KStdAction::copy( this, SLOT(slotCopy()), coll);
	KStdAction::paste( d->txtEntry, SLOT( paste() ), coll );

	new KAction( i18n( "&Set Font..." ), QString::fromLatin1( "charset" ), 0,
		this, SLOT( slotSetFont() ), coll, "format_font" );
	new KAction( i18n( "Set Text &Color..." ), QString::fromLatin1( "pencil" ), 0,
		this, SLOT( slotSetFgColor() ), coll, "format_color" );
	new KAction( i18n( "Set &Background Color..." ), QString::fromLatin1( "fill" ), 0,
		this, SLOT( slotSetBgColor() ), coll, "format_bgcolor" );

	KStdAction::showMenubar( this, SLOT( slotViewMenuBar() ), coll );
	KStdAction::showToolbar( this, SLOT( slotViewToolBar() ), coll );

	d->actionSmileyMenu = new KopeteEmoticonAction( coll, "format_smiley" );
	d->actionSmileyMenu->setDelayed( false );
	connect(d->actionSmileyMenu, SIGNAL(activated(const QString &)), this, SLOT(slotSmileyActivated(const QString &)));

	// add configure key bindings menu item
	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), coll );
	KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), coll);
	KopeteStdAction::preferences( coll , "settings_prefs" );
	setStandardToolBarMenuEnabled( true );

	// The animated toolbarbutton
	d->normalIcon = QPixmap( BarIcon( QString::fromLatin1( "kopete" ) ) );
#if KDE_IS_VERSION(3, 1, 90)
	d->animIcon = KGlobal::iconLoader()->loadMovie( QString::fromLatin1( "newmessage" ), KIcon::Toolbar);
#else
	d->animIcon = KopeteCompat::loadMovie( QString::fromLatin1( "newmessage" ), KIcon::Toolbar);
#endif
	d->animIcon.pause();

	d->anim = new QLabel( this, "kde toolbar widget" );
	d->anim->setMargin(5);
	d->anim->setPixmap( d->normalIcon );
	//toolBar()->insertWidget( 99, d->anim->width(), d->anim );
	new KWidgetAction( d->anim , i18n("Toolbar Animation") , 0, 0 , 0 , coll , "toolbar_animation");

	setXMLFile( QString::fromLatin1( "kopeteemailwindow.rc" ) );
	createGUI( d->editpart );
	//createGUI( QString::fromLatin1( "kopeteemailwindow.rc" ) );
	guiFactory()->addClient(m_manager);
}

bool KopeteEmailWindow::eventFilter( QObject *o, QEvent *e )
{
	if ( o->inherits( "KTextEdit" ) )
		KCursor::autoHideEventFilter( o, e );

	if( e->type() == QEvent::KeyPress )
	{
		QKeyEvent *event = static_cast<QKeyEvent*>( e );
		KKey key( event );

		// NOTE:
		// shortcut.contains( key ) doesn't work. It was the old way we used to do it, but it is incorrect
		// because if you have a multi-key shortcut then pressing any of the keys in
		// that shortcut individually causes the shortcut to be activated.

		if( d->chatSend->isEnabled() )
		{
			for( uint i = 0; i < d->chatSend->shortcut().count(); i++ )
			{
				if( key == d->chatSend->shortcut().seq(i).key(0) )
				{
					sendMessage();
					return true;
				}
			}
		}
	}

	return false;
}

void KopeteEmailWindow::closeEvent( QCloseEvent *e )
{
	// DO NOT call base class's closeEvent - see comment in KopeteApplication constructor for reason

	// Save settings if auto-save is enabled, and settings have changed
	if ( settingsDirty() && autoSaveSettings() )
		saveAutoSaveSettings();

	e->accept();
}

void KopeteEmailWindow::slotViewToolBar()
{
	if(toolBar()->isVisible())
		toolBar()->hide();
	else
		toolBar()->show();
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
		d->txtEntry->insert( sm );
}

void KopeteEmailWindow::slotConfToolbar()
{
	saveMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteEmailWindow" ));
	KEditToolbar *dlg = new KEditToolbar(actionCollection(), QString::fromLatin1("kopeteemailwindow.rc") );
	if (dlg->exec())
	{
		createGUI( d->editpart );
		applyMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteEmailWindow" ));
	}
	delete dlg;
}


void KopeteEmailWindow::slotSetFont()
{
	KFontDialog::getFont( d->font, false, this );
	d->txtEntry->setFont( d->font );
}

void KopeteEmailWindow::slotSetFont( const QFont &newFont )
{
	d->font = newFont;
	d->txtEntry->setFont( d->font );
}

void KopeteEmailWindow::slotSetFgColor( const QColor &newColor )
{
	if( newColor == QColor() )
		KColorDialog::getColor( d->fgColor, this );
	else
		d->fgColor = newColor;

	QPalette pal = d->txtEntry->palette();
	pal.setColor( QPalette::Active, QColorGroup::Text, d->fgColor );
	pal.setColor( QPalette::Inactive, QColorGroup::Text, d->fgColor );

	// unsetPalette() so that color changes in kcontrol are honoured
	// if we ever have a subclass of KTextEdit, reimplement setPalette()
	// and check it there.
	if ( pal == QApplication::palette( d->txtEntry ) )
		d->txtEntry->unsetPalette();
	else
		d->txtEntry->setPalette( pal );
}

void KopeteEmailWindow::slotSetBgColor( const QColor &newColor )
{
	if( newColor == QColor() )
		KColorDialog::getColor( d->bgColor, this );
	else
		d->bgColor = newColor;

	QPalette pal = d->txtEntry->palette();
	pal.setColor( QPalette::Active,   QColorGroup::Base, d->bgColor );
	pal.setColor( QPalette::Inactive, QColorGroup::Base, d->bgColor );
	pal.setColor( QPalette::Disabled, QColorGroup::Base, d->bgColor );

	// unsetPalette() so that color changes in kcontrol are honoured
	// if we ever have a subclass of KTextEdit, reimplement setPalette()
	// and check it there.
	if ( pal == QApplication::palette( d->txtEntry ) )
		d->txtEntry->unsetPalette();
	else
		d->txtEntry->setPalette( pal );
}

void KopeteEmailWindow::slotCopy()
{
//	kdDebug(14010) << k_funcinfo << endl;

	if ( d->messagePart->hasSelection() )
		d->messagePart->copy();
	else
		d->txtEntry->copy();
}


void KopeteEmailWindow::appendMessage(Kopete::Message &message)
{
	if( message.from() != m_manager->user() )
	{
		if( d->mode == Send )
			toggleMode( Reply );

		d->messageQueue.append( message );

		if( !d->blnShowingMessage )
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

void KopeteEmailWindow::slotTextChanged()
{
	bool canSend = !d->txtEntry->text().isEmpty();
	if( d->mode != Read )
	{
		d->btnReplySend->setEnabled( canSend );
		d->chatSend->setEnabled( canSend );
	}
}

void KopeteEmailWindow::slotReadNext()
{
//	kdDebug(14010) << k_funcinfo << endl;

	d->blnShowingMessage = true;

	d->queuePosition++;

	writeMessage( (*d->messageQueue.at( d->queuePosition - 1 )) );

	updateNextButton();
}

void KopeteEmailWindow::slotReadPrev()
{
//	kdDebug(14010) << k_funcinfo << endl;

	d->blnShowingMessage = true;

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
	d->sendInProgress = true;
	d->anim->setMovie( d->animIcon );
	d->animIcon.unpause();
	d->btnReplySend->setEnabled( false );
	d->txtEntry->setEnabled( false );
	Kopete::Message sentMessage = currentMessage();
	emit messageSent( sentMessage );
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
		d->txtEntry->setEnabled( true );
		d->txtEntry->setText( QString::null );
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
			slotTextChanged();
			d->txtEntry->show();
			d->messagePart->view()->hide();
			d->btnReadNext->hide();
			d->btnReadPrev->hide();
			break;
		case Read:
			d->btnReplySend->setText( i18n( "Reply" ) );
			d->btnReplySend->setEnabled( true );
			d->txtEntry->hide();
			d->messagePart->view()->show();
			d->btnReadNext->show();
			d->btnReadPrev->show();
			break;
		case Reply:
			QValueList<int> splitPercent;
			splitPercent.append(50);
			splitPercent.append(50);
			d->btnReplySend->setText( i18n( "Send" ) );
			slotTextChanged();
			d->txtEntry->show();
			d->messagePart->view()->show();
			d->btnReadNext->show();
			d->btnReadPrev->show();
			d->split->setSizes( splitPercent );
			d->txtEntry->setFocus();
			break;
	}
}

void KopeteEmailWindow::slotReplySendClicked()
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
#if KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
		KWin::setActiveWindow( winId() );
#else
		KWin::activateWindow( winId() );
#endif
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
	Kopete::Message currentMsg = Kopete::Message( m_manager->user(), m_manager->members(), d->txtEntry->text(),
		Kopete::Message::Outbound, d->editpart ? Kopete::Message::RichText : Kopete::Message::PlainText );

	currentMsg.setFont( d->font );
	currentMsg.setBg( d->bgColor );
	currentMsg.setFg( d->fgColor );

	return currentMsg;
}

void KopeteEmailWindow::setCurrentMessage( const Kopete::Message &newMessage )
{
	d->txtEntry->setText( newMessage.plainBody() );
}

QTextEdit * KopeteEmailWindow::editWidget()
{
	return d->txtEntry;
}

void KopeteEmailWindow::slotCloseView()
{
	closeView();
}


#include "kopeteemailwindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

