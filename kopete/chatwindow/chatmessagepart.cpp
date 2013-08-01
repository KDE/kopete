/*
    chatmessagepart.cpp - Chat Message KPart

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2005-2006 by Michaël Larouche     <larouche@kde.org>
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "chatmessagepart.h"

// STYLE_TIMETEST is for time staticstic gathering.
//#define STYLE_TIMETEST

#include <ctime>

// Qt includes
#include <QtCore/QByteArray>
#include <QtCore/QLatin1String>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QRect>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtCore/QBuffer>
#include <QtGui/QClipboard>
#include <QtGui/QCursor>
#include <QtGui/QPixmap>
#include <QtGui/QTextDocument>
#include <QtGui/QScrollBar>
#include <QMimeData>
#include <QApplication>
#include <QFileDialog>

#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

// KHTML::DOM includes
#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <dom/dom_element.h>
#include <dom/html_base.h>
#include <dom/html_document.h>
#include <dom/html_inline.h>
#include <dom/html_form.h>
#include <dom/dom2_events.h>


// KDE includes
#include <kactioncollection.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kstringhandler.h>
#include <ktemporaryfile.h>
#include <kio/copyjob.h>
#include <kstandardaction.h>
#include <kiconloader.h>
#include <kcodecs.h>
#include <kicon.h>

// Kopete includes
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetechatwindow.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopeteemoticons.h"
#include "kopeteview.h"
#include "kopetepicture.h"
#include "kopeteappearancesettings.h"
#include "kopetebehaviorsettings.h"
#include "kopetechatwindowsettings.h"
#include "kopetetransfermanager.h"

#include "kopetechatwindowstyle.h"
#include "kopetechatwindowstylemanager.h"

static const uint ConsecutiveMessageTimeout = 15 /* minutes */ * 60 /* (seconds/minute) */;

class ToolTip;

class ChatMessagePart::Private
{
public:
	Private()
	 : /*tt(0L),*/ scrollPressed(false), scrollToEndDelayed(false), manager(0),
	   copyAction(0), saveAction(0), printAction(0),
	   closeAction(0),copyURLAction(0), currentChatStyle(0),
	   latestDirection(Kopete::Message::Inbound), latestType(Kopete::Message::TypeNormal),
	   latestTime(0), htmlEventListener(0)
	{}

	~Private()
	{
		// Don't delete manager and latestContact, because they could be still used.
		// Don't delete currentChatStyle, it is handled by ChatWindowStyleManager.
	}

	bool fmtOverride;

//	ToolTip *tt;
	bool scrollPressed;
	bool scrollToEndDelayed;
	Kopete::ChatSession *manager;

	DOM::HTMLElement activeElement;

	KAction *copyAction;
	KAction *saveAction;
	KAction *printAction;
	KAction *closeAction;
	KAction *copyURLAction;

	ChatWindowStyle* currentChatStyle;
	QPointer<Kopete::Contact> latestContact;
	Kopete::Message::MessageDirection latestDirection;
	Kopete::Message::MessageType latestType;
	uint latestTime;
	// Yep I know it will take memory, but I don't have choice
	// to enable on-the-fly style changing.
	QList<Kopete::Message> allMessages;

	// No need to delete, HTMLEventListener is ref counted.
	QPointer<HTMLEventListener> htmlEventListener;

	QFont chatFont;
};
/*
class ChatMessagePart::ToolTip : public Q3ToolTip
{
public:
	ToolTip( ChatMessagePart *c ) : Q3ToolTip( c->view()->viewport() )
	{
		m_chat = c;
	}

	void maybeTip( const QPoint &p )
	{
		// FIXME: it's wrong to look for the node under the mouse - this makes too many
		//        assumptions about how tooltips work. but there is no nodeAtPoint.
		DOM::Node node = m_chat->nodeUnderMouse();
		Kopete::Contact *contact = m_chat->contactFromNode( node );
		QString toolTipText;

		if(node.isNull())
			return;

		// this tooltip is attached to the viewport widget, so translate the node's rect
		// into its coordinates.
		QRect rect = node.getRect();
		rect = QRect( m_chat->view()->contentsToViewport( rect.topLeft() ),
			      m_chat->view()->contentsToViewport( rect.bottomRight() ) );

		if( contact )
		{
			toolTipText = contact->toolTip();
		}
		else
		{
			m_chat->emitTooltipEvent( m_chat->textUnderMouse(), toolTipText );

			if( toolTipText.isEmpty() )
			{
				//Fall back to the title attribute
				for( DOM::HTMLElement element = node; !element.isNull(); element = element.parentNode() )
				{
					if( element.hasAttribute( "title" ) )
					{
						toolTipText = element.getAttribute( "title" ).string();
						break;
					}
				}
			}
		}

		if( !toolTipText.isEmpty() )
			tip( rect, toolTipText );
	}

private:
	ChatMessagePart *m_chat;
};
*/

ChatMessagePart::ChatMessagePart( Kopete::ChatSession *mgr, QWidget *parent )
	: KHTMLPart( parent ), d( new Private )
{
	d->manager = mgr;
	d->currentChatStyle = ChatWindowStyleManager::self()->getValidStyleFromPool( KopeteChatWindowSettings::self()->styleName() );
	if (d->currentChatStyle)
		connect( d->currentChatStyle, SIGNAL(destroyed(QObject*)), this, SLOT(clearStyle()) );

	connect( this, SIGNAL(completed()), this, SLOT(slotRenderingFinished()) );

	//Security settings, we don't need this stuff
	setJScriptEnabled( false ) ;
	setJavaEnabled( false );
	setPluginsEnabled( false );
	setMetaRefreshEnabled( false );
	setOnlyLocalReferences( true );

	// read the font for the chat
	readChatFont();

	// Write the template to KHTMLPart
	writeTemplate();

	// It is not possible to drag and drop on our widget
	view()->setAcceptDrops(false);

	connect( Kopete::AppearanceSettings::self(), SIGNAL(messageOverridesChanged()),
	         this, SLOT(slotAppearanceChanged()) );
	connect( Kopete::AppearanceSettings::self(), SIGNAL(appearanceChanged()),
	         this, SLOT(slotRefreshView()) );
	connect( KopeteChatWindowSettings::self(), SIGNAL(chatwindowAppearanceChanged()),
	         this, SLOT(slotRefreshView()) );
	connect( KopeteChatWindowSettings::self(), SIGNAL(styleChanged(QString)),
			 this, SLOT(setStyle(QString)) );
	connect( KopeteChatWindowSettings::self(), SIGNAL(styleVariantChanged(QString)),
			 this, SLOT(setStyleVariant(QString)) );

	// Refresh the style if the display name change.
	connect( d->manager, SIGNAL(displayNameChanged()), this, SLOT(slotUpdateHeaderDisplayName()) );
	connect( d->manager, SIGNAL(photoChanged()), this, SLOT(slotUpdateHeaderPhoto()) );

	connect( d->manager, SIGNAL(messageStateChanged(uint,Kopete::Message::MessageState)),
	         this, SLOT(messageStateChanged(uint,Kopete::Message::MessageState)) );

	connect ( browserExtension(), SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
	          this, SLOT(slotOpenURLRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );

	connect( this, SIGNAL(popupMenu(QString,QPoint)),
	         this, SLOT(slotRightClick(QString,QPoint)) );

	// setup scrollbar
	view()->verticalScrollBar()->setTracking(true);
	connect( view()->verticalScrollBar(), SIGNAL(valueChanged(int)),
	         this, SLOT(slotScrollingTo(int)) );

	connect( Kopete::TransferManager::transferManager(), SIGNAL(askIncomingDone(uint)),
	         this, SLOT(slotFileTransferIncomingDone(uint)) );

	connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
	         this, SLOT(slotRefreshView()) );

	//initActions
	d->copyAction = KStandardAction::copy( this, SLOT(copy()), actionCollection() );
	d->saveAction = KStandardAction::saveAs( this, SLOT(save()), actionCollection() );
	d->printAction = KStandardAction::print( this, SLOT(print()),actionCollection() );
	d->closeAction = KStandardAction::close( this, SLOT(slotCloseView()),actionCollection() );
	d->copyURLAction = new KAction( KIcon("edit-copy"), i18n( "Copy Link Address" ), actionCollection() );
        actionCollection()->addAction( "editcopy", d->copyURLAction );
	connect( d->copyURLAction, SIGNAL(triggered(bool)), this, SLOT(slotCopyURL()) );

	// read formatting override flags
	readOverrides();
}

ChatMessagePart::~ChatMessagePart()
{
	kDebug(14000) ;

	// Cancel all pending file transfer requests
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for ( it = d->allMessages.constBegin(); it != itEnd; ++it )
	{
		if ( (*it).type() == Kopete::Message::TypeFileTransferRequest && !(*it).fileTransferDisabled() )
		{
			Kopete::TransferManager::transferManager()->cancelIncomingTransfer( (*it).id() );
		}
	}

	//delete d->tt;
	delete d;
}

void ChatMessagePart::slotScrollingTo( int y )
{
	int scrolledTo = y + view()->visibleHeight();
	d->scrollPressed = scrolledTo < ( view()->contentsHeight() - 10 );
}

void ChatMessagePart::save()
{
	const KUrl dummyUrl;
	QPointer <KFileDialog> dlg = new KFileDialog( dummyUrl, QLatin1String( "text/html text/plain" ), view() );
	dlg->setCaption( i18n( "Save Conversation" ) );
	dlg->setOperationMode( KFileDialog::Saving );

	if ( dlg->exec() != QDialog::Accepted )
	{
		delete dlg;
		return;
	}

	if ( ! dlg )
		return;

	KUrl saveURL = dlg->selectedUrl();
	KTemporaryFile *tempFile = new KTemporaryFile();
	tempFile->setAutoRemove(false);
	tempFile->open();

	QTextStream stream ( tempFile );
	stream.setCodec(QTextCodec::codecForName("UTF-8"));

	if ( dlg->currentFilter() == QLatin1String( "text/plain" ) )
	{
		QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
		for(it = d->allMessages.constBegin(); it != itEnd; ++it)
		{
			Kopete::Message tempMessage = *it;
			stream << "[" << KGlobal::locale()->formatDateTime(tempMessage.timestamp()) << "] ";
			if( tempMessage.from() && tempMessage.from()->metaContact() )
			{
				stream << formatName(tempMessage.from()->metaContact()->displayName(), Qt::RichText);
			}
			stream << ": " << tempMessage.plainBody() << "\n";
		}
	}
	else
	{
		stream << htmlDocument().toString().string() << '\n';
	}

	delete dlg;

	stream.flush();
	QString fileName = tempFile->fileName();
	delete tempFile;

	KIO::CopyJob *moveJob = KIO::move( KUrl( fileName ), saveURL, KIO::HideProgressInfo );

	if ( !moveJob )
	{
		KMessageBox::queuedMessageBox( view(), KMessageBox::Error,
				i18n("<qt>Could not open <b>%1</b> for writing.</qt>", saveURL.prettyUrl() ), // Message
				i18n("Error While Saving") ); //Caption
	}
}

void ChatMessagePart::pageUp()
{
	view()->scrollBy( 0, -view()->visibleHeight() );
}

void ChatMessagePart::pageDown()
{
	view()->scrollBy( 0, view()->visibleHeight() );
}

void ChatMessagePart::slotOpenURLRequest(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)
{
	kDebug(14000) << "url=" << url.url();
	if ( url.protocol() == QLatin1String("kopetemessage") )
	{
		Kopete::Contact *contact = d->manager->account()->contacts().value( url.host() );
		if ( contact )
			contact->execute();
	}
	else
	{
		KRun *runner = new KRun( url, 0, 0, false ); // false = non-local files
		runner->setRunExecutables( false ); //security
		//KRun autodeletes itself by default when finished.
	}
}

void ChatMessagePart::slotFileTransferIncomingDone( unsigned int id )
{
	QList<Kopete::Message>::Iterator it = d->allMessages.end();
	while ( it != d->allMessages.begin() )
	{
		--it;
		if ( (*it).id() == id )
		{
			(*it).setFileTransferDisabled( true );
			disableFileTransferButtons( id );
			break;
		}
	}
}

void ChatMessagePart::readOverrides()
{
	d->fmtOverride = Kopete::AppearanceSettings::self()->chatFmtOverride();
}

void ChatMessagePart::slotToggleGraphicOverride(bool)
{
}

void ChatMessagePart::setStyle( const QString &styleName )
{
	// Create a new ChatWindowStyle
	setStyle( ChatWindowStyleManager::self()->getValidStyleFromPool(styleName) );
}

void ChatMessagePart::setStyle( ChatWindowStyle *style )
{
	// Change the current style
	if (d->currentChatStyle)
		disconnect( d->currentChatStyle, SIGNAL(destroyed(QObject*)), this, SLOT(clearStyle()) );

	d->currentChatStyle = style;
	if (d->currentChatStyle)
		connect( d->currentChatStyle, SIGNAL(destroyed(QObject*)), this, SLOT(clearStyle()) );

	// Do the actual style switch
	// Wait for the event loop before switching the style
	QTimer::singleShot( 0, this, SLOT(changeStyle()) );
}

void ChatMessagePart::clearStyle()
{
	setStyle( QString() );
}

void ChatMessagePart::setStyleVariant( const QString &variantPath )
{
	DOM::HTMLElement variantNode = document().getElementById( QString("mainStyle") );
	if( !variantNode.isNull() )
		variantNode.setInnerText( QString("@import url(\"%1\");").arg( adjustStyleVariantForChatSession( variantPath) ) );
}

void ChatMessagePart::messageStateChanged( uint messageId, Kopete::Message::MessageState state )
{
	QList<Kopete::Message>::Iterator it = d->allMessages.end();
	while ( it != d->allMessages.begin() )
	{
		--it;
		if ( (*it).id() == messageId )
		{
			(*it).setState( state );
			changeMessageStateElement( messageId, state );
			break;
		}
	}
}

void ChatMessagePart::slotAppearanceChanged()
{
	readOverrides();

	changeStyle();
}

void ChatMessagePart::appendMessage( Kopete::Message &message, bool restoring )
{
	if ( !d->currentChatStyle )
		return;

	if ( !message.classes().contains("history") )
		message.setFormattingOverride( d->fmtOverride );

#ifdef STYLE_TIMETEST
	QTime beforeMessage = QTime::currentTime();
#endif

	QString formattedMessageHtml;
	bool isConsecutiveMessage = false;
	int bufferLen = Kopete::BehaviorSettings::self()->chatWindowBufferViewSize();

	// Find the "Chat" div element.
	// If the "Chat" div element is not found, do nothing. It's the central part of Adium format.
	DOM::HTMLElement chatNode = htmlDocument().getElementById( "Chat" );

	if( chatNode.isNull() )
	{
		kDebug(14000) << "WARNING: Chat Node was null !";
		return;
	}

	// Check if it's a consecutive Message
	// Consecutive messages are only for normal messages, status messages do not have a <div id="insert" />
	// We check if the from() is the latestContact, because consecutive incoming/outgoing message can come from differents peopole(in groupchat and IRC)
	// Group only if the user want it.
	if( KopeteChatWindowSettings::self()->groupConsecutiveMessages() )
	{
		isConsecutiveMessage = (message.direction() == d->latestDirection && !d->latestContact.isNull()
		                        && d->latestContact == message.from() && message.type() == d->latestType
		                        && message.type() != Kopete::Message::TypeFileTransferRequest );

		if(message.timestamp().isValid()){
			uint next = message.timestamp().toTime_t();
			if(isConsecutiveMessage && (next - d->latestTime) > ConsecutiveMessageTimeout){
				isConsecutiveMessage = false;
			}
			d->latestTime = next;
		}
	}

	// Don't test it in the switch to don't break consecutive messages.
	if(message.type() == Kopete::Message::TypeAction)
	{
		// Check if chat style support Action template (Kopete extension)
		if( d->currentChatStyle->hasActionTemplate() )
		{
			switch(message.direction())
			{
				case Kopete::Message::Inbound:
					formattedMessageHtml = d->currentChatStyle->getActionIncomingHtml();
					break;
				case Kopete::Message::Outbound:
					formattedMessageHtml = d->currentChatStyle->getActionOutgoingHtml();
					break;
				default:
					break;
			}
		}
		// Use status template if no Action template.
		else
		{
			formattedMessageHtml = d->currentChatStyle->getStatusHtml();
		}
	}
	else if(message.type() == Kopete::Message::TypeFileTransferRequest)
	{
		formattedMessageHtml = d->currentChatStyle->getFileTransferIncomingHtml();
	}
	else if(message.type() == Kopete::Message::TypeVoiceClipRequest)
	{
		formattedMessageHtml = d->currentChatStyle->getVoiceClipIncomingHtml();
	}
	else
	{
		switch(message.direction())
		{
			case Kopete::Message::Inbound:
			{
				if(isConsecutiveMessage)
				{
					formattedMessageHtml = d->currentChatStyle->getNextIncomingHtml();
				}
				else
				{
					formattedMessageHtml = d->currentChatStyle->getIncomingHtml();
				}
				break;
			}
			case Kopete::Message::Outbound:
			{
				if(isConsecutiveMessage)
				{
					formattedMessageHtml = d->currentChatStyle->getNextOutgoingHtml();
				}
				else
				{
					formattedMessageHtml = d->currentChatStyle->getOutgoingHtml();
				}
				break;
			}
			case Kopete::Message::Internal:
			{
				formattedMessageHtml = d->currentChatStyle->getStatusHtml();
				break;
			}
		}
	}

	formattedMessageHtml = formatStyleKeywords( formattedMessageHtml, message );

	// newMessageNode is common to both code path
	// FIXME: Find a better than to create a dummy span.
	DOM::HTMLElement newMessageNode = document().createElement( QString("span") );
	newMessageNode.setInnerHTML( formattedMessageHtml );

	// Find the insert Node
	DOM::HTMLElement insertNode = document().getElementById( QString("insert") );

	if( isConsecutiveMessage && !insertNode.isNull() )
	{
		// Replace the insert block, because it's a consecutive message.
		insertNode.parentNode().replaceChild(newMessageNode, insertNode);
	}
	else
	{
		// Remove the insert block, because it's a new message.
		if( !insertNode.isNull() )
			insertNode.parentNode().removeChild(insertNode);
		// Append to the chat.
		chatNode.appendChild(newMessageNode);
	}

	if ( message.type() == Kopete::Message::TypeNormal )
	{
		if ( message.direction() == Kopete::Message::Outbound )
			changeMessageStateElement( message.id(), message.state() );
	}
	else if ( message.type() == Kopete::Message::TypeFileTransferRequest )
	{
		if ( message.fileTransferDisabled() )
			disableFileTransferButtons( message.id() );
		else
			addFileTransferButtonsEventListener( message.id() );
	}
	else if ( message.type() == Kopete::Message::TypeVoiceClipRequest )
	{
		addVoiceClipsButtonsEventListener( message.id() );
	}

	// Keep the direction to see on next message
	// if it's a consecutive message
	// Keep also the from() contact.
	d->latestDirection = message.direction();
	d->latestType = message.type();
	d->latestContact = const_cast<Kopete::Contact*>(message.from());

	// Add the message to the list for futher restoring if needed
	if(!restoring)
		d->allMessages.append(message);

	while ( bufferLen>0 && d->allMessages.count() >= bufferLen )
	{
		d->allMessages.pop_front();

		// FIXME: Find a way to make work Chat View Buffer efficiently with consecutives messages.
		// Before it was calling changeStyle() but it's damn too slow.
		if( !KopeteChatWindowSettings::self()->groupConsecutiveMessages() )
		{
			chatNode.removeChild( chatNode.firstChild() );
		}
	}

	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT(slotScrollView()) );

#ifdef STYLE_TIMETEST
	kDebug(14000) << "Message time: " << beforeMessage.msecsTo( QTime::currentTime());
#endif
}

void ChatMessagePart::slotRefreshView()
{
	// refresh the chat font
	readChatFont();

	DOM::HTMLElement kopeteNode = document().getElementById( QString("KopeteStyle") );
	if( !kopeteNode.isNull() )
		kopeteNode.setInnerText( styleHTML() );

	DOM::HTMLBodyElement bodyElement = htmlDocument().body();
	bodyElement.setBgColor( Kopete::AppearanceSettings::self()->chatBackgroundColor().name() );
}

void ChatMessagePart::keepScrolledDown()
{
	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT(slotScrollView()) );
}

const QString ChatMessagePart::styleHTML() const
{
	Kopete::AppearanceSettings *settings = Kopete::AppearanceSettings::self();

	QString style = QString(
		"body{background-color:%1;font-family:%2;font-size:%3pt;color:%4}"
		"td{font-family:%5;font-size:%6pt;color:%7}"
		"input{font-family:%8;font-size:%9pt;color:%10}"
		"a{color:%11}a.visited{color:%12}"
		"a.KopeteDisplayName{text-decoration:none;color:inherit;}"
		"a.KopeteDisplayName:hover{text-decoration:underline;color:inherit}"
		".KopeteLink{cursor:pointer;}.KopeteLink:hover{text-decoration:underline}"
		".KopeteMessageBody > p:first-child{margin:0;padding:0;display:inline;}" /* some html messages are encapsuled into a <p> */ )
		.arg( settings->chatBackgroundColor().name() )
		.arg( d->chatFont.family() )
		.arg( d->chatFont.pointSize() )
		.arg( settings->chatTextColor().name() )
		.arg( d->chatFont.family() )
		.arg( d->chatFont.pointSize() )
		.arg( settings->chatTextColor().name() )
		.arg( d->chatFont.family() )
		.arg( d->chatFont.pointSize() )
		.arg( settings->chatTextColor().name() )
		.arg( settings->chatLinkColor().name() )
		.arg( settings->chatLinkColor().name() );

	return style;
}

void ChatMessagePart::clear()
{
	// writeTemplate actually reset the HTML chat session from the beginning.
	writeTemplate();

	// Reset consecutive messages
	d->latestContact = 0;

	// Cancel all pending file transfer requests
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for ( it = d->allMessages.constBegin(); it != itEnd; ++it )
	{
		if ( (*it).type() == Kopete::Message::TypeFileTransferRequest && !(*it).fileTransferDisabled() )
		{
			Kopete::TransferManager::transferManager()->cancelIncomingTransfer( (*it).id() );
		}
	}

	// Remove all stored messages.
	d->allMessages.clear();
}

Kopete::Contact *ChatMessagePart::contactFromNode( const DOM::Node &n ) const
{
	DOM::Node node = n;
	QList<Kopete::Contact*> m;

	if ( node.isNull() )
		return 0;

	while ( !node.isNull() && ( node.nodeType() == DOM::Node::TEXT_NODE || ((DOM::HTMLElement)node).className() != "KopeteDisplayName" ) )
		node = node.parentNode();

	DOM::HTMLElement element = node;
	if ( element.className() != "KopeteDisplayName" )
		return 0;

	m = d->manager->members();
	if ( element.hasAttribute( "contactid" ) )
	{
		QString contactId = element.getAttribute( "contactid" ).string();
		for ( int i =0; i != m.size(); ++i )
			if ( m.at(i)->contactId() == contactId )
				return m[i];
	}
	else
	{
		QString nick = element.innerText().string().trimmed();
		foreach ( Kopete::Contact *contact, m )
		{
			QString contactNick;
			if( contact->metaContact() && contact->metaContact() != Kopete::ContactList::self()->myself() )
				contactNick = contact->metaContact()->displayName();
			else
				contactNick = contact->displayName();

			if ( contactNick == nick )
				return contact;
		}
	}

	return 0;
}

void ChatMessagePart::slotRightClick( const QString &, const QPoint &point )
{
	// look through parents until we find an Element
	DOM::Node activeNode = nodeUnderMouse();
	while ( !activeNode.isNull() && activeNode.nodeType() != DOM::Node::ELEMENT_NODE )
		activeNode = activeNode.parentNode();

	// make sure it's valid
	d->activeElement = activeNode;
	if ( d->activeElement.isNull() )
		return;

	KMenu *chatWindowPopup = 0L;

	if ( Kopete::Contact *contact = contactFromNode( d->activeElement ) )
	{
		chatWindowPopup = contact->popupMenu();
		connect( chatWindowPopup, SIGNAL(aboutToHide()), chatWindowPopup , SLOT(deleteLater()) );
	}
	else
	{
		chatWindowPopup = new KMenu();

		QAction *action;
		if ( d->activeElement.className() == QLatin1String("KopeteDisplayName") )
		{
			action = chatWindowPopup->addAction( i18n( "User Has Left" ) );
			action->setEnabled(false);
			chatWindowPopup->addSeparator();
		}
		else if ( d->activeElement.tagName().lower() == QLatin1String( "a" ) )
		{
			chatWindowPopup->addAction( d->copyURLAction );
			chatWindowPopup->addSeparator();
		}

		d->copyAction->setEnabled( hasSelection() );
		chatWindowPopup->addAction( d->copyAction );
		chatWindowPopup->addAction( d->saveAction );
		chatWindowPopup->addAction( d->printAction );
		chatWindowPopup->addSeparator();
		chatWindowPopup->addAction( d->closeAction );

		connect( chatWindowPopup, SIGNAL(aboutToHide()), chatWindowPopup, SLOT(deleteLater()) );
		chatWindowPopup->popup( point );
	}

	//Emit for plugin hooks
	emit contextMenuEvent( textUnderMouse(), chatWindowPopup );

	chatWindowPopup->popup( point );
}

QString ChatMessagePart::textUnderMouse()
{
	DOM::Node activeNode = nodeUnderMouse();
	if( activeNode.nodeType() != DOM::Node::TEXT_NODE )
		return QString();

	DOM::Text textNode = activeNode;
	QString data = textNode.data().string();

	//Ok, we have the whole node. Now, find the text under the mouse.
	int mouseLeft = view()->mapFromGlobal( QCursor::pos() ).x(),
		nodeLeft = activeNode.getRect().x(),
		cPos = 0,
		dataLen = data.length();

	QFontMetrics metrics( d->chatFont );
	QString buffer;
	while( cPos < dataLen && nodeLeft < mouseLeft )
	{
		QChar c = data[cPos++];
		if( c.isSpace() )
			buffer.truncate(0);
		else
			buffer += c;

		nodeLeft += metrics.width(c);
	}

	if( cPos < dataLen )
	{
		QChar c = data[cPos++];
		while( cPos < dataLen && !c.isSpace() )
		{
			buffer += c;
			c = data[cPos++];
		}
	}

	return buffer;
}

void ChatMessagePart::slotCopyURL()
{
	DOM::HTMLAnchorElement a = d->activeElement;
	if ( !a.isNull() )
	{
		QApplication::clipboard()->setText( a.href().string(), QClipboard::Clipboard );
		QApplication::clipboard()->setText( a.href().string(), QClipboard::Selection );
	}
}

void ChatMessagePart::slotScrollView()
{
	if ( inProgress() )
		d->scrollToEndDelayed = true;
	else
		view()->scrollBy( 0, view()->contentsHeight() );
}

void ChatMessagePart::slotRenderingFinished()
{
	if ( d->scrollToEndDelayed )
	{
		d->scrollToEndDelayed = false;
		if ( !d->scrollPressed )
			view()->scrollBy( 0, view()->contentsHeight() );
	}
}

void ChatMessagePart::copy(bool justselection /* default false */)
{
	/*
	* The objective of this function is to keep the text of emoticons (or of LaTeX image) when copying.
	*   see Bug 61676
	* This also copies the text as type text/html
	* RangeImpl::toHTML  was not implemented before KDE 3.4
	*/
	QString htmltext = selectedTextAsHTML();
	QString text = selectedText();
        //selectedText is now sufficient
//      text=Kopete::Message::unescape( htmltext ).trimmed();
        // Message::unsescape will replace image by his title attribute
        // trimmed is for removing the newline added by the <!DOCTYPE> and other xml things of RangeImpl::toHTML

	if(text.isEmpty())
            return;

	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(slotClearSelection()));

#ifndef QT_NO_MIMECLIPBOARD
	if(!justselection)
	{
		QMimeData *mimeData = new QMimeData();
		mimeData->setText(text);

		if(!htmltext.isEmpty()) {
			htmltext.replace( QChar( 0xa0 ), ' ' );
			mimeData->setHtml(htmltext);
		}

		QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
	}
	QApplication::clipboard()->setText( text, QClipboard::Selection );
#else
	if(!justselection)
		QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	QApplication::clipboard()->setText( text, QClipboard::Selection );
#endif
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()), SLOT(slotClearSelection()));

}

void ChatMessagePart::print()
{
	view()->print();
}

void ChatMessagePart::khtmlDrawContentsEvent( khtml::DrawContentsEvent * event) //virtual
{
	KHTMLPart::khtmlDrawContentsEvent(event);
	//copy(true /*selection only*/); not needed anymore.
}

void ChatMessagePart::slotCloseView( bool force )
{
	if (d->manager && d->manager->view())
		d->manager->view()->closeView( force );
}

void ChatMessagePart::emitTooltipEvent(  const QString &textUnderMouse, QString &toolTip )
{
	emit tooltipEvent(  textUnderMouse, toolTip );
}

// Style formatting for messages(incoming, outgoing, status)
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML, const Kopete::Message &_message )
{
	if ( !d->currentChatStyle )
		return QString();

	Kopete::Message message=_message; //we will eventually need to modify it before showing it.
	QString resultHTML = sourceHTML;
	QString nick, contactId, service, protocolIcon, nickLink;

	if( message.from() )
	{
		nick = formatName(message.from(), Qt::RichText);
		contactId = message.from()->contactId();
		// protocol() returns NULL here in the style preview in appearance config.
		// this isn't the right place to work around it, since contacts should never have
		// no protocol, but it works for now.
		//
		// Use default if protocol() and protocol()->displayName() is NULL.
		// For preview and unit tests.
		QString iconName = QLatin1String("kopete");
		service = QLatin1String("Kopete");
		if(message.from()->protocol() && !message.from()->protocol()->displayName().isNull())
		{
			service =  message.from()->protocol()->displayName();
			iconName = message.from()->protocol()->pluginIcon();
		}

		protocolIcon = KIconLoader::global()->iconPath( iconName, KIconLoader::Small );

		nickLink=QString("<a href=\"kopetemessage://%1/?protocolId=%2&amp;accountId=%3\" class=\"KopeteDisplayName\">")
				.arg( Qt::escape(message.from()->contactId()).replace('"',"&quot;"),
					  Qt::escape(message.from()->protocol()->pluginId()).replace('"',"&quot;"),
					  Qt::escape(message.from()->account()->accountId() ).replace('"',"&quot;"));
	}
	else
	{
		nickLink="<a>";
	}


	// Replace sender (contact nick)
	resultHTML.replace( QLatin1String("%sender%"), nickLink+nick+"</a>" );
	// Replace time, by default display only time and display seconds(that was true means).
	if ( Kopete::BehaviorSettings::showDates() && message.timestamp().date() != QDate::currentDate() )
		resultHTML.replace( QLatin1String("%time%"), KGlobal::locale()->formatDateTime(message.timestamp(), KLocale::ShortDate, true) );
	else
		resultHTML.replace( QLatin1String("%time%"), KGlobal::locale()->formatTime(message.timestamp().time(), true) );
	// Replace %screenName% (contact ID)
	resultHTML.replace( QLatin1String("%senderScreenName%"), nickLink+Qt::escape(contactId)+"</a>" );
	// Replace service name (protocol name)
	resultHTML.replace( QLatin1String("%service%"), Qt::escape(service) );
	// Replace protocolIcon (sender statusIcon)
	resultHTML.replace( QLatin1String("%senderStatusIcon%"), Qt::escape(protocolIcon).replace('"',"&quot;") );

	// Look for %time{X}%
	QRegExp timeRegExp("%time\\{([^}]*)\\}%");
	int pos=0;
	while( (pos=timeRegExp.indexIn(resultHTML , pos) ) != -1 )
	{
		QString timeKeyword = formatTime( timeRegExp.cap(1), message.timestamp() );
		resultHTML.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
	}

	// Look for %textbackgroundcolor{X}%
	// TODO: use the X value.
	// Replace with user-selected highlight color if to be highlighted or
	// with "inherit" otherwise to keep CSS clean
	QString bgColor = QLatin1String("inherit");
	if( message.importance() == Kopete::Message::Highlight && Kopete::BehaviorSettings::self()->highlightEnabled() )
	{
		bgColor = Kopete::AppearanceSettings::self()->highlightBackgroundColor().name();
	}

	QRegExp textBackgroundRegExp("%textbackgroundcolor\\{([^}]*)\\}%");
	int textPos=0;
	while( (textPos=textBackgroundRegExp.indexIn(resultHTML, textPos) ) != -1 )
	{
		resultHTML.replace( textPos , textBackgroundRegExp.cap(0).length() , bgColor );
	}

	// Replace userIconPath
	if( message.from() )
	{
		QString photoPath = photoForContact( message.from() );
		if( photoPath.isEmpty() )
		{
			if(message.direction() == Kopete::Message::Inbound)
				photoPath = d->currentChatStyle->getStyleBaseHref() + QLatin1String("Incoming/buddy_icon.png");
			else if(message.direction() == Kopete::Message::Outbound)
				photoPath = d->currentChatStyle->getStyleBaseHref() + QLatin1String("Outgoing/buddy_icon.png");
		}
		resultHTML.replace(QLatin1String("%userIconPath%"), photoPath);
	}

	// Replace messages.
	// Build the action message if the currentChatStyle do not have Action template.
	if( message.type() == Kopete::Message::TypeAction && !d->currentChatStyle->hasActionTemplate() )
	{
		kDebug(14000) << "Map Action message to Status template. ";

		QString boldNick = QString("%1<b>%2</b></a> ").arg(nickLink,nick);
		QString newBody = boldNick + message.parsedBody();
		message.setHtmlBody(newBody );
	}

	// Set message direction("rtl"(Right-To-Left) or "ltr"(Left-to-right))
	resultHTML.replace( QLatin1String("%messageDirection%"), message.isRightToLeft() ? "rtl" : "ltr" );

	// These colors are used for coloring nicknames. I tried to use
	// colors both visible on light and dark background.
	static const char* const nameColors[] =
	{
		"red", "blue" , "gray", "magenta", "violet", /*"olive"*/ "#808000", "yellowgreen",
		"darkred", "darkgreen", "darksalmon", "darkcyan", /*"darkyellow"*/   "#B07D2B",
		"mediumpurple", "peru", "olivedrab", /*"royalred"*/ "#B01712", "darkorange", "slateblue",
		"slategray", "goldenrod", "orangered", "tomato", /*"dogderblue"*/ "#1E90FF", "steelblue",
		"deeppink", "saddlebrown", "coral", "royalblue"
	};

	static const int nameColorsLen = sizeof(nameColors) / sizeof(nameColors[0]) - 1;
	// hash contactId to deterministically pick a color for the contact
	int hash = 0;
	for( int f = 0; f < contactId.length(); ++f )
		hash += contactId[f].unicode() * f;
	const QString colorName = nameColors[ hash % nameColorsLen ];
	QString lightColorName;	// Do not initialize, QColor::name() is expensive!
	//kDebug(14000) << "Hash " << hash << " has color " << colorName;
	QRegExp senderColorRegExp("%senderColor(?:\\{([^}]*)\\})?%");
	textPos=0;
	while( (textPos=senderColorRegExp.indexIn(resultHTML, textPos) ) != -1 )
	{
		int light=100;
		bool doLight=false;
		if(senderColorRegExp.numCaptures()>=1)
		{
			light=senderColorRegExp.cap(1).toUInt(&doLight);
		}

		// Lazily init light color
		if ( doLight && lightColorName.isNull() )
			lightColorName = QColor( colorName ).light( light ).name();

		resultHTML.replace( textPos , senderColorRegExp.cap(0).length(),
			doLight ? lightColorName : colorName );
	}

	if ( message.type() == Kopete::Message::TypeFileTransferRequest )
	{
		QString fileIcon;
		if ( !message.filePreview().isNull() )
		{
			QByteArray tempArray;
			QBuffer tempBuffer( &tempArray );
			tempBuffer.open( QIODevice::WriteOnly );
			if( message.filePreview().save( &tempBuffer, "PNG" ) )
				fileIcon = QString( "data:image/png;base64," ) + tempArray.toBase64();
		}

		if ( fileIcon.isEmpty() )
		{
			QString iconName = KMimeType::iconNameForUrl( message.fileName() );
			fileIcon = KIconLoader::global()->iconPath( iconName, -KIconLoader::SizeMedium );
		}

		resultHTML.replace( QLatin1String("%fileName%"), Qt::escape( message.fileName() ).replace('"',"&quot;") );
		resultHTML.replace( QLatin1String("%fileSize%"), KGlobal::locale()->formatByteSize( message.fileSize() ).replace('"',"&quot;") );
		resultHTML.replace( QLatin1String("%fileIconPath%"), fileIcon );

		resultHTML.replace( QLatin1String("%saveFileHandlerId%"), QString( "ftSV%1" ).arg( message.id() ) );
		resultHTML.replace( QLatin1String("%saveFileAsHandlerId%"), QString( "ftSA%1" ).arg( message.id() ) );
		resultHTML.replace( QLatin1String("%cancelRequestHandlerId%"), QString( "ftCC%1" ).arg( message.id() ) );
	}

	if ( message.type() == Kopete::Message::TypeVoiceClipRequest )
	{
		QString fileIcon;

		QString iconName = KMimeType::iconNameForUrl( message.fileName() );
		fileIcon = KIconLoader::global()->iconPath( iconName, -KIconLoader::SizeMedium );

		resultHTML.replace( QLatin1String("%fileIconPath%"), fileIcon );

		resultHTML.replace( QLatin1String("%playVoiceHandlerId%"), QString( "vcPL%1" ).arg( message.id() ) );
		resultHTML.replace( QLatin1String("%saveAsVoiceHandlerId%"), QString( "vcSA%1" ).arg( message.id() ) );
	}

	if ( message.type() == Kopete::Message::TypeNormal && message.direction() == Kopete::Message::Outbound )
		resultHTML.replace( QLatin1String( "%stateElementId%" ), QString( "msST%1" ).arg( message.id() ) );

	// Replace message at the end, maybe someone could put a Adium keyword in his message :P
	resultHTML.replace( QLatin1String("%message%"), formatMessageBody(message) );

	// TODO: %status
//	resultHTML = addNickLinks( resultHTML );
	return resultHTML;
}

// Style formatting for header and footer.
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML )
{
	QString resultHTML = sourceHTML;

	// Verify that all contacts are not null before doing anything
	if( !d->manager->members().isEmpty() && d->manager->myself() )
	{
		QString sourceName, destinationName;

		Kopete::Contact *remoteContact = d->manager->members().first();

		// Use contact nickname for ourselfs, Myself metacontact display name isn't a reliable source.
		sourceName = d->manager->myself()->displayName();
		if( remoteContact->metaContact() )
			destinationName = remoteContact->metaContact()->displayName();
		else
			destinationName = remoteContact->displayName();

		// Replace %chatName%, create a internal span to update it by DOM when asked.
		resultHTML.replace( QLatin1String("%chatName%"), QString("<span id=\"KopeteHeaderChatNameInternal\">%1</span>").arg( formatName(d->manager->displayName(), Qt::RichText) ) );
		// Replace %sourceName%
		resultHTML.replace( QLatin1String("%sourceName%"), formatName(sourceName, Qt::RichText) );
		// Replace %destinationName%
		resultHTML.replace( QLatin1String("%destinationName%"), formatName(destinationName, Qt::RichText) );
		// For %timeOpened%, display the date and time (also the seconds).
		resultHTML.replace( QLatin1String("%timeOpened%"), KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), KLocale::ShortDate, true ) );

		// Look for %timeOpened{X}%
		QRegExp timeRegExp("%timeOpened\\{([^}]*)\\}%");
		int pos=0;
		while( (pos=timeRegExp.indexIn(resultHTML, pos) ) != -1 )
		{
			QString timeKeyword = formatTime( timeRegExp.cap(1), QDateTime::currentDateTime() );
			resultHTML.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
		}
		// Get contact image paths
		QString photoIncoming = photoForContact( remoteContact );
		QString photoOutgoing = photoForContact( d->manager->myself() );
		if( photoIncoming.isEmpty() )
		{
			photoIncoming = d->currentChatStyle->getStyleBaseHref() + QLatin1String("Incoming/buddy_icon.png");
		}

		if( photoOutgoing.isEmpty() )
		{
			photoOutgoing = d->currentChatStyle->getStyleBaseHref() + QLatin1String("Outgoing/buddy_icon.png");
		}

		resultHTML.replace( QLatin1String("%incomingIconPath%"), photoIncoming );
		resultHTML.replace( QLatin1String("%outgoingIconPath%"), photoOutgoing );
	}

	return resultHTML;
}

QString ChatMessagePart::formatTime(const QString &_timeFormat, const QDateTime &dateTime)
{
	char buffer[256];
#ifdef Q_WS_WIN
	QString timeFormat = _timeFormat;
	// some formats are not supported on windows (gnu extension?)
	timeFormat = timeFormat.replace(QLatin1String("%e"), QLatin1String("%d"));
	timeFormat = timeFormat.replace(QLatin1String("%T"), QLatin1String("%H:%M:%S"));
#else
	const QString timeFormat = _timeFormat;
#endif
	// Get current time
	time_t timeT = dateTime.toTime_t();
	// Convert it to local time representation.
	struct tm* loctime = localtime (&timeT);
	strftime (buffer, 256, timeFormat.toAscii(), loctime);

	return QString(buffer);
}

QString ChatMessagePart::formatName(const QString &sourceName, Qt::TextFormat format ) const
{
	QString formattedName = sourceName;

	// Squeeze the nickname if the user want it
	if( Kopete::BehaviorSettings::self()->truncateContactName() )
	{
		formattedName = KStringHandler::csqueeze( sourceName, Kopete::BehaviorSettings::self()->truncateContactNameLength() );
	}

	if ( format == Qt::RichText )
	{ // Escape the name.
		formattedName = Kopete::Message::escape(formattedName);
	}

	return formattedName;
}

QString ChatMessagePart::formatName( const Kopete::Contact* contact, Qt::TextFormat format ) const
{
	if (!contact)
	{
		return QString();
	}

	// Use metacontact display name if the metacontact exists and if it is not the myself metacontact.
	// Myself metacontact is not a reliable source.
	if ( contact->metaContact() && contact->metaContact() != Kopete::ContactList::self()->myself() )
	{
		return formatName( contact->metaContact()->displayName(), format );
	}
	// Use contact nickname for no metacontact or myself.
	else
	{
		return formatName( contact->displayName(), format );
	}
}

QString ChatMessagePart::formatMessageBody(const Kopete::Message &message)
{
	QString formattedBody("<span ");

	formattedBody += message.getHtmlStyleAttribute();

	QStringList classes("KopeteMessageBody");
	classes+=message.classes();

	// Affect the parsed body.
	formattedBody += QString("class=\"%1\">%2</span>")
		.arg(classes.join(" "), message.parsedBody());

	return formattedBody;
}

void ChatMessagePart::slotUpdateHeaderDisplayName()
{
	kDebug(14000) ;
	DOM::HTMLElement kopeteChatNameNode = document().getElementById( QString("KopeteHeaderChatNameInternal") );
	if( !kopeteChatNameNode.isNull() )
		kopeteChatNameNode.setInnerText( formatName(d->manager->displayName(), Qt::RichText) );
}

void ChatMessagePart::slotUpdateHeaderPhoto()
{
	// Do the actual style switch
	// Wait for the event loop before switching the style
	QTimer::singleShot( 0, this, SLOT(changeStyle()) );
}

void ChatMessagePart::changeStyle()
{
#ifdef STYLE_TIMETEST
	QTime beforeChange = QTime::currentTime();
#endif
	// Make latestContact null to reset consecutives messages.
	d->latestContact = 0;

	// Rewrite the header and footer.
	writeTemplate();

	// Readd all current messages.
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for(it = d->allMessages.constBegin(); it != itEnd; ++it)
	{
		Kopete::Message tempMessage = *it;
		appendMessage(tempMessage, true); // true means that we are restoring.
	}
	kDebug(14000) << "Finish changing style.";
#ifdef STYLE_TIMETEST
	kDebug(14000) << "Change time: " << beforeChange.msecsTo( QTime::currentTime());
#endif
}

void ChatMessagePart::writeTemplate()
{
	kDebug(14000) ;

#ifdef STYLE_TIMETEST
	QTime beforeHeader = QTime::currentTime();
#endif
	// Clear all the page, and begin a new page.
	begin();

	// NOTE: About styles
	// Order of style tag in the template is important.
	// mainStyle take over all other style definition (which is what we want).
	//
	// KopeteStyle: Kopete appearance configuration into a style. It loaded first because
	// we don't want Kopete settings to override CSS Chat Window Style.
	// baseStyle: Import the main.css from the Chat Window Style
	// mainStyle: Currrent variant CSS url.

	QString xhtmlBase;
	if ( d->currentChatStyle )
	{
		// FIXME: Maybe this string should be load from a file, then parsed for args.
		xhtmlBase += QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
			"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
			"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
			"<head>\n"
			"<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\n\" />\n"
			"<base href=\"%1\">\n"
			"<style id=\"KopeteStyle\" type=\"text/css\" media=\"screen,print\">\n"
			"	%5\n"
			"</style>\n"
			"<style id=\"baseStyle\" type=\"text/css\" media=\"screen,print\">\n"
			"	@import url(\"main.css\");\n"
			"	*{ word-wrap:break-word; }\n"
			"</style>\n"
			"<style id=\"mainStyle\" type=\"text/css\" media=\"screen,print\">\n"
			"	@import url(\"%4\");\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"%2\n"
			"<div id=\"Chat\">\n</div>\n"
			"%3\n"
			"</body>"
			"</html>"
			).arg( d->currentChatStyle->getStyleBaseHref() )
			.arg( formatStyleKeywords(d->currentChatStyle->getHeaderHtml()) )
			.arg( formatStyleKeywords(d->currentChatStyle->getFooterHtml()) )
			.arg( adjustStyleVariantForChatSession( KopeteChatWindowSettings::self()->styleVariant() ) )
			.arg( styleHTML() );
	}
	else
	{
		xhtmlBase = i18n( "Chat style could not be found, or is invalid." );
	}
	write(xhtmlBase);
	end();
#ifdef STYLE_TIMETEST
	kDebug(14000) << "Header time: " << beforeHeader.msecsTo( QTime::currentTime());
#endif
}

void ChatMessagePart::resendMessage( uint messageId )
{
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for ( it = d->allMessages.constBegin(); it != itEnd; ++it )
	{
		if ( (*it).id() == messageId )
		{
			if ( !( d->manager->protocol()->capabilities() & Kopete::Protocol::CanSendOffline ) )
			{
				bool reachableContactFound = false;
				foreach ( Kopete::Contact* c, (*it).to() )
				{
					if ( c->isReachable() )
					{
						reachableContactFound = true;
						break;
					}
				}

				// no online contact found and can't send offline? can't send.
				if ( !reachableContactFound )
					return;
			}

			Kopete::Message msg( (*it).from(), (*it).to() );
			msg.setDirection( Kopete::Message::Outbound );
			msg.setBody( (*it).body() );

// 			msg.setBackgroundColor( (*it).backgroundColor() );
			msg.setForegroundColor( (*it).foregroundColor() );
			msg.setFont( (*it).font() );
			d->manager->sendMessage( msg );
			break;
		}
	}
}

void ChatMessagePart::saveVoiceClip( uint messageId )
{
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for ( it = d->allMessages.constBegin(); it != itEnd; ++it )
	{
		if ( (*it).id() == messageId )
		{
			if(!(*it).fileName().isEmpty())
			{
				QString temporaryExtension = "*.wav";
				QString newFileName = QFileDialog::getSaveFileName(NULL,
						i18n("Save File as"), QString(), i18n("Wav file (*.wav)"), &temporaryExtension);
				if(!newFileName.isEmpty())
					QFile::copy((*it).fileName(), newFileName);
			}
			break;
		}
	}
}

void ChatMessagePart::playVoiceClip( uint messageId )
{
	QList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for ( it = d->allMessages.constBegin(); it != itEnd; ++it )
	{
		if ( (*it).id() == messageId )
		{
			if(!(*it).fileName().isEmpty())
			{
				Phonon::MediaObject *media = new Phonon::MediaObject(this);
				Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
				connect(media, SIGNAL(finished()), media, SLOT(deleteLater()));
				connect(media, SIGNAL(finished()), audioOutput, SLOT(deleteLater()));
				createPath(media, audioOutput);
				media->setCurrentSource((*it).fileName());
				media->play();
			}
			break;
		}
	}
}

QString ChatMessagePart::adjustStyleVariantForChatSession( const QString & styleVariant ) const
{
	if ( d->currentChatStyle && d->manager->form() == Kopete::ChatSession::Chatroom
			&& KopeteChatWindowSettings::self()->useCompact() ) {
		return d->currentChatStyle->compact( styleVariant );
	}
	return styleVariant;
}

QString ChatMessagePart::photoForContact( const Kopete::Contact *contact ) const
{
	QString photo;
	if ( !contact )
		return photo;

	if( contact->metaContact() == Kopete::ContactList::self()->myself() )
	{ // all myself contacts have the same metaContact so take photo directly from contact otherwise the photo could be wrong.
		photo = contact->property(Kopete::Global::Properties::self()->photo().key()).value().toString();
	}
	else if( !contact->metaContact()->picture().isNull() )
	{
		photo = QString( "data:image/png;base64," ) + contact->metaContact()->picture().base64();
	}

	return photo;
}

void ChatMessagePart::addFileTransferButtonsEventListener( unsigned int id )
{
	QString elementId = QString( "ftSV%1" ).arg( id );
	registerClickEventListener( document().getElementById( elementId ) );

	elementId = QString( "ftSA%1" ).arg( id );
	registerClickEventListener( document().getElementById( elementId ) );

	elementId = QString( "ftCC%1" ).arg( id );
	registerClickEventListener( document().getElementById( elementId ) );
}

void ChatMessagePart::addVoiceClipsButtonsEventListener( unsigned int id )
{
	QString elementId = QString( "vcSA%1" ).arg( id );
	registerClickEventListener( document().getElementById( elementId ) );

	elementId = QString( "vcPL%1" ).arg( id );
	registerClickEventListener( document().getElementById( elementId ) );
}


void ChatMessagePart::disableFileTransferButtons( unsigned int id )
{
	QString elementId = QString( "ftSV%1" ).arg( id );
	DOM::HTMLInputElement element = document().getElementById( elementId );
	if ( !element.isNull() )
		element.setDisabled( true );

	elementId = QString( "ftSA%1" ).arg( id );
	element = document().getElementById( elementId );
	if ( !element.isNull() )
		element.setDisabled( true );

	elementId = QString( "ftCC%1" ).arg( id );
	element = document().getElementById( elementId );
	if ( !element.isNull() )
		element.setDisabled( true );
}

void ChatMessagePart::changeMessageStateElement( uint id, Kopete::Message::MessageState state )
{
	if ( !d->currentChatStyle )
		return;

	QString elementId = QString( "msST%1" ).arg( id );
	DOM::HTMLElement element = document().getElementById( elementId );
	if ( element.isNull() )
		return;

	QString statusHTML;
	switch ( state )
	{
	case Kopete::Message::StateUnknown:
		statusHTML = d->currentChatStyle->getOutgoingStateUnknownHtml();
		break;
	case Kopete::Message::StateSending:
		statusHTML = d->currentChatStyle->getOutgoingStateSendingHtml();
		break;
	case Kopete::Message::StateSent:
		statusHTML = d->currentChatStyle->getOutgoingStateSentHtml();
		break;
	case Kopete::Message::StateError:
		statusHTML = d->currentChatStyle->getOutgoingStateErrorHtml();
		break;
	}

	QString resendId = QString( "msRS%1" ).arg( id );
	statusHTML.replace( QLatin1String( "%resendHandlerId%" ), resendId );
	element.setInnerHTML( statusHTML );

	registerClickEventListener( document().getElementById( resendId ) );
}

void ChatMessagePart::registerClickEventListener( DOM::HTMLElement element )
{
	if ( element.isNull() )
		return;

	if ( !d->htmlEventListener )
	{
		d->htmlEventListener = new HTMLEventListener();
		connect( d->htmlEventListener, SIGNAL(resendMessage(uint)), this, SLOT(resendMessage(uint)) );
		connect( d->htmlEventListener, SIGNAL(playVoiceClip(uint)), this, SLOT(playVoiceClip(uint)) );
		connect( d->htmlEventListener, SIGNAL(saveVoiceClip(uint)), this, SLOT(saveVoiceClip(uint)) );
	}
	element.addEventListener( "click", d->htmlEventListener, false );
}

void ChatMessagePart::readChatFont()
{
	Kopete::AppearanceSettings *settings = Kopete::AppearanceSettings::self();

	d->chatFont = KGlobalSettings::generalFont();
	if ( settings->chatFontSelection() == 1 )
		d->chatFont = settings->chatFont();
}

void HTMLEventListener::handleEvent( DOM::Event &event )
{
	DOM::HTMLInputElement element = event.currentTarget();
	if ( !element.isNull() )
	{
		QString idType = element.id().string().left(4);
		unsigned int messageId = element.id().string().mid(4).toUInt();

		if ( idType == QLatin1String( "ftSV" ) )
			Kopete::TransferManager::transferManager()->saveIncomingTransfer( messageId );
		else if ( idType == QLatin1String( "ftSA" ) )
			Kopete::TransferManager::transferManager()->saveIncomingTransfer( messageId );
		else if ( idType == QLatin1String( "ftCC" ) )
			Kopete::TransferManager::transferManager()->cancelIncomingTransfer( messageId );
		else if ( idType == QLatin1String( "msRS" ) )
			emit resendMessage( messageId );
		else if ( idType == QLatin1String( "vcPL" ) )
			emit playVoiceClip( messageId );
		else if ( idType == QLatin1String( "vcSA" ) )
			emit saveVoiceClip( messageId );
	}
}

#include "chatmessagepart.moc"

// vim: set noet ts=4 sts=4 sw=4:
