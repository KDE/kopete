/*
    chatview.cpp - Chat View

    Copyright (c) 2008      by Benson Tsai           <btsai@vrwarp.com>
    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2008 by Benson Tsai                <btsai@vrwarp.com>

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

#include "chatmessagepart.h"
#include "chattexteditpart.h"
#include "kopetechatwindow.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetecontactlist.h"
#include "kopeteviewmanager.h"
#include "kopetebehaviorsettings.h"
#include "kopetechatwindowstylemanager.h"

#include <kconfig.h>
#include <ktabwidget.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstringhandler.h>
#include <kwindowsystem.h>
#include <kglobalsettings.h>
#include <kgenericfactory.h>
#include <khtmlview.h>
#include <kxmlguifactory.h>

#include <QTimer>
#include <QSplitter>
#include <Q3UriDrag>
#include <QScrollBar>

K_PLUGIN_FACTORY( ChatWindowPluginFactory, registerPlugin<ChatWindowPlugin>(); )
K_EXPORT_PLUGIN( ChatWindowPluginFactory( "kopete_chatwindow" ) )

ChatWindowPlugin::ChatWindowPlugin(QObject *parent, const QVariantList &) :
	Kopete::ViewPlugin( ChatWindowPluginFactory::componentData(), parent )
{
	// Load styles to make style fallback work
	ChatWindowStyleManager::self();
}

KopeteView* ChatWindowPlugin::createView( Kopete::ChatSession *manager )
{
    return new ChatView(manager,this);
}

class KopeteChatViewPrivate
{
public:
	QString captionText;
	QString statusText;
	bool isActive;
	bool sendInProgress;
	bool visibleMembers;
	bool warnGroupChat;
	QSplitter * splitter;
};

ChatView::ChatView( Kopete::ChatSession *mgr, ChatWindowPlugin *parent )
	 : KVBox( 0l ), KopeteView( mgr, parent )
         , d(new KopeteChatViewPrivate)
{
	d->isActive = false;
	d->visibleMembers = false;
	d->sendInProgress = false;

	KVBox *vbox=this;

	m_mainWindow = 0L;
	m_tabState = Normal;

	d->warnGroupChat = mgr->warnGroupChat();

	//FIXME: don't widgets start off hidden anyway?
	hide();

	d->splitter = new QSplitter( Qt::Vertical, vbox );

	//Create the view dock widget (KHTML Part), and set it to no docking (lock it in place)
	m_messagePart = new ChatMessagePart( mgr , this );

	//Create the bottom dock widget, with the edit area, statusbar and send button
	m_editPart = new ChatTextEditPart( mgr, vbox );

	d->splitter->addWidget(m_messagePart->view());
	d->splitter->addWidget(m_editPart->widget());
	d->splitter->setChildrenCollapsible( false );
	QList<int> sizes;
	sizes << 240 << 40;
	d->splitter->setSizes( sizes );

	// FIXME: is this used these days? it seems totally unnecessary
	connect( editPart(), SIGNAL(toolbarToggled(bool)), this, SLOT(slotToggleRtfToolbar(bool)) );

	connect( editPart(), SIGNAL(messageSent(Kopete::Message&)),
	         this, SIGNAL(messageSent(Kopete::Message&)) );
	connect( editPart(), SIGNAL(canSendChanged(bool)),
	         this, SIGNAL(canSendChanged(bool)) );
	connect( editPart(), SIGNAL(typing(bool)),
		 mgr, SLOT(typing(bool)) );
	connect( editPart()->textEdit(), SIGNAL(documentSizeUpdated(int)),
	         this, SLOT(slotRecalculateSize(int)) );

	//Set the view as the main widget
//	setView(viewDock);

	//It is possible to drag and drop on this widget.
	// I had to disable the acceptDrop in the khtml widget to be able to intercept theses events.
	setAcceptDrops(true);
//	viewDock->setAcceptDrops(false);

	//Manager signals
	connect( mgr, SIGNAL(displayNameChanged()),
	         this, SLOT(slotChatDisplayNameChanged()) );
	connect( mgr, SIGNAL(statusMessageChanged(Kopete::Contact*)),
	         this, SLOT(slotStatusMessageChanged( Kopete::Contact*)));
	connect( mgr, SIGNAL(contactAdded(const Kopete::Contact*,bool)),
	         this, SLOT(slotContactAdded(const Kopete::Contact*,bool)) );
	connect( mgr, SIGNAL(contactRemoved(const Kopete::Contact*,QString,Qt::TextFormat,bool)),
	         this, SLOT(slotContactRemoved(const Kopete::Contact*,QString,Qt::TextFormat,bool)) );
	connect( mgr, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
	         this, SLOT(slotContactStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
	connect( mgr, SIGNAL(remoteTyping(const Kopete::Contact*,bool)),
		 this, SLOT(remoteTyping(const Kopete::Contact*,bool)) );
	connect( mgr, SIGNAL(eventNotification(QString)),
		 this, SLOT(setStatusText(QString)) );

	//Connections to the manager and the ViewManager that every view should have
	connect( this, SIGNAL(closing(KopeteView*)),
		 KopeteViewManager::viewManager(), SLOT(slotViewDestroyed(KopeteView*)) );
	connect( this, SIGNAL(activated(KopeteView*)),
		 KopeteViewManager::viewManager(), SLOT(slotViewActivated(KopeteView*)) );
	connect( this, SIGNAL(messageSent(Kopete::Message&)),
		 mgr, SLOT(sendMessage(Kopete::Message&)) );
	connect( mgr, SIGNAL(messageSuccess()),
		 this, SLOT(messageSentSuccessfully()));

	// add contacts
	slotContactAdded( mgr->myself(), true );
	for ( int i = 0; i != mgr->members().size(); ++i )
	{
		slotContactAdded( mgr->members()[i], true );
	}

	setFocusProxy( editPart()->widget() );
	m_messagePart->view()->setFocusProxy( editPart()->widget() );
	editPart()->widget()->setFocus();

	slotChatDisplayNameChanged();

	// restore docking positions
	readOptions();
}

ChatView::~ChatView()
{
	emit( closing( static_cast<KopeteView*>(this) ) );

	saveOptions();

	delete d;
}

KTextEdit *ChatView::editWidget()
{
	return editPart()->textEdit();
}

QWidget *ChatView::mainWidget()
{
	return this;
}

bool ChatView::canSend() const
{
	return editPart()->canSend();
}

bool ChatView::canSendFile() const
{
	Kopete::ContactPtrList contacts = msgManager()->members();
	if ( contacts.count() != 1 )
		return false;

	return contacts.first()->canAcceptFiles();
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
	editPart()->textEdit()->cut();
}

void ChatView::copy()
{
	if ( messagePart()->hasSelection() )
		messagePart()->copy();
	else
		editPart()->textEdit()->copy();
}

void ChatView::paste()
{
	editPart()->textEdit()->paste();
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
	int response = KMessageBox::Continue;

	if ( !unreadMessageFrom.isNull() )
	{
		response = KMessageBox::warningContinueCancel( this, i18n("<qt>You have received a message from <b>%1</b> in the last "
			"second. Are you sure you want to clear this chat?</qt>", unreadMessageFrom ), i18n( "Unread Message" ),
			KGuiItem( i18nc( "@action:button", "Clear Chat" ) ), KStandardGuiItem::cancel(), QLatin1String("AskClearChatRecentMessage" ) );
	}

	if ( response == KMessageBox::Continue )
		messagePart()->clear();
}

void ChatView::resetFontAndColor()
{
	Kopete::ContactPtrList contacts = msgManager()->members();
	if ( contacts.count() != 1 )
		return;

	Kopete::MetaContact* mc = contacts.first()->metaContact();
	if ( !mc )
		return;

	QString contactListGroup = QString(QLatin1String("chatwindow_") + QString(mc->metaContactId()));
	KConfigGroup config = KGlobal::config()->group(contactListGroup);
	editPart()->resetConfig( config );
	config.sync();
}

void ChatView::raise( bool activate )
{
	// this shouldn't change the focus. When the window is raised when a new message arrives
	// if i am coding, or talking to someone else, i want to end my sentence before switching to
	// the other chat. i just want to KNOW and SEE the other chat to switch to it later
	// (except if activate==true)

	if ( !m_mainWindow || !m_mainWindow->isActiveWindow() || activate )
		makeVisible();
#ifdef Q_WS_X11
	if (!KWindowSystem::windowInfo(m_mainWindow->winId(), NET::WMDesktop).onAllDesktops())
	{
		if (Kopete::BehaviorSettings::self()->trayflashNotifySetCurrentDesktopToChatView() && activate)
			KWindowSystem::setCurrentDesktop(KWindowSystem::windowInfo(m_mainWindow->winId(), NET::WMDesktop).desktop());
		else
			KWindowSystem::setOnDesktop(m_mainWindow->winId(), KWindowSystem::currentDesktop());
	}
#endif
	if(m_mainWindow->isMinimized())
	{
		KWindowSystem::unminimizeWindow( m_mainWindow->winId());
	}


	m_mainWindow->raise();

	//Will not activate window if user was typing
	if ( activate )
		KWindowSystem::forceActiveWindow( m_mainWindow->winId() );

}

void ChatView::makeVisible()
{
	if ( !m_mainWindow )
	{
		m_mainWindow = KopeteChatWindow::window( m_manager );
		m_mainWindow->setObjectName( QLatin1String("KopeteChatWindow") );
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

bool ChatView::sendInProgress() const
{
	return d->sendInProgress;
}

bool ChatView::closeView( bool force )
{
	int response = KMessageBox::Continue;

	if ( !force )
	{
		if ( m_manager->members().count() > 1 && ! d->warnGroupChat )
		{
			QString shortCaption = d->captionText;
			shortCaption = KStringHandler::rsqueeze( shortCaption );

			response = KMessageBox::warningContinueCancel( this, i18n("<qt>You are about to leave the groupchat session <b>%1</b>.<br />"
				"You will not receive future messages from this conversation.</qt>", shortCaption ), i18n( "Closing Group Chat" ),
				KGuiItem( i18nc( "@action:button", "Close Chat" ) ), KStandardGuiItem::cancel(), QLatin1String( "AskCloseGroupChat" ) );
		}

		if ( !unreadMessageFrom.isNull() && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel( this, i18n("<qt>You have received a message from <b>%1</b> in the last "
				"second. Are you sure you want to close this chat?</qt>", unreadMessageFrom ), i18n( "Unread Message" ),
				KGuiItem( i18nc( "@action:button", "Close Chat" ) ), KStandardGuiItem::cancel(), QLatin1String("AskCloseChatRecentMessage" ) );
		}

		if ( d->sendInProgress && ( response == KMessageBox::Continue ) )
		{
			response = KMessageBox::warningContinueCancel( this, i18n( "You have a message send in progress, which will be "
				"aborted if this chat is closed. Are you sure you want to close this chat?" ), i18n( "Message in Transit" ),
				KGuiItem( i18nc( "@action:button", "Close Chat" ) ), KStandardGuiItem::cancel(), QLatin1String( "AskCloseChatMessageInProgress" ) );
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

ChatView::KopeteTabState ChatView::tabState() const
{
	return m_tabState;
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
		setStatusText( i18np( "One other person in the chat",
			       "%1 other people in the chat", m_manager->members().count() ) );
	}
}

void ChatView::setMainWindow( KopeteChatWindow* parent )
{
	if (m_mainWindow)
	{
		m_mainWindow->guiFactory()->removeClient(editPart());
	}
	
	m_mainWindow = parent;
	
	if (m_mainWindow)
	{
		m_mainWindow->guiFactory()->addClient(editPart());
	}
}

void ChatView::remoteTyping( const Kopete::Contact *contact, bool isTyping )
{
	TypingMap::iterator it = m_remoteTypingMap.find(contact);
	if (it != m_remoteTypingMap.end())
	{
		if (it.value()->isActive())
			it.value()->stop();
		delete it.value();
		m_remoteTypingMap.erase(it);
	}
	if( isTyping )
	{
		m_remoteTypingMap.insert( contact, new QTimer(this) );
		connect( m_remoteTypingMap[ contact ], SIGNAL(timeout()), SLOT(slotRemoteTypingTimeout()) );

		m_remoteTypingMap[ contact ]->setSingleShot( true );
		m_remoteTypingMap[ contact ]->start( 6000 );
	}

	// Loop through the map, constructing a string of people typing
	QStringList typingList;

	for( it = m_remoteTypingMap.begin(); it != m_remoteTypingMap.end(); ++it )
	{
		const Kopete::Contact *c = it.key();
		typingList.append( m_messagePart->formatName(c, Qt::PlainText) );
	}

	// Update the status area
	if( !typingList.isEmpty() )
	{
		if ( typingList.count() == 1 )
			setStatusText( i18n( "%1 is typing a message", typingList.first() ) );
		else
		{
			QString statusTyping = typingList.join( QLatin1String( ", " ) );
			setStatusText( i18nc( "%1 is a list of names", "%1 are typing a message", statusTyping ) );
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

void ChatView::slotRecalculateSize(int difference)
{
	// Firstly, save the scrollbar state
	QScrollBar *messageAreaScrollBar = messagePart()->view()->verticalScrollBar();
	bool isScrolledDown = messageAreaScrollBar->value() == messageAreaScrollBar->maximum();

	// Apply sizes changing to splitter
	QList<int> sizes = d->splitter->sizes();
	sizes.first() -= difference;
	sizes.last() += difference;
	d->splitter->setSizes(sizes);

	// Restore scrollbar state
	if (isScrolledDown) {
		messagePart()->keepScrolledDown();
	}
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

	foreach (const Kopete::Contact *c, msgManager()->members())
	{
		QString contactName = m_messagePart->formatName(c, Qt::PlainText);
		if ( c->metaContact() )
		{
			chatName.replace( c->metaContact()->displayName(), contactName );
		}
		else
		{
			chatName.replace( c->displayName(), contactName );
		}
	}


	if ( chatName != d->captionText )
		setCaption( chatName, true );
}

void ChatView::slotDisplayNameChanged( const QString &oldValue, const QString &newValue )
{
	if( Kopete::BehaviorSettings::self()->showEvents() )
	{
		if( oldValue != newValue )
			sendInternalMessage( i18n( "%1 is now known as %2", oldValue, newValue ) );
	}
}

void ChatView::slotStatusMessageChanged( Kopete::Contact* contact )
{
	if ( contact == m_manager->myself() )
		return;
	const QString contactName = m_messagePart->formatName(contact, Qt::PlainText);
	const QString statusTitle = contact->statusMessage().title();
	const QString statusMessage = contact->statusMessage().message();
	QString msg;
	if ( statusTitle.isEmpty() && statusMessage.isEmpty() )
		msg = i18nc( "%1 is a contact's name", "%1 deleted status message", contactName );
	else
	{
		if ( statusTitle.isEmpty() )
			msg = statusMessage;
		else if ( statusMessage.isEmpty() )
			msg = statusTitle;
		else
			msg = statusTitle + " - " + statusMessage;
		msg = i18nc( "%1 is a contact's name", "%1 changed status message: %2", contactName, msg );
	}

	sendInternalMessage( msg );
}

void ChatView::slotContactAdded(const Kopete::Contact *contact, bool suppress)
{
	if( contact->metaContact() && contact->metaContact() != Kopete::ContactList::self()->myself() )
	{
		connect( contact->metaContact(), SIGNAL(displayNameChanged(QString,QString)),
			this, SLOT(slotDisplayNameChanged(QString,QString)) );
	}
	else
	{
		connect( contact, SIGNAL(displayNameChanged(QString,QString)),
			this, SLOT(slotDisplayNameChanged(QString,QString)) );
	}

	const QString contactName = m_messagePart->formatName(contact, Qt::PlainText);
	if( !suppress && Kopete::BehaviorSettings::self()->showEvents() && m_manager->members().count() > 1 )
		sendInternalMessage(  i18n("%1 has joined the chat.", contactName) );

	if ( m_manager->members().count() == 1 )
	{
		connect( m_manager->members().first(), SIGNAL(canAcceptFilesChanged()), this, SIGNAL(canAcceptFilesChanged()) );
		updateChatState();
		emit updateStatusIcon( this );
		emit canAcceptFilesChanged();
	}
	else
		disconnect( m_manager->members().first(), SIGNAL(canAcceptFilesChanged()), this, SIGNAL(canAcceptFilesChanged()) );

	const QString statusTitle = contact->statusMessage().title();
	const QString statusMessage = contact->statusMessage().message();
	if ( contact != m_manager->myself() && ( !statusTitle.isEmpty() || !statusMessage.isEmpty() ) )
	{
		QString msg;
		if ( statusTitle.isEmpty() )
			msg = statusMessage;
		else if ( statusMessage.isEmpty() )
			msg = statusTitle;
		else
			msg = statusTitle + " - " + statusMessage;
		sendInternalMessage( i18n( "%1 status message is %2", contactName, msg ) );
	}
}

void ChatView::slotContactRemoved( const Kopete::Contact *contact, const QString &reason, Qt::TextFormat format, bool suppressNotification )
{
// 	kDebug(14000) ;
	if ( contact != m_manager->myself() )
	{
		TypingMap::iterator it = m_remoteTypingMap.find( contact );
		if ( it != m_remoteTypingMap.end() )
		{
			if ((*it)->isActive())
				(*it)->stop();
			delete (*it);
			m_remoteTypingMap.remove( contact );
		}

		// When the last person leaves, don't disconnect the signals, since we're in a one-to-one chat
		if ( m_manager->members().count() > 0 )
		{
			if( contact->metaContact() )
			{
				disconnect( contact->metaContact(), SIGNAL(displayNameChanged(QString,QString)),
				this, SLOT(slotDisplayNameChanged(QString,QString)) );
			}
			else
			{
				disconnect( contact, SIGNAL(displayNameChanged(QString,QString)),
				this, SLOT(slotDisplayNameChanged(QString,QString)) );
			}
		}

		if ( !suppressNotification && Kopete::BehaviorSettings::self()->showEvents() )
		{
			QString contactName = m_messagePart->formatName(contact, Qt::PlainText);
			if ( reason.isEmpty() )
				sendInternalMessage( i18n( "%1 has left the chat.", contactName ), format ) ;
			else
				sendInternalMessage( i18n( "%1 has left the chat (%2).", contactName, reason ), format);
		}

		disconnect( contact, SIGNAL(canAcceptFilesChanged()), this, SIGNAL(canAcceptFilesChanged()) );
	}

	updateChatState();
	emit updateStatusIcon( this );
	emit canAcceptFilesChanged();
}

QString& ChatView::caption() const
{
	 return d->captionText;
}

void ChatView::setCaption( const QString &text, bool modified )
{
// 	kDebug(14000) ;
	QString newCaption = text;

	//Save this caption
	d->captionText = text;

	//Turncate if needed
	newCaption = KStringHandler::rsqueeze( d->captionText, 20 );

	//Call the original set caption
	QWidget::setWindowTitle( newCaption );

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
				} // if it's an enternal message or a outgoing, fall thought
			default:
				updateChatState( Changed );
		}
	}

	if( message.direction() == Kopete::Message::Inbound )
	{
		unreadMessageFrom = m_messagePart->formatName ( message.from(), Qt::PlainText );
		QTimer::singleShot( 1000, this, SLOT(slotMarkMessageRead()) );
	}
	else
		unreadMessageFrom.clear();
}

void ChatView::sendFile()
{
	Kopete::ContactPtrList contacts = msgManager()->members();
	if ( contacts.count() != 1 )
		return;

	Kopete::Contact* contact = contacts.first();
	if ( contact->canAcceptFiles() )
		contact->sendFile();
}

void ChatView::slotMarkMessageRead()
{
	unreadMessageFrom.clear();
}

void ChatView::slotToggleRtfToolbar( bool enabled )
{
	emit rtfEnabled( this, enabled );
}

void ChatView::slotContactStatusChanged( Kopete::Contact *contact, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus )
{
 	kDebug(14000) << contact;
	bool inhibitNotification = ( newStatus.status() == Kopete::OnlineStatus::Unknown ||
	                             oldStatus.status() == Kopete::OnlineStatus::Unknown );
	if ( contact && Kopete::BehaviorSettings::self()->showEvents() && !inhibitNotification )
	{
		if ( contact->account() && contact == contact->account()->myself() )
		{
			// Separate notification for the 'self' contact
			if ( newStatus.status() != Kopete::OnlineStatus::Connecting )
				sendInternalMessage( i18n( "You are now marked as %1.", newStatus.description() ) );
		}
		else if ( !contact->account() || !contact->account()->suppressStatusNotification() )
		{
			// We shouldn't show an internal message if status have changed
			// but status visible by user hadn't. It can be happened
			// if contact changed Xtraz status (oscar/icq protocol)
			// In this case only status' metadata changes, and no need
			// to show this message (see bug #193402)
			if ( newStatus.status() != oldStatus.status() )
			{
				QString contactName = m_messagePart->formatName(contact, Qt::PlainText);
				sendInternalMessage( i18n( "%2 is now %1.",
					newStatus.description(), contactName ) );
			}
		}
	}

	// update the windows caption
	slotChatDisplayNameChanged();
	emit updateStatusIcon( this );
}

void ChatView::sendInternalMessage(const QString &msg, Qt::TextFormat format )
{
	// When closing kopete, some internal message may be sent because some contact are deleted
	// these contacts can already be deleted
	Kopete::Message message = Kopete::Message();
	message.setDirection( Kopete::Message::Internal );
	switch(format)
	{
		default:
		case Qt::PlainText:
			message.setPlainBody( msg );
			break;
		case Qt::RichText:
			message.setHtmlBody( msg );
			break;
	}

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
	KSharedConfig::Ptr config = KGlobal::config();
	KConfigGroup kopeteChatWindowMainWinSettings( config, ( msgManager()->form() == Kopete::ChatSession::Chatroom ? QLatin1String( "KopeteChatWindowGroupMode" ) : QLatin1String( "KopeteChatWindowIndividualMode" ) ) );
	kopeteChatWindowMainWinSettings.writeEntry( QLatin1String("ChatViewSplitter"), d->splitter->saveState().toBase64() );
	saveChatSettings();
	config->sync();
}

void ChatView::saveChatSettings()
{
	Kopete::ContactPtrList contacts = msgManager()->members();
	if ( contacts.count() != 1 )
		return; //can't save with more than one other person in the chat
	
	Kopete::MetaContact* mc = contacts.first()->metaContact();
	if ( !mc )
		return;

	QString contactListGroup = QString(QLatin1String("chatwindow_") +
	                           QString(mc->metaContactId()));
    KConfigGroup config = KGlobal::config()->group(contactListGroup);

	// If settings are the same as default delete entry from config. This will propagate global setting change.
	if ( editPart()->isRichTextEnabled() != Kopete::BehaviorSettings::self()->richTextByDefault() )
		config.writeEntry( "EnableRichText", editPart()->isRichTextEnabled() );
	else
		config.deleteEntry( "EnableRichText" );

	if ( editPart()->checkSpellingEnabled() != Kopete::BehaviorSettings::self()->spellCheck() )
		config.writeEntry( "EnableAutoSpellCheck", editPart()->checkSpellingEnabled() );
	else
		config.deleteEntry( "EnableAutoSpellCheck" );

	editPart()->writeConfig( config );
	config.sync();
}

void ChatView::loadChatSettings()
{
	Kopete::ContactPtrList contacts = msgManager()->members();
	if ( contacts.count() != 1 )
		return; //can't load with more than one other person in the chat

	//read settings for metacontact
	QString contactListGroup = QString(QLatin1String("chatwindow_") +
	                           QString(contacts.first()->metaContact()->metaContactId()));
	KConfigGroup config(KGlobal::config(), contactListGroup );
	bool enableRichText = config.readEntry( "EnableRichText", Kopete::BehaviorSettings::self()->richTextByDefault() );
	editPart()->textEdit()->setRichTextEnabled( enableRichText );
	emit rtfEnabled( this, editPart()->isRichTextEnabled() );
	bool enableAutoSpell = config.readEntry( "EnableAutoSpellCheck", Kopete::BehaviorSettings::self()->spellCheck() );
	emit autoSpellCheckEnabled( this, enableAutoSpell );
	editPart()->readConfig( config );
}

void ChatView::readOptions()
{
	KConfigGroup kopeteChatWindowMainWinSettings( KGlobal::config(), ( msgManager()->form() == Kopete::ChatSession::Chatroom ? QLatin1String( "KopeteChatWindowGroupMode" ) : QLatin1String( "KopeteChatWindowIndividualMode" ) ) );
	//kDebug(14000) << "reading splitterpos from key: " << dockKey;
	QByteArray state;
	state = kopeteChatWindowMainWinSettings.readEntry( QLatin1String("ChatViewSplitter"), state );
	d->splitter->restoreState( QByteArray::fromBase64( state ) );
}

void ChatView::setActive( bool value )
{
	d->isActive = value;
	if (d->isActive)
	{
		updateChatState(Normal);
	
		// attach editpart back on...
		KXMLGUIFactory * f = msgManager()->factory();
		if (f)
		{
			f->addClient(m_editPart);
		}
	
		emit(activated(static_cast<KopeteView*>(this)));
	}
	else
	{
		KXMLGUIFactory * f = m_editPart->factory();
		if (f)
		{
			f->removeClient(m_editPart);
		}
	}
}

void ChatView::slotRemoteTypingTimeout()
{
	// Remove the topmost timer from the list. Why does QPtrDict use void* keys and not typed keys? *sigh*
	// FIXME: should remove the right item, not the topmost
	if ( !m_remoteTypingMap.isEmpty() )
		remoteTyping( m_remoteTypingMap.begin().key(), false );
}

void ChatView::dragEnterEvent ( QDragEnterEvent * event )
{
	const bool accept = isDragEventAccepted( event );
	if ( accept )
	{
		event->setAccepted( true );
		return;
	}
	QWidget::dragEnterEvent( event );
}

void ChatView::dragMoveEvent( QDragMoveEvent * event )
{
	const bool accept = isDragEventAccepted( event );
	if ( accept )
	{
		event->setAccepted( true );
		return;
	}
	QWidget::dragMoveEvent( event );
}

bool ChatView::isDragEventAccepted( const QDragMoveEvent * event ) const
{
	if( event->provides( "application/kopete.metacontacts.list" ) )
	{
		QByteArray encodedData = event->encodedData ( "application/kopete.metacontacts.list" );
		QDataStream stream( &encodedData, QIODevice::ReadOnly );
		QString metacontactID;
		stream >> metacontactID;

		metacontactID.remove( 0, metacontactID.indexOf('/')+1 ); // strip groupid
		kDebug() << metacontactID;
		Kopete::MetaContact *parent = Kopete::ContactList::self()->metaContact(metacontactID);
		if ( parent && m_manager->mayInvite() )
		{
			foreach ( Kopete::Contact * candidate, parent->contacts() )
			{
				if( candidate && candidate->account() == m_manager->account() && candidate->isOnline())
				{
					if( candidate != m_manager->myself() &&  !m_manager->members().contains( candidate ) )
						return true;
				}
			}
		}
	}
	// make sure it doesn't come from the current chat view - then it's an emoticon
	else if ( KUrl::List::canDecode( event->mimeData() ) && m_manager->members().count() == 1 &&
				 ( event->source() != (QWidget*)m_messagePart->view()->viewport() ) )
	{
		Kopete::ContactPtrList members = m_manager->members();
		Kopete::Contact *contact = members.first();
		if ( contact && contact->canAcceptFiles() )
			return true;
	}
	return false;
}

void ChatView::dropEvent ( QDropEvent * event )
{
	Kopete::ContactPtrList contacts;

	if( event->provides( "application/kopete.metacontacts.list" ) )
	{
		QByteArray encodedData = event->encodedData ( "application/kopete.metacontacts.list" );
		QDataStream stream( &encodedData, QIODevice::ReadOnly );
		QString metacontactID;
		stream >> metacontactID;

		metacontactID.remove( 0, metacontactID.indexOf('/')+1 ); // strip groupid
		Kopete::MetaContact *parent = Kopete::ContactList::self()->metaContact(metacontactID);
		if ( parent && m_manager->mayInvite() )
		{
			foreach ( Kopete::Contact * candidate, parent->contacts() )
			{
				if( candidate && candidate->account() == m_manager->account() && candidate->isOnline())
				{
					if( candidate != m_manager->myself() &&  !m_manager->members().contains( candidate ) )
						m_manager->inviteContact(candidate->contactId());
				}
			}
		}
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

		KUrl::List urlList = KUrl::List::fromMimeData( event->mimeData() );

		for ( KUrl::List::Iterator it = urlList.begin(); it != urlList.end(); ++it )
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
		event->accept();
		return;
	}
	else
		QWidget::dropEvent(event);

}

void ChatView::registerContextMenuHandler( QObject *target, const char* slot )
{
	connect( m_messagePart,
		SIGNAL(contextMenuEvent(Kopete::Message&,QString,KMenu*)),
		target,
		slot
	);
}

void ChatView::registerTooltipHandler( QObject *target, const char* slot )
{
	connect( m_messagePart,
		SIGNAL(tooltipEvent(Kopete::Message&,QString,QString&)),
		target,
		slot
	);
}

#include "chatview.moc"

// vim: set noet ts=4 sts=4 sw=4:

