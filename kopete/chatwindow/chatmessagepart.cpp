/*
    chatmessagepart.cpp - Chat Message KPart

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart @ kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Copyright (c) 2005      by Michaël Larouche      <michael.larouche@kdemail.net>

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

// KOPETE_XSLT enable old style engine
//#define KOPETE_XSLT
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
#include <krootpixmap.h>
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
#include "kopetechatwindow.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetexsl.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopeteemoticons.h"
#include "kopeteview.h"
#include "kopetepicture.h"

#include "kopetechatwindowstyle.h"

class ToolTip;

class ChatMessagePart::Private
{
public:
	Private() 
	 : tt(0L), manager(0L), messageId(0), scrollPressed(false), bgChanged(false),
	   backgroundFile(0L), root(0L), copyAction(0L), saveAction(0L), printAction(0L),
	   closeAction(0L),copyURLAction(0L), currentChatStyle(0L), latestContact(0L)
	{}
	
	~Private()
	{
		delete xsltParser;
		// Don't delete manager and latestContact, because they could be still used.
		delete currentChatStyle;

		if( backgroundFile )
		{
			backgroundFile->close();
			backgroundFile->unlink();
			delete backgroundFile;
		}
	}

	Kopete::XSLT *xsltParser;
	bool transparencyEnabled;
	bool bgOverride;
	bool fgOverride;
	bool rtfOverride;
	/**
	 * we want to render several messages in one pass if several message are apended at the same time.
	 */
	QTimer refreshtimer;
	bool transformAllMessages;
	ToolTip *tt;

	Kopete::ChatSession *manager;
	unsigned long messageId;
	QStringList messageMap;
	bool scrollPressed;
	bool bgChanged;

	DOM::HTMLElement activeElement;

	// FIXME: share
	KTempFile *backgroundFile;
	KRootPixmap *root;

	KAction *copyAction;
	KAction *saveAction;
	KAction *printAction;
	KAction *closeAction;
	KAction *copyURLAction;

	ChatWindowStyle *currentChatStyle;
	Kopete::Message::MessageDirection latestDirection;
	Kopete::Contact *latestContact;
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
	d->xsltParser = new Kopete::XSLT( KopetePrefs::prefs()->styleContents() );
	d->transformAllMessages = ( d->xsltParser->flags() & Kopete::XSLT::TransformAllMessages );

#ifndef KOPETE_XSLT
	KopetePrefs *kopetePrefs = KopetePrefs::prefs();

	d->currentChatStyle = new ChatWindowStyle(kopetePrefs->stylePath(), ChatWindowStyle::StyleBuildFast);
#endif
	//Security settings, we don't need this stuff
	setJScriptEnabled( true ) ;
	setJavaEnabled( false );
	setPluginsEnabled( false );
	setMetaRefreshEnabled( false );
	setOnlyLocalReferences( true );

#ifdef KOPETE_XSLT
	begin();
	write( QString::fromLatin1( "<html><head>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=") +
		encoding() + QString::fromLatin1("\">\n<style>") + styleHTML() +
		QString::fromLatin1("</style></head><body></body></html>") );
	end();
#else
	// Write the template to KHTMLPart
	writeTemplate();
#endif

	view()->setFocusPolicy( QWidget::NoFocus );

	d->tt=new ToolTip( this );

	// It is not possible to drag and drop on our widget
	view()->setAcceptDrops(false);

	// some signals and slots connections
	connect( KopetePrefs::prefs(), SIGNAL(transparencyChanged()),
	         this, SLOT( slotTransparencyChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(messageAppearanceChanged()),
	         this, SLOT( slotAppearanceChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(windowAppearanceChanged()),
	         this, SLOT( slotRefreshView() ) );
#ifndef KOPETE_XSLT
	connect( KopetePrefs::prefs(), SIGNAL(styleChanged(const QString &)),
			 this, SLOT( setStyle(const QString &) ) );
	connect( KopetePrefs::prefs(), SIGNAL(styleVariantChanged(const QString &)),
			 this, SLOT( setStyleVariant(const QString &) ) );
	// Refresh the style if the display name change.
	connect( d->manager, SIGNAL(displayNameChanged()), this, SLOT(changeStyle()) );
#endif
	connect ( browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
	          this, SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );

	connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)),
	         this, SLOT(slotRightClick(const QString &, const QPoint &)) );
	connect( view(), SIGNAL(contentsMoving(int,int)),
	         this, SLOT(slotScrollingTo(int,int)) );

	connect( &d->refreshtimer , SIGNAL(timeout()) , this, SLOT(slotRefreshNodes()));

	//initActions
	d->copyAction = KStdAction::copy( this, SLOT(copy()), actionCollection() );
	d->saveAction = KStdAction::saveAs( this, SLOT(save()), actionCollection() );
	d->printAction = KStdAction::print( this, SLOT(print()),actionCollection() );
	d->closeAction = KStdAction::close( this, SLOT(slotCloseView()),actionCollection() );
	d->copyURLAction = new KAction( i18n( "Copy Link Address" ), QString::fromLatin1( "editcopy" ), 0, this, SLOT( slotCopyURL() ), actionCollection() );

	// read formatting override flags
	readOverrides();

	slotTransparencyChanged();
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

void ChatMessagePart::setStylesheet( const QString &style )
{
	d->xsltParser->setXSLT( style );
	d->transformAllMessages = ( d->xsltParser->flags() & Kopete::XSLT::TransformAllMessages );
	slotRefreshNodes();
}

void ChatMessagePart::setStyle( const QString &stylePath )
{
	// Clear the old style.
	delete d->currentChatStyle;
	d->currentChatStyle = 0L;
	
	// Create a new ChatWindowStyle
	d->currentChatStyle = new ChatWindowStyle(stylePath);

	// Do the actual style switch
	changeStyle();
}

void ChatMessagePart::setStyle( ChatWindowStyle *style )
{
	// Change the current style
	d->currentChatStyle = style;

	// Do the actual style switch
	changeStyle();
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
#ifdef KOPETE_XSLT
	d->xsltParser->setXSLT( KopetePrefs::prefs()->styleContents() );
	slotRefreshNodes();
#else
	changeStyle();
#endif
}

void ChatMessagePart::appendMessage( Kopete::Message &message, bool restoring )
{
	// parse emoticons and URL now.
	message.setBody( message.parsedBody() , Kopete::Message::ParsedHTML );

	message.setBgOverride( d->bgOverride );
	message.setFgOverride( d->fgOverride );
	message.setRtfOverride( d->rtfOverride );
#ifdef STYLE_TIMETEST
	QTime beforeMessage = QTime::currentTime();
#endif
#ifdef KOPETE_XSLT
	d->messageMap.append(  message.asXML().toString() );

	uint bufferLen = (uint)KopetePrefs::prefs()->chatViewBufferSize();

	// transform all messages every time. needed for Adium style.
	if(d->transformAllMessages)
	{
		while ( bufferLen>0 && d->messageMap.count() >= bufferLen )
			d->messageMap.pop_front();

		d->refreshtimer.start(50,true); //let 50ms delay in the case several message are appended in the same time.
	}
	else
	{
		QDomDocument domMessage = message.asXML();
		domMessage.documentElement().setAttribute( QString::fromLatin1( "id" ), QString::number( d->messageId ) );
		QString resultHTML = addNickLinks( d->xsltParser->transform( domMessage.toString() ) );

		QString direction = ( message.plainBody().isRightToLeft() ? QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
		DOM::HTMLElement newNode = document().createElement( QString::fromLatin1("span") );
		newNode.setAttribute( QString::fromLatin1("dir"), direction );
		newNode.setInnerHTML( resultHTML );

		htmlDocument().body().appendChild( newNode );

		while ( bufferLen>0 && d->messageMap.count() >= bufferLen )
		{
			htmlDocument().body().removeChild( htmlDocument().body().firstChild() );
			d->messageMap.pop_front();
		}
#else
		QString formattedMessageHtml;
		bool isConsecutiveMessage = false;

		// Check if it's a consecutive Message
		// Consecutive messages are only for normal messages, status messages do not have a <div id="insert" />
		// We check if the from() is the latestContact, because consecutive incoming/outgoing message can come from differents peopole(in groupchat and IRC)
		// Group only if the user want it.
		if( KopetePrefs::prefs()->groupConsecutiveMessages() )
		{
			isConsecutiveMessage = (message.direction() == d->latestDirection && d->latestContact && d->latestContact == message.from());
		}

		switch(message.direction())
		{
			case Kopete::Message::Inbound:
			{
				// Use status Template for Action messages.
				if(message.type() == Kopete::Message::TypeAction)
				{
					formattedMessageHtml = d->currentChatStyle->getStatusHtml();
				}
				else if(isConsecutiveMessage)
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
				// Use status Template for Action messages.
				if(message.type() == Kopete::Message::TypeAction)
				{
					formattedMessageHtml = d->currentChatStyle->getStatusHtml();
				}
				else if(isConsecutiveMessage)
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
			
			// Find the "Chat" 
			DOM::HTMLElement chatNode = document().getElementById( QString::fromUtf8("Chat") );

			// Append to the chat.
			chatNode.appendChild(newMessageNode);
		}

		// Keep the direction to see on next message
		// if it's a consecutive message
		// Keep also the from() contact.
		d->latestDirection = message.direction();
		d->latestContact = const_cast<Kopete::Contact*>(message.from());

		// Add the message to the list for futher restoring if needed
		if(!restoring)
			d->allMessages.append(message);
#endif
		if ( !d->scrollPressed )
			QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
#ifdef KOPETE_XSLT
	}
#endif
#ifdef STYLE_TIMETEST
	kdDebug(14000) << "Message time: " << beforeMessage.msecsTo( QTime::currentTime()) << endl;
#endif
}

const QString ChatMessagePart::addNickLinks( const QString &html ) const
{
	QString retVal = html;

	Kopete::ContactPtrList members = d->manager->members();
	for ( QPtrListIterator<Kopete::Contact> it( members ); it.current(); ++it )
	{
		QString nick = (*it)->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString();
		//FIXME: this is really slow in channels with lots of contacts
		QString parsed_nick = Kopete::Emoticons::parseEmoticons( nick );

		if ( nick != parsed_nick )
		{
			retVal.replace( QRegExp( QString::fromLatin1("([\\s&;>])%1([\\s&;<:])")
					.arg( QRegExp::escape( parsed_nick ) )  ), QString::fromLatin1("\\1%1\\2").arg( nick ) );
		}
		if ( nick.length() > 0 && ( retVal.find( nick ) > -1 ) && d->manager->protocol() )
		{
			retVal.replace(
				QRegExp( QString::fromLatin1("([\\s&;>])(%1)([\\s&;<:])")
					.arg( QRegExp::escape( nick ) )  ),
			QString::fromLatin1("\\1<a href=\"kopetemessage://%1/?protocolId=%2&accountId=%3\" class=\"KopeteDisplayName\">\\2</a>\\3")
				.arg( (*it)->contactId(), d->manager->protocol()->pluginId(), d->manager->account()->accountId() )
			);
		}
	}
	QString nick = d->manager->myself()->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString();
	retVal.replace( QRegExp( QString::fromLatin1("([\\s&;>])%1([\\s&;<:])")
			.arg( QRegExp::escape( Kopete::Emoticons::parseEmoticons( nick ) ) )  ), QString::fromLatin1("\\1%1\\2").arg( nick ) );

	return retVal;
}

void ChatMessagePart::slotRefreshNodes()
{
	d->refreshtimer.stop();
	DOM::HTMLBodyElement bodyElement = htmlDocument().body();

	QString xmlString = QString::fromLatin1( "<document>" );
	xmlString += d->messageMap.join("\n");
	xmlString += QString::fromLatin1( "</document>" );

	d->xsltParser->transformAsync( xmlString, this, SLOT( slotTransformComplete( const QVariant & ) ) );
}

void ChatMessagePart::slotRefreshView()
{
#ifdef KOPETE_XSLT
	DOM::Element htmlElement = document().documentElement();
	DOM::Element headElement = htmlElement.getElementsByTagName( QString::fromLatin1( "head" ) ).item(0);
	DOM::HTMLElement styleElement = headElement.getElementsByTagName( QString::fromLatin1( "style" ) ).item(0);
	if ( !styleElement.isNull() )
		styleElement.setInnerText( styleHTML() );
#else
	DOM::HTMLElement kopeteNode = document().getElementById( QString::fromUtf8("KopeteStyle") );
	if( !kopeteNode.isNull() )
		kopeteNode.setInnerText( styleHTML() );
#endif
	DOM::HTMLBodyElement bodyElement = htmlDocument().body();
	bodyElement.setBgColor( KopetePrefs::prefs()->bgColor().name() );
}

void ChatMessagePart::slotTransformComplete( const QVariant &result )
{
	htmlDocument().body().setInnerHTML( addNickLinks( result.toString() ) );

	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

void ChatMessagePart::keepScrolledDown()
{
	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

const QString ChatMessagePart::styleHTML() const
{
	KopetePrefs *p = KopetePrefs::prefs();

#ifdef KOPETE_XSLT
	QString style = QString::fromLatin1(
		"body{margin:4px;background-color:%1;font-family:%2;font-size:%3pt;color:%4;background-repeat:no-repeat;background-attachment:fixed}"
		"td{font-family:%5;font-size:%6pt;color:%7}"
		"a{color:%8}a.visited{color:%9}"
		"a.KopeteDisplayName{text-decoration:none;color:inherit;}"
		"a.KopeteDisplayName:hover{text-decoration:underline;color:inherit}"
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
#else
	QString style = QString::fromLatin1(
		"body{background-color:%1;font-family:%2;font-size:%3pt;color:%4}"
		"td{font-family:%5;font-size:%6pt;color:%7}"
		"a{color:%8}a.visited{color:%9}"
		"a.KopeteDisplayName{text-decoration:none;color:inherit;}"
		"a.KopeteDisplayName:hover{text-decoration:underline;color:inherit}"
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
#endif

	//JASON, VA TE FAIRE FOUTRE AVEC TON *default* HIGHLIGHT!
	// that broke my highlight plugin
	// if the user has not Spetialy specified that it you theses 'putaint de' default color, WE DON'T USE THEM
	if ( p->highlightEnabled() )
	{
		style += QString::fromLatin1( ".highlight{color:%1;background-color:%2}" )
			.arg( p->highlightForeground().name() )
			.arg( p->highlightBackground().name() );
	}

	return style;
}

void ChatMessagePart::clear()
{
	DOM::HTMLElement body = htmlDocument().body();
	while ( body.hasChildNodes() )
		body.removeChild( body.childNodes().item( body.childNodes().length() - 1 ) );

	d->messageMap.clear();
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

	htmltext = selectedTextAsHTML();
	text = selectedText();
	//selectedText is now sufficent
//	text=Kopete::Message::unescape( htmltext ).stripWhiteSpace();
	// Message::unsescape will replace image by his title attribute
	// stripWhiteSpace is for removing the newline added by the <!DOCTYPE> and other xml things of RangeImpl::toHTML

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

void ChatMessagePart::slotTransparencyChanged()
{
	d->transparencyEnabled = KopetePrefs::prefs()->transparencyEnabled();

//	kdDebug(14000) << k_funcinfo << "transparencyEnabled=" << transparencyEnabled << ", bgOverride=" << bgOverride << "." << endl;

	if ( d->transparencyEnabled )
	{
		if ( !d->root )
		{
//			kdDebug(14000) << k_funcinfo << "enabling transparency" << endl;
			d->root = new KRootPixmap( view() );
			connect(d->root, SIGNAL( backgroundUpdated( const QPixmap & ) ), this, SLOT( slotUpdateBackground( const QPixmap & ) ) );
			d->root->setCustomPainting( true );
			d->root->setFadeEffect( KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			d->root->start();
		}
		else
		{
			d->root->setFadeEffect( KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			d->root->repaint( true );
		}
	}
	else
	{
		if ( d->root )
		{
//			kdDebug(14000) << k_funcinfo << "disabling transparency" << endl;
			delete d->root;
			d->root = 0;
			if( d->backgroundFile )
			{
				d->backgroundFile->close();
				d->backgroundFile->unlink();
				delete d->backgroundFile;
				d->backgroundFile = 0;
			}
			executeScript( QString::fromLatin1("document.body.background = \"\";") );
		}
	}
}

void ChatMessagePart::slotUpdateBackground( const QPixmap &pixmap )
{
	if( d->backgroundFile )
	{
		d->backgroundFile->close();
		d->backgroundFile->unlink();
		delete d->backgroundFile;
	}

	d->backgroundFile = new KTempFile( QString::null, QString::fromLatin1( ".bmp" ) );
	pixmap.save( d->backgroundFile->name(), "BMP" );

	d->bgChanged = true;

	//This doesn't work well using the DOM, so just use some JS
	if ( d->bgChanged && d->backgroundFile && !d->backgroundFile->name().isNull() )
	{
		setJScriptEnabled( true ) ;
		executeScript( QString::fromLatin1( "document.body.background = \"%1\";" ).arg( d->backgroundFile->name() ) );
		setJScriptEnabled( false ) ;
	}

	d->bgChanged = false;

	if ( !d->scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
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
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML, Kopete::Message &message )
{
	QString resultHTML = sourceHTML;
	QString nick, contactId, service, protocolIcon;
	
	if( message.from() )
	{
		// Use either MetaContact displayname or contact nickname
		// Depending on settings.
		if( KopetePrefs::prefs()->metaContactDisplay() && message.from()->metaContact() )
		{
			nick = message.from()->metaContact()->displayName();
		}
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
	}
	
	// Replace sender (contact nick)
	resultHTML = resultHTML.replace( QString::fromUtf8("%sender%"), nick );
	// Replace time
	resultHTML = resultHTML.replace( QString::fromUtf8("%time%"), KGlobal::locale()->formatDateTime(message.timestamp()) );
	// Replace %screenName% (contact ID)
	resultHTML = resultHTML.replace( QString::fromUtf8("%senderScreenName%"), contactId );
	// Replace service name (protocol name)
	resultHTML = resultHTML.replace( QString::fromUtf8("%service%"), service );
	// Replace protocolIcon (sender statusIcon)
	resultHTML = resultHTML.replace( QString::fromUtf8("%senderStatusIcon%"), protocolIcon );
	
	// Look for %time{X}%
	QRegExp timeRegExp("%time\\{([^}]*)\\}%");
	int pos=0;
	while( (pos=timeRegExp.search(resultHTML , pos) ) != -1 )
	{
		QString timeKeyword = formatTime( timeRegExp.cap(1), message.timestamp() );
		resultHTML = resultHTML.replace( pos , timeRegExp.cap(0).length() , timeKeyword );
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
			photoPath = QString("data:image/png;base64,%1").arg( message.from()->metaContact()->picture().base64() );
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
	// Check if the message is an action, if yes then we will build the "status" message
	if( message.type() == Kopete::Message::TypeAction )
	{
		message.setBody( QString("<b>%1</b> %2").arg(nick).arg(message.parsedBody()), Kopete::Message::ParsedHTML );
	}

	// Replace message at the end, maybe someone could put a Adium keyword in his message :P
	resultHTML = resultHTML.replace( QString::fromUtf8("%message%"), formatMessageBody(message) );

	// TODO: %status, %textbackgroundcolor{X}%
	resultHTML = addNickLinks( resultHTML );
	return resultHTML;
}

// Style formatting for header and footer.
QString ChatMessagePart::formatStyleKeywords( const QString &sourceHTML )
{
	QString resultHTML = sourceHTML;

	QPtrList<Kopete::Contact> membersList =  d->manager->members();
	Kopete::Contact *remoteContact = membersList.first();

	// Verify that all contacts are not null before doing anything
	if( remoteContact && d->manager->myself() )
	{
		QString sourceName, destinationName;
		// Use either MetaContact displayname or contact nickname
		// Depending on settings.
		if( KopetePrefs::prefs()->metaContactDisplay() )
		{
			sourceName = d->manager->myself()->metaContact()->displayName();
			destinationName = remoteContact->metaContact()->displayName();
		}
		else
		{
			sourceName = d->manager->myself()->nickName();
			destinationName = remoteContact->nickName();
		}
		// Replace %chatName%
		resultHTML = resultHTML.replace( QString::fromUtf8("%chatName%"), formatName(d->manager->displayName()) );
		// Replace %sourceName%
		resultHTML = resultHTML.replace( QString::fromUtf8("%sourceName%"), formatName(sourceName) );
		// Replace %destinationName%
		resultHTML = resultHTML.replace( QString::fromUtf8("%destinationName%"), formatName(destinationName) );
		resultHTML = resultHTML.replace( QString::fromUtf8("%timeOpened%"), KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );

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
		if( !remoteContact->metaContact()->picture().isNull() )
		{
			photoIncoming = QString("data:image/png;base64,%1").arg( remoteContact->metaContact()->picture().base64() );
		}
		else
		{
			photoIncoming = QString::fromUtf8("Incoming/buddy_icon.png");
		}
		
		if( !d->manager->myself()->metaContact()->picture().isNull() )
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
	QString formattedBody("<span style=\"");
	
	// Affect foreground(color) and background color to message.
	if( !d->fgOverride && message.fg().isValid() )
	{
		formattedBody += QString::fromUtf8("color: %1; ").arg(message.fg().name());
	}
	if( !d->bgOverride && message.bg().isValid() )
	{
		formattedBody += QString::fromUtf8("background-color: %1; ").arg(message.bg().name());
	}
	
	// Affect font parameters.
	if( !d->rtfOverride && message.font() != QFont() )
	{
		QString fontstr;
		QFont font = message.font();
		if( !font.family().isNull() )
			fontstr += QString::fromUtf8("font-family: ") + font.family() + QString::fromUtf8("; ");
		if( font.italic() )
			fontstr += QString::fromUtf8("font-style: italic; ");
		if( font.strikeOut() )
			fontstr += QString::fromUtf8("text-decoration: line-through; ");
		if( font.underline() )
			fontstr += QString::fromUtf8("text-decoration: underline; ");
		if( font.bold() )
			fontstr += QString::fromUtf8("font-weight: bold;");

		formattedBody += fontstr;
	}
	
	// Close the span tag and affect the parsed body.
	formattedBody += QString::fromUtf8("\">%1</span>").arg(message.parsedBody());
	
	return formattedBody;
}

void ChatMessagePart::changeStyle()
{
#ifdef STYLE_TIMETEST
	QTime beforeChange = QTime::currentTime();
#endif
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
		"<div id=\"Chat\">\n"
		"%3\n"
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

