/*
    kopeteemailwindow.cpp - Kopete "email window" for single-shot messages

    Copyright (c) 2002      by Daniel Stone          <dstone@kde.org>
    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>

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
#include <qlayout.h>
#include <qsplitter.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qmovie.h>

#include <kaction.h>
#include <kapplication.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kedittoolbar.h>
#include <kfontdialog.h>
#include <kglobalsettings.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include "kopeteplugin.h"
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <kstdaction.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <ktextedit.h>

#include "kopeteemoticons.h"
#include "kopetemessagemanager.h"
#include "kopeteprefs.h"
#include "pluginloader.h"
#include "kopetexsl.h"

#include "kopeteemailwindow.h"

class KopeteEmailWindowPrivate
{
public:
	QValueList<KopeteMessage> messageQueue;
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
	KHTMLView *htmlView;
	KHTMLPart *htmlPart;
	KopeteEmailWindow::WindowMode mode;
	KAction *chatSend;
	QLabel *anim;
	QMovie animIcon;
	QPixmap normalIcon;
	QString unreadMessageFrom;

	KActionMenu *actionActionMenu;
	KActionMenu *actionSmileyMenu;
};

KopeteEmailWindow::KopeteEmailWindow( KopeteMessageManager *manager, bool foreignMessage )
: KMainWindow( 0L ), KopeteView( manager )
{
	d = new KopeteEmailWindowPrivate;

	QVBox *v = new QVBox( this );
	setCentralWidget( v );

	setMinimumSize( QSize( 75, 20 ) );

	d->split = new QSplitter( v );
	d->split->setOrientation( QSplitter::Vertical );

	d->htmlPart = new KHTMLPart( d->split );

	//Security settings, we don't need this stuff
	d->htmlPart->setJScriptEnabled( false ) ;
	d->htmlPart->setJavaEnabled( false );
	d->htmlPart->setPluginsEnabled( false );
	d->htmlPart->setMetaRefreshEnabled( false );

	d->htmlView = d->htmlPart->view();
	d->htmlView->setMarginWidth( 4 );
	d->htmlView->setMarginHeight( 4 );
	d->htmlView->setMinimumSize( QSize( 75, 20 ) );
	connect ( d->htmlPart->browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
		SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );
	d->htmlView->setFocusPolicy( NoFocus );

	d->txtEntry = new KTextEdit( d->split );
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

	connect( manager, SIGNAL(messageSuccess()), this, SLOT(slotMessageSentSuccessfully()) );
	connect( KopetePrefs::prefs(), SIGNAL(messageAppearanceChanged()), this, SLOT( slotRefreshAppearance() ) );

	d->sendInProgress = false;

	d->normalIcon = QPixmap( BarIcon( QString::fromLatin1( "kopete" ) ) );
	d->animIcon = KGlobal::iconLoader()->loadMovie( QString::fromLatin1( "newmessage" ), KIcon::User);

	d->anim = new QLabel( toolBar(), "kde toolbar widget" );
	d->anim->setMargin(5);
	d->anim->setPixmap( d->normalIcon );
	toolBar()->insertWidget( 99, d->anim->width(), d->anim );
	toolBar()->alignItemRight( 99 );

	d->visible = false;
	d->queuePosition = 0;

	setCaption( manager->displayName() );

	m_type = KopeteMessage::Email;

	kdDebug(14010) << k_funcinfo << endl;
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
	//Default to "send" shortcut as used by KMail and KNode
	d->chatSend->setShortcut( QKeySequence( CTRL + Key_Return ) );
	d->chatSend->setEnabled( false );

	KStdAction::quit ( this, SLOT( closeView() ), coll );

	KStdAction::cut( d->txtEntry, SLOT( cut() ), coll );
	KStdAction::copy( this, SLOT(slotCopy()), coll);
	KStdAction::paste( d->txtEntry, SLOT( paste() ), coll );

	new KAction( i18n( "&Set Font..." ), QString::fromLatin1( "charset" ), 0,
		this, SLOT( slotSetFont() ), coll, "format_font" );
	new KAction( i18n( "Set Text &Color..." ), QString::fromLatin1( "pencil" ), 0,
		this, SLOT( slotSetFgColor() ), coll, "format_color" );
	new KAction( i18n( "Set &Background Color..." ), QString::fromLatin1( "fill" ), 0,
		this, SLOT( slotSetBgColor() ), coll, "format_bgcolor" );

	// THIS CODE IS SPECIFIC TO KDE < 3.1
	// maybe #if KDE_VERSION < 310 around it?
	KStdAction::showMenubar( this, SLOT( slotViewMenuBar() ), coll );
	KStdAction::showToolbar( this, SLOT( slotViewToolBar() ), coll );

	d->actionSmileyMenu = new KActionMenu( i18n( "Add Smiley" ), QString::fromLatin1( "emoticon" ), coll, "format_smiley" );
	d->actionSmileyMenu->setDelayed( false );

	d->actionActionMenu = new KActionMenu( i18n( "&Actions" ), coll, "actions_menu" );
	connect( d->actionActionMenu->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotPrepareActionMenu() ) );

	connect( d->actionSmileyMenu->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT( slotPrepareSmileyMenu() ) );
	connect( d->actionSmileyMenu->popupMenu(), SIGNAL( activated( int ) ), this, SLOT( slotSmileyActivated( int ) ) );

	// add configure key bindings menu item
	KStdAction::keyBindings(this, SLOT(slotConfKeys()), coll);
	KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), coll);

	createGUI( QString::fromLatin1( "kopeteemailwindow.rc" ) );
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

void KopeteEmailWindow::slotSmileyActivated(int sm)
{
	// FIXME: it should read from the emoticonlist instead of using the text
	d->txtEntry->setText( d->txtEntry->text().append( d->actionSmileyMenu->popupMenu()->text( sm ) ) );
}

void KopeteEmailWindow::slotConfKeys()
{
	KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}

void KopeteEmailWindow::slotConfToolbar()
{
	saveMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteChatWindow" ));
	KEditToolbar *dlg = new KEditToolbar(actionCollection(), QString::fromLatin1("kopetechatwindow.rc") );
	if (dlg->exec())
	{
		createGUI( QString::fromLatin1("kopetechatwindow.rc") );
		applyMainWindowSettings(KGlobal::config(), QString::fromLatin1( "KopeteChatWindow" ));
	}
	delete dlg;
}

void KopeteEmailWindow::slotPrepareActionMenu(void)
{
	QPopupMenu *actionsMenu = d->actionActionMenu->popupMenu();

	actionsMenu->clear();

	QPtrList<KopetePlugin> ps = LibraryLoader::pluginLoader()->plugins();
	bool actions = false;

	for( KopetePlugin *p = ps.first() ; p ; p = ps.next() )
	{
		KActionCollection *customActions = p->customChatActions( m_manager );
		if( customActions )
		{
			kdDebug(14010) << k_funcinfo << "Found custom Actions defined by Plugins" << endl;
			actions = true;
			for(unsigned int i = 0; i < customActions->count(); i++)
			{
				customActions->action(i)->plug( actionsMenu );
			}
		}
	}

	if ( !actions )
	{
		kdDebug(14010) << k_funcinfo << "No Action defined by any Plugin" << endl;
		int id = actionsMenu->insertItem( i18n("No Action Defined by Any Plugin") );
		actionsMenu->setItemEnabled(id, false);
	}
}

void KopeteEmailWindow::slotPrepareSmileyMenu(void)
{
	QPopupMenu *smileyMenu = d->actionSmileyMenu->popupMenu();

	smileyMenu->clear();

	QMap<QString, QString> list = KopeteEmoticons::emoticons()->emoticonAndPicList();

	for (QMap<QString, QString>::Iterator it = list.begin(); it != list.end(); ++it )
		smileyMenu->insertItem(QPixmap( it.data() ),it.key());
}

bool KopeteEmailWindow::queryExit()
{
//	 never quit kopete
	return false;
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
	kdDebug(14010) << k_funcinfo << endl;

	if ( d->htmlPart->hasSelection() )
		QApplication::clipboard()->setText( d->htmlPart->selectedText() );
	else
		d->txtEntry->copy();
}


void KopeteEmailWindow::messageReceived(KopeteMessage &message)
{
	kdDebug(14010) << k_funcinfo << endl;

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

		d->unreadMessageFrom = message.from()->displayName();
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
	kdDebug(14010) << k_funcinfo << endl;

	d->blnShowingMessage = true;

	const QString model = KopetePrefs::prefs()->styleContents();

	d->queuePosition++;

	writeMessage( (*d->messageQueue.at( d->queuePosition - 1 )) );

	updateNextButton();
}

void KopeteEmailWindow::slotReadPrev()
{
	kdDebug(14010) << k_funcinfo << endl;

	d->blnShowingMessage = true;

	d->queuePosition--;

	writeMessage( (*d->messageQueue.at( d->queuePosition - 1 )) );

	updateNextButton();
}

void KopeteEmailWindow::slotRefreshAppearance()
{
	KopeteMessage m = currentMessage();
	writeMessage( m );
}

void KopeteEmailWindow::writeMessage( KopeteMessage &msg )
{
	const QString model = KopetePrefs::prefs()->styleContents();

	d->htmlPart->begin();
	d->htmlPart->write( QString::fromLatin1( "<html><head><style>body{font-family:%1;font-size:%2pt;color:%3}td{font-family:%4;font-size:%5pt;color:%6}</style></head><body style=\"background-repeat:no-repeat;background-attachment:fixed\" bgcolor=\"%7\" vlink=\"%9\" link=\"%9\">%10</body></html>" )
		.arg( KopetePrefs::prefs()->fontFace().family() )
		.arg( KopetePrefs::prefs()->fontFace().pointSize() )
		.arg( KopetePrefs::prefs()->textColor().name() )
		.arg( KopetePrefs::prefs()->fontFace().family() )
		.arg( KopetePrefs::prefs()->fontFace().pointSize() )
		.arg( KopetePrefs::prefs()->textColor().name() )
		.arg( KopetePrefs::prefs()->bgColor().name() )
		.arg( KopetePrefs::prefs()->linkColor().name() )
		.arg( KopetePrefs::prefs()->linkColor().name() )
		.arg( KopeteXSL::xsltTransform( msg.asXML().toString(), model ) ) );
	d->htmlPart->end();
}

void KopeteEmailWindow::sendMessage()
{
	d->sendInProgress = true;
	d->anim->setMovie( d->animIcon );
	d->btnReplySend->setEnabled( false );
	d->txtEntry->setEnabled( false );
	KopeteMessage sentMessage = currentMessage();
	emit messageSent( sentMessage );
}

void KopeteEmailWindow::messageSentSuccessfully()
{
	d->sendInProgress = false;
	d->anim->setPixmap( d->normalIcon );
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

			response = KMessageBox::warningContinueCancel(this, i18n("You are about to leave the group chat session \"%1\"."
				"You will not receive future messages from this conversation").arg(shortCaption), i18n("Closing Group Chat"),
				i18n("&Close Chat"), i18n("Do not ask me this again"));
		}

		if( !d->unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have recieved a message from \"%1\" in the last "
				"second, are you sure you want to close this chat?").arg(d->unreadMessageFrom), i18n("Unread Message"),
				i18n("&Close Chat"), i18n("Do not ask me this again"));
		}

		if( d->sendInProgress  && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel(this, i18n("You have a message send in progress, which will be "
				"aborted if this window is closed. Are you sure you want to close this chat?"), i18n("Message In Transit"),
				i18n("&Close Chat"), i18n("Do not ask me this again"));
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
			d->htmlView->hide();
			d->btnReadNext->hide();
			break;
		case Read:
			d->btnReplySend->setText( i18n( "Reply" ) );
			d->btnReplySend->setEnabled( true );
			d->txtEntry->hide();
			d->htmlView->show();
			d->btnReadNext->show();
			break;
		case Reply:
			QValueList<int> splitPercent;
			splitPercent.append(50);
			splitPercent.append(50);
			d->btnReplySend->setText( i18n( "Send" ) );
			slotTextChanged();
			d->txtEntry->show();
			d->htmlView->show();
			d->btnReadNext->show();
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

void KopeteEmailWindow::raise()
{
	makeVisible();
	KWin::setOnDesktop( winId(), KWin::currentDesktop() );
	setActiveWindow();
	KMainWindow::raise();
}

void KopeteEmailWindow::windowActivationChange( bool )
{
	if( isActiveWindow() )
		emit( activated( static_cast<KopeteView*>(this) ) );
}

void KopeteEmailWindow::makeVisible()
{
	kdDebug(14010) << k_funcinfo << endl;
	d->visible = true;
	show();
}

bool KopeteEmailWindow::isVisible()
{
	return d->visible;
}

KopeteMessage KopeteEmailWindow::currentMessage()
{
	KopeteMessage currentMsg = KopeteMessage( m_manager->user(), m_manager->members(), d->txtEntry->text(),
		KopeteMessage::Outbound, KopeteMessage::PlainText );

	currentMsg.setFont( d->font );
	currentMsg.setBg( d->bgColor );
	currentMsg.setFg( d->fgColor );

	return currentMsg;
}

void KopeteEmailWindow::setCurrentMessage( const KopeteMessage &newMessage )
{
	d->txtEntry->setText( newMessage.plainBody() );
}

void KopeteEmailWindow::slotOpenURLRequest(const KURL &url, const KParts::URLArgs & /* args */ )
{
	kdDebug(14010) << k_funcinfo << "url=" << url.url() << endl;

	// FIXME: Doesn't KRun do the mime type check automagically for us? - Martijn
	if( url.protocol() == QString::fromLatin1( "mailto" ) )
		kapp->invokeMailer( url.url() );
	else
		kapp->invokeBrowser( url.url() );
}

QTextEdit * KopeteEmailWindow::editWidget()
{
	return d->txtEntry;
}

#include "kopeteemailwindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

