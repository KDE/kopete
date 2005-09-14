/*
    chatmessagepart.cpp - Chat Message KPart

    Copyright (c) 2002-2005 by Olivier Goffart       <ogoffart @ kde.org>
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
#include <qrect.h>
#include <qcursor.h>

//Added by qt3to4:
#include <QPixmap>
#include <QTextStream>
#include <QByteArray>
#include <qtextcodec.h> 
#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <dom/dom_string.h>
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
#include <kio/netaccess.h>

#include "chatmemberslistwidget.h"
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




class ChatMessagePart::Private
{
public:
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
//	ToolTip *tt;
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

ChatMessagePart::ChatMessagePart( Kopete::ChatSession *mgr, QWidget *parent, const char *name )
	: KHTMLPart( parent, name ), m_manager( mgr ), d( new Private )
{
	d->xsltParser = new Kopete::XSLT( KopetePrefs::prefs()->styleContents() );
	d->transformAllMessages = ( d->xsltParser->flags() & Kopete::XSLT::TransformAllMessages );
        codepage="Unicode";
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
	setOnlyLocalReferences( true );

	begin();
        write( QString::fromLatin1( "<html><head>\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=") +
			encoding() + QString::fromLatin1("\">\n<style>") + styleHTML() +
	QString::fromLatin1("</style></head><body></body></html>") );
	end();
//	d->tt=new ToolTip( this );

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

	connect( &d->refreshtimer , SIGNAL(timeout()) , this, SLOT(slotRefreshNodes()));

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

//	delete d->tt;
	delete d->xsltParser;
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

	KURL saveURL = dlg.selectedURL();
	KTempFile tempFile;
	tempFile.setAutoDelete( true );
	QFile* file = tempFile.file();

	QTextStream stream ( file );
	if ( dlg.currentFilter() == QString::fromLatin1( "text/xml" ) )
	{
		stream << QString::fromLatin1( "<document>" );
		stream << messageMap.join("\n");
		stream << QString::fromLatin1( "</document>\n" );
	}
	else if ( dlg.currentFilter() == QString::fromLatin1( "text/plain" ) )
	{
		for( QStringList::Iterator it = messageMap.begin(); it != messageMap.end(); ++it)
		{
			QDomDocument doc;
			doc.setContent(*it);
			stream << "[" << doc.elementsByTagName("message").item(0).toElement().attribute("formattedTimestamp");
			stream << "] " << doc.elementsByTagName("contact").item(0).toElement().attribute("contactId") ;
			stream << ": " << doc.elementsByTagName("body").item(0).toElement().text() << "\n";
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
	d->transformAllMessages = ( d->xsltParser->flags() & Kopete::XSLT::TransformAllMessages );
	slotRefreshNodes();
}

void ChatMessagePart::slotAppearanceChanged()
{
	readOverrides();

	d->xsltParser->setXSLT( KopetePrefs::prefs()->styleContents() );
	slotRefreshNodes();
}

void ChatMessagePart::appendMessage( Kopete::Message &message,bool encode)
{
	//parse emoticons and URL now.
	message.setBody( message.parsedBody() , Kopete::Message::ParsedHTML );

	message.setBgOverride( d->bgOverride );
	message.setFgOverride( d->fgOverride );
	message.setRtfOverride( d->rtfOverride );

	messageMap.append(  message.asXML().toString() );
	uint bufferLen = (uint)KopetePrefs::prefs()->chatViewBufferSize();

	// transform all messages every time. needed for Adium style.
	if(d->transformAllMessages)
	{
		while ( bufferLen>0 && messageMap.count() >= bufferLen )
			messageMap.pop_front();

		d->refreshtimer.start(50,true); //let 50ms delay in the case several message are appended in the same time.
	}
	else
	{
		QDomDocument domMessage = message.asXML();
		domMessage.documentElement().setAttribute( QString::fromLatin1( "id" ), QString::number( messageId ) );
		QString resultHTML = addNickLinks( d->xsltParser->transform( domMessage.toString() ) );
		if(encode && (codepage != "Unicode"))
		{
			QByteArray locallyEncoded = resultHTML.ascii();
			QTextCodec *codec = QTextCodec::codecForName(codepage.ascii());
			if(codec)
			   resultHTML = codec->toUnicode( locallyEncoded );
		}
		QString direction = ( message.plainBody().isRightToLeft() ? QString::fromLatin1("rtl") : QString::fromLatin1("ltr") );
		DOM::HTMLElement newNode = document().createElement( QString::fromLatin1("span") );
		newNode.setAttribute( QString::fromLatin1("dir"), direction );
		newNode.setInnerHTML( resultHTML );

		htmlDocument().body().appendChild( newNode );

		while ( bufferLen>0 && messageMap.count() >= bufferLen )
		{
			htmlDocument().body().removeChild( htmlDocument().body().firstChild() );
			messageMap.pop_front();
		}

		if ( !scrollPressed )
			QTimer::singleShot( 1, this, SLOT( slotScrollView() ) );
	}
}

const QString ChatMessagePart::addNickLinks( const QString &html ) const
{
	QString retVal = html;
	unsigned int i;
	
	Kopete::ContactPtrList members = m_manager->members();
	
	Kopete::Contact* ct;
	for ( i = 0; i != members.size(); i++ )
	{
		ct = members[i];
		QString nick = ct->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString();
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
			QString::fromLatin1("\\1<a href=\"kopetemessage://%1/?protocolId=%2&accountId=%3\" class=\"KopeteDisplayName\">\\2</a>\\3")
				.arg( ct->contactId(), m_manager->protocol()->pluginId(), m_manager->account()->accountId() )
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
	d->refreshtimer.stop();
	DOM::HTMLBodyElement bodyElement = htmlDocument().body();

	QString xmlString = QString::fromLatin1( "<document>" );
	xmlString += messageMap.join("\n");
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
        unsigned int i;
	QList<Kopete::Contact*> m;					       

	if ( node.isNull() )
		return 0;

	while ( !node.isNull() && ( node.nodeType() == DOM::Node::TEXT_NODE || ((DOM::HTMLElement)node).className() != "KopeteDisplayName" ) )
		node = node.parentNode();

	DOM::HTMLElement element = node;
	if ( element.className() != "KopeteDisplayName" )
		return 0;

	m = m_manager->members();
	if ( element.hasAttribute( "contactid" ) )
	{
		QString contactId = element.getAttribute( "contactid" ).string();
		for ( i =0; i != m.size(); i++ )
			if ( m[i]->contactId() == contactId )
				return m[i];
	}
	else
	{
		QString nick = element.innerText().string().stripWhiteSpace();
		for ( i = 0; i != m.size(); i++)
			if ( m[i]->property( Kopete::Global::Properties::self()->nickName().key() ).value().toString() == nick )
				return m[i];
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
      	Q3TextDrag *textdrag = new Q3TextDrag(text, 0L);
	    KMultipleDrag *drag = new KMultipleDrag( );
    	drag->addDragObject( textdrag );
    	if(!htmltext.isEmpty()) {
	    	htmltext.replace( QChar( 0xa0 ), ' ' );
    		Q3TextDrag *htmltextdrag = new Q3TextDrag(htmltext, 0L);
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
	//copy(true /*selection only*/); not needed anymore.
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

void ChatMessagePart::slotConvert(const QString& string)
{

    
    DOM::DOMString text=htmlDocument().body().innerHTML();
    if(codepage!="Unicode")
    {
	QByteArray locallyEncoded;
	QTextCodec *codec = QTextCodec::codecForName(codepage.ascii());
	if(!codec)
	{
		kdDebug()<<"codec for this name not exisit\n";
		return;	
	}
	locallyEncoded=codec->fromUnicode(text.string());
	if(string!="Unicode")
	{
		QTextCodec *codec = QTextCodec::codecForName(string.ascii());
		if(!codec)
		{
			kdDebug()<<"codec for this name not exisit\n";
			return;	
		}
		text=codec->toUnicode(locallyEncoded);
	}
	else 
	    text=QString(locallyEncoded);
    }
    else
    {
	if(string!="Unicode")
	{
		QTextCodec *codec = QTextCodec::codecForName(string.ascii());
		if(!codec)
		{
			kdDebug()<<"codec for this name not exisit\n";
			return;	
		}
		text=codec->toUnicode(text.string().ascii());
	}
    }
    htmlDocument().body().setInnerHTML(text);
    setCodepage(string);    
}

void ChatMessagePart::setCodepage(const QString& page)
{
    codepage=page;
}

