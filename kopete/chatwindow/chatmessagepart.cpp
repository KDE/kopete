/*
    chatmessagepart.cpp - Chat Message KPart

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

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

#include <qclipboard.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qcursor.h>

#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <dom/dom_element.h>
#include <dom/html_base.h>
#include <dom/html_document.h>
#include <dom/html_inline.h>
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

#include "chatmemberslistwidget.h"
#include "kopetechatwindow.h"
#include "kopetemessagemanager.h"
#include "kopetemetacontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetexsl.h"
#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopeteemoticons.h"

// uncomment this to transform all messages every time, for styles where
// messages aren't processed independently (eg, Adium) to work.
#define TRANSFORM_ALL_MESSAGES

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


class ChatMessagePart::Private
{
public:
	Kopete::XSLT *xsltParser;
	bool transparencyEnabled;
	bool bgOverride;
	bool fgOverride;
	bool rtfOverride;
#ifdef TRANSFORM_ALL_MESSAGES
	/**
	 * we want to render several messages in one pass if several message are apended at the same time.
	 */
	QTimer refreshtimer;
#endif
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



ChatMessagePart::ChatMessagePart( Kopete::ChatSession *mgr, QWidget *parent, const char *name )
	: KHTMLPart( parent, name ), m_manager( mgr ), d( new Private )
{
	d->xsltParser = new Kopete::XSLT( KopetePrefs::prefs()->styleContents() );

	backgroundFile = 0;
	root = 0;
	messageId = 0;
	bgChanged = false;
	scrollPressed = false;

	//Security settings, we don't need this stuff
	setJScriptEnabled( false ) ;
	setJavaEnabled( false );
	setPluginsEnabled( false );
	setMetaRefreshEnabled( false );

	begin();
	write( QString::fromLatin1( "<html><head>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=") +
		encoding() + QString::fromLatin1("\">\n<style>") + styleHTML() +
		QString::fromLatin1("</style></head><body></body></html>") );
	end();

	view()->setFocusPolicy( QWidget::NoFocus );

	new ToolTip( this );

	// It is not possible to drag and drop on our widget
	view()->setAcceptDrops(false);

	// some signals and slots connections
	connect( KopetePrefs::prefs(), SIGNAL(transparencyChanged()),
	         this, SLOT( slotTransparencyChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(messageAppearanceChanged()),
	         this, SLOT( slotAppearanceChanged() ) );
	connect( KopetePrefs::prefs(), SIGNAL(windowAppearanceChanged()),
	         this, SLOT( slotRefreshView() ) );

	connect ( browserExtension(), SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
	          this, SLOT( slotOpenURLRequest( const KURL &, const KParts::URLArgs & ) ) );

	connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)),
	         this, SLOT(slotRightClick(const QString &, const QPoint &)) );
	connect( view(), SIGNAL(contentsMoving(int,int)),
	         this, SLOT(slotScrollingTo(int,int)) );

#ifdef TRANSFORM_ALL_MESSAGES
	connect( &d->refreshtimer , SIGNAL(timeout()) , this, SLOT(slotRefreshNodes()));
#endif


	//initActions
	copyAction = KStdAction::copy( this, SLOT(copy()), actionCollection() );
	saveAction = KStdAction::saveAs( this, SLOT(save()), actionCollection() );
	printAction = KStdAction::print( this, SLOT(print()),actionCollection() );
	closeAction = KStdAction::close( this, SLOT(slotCloseView()),actionCollection() );
	copyURLAction = new KAction( i18n( "Copy Link Address" ), QString::fromLatin1( "editcopy" ), 0, this, SLOT( slotCopyURL() ), actionCollection() );

	// read formatting override flags
	readOverrides();

	slotTransparencyChanged();
	}

ChatMessagePart::~ChatMessagePart()
{
	if( backgroundFile )
	{
		backgroundFile->close();
		backgroundFile->unlink();
		delete backgroundFile;
	}

	delete d;
}

void ChatMessagePart::slotScrollingTo( int /*x*/, int y )
{
	int scrolledTo = y + view()->visibleHeight();
	if ( scrolledTo >= ( view()->contentsHeight() - 10 ) )
		scrollPressed = false;
	else
		scrollPressed = true;
}

void ChatMessagePart::save()
{
	KFileDialog dlg( QString::null, QString::fromLatin1( "text/html text/xml text/plain" ), view(), "fileSaveDialog", false );
	dlg.setCaption( i18n( "Save Conversation" ) );
	dlg.setOperationMode( KFileDialog::Saving );

	if ( dlg.exec() != QDialog::Accepted )
		return;

	QString fileName = dlg.selectedFile();
	QFile file( fileName );

	if( file.open( IO_WriteOnly ) )
	{
		QTextStream stream ( &file );
		if ( dlg.currentFilter() == QString::fromLatin1( "text/xml" ) )
		{
			stream << QString::fromLatin1( "<document>" );
			for ( MessageMap::Iterator it = messageMap.begin(); it != messageMap.end(); ++it )
				stream << (*it).asXML().toString();
			stream << QString::fromLatin1( "</document>\n" );
		}
		else if ( dlg.currentFilter() == QString::fromLatin1( "text/plain" ) )
		{
			for( MessageMap::Iterator it = messageMap.begin(); it != messageMap.end(); ++it)
			{
				stream << "[" << KGlobal::locale()->formatDateTime( (*it).timestamp(), true, true );
				stream << "] " << (*it).plainBody() << '\n';
			}
		}
		else
		{
			stream << htmlDocument().toHTML() << '\n';
		}

		file.close(); // maybe unneeded but I like to close opened files ;)
	}
	else
	{
		KMessageBox::queuedMessageBox( view(), KMessageBox::Error,
				i18n("<qt>Could not open <b>%1</b> for writing.</qt>").arg(fileName), // Message
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
		Kopete::Contact *contact = m_manager->account()->contacts()[ url.host() ];
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
	slotRefreshNodes();
}

void ChatMessagePart::slotAppearanceChanged()
{
	readOverrides();

	d->xsltParser->setXSLT( KopetePrefs::prefs()->styleContents() );
	slotRefreshNodes();
}

void ChatMessagePart::appendMessage( Kopete::Message &message )
{
	//parse emoticons and URL now.
	message.setBody( message.parsedBody() , Kopete::Message::ParsedHTML );

	messageMap.insert( ++messageId, message );

	// transform all messages every time. needed for Adium style.
#ifdef TRANSFORM_ALL_MESSAGES
	d->refreshtimer.start(50,true); //let 50ms delay in the case several message are appended in the same time.
	return;
#else

	uint bufferLen = (uint)KopetePrefs::prefs()->chatViewBufferSize();

	message.setBgOverride( d->bgOverride );
	message.setFgOverride( d->fgOverride );
	message.setRtfOverride( d->rtfOverride );

	QDomDocument domMessage = message.asXML();
	domMessage.documentElement().setAttribute( QString::fromLatin1( "id" ), QString::number( messageId ) );
	QString resultHTML = addNickLinks( d->xsltParser->transform( domMessage.toString() ) );

	QString direction = ( QApplication::reverseLayout() ? QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
	DOM::HTMLElement newNode = document().createElement( QString::fromLatin1("span") );
	newNode.setAttribute( QString::fromLatin1("dir"), direction );
	newNode.setInnerHTML( resultHTML );

	htmlDocument().body().appendChild( newNode );

	if ( messageMap.count() >= bufferLen )
	{
		htmlDocument().body().removeChild( htmlDocument().body().firstChild() );
		messageMap.remove( messageMap.begin() );
	}

	if ( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
#endif
}

const QString ChatMessagePart::addNickLinks( const QString &html ) const
{
	QString retVal = html;

	Kopete::ContactPtrList members = m_manager->members();
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
		if ( nick.length() > 0 && ( retVal.find( nick ) > -1 ) )
		{
			retVal.replace(
				QRegExp( QString::fromLatin1("([\\s&;>])(%1)([\\s&;<:])")
					.arg( QRegExp::escape( nick ) )  ),
				QString::fromLatin1("\\1<a href=\"kopetemessage://%1\" class=\"KopeteDisplayName\">\\2</a>\\3")
				.arg( (*it)->contactId() )
			);
		}
	}
	QString nick = m_manager->myself()->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString();
	retVal.replace( QRegExp( QString::fromLatin1("([\\s&;>])%1([\\s&;<:])")
			.arg( QRegExp::escape( Kopete::Emoticons::parseEmoticons( nick ) ) )  ), QString::fromLatin1("\\1%1\\2").arg( nick ) );

	return retVal;
}

void ChatMessagePart::slotRefreshNodes()
{
#ifdef TRANSFORM_ALL_MESSAGES
	d->refreshtimer.stop();
#endif
	DOM::HTMLBodyElement bodyElement = htmlDocument().body();

	QString xmlString = QString::fromLatin1( "<document>" );
	for( MessageMap::Iterator it = messageMap.begin(); it != messageMap.end(); ++it)
	{
		(*it).setBgOverride( d->bgOverride );
		(*it).setFgOverride( d->fgOverride );
		(*it).setRtfOverride( d->rtfOverride );

		QDomDocument message = (*it).asXML();
		message.documentElement().setAttribute( QString::fromLatin1( "id" ), QString::number( it.key() ) );
		xmlString += message.toString();
	}
	xmlString += QString::fromLatin1( "</document>" );

	d->xsltParser->transformAsync( xmlString, this, SLOT( slotTransformComplete( const QVariant & ) ) );
}

void ChatMessagePart::slotRefreshView()
{
	DOM::Element htmlElement = document().documentElement();
	DOM::Element headElement = htmlElement.getElementsByTagName( QString::fromLatin1( "head" ) ).item(0);
	DOM::HTMLElement styleElement = headElement.getElementsByTagName( QString::fromLatin1( "style" ) ).item(0);
	if ( !styleElement.isNull() )
		styleElement.setInnerText( styleHTML() );

	DOM::HTMLBodyElement bodyElement = htmlDocument().body();
	bodyElement.setBgColor( KopetePrefs::prefs()->bgColor().name() );
}

void ChatMessagePart::slotTransformComplete( const QVariant &result )
{
	htmlDocument().body().setInnerHTML( addNickLinks( result.toString() ) );

	if ( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

void ChatMessagePart::keepScrolledDown()
{
	if ( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

const QString ChatMessagePart::styleHTML() const
{
	KopetePrefs *p = KopetePrefs::prefs();

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

	messageMap.clear();
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
		for ( QPtrListIterator<Kopete::Contact> it ( m_manager->members() ); it.current(); ++it )
			if ( (*it)->contactId() == contactId )
				return *it;
	}
	else
	{
		QString nick = element.innerText().string().stripWhiteSpace();
		for ( QPtrListIterator<Kopete::Contact> it ( m_manager->members() ); it.current(); ++it )
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
	activeElement = activeNode;
	if ( activeElement.isNull() )
		return;

	KPopupMenu *chatWindowPopup = 0L;

	if ( Kopete::Contact *contact = contactFromNode( activeElement ) )
	{
		chatWindowPopup = contact->popupMenu( m_manager );
		connect( chatWindowPopup, SIGNAL( aboutToHide() ), chatWindowPopup , SLOT( deleteLater() ) );
	}
	else
	{
		chatWindowPopup = new KPopupMenu();

		if ( activeElement.className() == "KopeteDisplayName" )
		{
			chatWindowPopup->insertItem( i18n( "User Has Left" ), 1 );
			chatWindowPopup->setItemEnabled( 1, false );
			chatWindowPopup->insertSeparator();
		}
		else if ( activeElement.tagName().lower() == QString::fromLatin1( "a" ) )
		{
			copyURLAction->plug( chatWindowPopup );
			chatWindowPopup->insertSeparator();
		}

		copyAction->setEnabled( hasSelection() );
		copyAction->plug( chatWindowPopup );
		saveAction->plug( chatWindowPopup );
		printAction->plug( chatWindowPopup );
		chatWindowPopup->insertSeparator();
		closeAction->plug( chatWindowPopup );

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
	DOM::HTMLAnchorElement a = activeElement;
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
	text=Kopete::Message::unescape( htmltext ).stripWhiteSpace();
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
	if(justselection)
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
    	QApplication::clipboard()->setText( text, QClipboard::Selection );		
    } else
	{
    	QApplication::clipboard()->setText( text, QClipboard::Selection );		
	}

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
		if ( !root )
		{
//			kdDebug(14000) << k_funcinfo << "enabling transparency" << endl;
			root = new KRootPixmap( view() );
			connect(root, SIGNAL( backgroundUpdated( const QPixmap & ) ), this, SLOT( slotUpdateBackground( const QPixmap & ) ) );
			root->setCustomPainting( true );
			root->setFadeEffect( KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->start();
		}
		else
		{
			root->setFadeEffect( KopetePrefs::prefs()->transparencyValue() * 0.01, KopetePrefs::prefs()->transparencyColor() );
			root->repaint( true );
		}
	}
	else
	{
		if ( root )
		{
//			kdDebug(14000) << k_funcinfo << "disabling transparency" << endl;
			delete root;
			root = 0;
			if( backgroundFile )
			{
				backgroundFile->close();
				backgroundFile->unlink();
				delete backgroundFile;
				backgroundFile = 0;
			}
			executeScript( QString::fromLatin1("document.body.background = \"\";") );
		}
	}
}

void ChatMessagePart::slotUpdateBackground( const QPixmap &pixmap )
{
	if( backgroundFile )
	{
		backgroundFile->close();
		backgroundFile->unlink();
		delete backgroundFile;
	}

	backgroundFile = new KTempFile( QString::null, QString::fromLatin1( ".bmp" ) );
	pixmap.save( backgroundFile->name(), "BMP" );

	bgChanged = true;

	//This doesn't work well using the DOM, so just use some JS
	if ( bgChanged && backgroundFile && !backgroundFile->name().isNull() )
	{
		setJScriptEnabled( true ) ;
		executeScript( QString::fromLatin1( "document.body.background = \"%1\";" ).arg( backgroundFile->name() ) );
		setJScriptEnabled( false ) ;
	}

	bgChanged = false;

	if ( !scrollPressed )
		QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
}

void ChatMessagePart::khtmlDrawContentsEvent( khtml::DrawContentsEvent * event) //virtual
{
	KHTMLPart::khtmlDrawContentsEvent(event);
	copy(true /*selection only*/);
}
void ChatMessagePart::slotCloseView( bool force )
{
	m_manager->view()->closeView( force );
}

void ChatMessagePart::emitTooltipEvent(  const QString &textUnderMouse, QString &toolTip )
{
	emit tooltipEvent(  textUnderMouse, toolTip );
}

#include "chatmessagepart.moc"

// vim: set noet ts=4 sts=4 sw=4:

