/*
    chatmessagepart.cpp - Chat Message KPart

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2005-2006 by Michaël Larouche      <michael.larouche@kdemail.net>

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

#include "chatmessagepart.h"

// STYLE_TIMETEST is for time staticstic gathering.
//#define STYLE_TIMETEST

#include <ctime>

// Qt includes
#include <qclipboard.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qcursor.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qstylesheet.h>

// KHTML::DOM includes
#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <dom/dom_element.h>
#include <dom/html_base.h>
#include <dom/html_document.h>
#include <dom/html_inline.h>


// KDE includes
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmultipledrag.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kstringhandler.h>
#include <ktempfile.h>
#include <kurldrag.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

// Kopete includes
#include "chatmemberslistwidget.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetechatwindow.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopeteemoticons.h"
#include "kopeteview.h"
#include "kopetepicture.h"

#include "kopetechatwindowstyle.h"
#include "kopetechatwindowstylemanager.h"

#if !(KDE_IS_VERSION(3,3,90))
//From  kdelibs/khtml/misc/htmltags.h
//  used in ChatMessagePart::copy()
#define ID_BLOCKQUOTE 12
#define ID_BR 14
#define ID_DD 22
#define ID_DIV 26
#define ID_DL 27
#define ID_DT 28
#define ID_H1 36
#define ID_H2 37
#define ID_H3 38
#define ID_H4 39
#define ID_H5 40
#define ID_H6 41
#define ID_HR 43
#define ID_IMG 48
#define ID_LI 57
#define ID_OL 69
#define ID_P 72
#define ID_PRE 75
#define ID_TD 90
#define ID_TH 93
#define ID_TR 96
#define ID_TT 97
#define ID_UL 99
#endif

class ToolTip;

class ChatMessagePart::Private
{
public:
	Private() 
	 : tt(0L), manager(0L), scrollPressed(false),
	   copyAction(0L), saveAction(0L), printAction(0L),
	   closeAction(0L),copyURLAction(0L), currentChatStyle(0L), latestContact(0L),
	   latestDirection(Kopete::Message::Inbound), latestType(Kopete::Message::TypeNormal)
	{}
	
	~Private()
	{
		// Don't delete manager and latestContact, because they could be still used.
		// Don't delete currentChatStyle, it is handled by ChatWindowStyleManager.
	}

	bool bgOverride;
	bool fgOverride;
	bool rtfOverride;

	ToolTip *tt;

	Kopete::ChatSession *manager;
	bool scrollPressed;

	DOM::HTMLElement activeElement;

	KAction *copyAction;
	KAction *saveAction;
	KAction *printAction;
	KAction *closeAction;
	KAction *copyURLAction;

	ChatWindowStyle *currentChatStyle;
	Kopete::Contact *latestContact;
	Kopete::Message::MessageDirection latestDirection;
	Kopete::Message::MessageType latestType;
	// Yep I know it will take memory, but I don't have choice
	// to enable on-the-fly style changing.
	QValueList<Kopete::Message> allMessages;
};

class ChatMessagePart::ToolTip : public QToolTip
{
public:
	ToolTip( ChatMessagePart *c ) : QToolTip( c->view()->viewport() )
	{
		m_chat = c;
	}

	void maybeTip( const QPoint &/*p*/ )
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

ChatMessagePart::ChatMessagePart( Kopete::ChatSession *mgr, QWidget *parent, const char *name)
	: KHTMLPart( parent, name ), d( new Private )
{
	d->manager = mgr;

	KopetePrefs *kopetePrefs = KopetePrefs::prefs();
	d->currentChatStyle = ChatWindowStyleManager::self()->getStyleFromPool( kopetePrefs->stylePath() );

	//Security settings, we don't need this stuff
	setJScriptEnabled( false ) ;
	setJavaEnabled( false );
	setPluginsEnabled( false );
	setMetaRefreshEnabled( false );
	setOnlyLocalReferences( true );

	// Write the template to KHTMLPart
	writeTemplate();

	view()->setFocusPolicy( QWidget::NoFocus );

	d->tt=new ToolTip( this );

	// It is not possible to drag and drop on our widget
	view()->setAcceptDrops(false);

	connect( KopetePrefs::prefs(), SIGNAL(messageAppearanceChanged()),
	         this, SLOT( slotAppearanceChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(windowAppearanceChanged()),
	         this, SLOT( slotRefreshView() ) );
	connect( KopetePrefs::prefs(), SIGNAL(styleChanged(const QString &)),
			 this, SLOT( setStyle(const QString &) ) );
	connect( KopetePrefs::prefs(), SIGNAL(styleVariantChanged(const QString &)),
			 this, SLOT( setStyleVariant(const QString &) ) );
	// Refresh the style if the display name change.
	connect( d->manager, SIGNAL(displayNameChanged()), this, SLOT(slotUpdateHeaderDisplayName()) );
	connect( d->manager, SIGNAL(photoChanged()), this, SLOT(slotUpdateHeaderPhoto()) );

	connect ( browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
	          this, SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );

	connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)),
	         this, SLOT(slotRightClick(const QString &, const QPoint &)) );
	connect( view(), SIGNAL(contentsMoving(int,int)),
	         this, SLOT(slotScrollingTo(int,int)) );

	//initActions
	d->copyAction = KStdAction::copy( this, SLOT(copy()), actionCollection() );
	d->saveAction = KStdAction::saveAs( this, SLOT(save()), actionCollection() );
	d->printAction = KStdAction::print( this, SLOT(print()),actionCollection() );
	d->closeAction = KStdAction::close( this, SLOT(slotCloseView()),actionCollection() );
	d->copyURLAction = new KAction( i18n( "Copy Link Address" ), QString::fromLatin1( "editcopy" ), 0, this, SLOT( slotCopyURL() ), actionCollection() );

	// read formatting override flags
	readOverrides();
}

ChatMessagePart::~ChatMessagePart()
{
	kdDebug(14000) << k_funcinfo << endl;
	delete d->tt;
	delete d;
}

void ChatMessagePart::slotScrollingTo( int /*x*/, int y )
{
	int scrolledTo = y + view()->visibleHeight();
	if ( scrolledTo >= ( view()->contentsHeight() - 10 ) )
		d->scrollPressed = false;
	else
		d->scrollPressed = true;
}

void ChatMessagePart::save()
{
	KFileDialog dlg( QString::null, QString::fromLatin1( "text/html text/plain" ), view(), "fileSaveDialog", false );
	dlg.setCaption( i18n( "Save Conversation" ) );
	dlg.setOperationMode( KFileDialog::Saving );

	if ( dlg.exec() != QDialog::Accepted )
		return;

	KURL saveURL = dlg.selectedURL();
	KTempFile tempFile;
	tempFile.setAutoDelete( true );
	QFile* file = tempFile.file();

	QTextStream stream ( file );
	stream.setEncoding(QTextStream::UnicodeUTF8);

	if ( dlg.currentFilter() == QString::fromLatin1( "text/plain" ) )
	{
		QValueList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
		for(it = d->allMessages.constBegin(); it != itEnd; ++it)
		{
			Kopete::Message tempMessage = *it;
			stream << "[" << KGlobal::locale()->formatDateTime(tempMessage.timestamp()) << "] ";
			if( tempMessage.from() && tempMessage.from()->metaContact() )
			{
				stream << formatName(tempMessage.from()->metaContact()->displayName());
			}
			stream << ": " << tempMessage.plainBody() << "\n";
		}
	}
	else
	{
		stream << htmlDocument().toHTML() << '\n';
	}

	tempFile.close();

	if ( !KIO::NetAccess::move( KURL( tempFile.name() ), saveURL ) )
	{
		KMessageBox::queuedMessageBox( view(), KMessageBox::Error,
				i18n("<qt>Could not open <b>%1</b> for writing.</qt>").arg( saveURL.prettyURL() ), // Message
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

void ChatMessagePart::slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/)
{
	kdDebug(14000) << k_funcinfo << "url=" << url.url() << endl;
	if ( url.protocol() == QString::fromLatin1("kopetemessage") )
	{
		Kopete::Contact *contact = d->manager->account()->contacts()[ url.host() ];
		if ( contact )
			contact->execute();
	}
	else
	{
		KRun *runner = new KRun( url, 0, false ); // false = non-local files
		runner->setRunExecutables( false ); //security
		//KRun autodeletes itself by default when finished.
	}
}

void ChatMessagePart::readOverrides()
{
	d->bgOverride = KopetePrefs::prefs()->bgOverride();
	d->fgOverride = KopetePrefs::prefs()->fgOverride();
	d->rtfOverride = KopetePrefs::prefs()->rtfOverride();
}

void ChatMessagePart::setStyle( const QString &stylePath )
{
	// Create a new ChatWindowStyle
	d->currentChatStyle = ChatWindowStyleManager::self()->getStyleFromPool(stylePath);

	// Do the actual style switch
	// Wait for the event loop before switching the style
	QTimer::singleShot( 0, this, SLOT(changeStyle()) );
}

void ChatMessagePart::setStyle( ChatWindowStyle *style )
{
	// Change the current style
	d->currentChatStyle = style;

	// Do the actual style switch
	// Wait for the event loop before switching the style
	QTimer::singleShot( 0, this, SLOT(changeStyle()) );
}

void ChatMessagePart::setStyleVariant( const QString &variantPath )
{
	DOM::HTMLElement variantNode = document().getElementById( QString::fromUtf8("mainStyle") );
	if( !variantNode.isNull() )
		variantNode.setInnerText( QString("@import url(\"%1\");").arg(variantPath) );
}

void ChatMessagePart::slotAppearanceChanged()
{
	readOverrides();

	changeStyle();
}

void ChatMessagePart::appendMessage( Kopete::Message &message, bool restoring )
{
	message.setBgOverride( d->bgOverride );
	message.setFgOverride( d->fgOverride );
	message.setRtfOverride( d->rtfOverride );
	
	// parse emoticons and URL now.
	// Do not reparse emoticons on restoring, because it cause very intensive CPU usage on long chats.
	if( !restoring )
		message.setBody( message.parsedBody() , Kopete::Message::ParsedHTML );

#ifdef STYLE_TIMETEST
	QTime beforeMessage = QTime::currentTime();
#endif

	QString formattedMessageHtml;
	bool isConsecutiveMessage = false;
	uint bufferLen = (uint)KopetePrefs::prefs()->chatViewBufferSize();

	// Find the "Chat" div element.
	// If the "Chat" div element is not found, do nothing. It's the central part of Adium format.
	DOM::HTMLElement chatNode = htmlDocument().getElementById( "Chat" );

	if( chatNode.isNull() )
	{
		kdDebug(14000) << k_funcinfo << "WARNING: Chat Node was null !" << endl;
		return;
	}

	// Check if it's a consecutive Message
	// Consecutive messages are only for normal messages, status messages do not have a <div id="insert" />
	// We check if the from() is the latestContact, because consecutive incoming/outgoing message can come from differents peopole(in groupchat and IRC)
	// Group only if the user want it.
	if( KopetePrefs::prefs()->groupConsecutiveMessages() )
	{
		isConsecutiveMessage = (message.direction() == d->latestDirection && d->latestContact && d->latestContact == message.from() && message.type() == d->latestType);
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
	DOM::HTMLElement newMessageNode = document().createElement( QString::fromUtf8("span") );
	newMessageNode.setInnerHTML( formattedMessageHtml );

	// Find the insert Node
	DOM::HTMLElement insertNode = document().getElementById( QString::fromUtf8("insert") );

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
		if( !KopetePrefs::prefs()->groupConsecutiveMessages() )
		{
			chatNode.removeChild( chatNode.firstChild() );
		}
	}

	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );

#ifdef STYLE_TIMETEST
	kdDebug(14000) << "Message time: " << beforeMessage.msecsTo( QTime::currentTime()) << endl;
#endif
}

void ChatMessagePart::slotRefreshView()
{
	DOM::HTMLElement kopeteNode = document().getElementById( QString::fromUtf8("KopeteStyle") );
	if( !kopeteNode.isNull() )
		kopeteNode.setInnerText( styleHTML() );

	DOM::HTMLBodyElement bodyElement = htmlDocument().body();
	bodyElement.setBgColor( KopetePrefs::prefs()->bgColor().name() );
}

void ChatMessagePart::keepScrolledDown()
{
	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

const QString ChatMessagePart::styleHTML() const
{
	KopetePrefs *p = KopetePrefs::prefs();

	int fontSize = 0;
	QString fontSizeCss;
	// Use correct font size unit, depending of how the QFont was build.
	if( p->fontFace().pointSize() != -1 )
	{
		fontSize = p->fontFace().pointSize();
		fontSizeCss = QString::fromUtf8("%1pt;").arg(fontSize);
	}
	else if( p->fontFace().pixelSize() != -1 )
	{
		fontSize = p->fontFace().pixelSize();
		fontSizeCss = QString::fromUtf8("%1px;").arg(fontSize);
	}

	QString style = QString::fromLatin1(
		"body{background-color:%1;font-family:%2;font-size:%3;color:%4}"
		"td{font-family:%5;font-size:%6;color:%7}"
		"a{color:%8}a.visited{color:%9}"
		"a.KopeteDisplayName{text-decoration:none;color:inherit;}"
		"a.KopeteDisplayName:hover{text-decoration:underline;color:inherit}"
		".KopeteLink{cursor:pointer;}.KopeteLink:hover{text-decoration:underline}"
		".KopeteMessageBody > p:first-child{margin:0;padding:0;display:inline;}" /* some html messages are encapsuled into a <p> */ )
		.arg( p->bgColor().name() )
		.arg( p->fontFace().family() )
		.arg( fontSizeCss )
		.arg( p->textColor().name() )
		.arg( p->fontFace().family() )
		.arg( fontSizeCss )
		.arg( p->textColor().name() )
		.arg( p->linkColor().name() )
		.arg( p->linkColor().name() );

	return style;
}

void ChatMessagePart::clear()
{
	// writeTemplate actually reset the HTML chat session from the beginning.
	writeTemplate();

	// Reset consecutive messages
	d->latestContact = 0;
	// Remove all stored messages.
	d->allMessages.clear();
}

Kopete::Contact *ChatMessagePart::contactFromNode( const DOM::Node &n ) const
{
	DOM::Node node = n;

	if ( node.isNull() )
		return 0;

	while ( !node.isNull() && ( node.nodeType() == DOM::Node::TEXT_NODE || ((DOM::HTMLElement)node).className() != "KopeteDisplayName" ) )
		node = node.parentNode();

	DOM::HTMLElement element = node;
	if ( element.className() != "KopeteDisplayName" )
		return 0;

	if ( element.hasAttribute( "contactid" ) )
	{
		QString contactId = element.getAttribute( "contactid" ).string();
		for ( QPtrListIterator<Kopete::Contact> it ( d->manager->members() ); it.current(); ++it )
			if ( (*it)->contactId() == contactId )
				return *it;
	}
	else
	{
		QString nick = element.innerText().string().stripWhiteSpace();
		for ( QPtrListIterator<Kopete::Contact> it ( d->manager->members() ); it.current(); ++it )
			if ( (*it)->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString() == nick )
				return *it;
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

	KPopupMenu *chatWindowPopup = 0L;

	if ( Kopete::Contact *contact = contactFromNode( d->activeElement ) )
	{
		chatWindowPopup = contact->popupMenu( d->manager );
		connect( chatWindowPopup, SIGNAL( aboutToHide() ), chatWindowPopup , SLOT( deleteLater() ) );
	}
	else
	{
		chatWindowPopup = new KPopupMenu();

		if ( d->activeElement.className() == "KopeteDisplayName" )
		{
			chatWindowPopup->insertItem( i18n( "User Has Left" ), 1 );
			chatWindowPopup->setItemEnabled( 1, false );
			chatWindowPopup->insertSeparator();
		}
		else if ( d->activeElement.tagName().lower() == QString::fromLatin1( "a" ) )
		{
			d->copyURLAction->plug( chatWindowPopup );
			chatWindowPopup->insertSeparator();
		}

		d->copyAction->setEnabled( hasSelection() );
		d->copyAction->plug( chatWindowPopup );
		d->saveAction->plug( chatWindowPopup );
		d->printAction->plug( chatWindowPopup );
		chatWindowPopup->insertSeparator();
		d->closeAction->plug( chatWindowPopup );

		connect( chatWindowPopup, SIGNAL( aboutToHide() ), chatWindowPopup, SLOT( deleteLater() ) );
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
		return QString::null;

	DOM::Text textNode = activeNode;
	QString data = textNode.data().string();

	//Ok, we have the whole node. Now, find the text under the mouse.
	int mouseLeft = view()->mapFromGlobal( QCursor::pos() ).x(),
		nodeLeft = activeNode.getRect().x(),
		cPos = 0,
		dataLen = data.length();

	QFontMetrics metrics( KopetePrefs::prefs()->fontFace() );
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
	// NB: view()->contentsHeight() is incorrect before the view has been shown in its window.
	// Until this happens, the geometry has not been correctly calculated, so this scrollBy call
	// will usually scroll to the top of the view.
	view()->scrollBy( 0, view()->contentsHeight() );
}

void ChatMessagePart::copy(bool justselection /* default false */)
{
	/*
	* The objective of this function is to keep the text of emoticons (or of latex image) when copying.
	*   see Bug 61676
	* This also copies the text as type text/html
	* RangeImpl::toHTML  was not implemented before KDE 3.4
	*/
	QString text;
	QString htmltext;
	
#if KDE_IS_VERSION(3,3,90)
        htmltext = selectedTextAsHTML();
        text = selectedText();
        //selectedText is now sufficent
//      text=Kopete::Message::unescape( htmltext ).stripWhiteSpace();
        // Message::unsescape will replace image by his title attribute
        // stripWhiteSpace is for removing the newline added by the <!DOCTYPE> and other xml things of RangeImpl::toHTML
#else

        DOM::Node startNode, endNode;
        long startOffset, endOffset;
        selection( startNode, startOffset, endNode, endOffset );

        //BEGIN: copied from KHTMLPart::selectedText

        bool hasNewLine = true;
        DOM::Node n = startNode;
        while(!n.isNull())
        {
                if(n.nodeType() == DOM::Node::TEXT_NODE /*&& n.handle()->renderer()*/)
                {
                        QString str = n.nodeValue().string();
                        hasNewLine = false;
                        if(n == startNode && n == endNode)
                                text = str.mid(startOffset, endOffset - startOffset);
                        else if(n == startNode)
                                text = str.mid(startOffset);
                        else if(n == endNode)
                                text += str.left(endOffset);
                        else
                                text += str;
                }
                else
                { // This is our simple HTML -> ASCII transformation:
                        unsigned short id = n.elementId();
                        switch(id)
                        {
                        case ID_IMG: //here is the main difference with KHTMLView::selectedText
                        {
                                DOM::HTMLElement e = n;
                                if( !e.isNull() && e.hasAttribute( "title" ) )
                                        text+=e.getAttribute( "title" ).string();
                                break;
                        }
                        case ID_BR:
                                text += "\n";
                                hasNewLine = true;
                                break;
                        case ID_TD:  case ID_TH:  case ID_HR:
                        case ID_OL:  case ID_UL:  case ID_LI:
                        case ID_DD:  case ID_DL:  case ID_DT:
                        case ID_PRE: case ID_BLOCKQUOTE: case ID_DIV:
                                if (!hasNewLine)
                                        text += "\n";
                                hasNewLine = true;
                                break;
                        case ID_P:   case ID_TR:
                        case ID_H1:  case ID_H2:  case ID_H3:
                        case ID_H4:  case ID_H5:  case ID_H6:
                                if (!hasNewLine)
                                        text += "\n";
                                text += "\n";
                                hasNewLine = true;
                                break;
                        }
                }
                if(n == endNode)
                        break;
                DOM::Node next = n.firstChild();
                if(next.isNull())
                        next = n.nextSibling();
                while( next.isNull() && !n.parentNode().isNull() )
                {
                        n = n.parentNode();
                        next = n.nextSibling();
                        unsigned short id = n.elementId();
                        switch(id)
                        {
                        case ID_TD:  case ID_TH:  case ID_HR:
                        case ID_OL:  case ID_UL:  case ID_LI:
                        case ID_DD:  case ID_DL:  case ID_DT:
                        case ID_PRE: case ID_BLOCKQUOTE:  case ID_DIV:
                                if (!hasNewLine)
                                        text += "\n";
                                hasNewLine = true;
                                break;
                        case ID_P:   case ID_TR:
                        case ID_H1:  case ID_H2:  case ID_H3:
                        case ID_H4:  case ID_H5:  case ID_H6:
                                if (!hasNewLine)
                                        text += "\n";
                                text += "\n";
                                hasNewLine = true;
                                break;
                        }
                }
                n = next;
        }

        if(text.isEmpty())
                return;

        int start = 0;
        int end = text.length();

        // Strip leading LFs
        while ((start < end) && (text[start] == '\n'))
                start++;

        // Strip excessive trailing LFs
        while ((start < (end-1)) && (text[end-1] == '\n') && (text[end-2] == '\n'))
                end--;

        text=text.mid(start, end-start);

        //END: copied from KHTMLPart::selectedText
#endif

	if(text.isEmpty()) return;

	disconnect( kapp->clipboard(), SIGNAL( selectionChanged()), this, SLOT( slotClearSelection()));

#ifndef QT_NO_MIMECLIPBOARD
	if(!justselection)
	{
      	QTextDrag *textdrag = new QTextDrag(text, 0L);
	    KMultipleDrag *drag = new KMultipleDrag( );
    	drag->addDragObject( textdrag );
    	if(!htmltext.isEmpty()) {
	    	htmltext.replace( QChar( 0xa0 ), ' ' );
    		QTextDrag *htmltextdrag = new QTextDrag(htmltext, 0L);
    		htmltextdrag->setSubtype("html");
            drag->addDragObject( htmltextdrag );
    	}
    	QApplication::clipboard()->setData( drag, QClipboard::Clipboard );
	}
    QApplication::clipboard()->setText( text, QClipboard::Selection );
#else
	if(!justselection)
    	QApplication::clipboard()->setText( text, QClipboard::Clipboard );
	QApplication::clipboard()->setText( text, QClipboard::Selection );
#endif
	connect( kapp->clipboard(), SIGNAL( selectionChanged()), SLOT( slotClearSelection()));

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
	d->manager->view()->closeView( force );
}

void ChatMessagePart::emitTooltipEvent(  const QString &textUnderMouse, QString &toolTip )
{
	emit tooltipEvent(  textUnderMouse, toolTip );
}

// Style formatting for messages(incoming, outgoing, status)
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML, const Kopete::Message &_message )
{
	Kopete::Message message=_message; //we will eventually need to modify it before showing it.
	QString resultHTML = sourceHTML;
	QString nick, contactId, service, protocolIcon, nickLink;
	
	if( message.from() )
	{
		// Use metacontact display name if the metacontact exists and if its not the myself metacontact.
		if( message.from()->metaContact() && message.from()->metaContact() != Kopete::ContactList::self()->myself() )
		{
			nick = message.from()->metaContact()->displayName();
		}
		// Use contact nickname for no metacontact or myself.
		else
		{
			nick = message.from()->nickName();
		}
		nick = formatName(nick);
		contactId = message.from()->contactId();
		// protocol() returns NULL here in the style preview in appearance config.
		// this isn't the right place to work around it, since contacts should never have
		// no protocol, but it works for now.
		//
		// Use default if protocol() and protocol()->displayName() is NULL.
		// For preview and unit tests.
		QString iconName = QString::fromUtf8("kopete");
		service = QString::fromUtf8("Kopete");
		if(message.from()->protocol() && !message.from()->protocol()->displayName().isNull())
		{
			service =  message.from()->protocol()->displayName();
			iconName = message.from()->protocol()->pluginIcon();
		}

		protocolIcon = KGlobal::iconLoader()->iconPath( iconName, KIcon::Small );
		
		nickLink=QString::fromLatin1("<a href=\"kopetemessage://%1/?protocolId=%2&amp;accountId=%3\" class=\"KopeteDisplayName\">")
				.arg( QStyleSheet::escape(message.from()->contactId()).replace('"',"&quot;"),
					  QStyleSheet::escape(message.from()->protocol()->pluginId()).replace('"',"&quot;"), 
					  QStyleSheet::escape(message.from()->account()->accountId() ).replace('"',"&quot;"));
	}
	else
	{
		nickLink="<a>";
	}
	
	
	// Replace sender (contact nick)
	resultHTML = resultHTML.replace( QString::fromUtf8("%sender%"), nickLink+nick+"</a>" );
	// Replace time, by default display only time and display seconds(that was true means).
	resultHTML = resultHTML.replace( QString::fromUtf8("%time%"), KGlobal::locale()->formatTime(message.timestamp().time(), true) );
	// Replace %screenName% (contact ID)
	resultHTML = resultHTML.replace( QString::fromUtf8("%senderScreenName%"), nickLink+QStyleSheet::escape(contactId)+"</a>" );
	// Replace service name (protocol name)
	resultHTML = resultHTML.replace( QString::fromUtf8("%service%"), QStyleSheet::escape(service) );
	// Replace protocolIcon (sender statusIcon)
	resultHTML = resultHTML.replace( QString::fromUtf8("%senderStatusIcon%"), QStyleSheet::escape(protocolIcon).replace('"',"&quot;") );
	
	// Look for %time{X}%
	QRegExp timeRegExp("%time\\{([^}]*)\\}%");
	int pos=0;
	while( (pos=timeRegExp.search(resultHTML , pos) ) != -1 )
	{
		QString timeKeyword = formatTime( timeRegExp.cap(1), message.timestamp() );
		resultHTML = resultHTML.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
	}

	// Look for %textbackgroundcolor{X}% 
	// TODO: use the X value.
	// Replace with user-selected highlight color if to be highlighted or
	// with "inherit" otherwise to keep CSS clean
	QString bgColor = QString::fromUtf8("inherit");
	if( message.importance() == Kopete::Message::Highlight && KopetePrefs::prefs()->highlightEnabled() )
	{
		bgColor = KopetePrefs::prefs()->highlightBackground().name();
	}

	QRegExp textBackgroundRegExp("%textbackgroundcolor\\{([^}]*)\\}%");
	int textPos=0;
	while( (textPos=textBackgroundRegExp.search(resultHTML, textPos) ) != -1 )
	{
		resultHTML = resultHTML.replace( textPos , textBackgroundRegExp.cap(0).length() , bgColor );
	}

	// Replace userIconPath
	if( message.from() )
	{
		QString photoPath;
#if 0
		photoPath = message.from()->property(Kopete::Global::Properties::self()->photo().key()).value().toString();
		// If the photo path is empty, set the default buddy icon for the theme
		if( photoPath.isEmpty() )
		{
			if(message.direction() == Kopete::Message::Inbound)
				photoPath = QString::fromUtf8("Incoming/buddy_icon.png");
			else if(message.direction() == Kopete::Message::Outbound)
				photoPath = QString::fromUtf8("Outgoing/buddy_icon.png");
		}
#endif
		if( !message.from()->metaContact()->picture().isNull() )
		{
			photoPath = QString( "data:image/png;base64," ) + message.from()->metaContact()->picture().base64();
		}
		else
		{
			if(message.direction() == Kopete::Message::Inbound)
				photoPath = QString::fromUtf8("Incoming/buddy_icon.png");
			else if(message.direction() == Kopete::Message::Outbound)
				photoPath = QString::fromUtf8("Outgoing/buddy_icon.png");
		}
		resultHTML = resultHTML.replace(QString::fromUtf8("%userIconPath%"), photoPath);
	}

	// Replace messages.
	// Build the action message if the currentChatStyle do not have Action template.
	if( message.type() == Kopete::Message::TypeAction && !d->currentChatStyle->hasActionTemplate() )
	{
		kdDebug(14000) << k_funcinfo << "Map Action message to Status template. " << endl;

		QString boldNick = QString::fromUtf8("%1<b>%2</b></a> ").arg(nickLink,nick);
		QString newBody = boldNick + message.parsedBody();
		message.setBody(newBody, Kopete::Message::ParsedHTML );
	}

	// Set message direction("rtl"(Right-To-Left) or "ltr"(Left-to-right))
	resultHTML = resultHTML.replace( QString::fromUtf8("%messageDirection%"), message.isRightToLeft() ? "rtl" : "ltr" );

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
	for( uint f = 0; f < contactId.length(); ++f )
		hash += contactId[f].unicode() * f;
	const QString colorName = nameColors[ hash % nameColorsLen ];
	QString lightColorName;	// Do not initialize, QColor::name() is expensive!
	kdDebug(14000) << k_funcinfo << "Hash " << hash << " has color " << colorName << endl;
	QRegExp senderColorRegExp("%senderColor(?:\\{([^}]*)\\})?%");
	textPos=0;
	while( (textPos=senderColorRegExp.search(resultHTML, textPos) ) != -1 )
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

		resultHTML = resultHTML.replace( textPos , senderColorRegExp.cap(0).length(),
			doLight ? lightColorName : colorName );
	}

	// Replace message at the end, maybe someone could put a Adium keyword in his message :P
	resultHTML = resultHTML.replace( QString::fromUtf8("%message%"), formatMessageBody(message) );

	// TODO: %status
//	resultHTML = addNickLinks( resultHTML );
	return resultHTML;
}

// Style formatting for header and footer.
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML )
{
	QString resultHTML = sourceHTML;

	Kopete::Contact *remoteContact = d->manager->members().getFirst();

	// Verify that all contacts are not null before doing anything
	if( remoteContact && d->manager->myself() )
	{
		QString sourceName, destinationName;
		// Use contact nickname for ourselfs, Myself metacontact display name isn't a reliable source.
		sourceName = d->manager->myself()->nickName();
		if( remoteContact->metaContact() )
			destinationName = remoteContact->metaContact()->displayName();
		else
			destinationName = remoteContact->nickName();

		// Replace %chatName%, create a internal span to update it by DOM when asked.
		resultHTML = resultHTML.replace( QString::fromUtf8("%chatName%"), QString("<span id=\"KopeteHeaderChatNameInternal\">%1</span>").arg( formatName(d->manager->displayName()) ) );
		// Replace %sourceName%
		resultHTML = resultHTML.replace( QString::fromUtf8("%sourceName%"), formatName(sourceName) );
		// Replace %destinationName%
		resultHTML = resultHTML.replace( QString::fromUtf8("%destinationName%"), formatName(destinationName) );
		// For %timeOpened%, display the date and time (also the seconds).
		resultHTML = resultHTML.replace( QString::fromUtf8("%timeOpened%"), KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), true, true ) );

		// Look for %timeOpened{X}%
		QRegExp timeRegExp("%timeOpened\\{([^}]*)\\}%");
		int pos=0;
		while( (pos=timeRegExp.search(resultHTML, pos) ) != -1 )
		{
			QString timeKeyword = formatTime( timeRegExp.cap(1), QDateTime::currentDateTime() );
			resultHTML = resultHTML.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
		}
		// Get contact image paths
#if 0
		QString photoIncomingPath, photoOutgoingPath;
		photoIncomingPath = remoteContact->property( Kopete::Global::Properties::self()->photo().key()).value().toString();
		photoOutgoingPath = d->manager->myself()->property(Kopete::Global::Properties::self()->photo().key()).value().toString();

		if( photoIncomingPath.isEmpty() )
			photoIncomingPath = QString::fromUtf8("Incoming/buddy_icon.png");
		if( photoOutgoingPath.isEmpty() )
			photoOutgoingPath = QString::fromUtf8("Outgoing/buddy_icon.png");

		resultHTML = resultHTML.replace( QString::fromUtf8("%incomingIconPath%"), photoIncomingPath);
		resultHTML = resultHTML.replace( QString::fromUtf8("%outgoingIconPath%"), photoOutgoingPath);
#endif
		QString photoIncoming, photoOutgoing;
		if( remoteContact->metaContact() && !remoteContact->metaContact()->picture().isNull() )
		{
			photoIncoming = QString("data:image/png;base64,%1").arg( remoteContact->metaContact()->picture().base64() );
		}
		else
		{
			photoIncoming = QString::fromUtf8("Incoming/buddy_icon.png");
		}
		
		if( d->manager->myself()->metaContact() && !d->manager->myself()->metaContact()->picture().isNull() )
		{
			photoOutgoing =  QString("data:image/png;base64,%1").arg( d->manager->myself()->metaContact()->picture().base64() );
		}
		else
		{
			photoOutgoing = QString::fromUtf8("Outgoing/buddy_icon.png");
		}


		resultHTML = resultHTML.replace( QString::fromUtf8("%incomingIconPath%"), photoIncoming);
		resultHTML = resultHTML.replace( QString::fromUtf8("%outgoingIconPath%"), photoOutgoing );
	}

	return resultHTML;
}

QString ChatMessagePart::formatTime(const QString &timeFormat, const QDateTime &dateTime)
{
	char buffer[256];

	time_t timeT;
	struct tm *loctime;
	// Get current time
	timeT = dateTime.toTime_t();
	// Convert it to local time representation.
	loctime = localtime (&timeT);
	strftime (buffer, 256, timeFormat.ascii(), loctime);

	return QString(buffer);
}

QString ChatMessagePart::formatName(const QString &sourceName)
{
	QString formattedName = sourceName;
	// Escape the name.
	formattedName = Kopete::Message::escape(formattedName);

	// Squeeze the nickname if the user want it
	if( KopetePrefs::prefs()->truncateContactNames() )
	{
		formattedName = KStringHandler::csqueeze( sourceName, KopetePrefs::prefs()->maxConactNameLength() );
	}

	return formattedName;
}

QString ChatMessagePart::formatMessageBody(const Kopete::Message &message)
{
	QString formattedBody("<span ");
	
	formattedBody += message.getHtmlStyleAttribute();

	// Affect the parsed body.
	formattedBody += QString::fromUtf8("class=\"KopeteMessageBody\">%1</span>").arg(message.parsedBody());
	
	return formattedBody;
}

void ChatMessagePart::slotUpdateHeaderDisplayName()
{
	kdDebug(14000) << k_funcinfo << endl;
	DOM::HTMLElement kopeteChatNameNode = document().getElementById( QString::fromUtf8("KopeteHeaderChatNameInternal") );
	if( !kopeteChatNameNode.isNull() )
		kopeteChatNameNode.setInnerText( formatName(d->manager->displayName()) );
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
	QValueList<Kopete::Message>::ConstIterator it, itEnd = d->allMessages.constEnd();
	for(it = d->allMessages.constBegin(); it != itEnd; ++it)
	{
		Kopete::Message tempMessage = *it;
		appendMessage(tempMessage, true); // true means that we are restoring.
	}
	kdDebug(14000) << k_funcinfo << "Finish changing style." << endl;
#ifdef STYLE_TIMETEST
	kdDebug(14000) << "Change time: " << beforeChange.msecsTo( QTime::currentTime()) << endl;
#endif
}

void ChatMessagePart::writeTemplate()
{
	kdDebug(14000) << k_funcinfo << endl;

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

	// FIXME: Maybe this string should be load from a file, then parsed for args.
	QString xhtmlBase;
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
		.arg( KopetePrefs::prefs()->styleVariant() )
		.arg( styleHTML() );
	write(xhtmlBase);
	end();
#ifdef STYLE_TIMETEST
	kdDebug(14000) << "Header time: " << beforeHeader.msecsTo( QTime::currentTime()) << endl;
#endif
}

#include "chatmessagepart.moc"

// vim: set noet ts=4 sts=4 sw=4:

