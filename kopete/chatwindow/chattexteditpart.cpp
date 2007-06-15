/*
    chattexteditpart.cpp - Chat Text Edit Part

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

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

#include "chattexteditpart.h"

#include "kopetecontact.h"
#include "kopetechatsession.h"
#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"
#include "kopeteglobal.h"
#include <kopeteappearancesettings.h>

#include <kcompletion.h>
#include <kdebug.h>
#include <ktextedit.h>
//#include <ksyntaxhighlighter.h>

#include <QtCore/QTimer>
#include <QtCore/QRegExp>

ChatTextEditPart::ChatTextEditPart( Kopete::ChatSession *session, QWidget *parent )
	: KRichTextEditPart(parent, 0, QStringList()), m_session(session)
{
	// Set rich support in the part
	setProtocolRichTextSupport();

	m_autoSpellCheckEnabled = true;
	historyPos = -1;
	
	mComplete = new KCompletion();
	mComplete->setIgnoreCase( true );
	mComplete->setOrder( KCompletion::Weighted );
	
	// set params on the edit widget
	textEdit()->setMinimumSize( QSize( 75, 20 ) );
//	textEdit()->setWordWrap( Q3TextEdit::WidgetWidth );
//	textEdit()->setWrapPolicy( Q3TextEdit::AtWhiteSpace );
//	textEdit()->setAutoFormatting( Q3TextEdit::AutoNone );

	// some signals and slots connections
	connect( textEdit(), SIGNAL( textChanged()), this, SLOT( slotTextChanged() ) );

	// timers for typing notifications
	m_typingRepeatTimer = new QTimer(this);
	m_typingRepeatTimer->setObjectName("m_typingRepeatTimer");
	m_typingStopTimer   = new QTimer(this);
	m_typingStopTimer->setObjectName("m_typingStopTimer");

	connect( m_typingRepeatTimer, SIGNAL( timeout() ), this, SLOT( slotRepeatTypingTimer() ) );
	connect( m_typingStopTimer,   SIGNAL( timeout() ), this, SLOT( slotStoppedTypingTimer() ) );

	connect( session, SIGNAL( contactAdded(const Kopete::Contact*, bool) ),
	         this, SLOT( slotContactAdded(const Kopete::Contact*) ) );
	connect( session, SIGNAL( contactRemoved(const Kopete::Contact*, const QString&, Qt::TextFormat, bool) ),
	         this, SLOT( slotContactRemoved(const Kopete::Contact*) ) );
	connect( session, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & , const Kopete::OnlineStatus &) ),
	         this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
	
	setFont( Kopete::AppearanceSettings::self()->chatFont() );

	slotContactAdded( session->myself() );

	foreach( Kopete::Contact *contact, session->members() )
		slotContactAdded( contact );
}

ChatTextEditPart::~ChatTextEditPart()
{
	delete mComplete;
}

void ChatTextEditPart::toggleAutoSpellCheck( bool enabled )
{
	if ( useRichText() )
		enabled = false;

	m_autoSpellCheckEnabled = enabled;
#ifdef __GNUC__
#warning Port to new SpellHightlighter interface, disabled to make compile (-DarkShock)
#endif
#if 0
	if ( spellHighlighter() )
	{
		spellHighlighter()->setAutomatic( enabled );
		spellHighlighter()->setActive( enabled );
	}
	textEdit()->setCheckSpellingEnabled( enabled );
#endif
}

bool ChatTextEditPart::autoSpellCheckEnabled() const
{
	return m_autoSpellCheckEnabled;
}

KDictSpellingHighlighter* ChatTextEditPart::spellHighlighter()
{
#ifdef __GNUC__
#warning disabled to make it compile
#endif
#if 0
	Q3SyntaxHighlighter *qsh = textEdit()->syntaxHighlighter();
	KDictSpellingHighlighter* kdsh = dynamic_cast<KDictSpellingHighlighter*>( qsh );
	return kdsh;
#else 
	return 0l;
#endif
}

// NAUGHTY, BAD AND WRONG! (but needed to fix nick complete bugs)
/*
#include <private/qrichtext_p.h>
class EvilTextEdit : public KTextEdit
{
public:
	// grab the paragraph as plain text - very very evil.
	QString plainText( int para )
	{
		QString str = document()->paragAt( para )->string()->toString();
		// str includes an extra space on the end (from the newline character?) - remove it
		return str.left( str.length() - 1 );
	}
};
*/
void ChatTextEditPart::complete()
{
#ifdef __GNUC__
#warning disabled to make it compile
#endif
#if 0
	int para = 1, parIdx = 1;
	textEdit()->getCursorPosition( &para, &parIdx);

	// FIXME: strips out all formatting
//QString txt = static_cast<EvilTextEdit*>(textEdit())->plainText( para );
	QString txt = textEdit()->text(para);

	if ( parIdx > 0 )
	{
		int firstSpace = txt.lastIndexOf( QRegExp( QLatin1String("\\s\\S+") ), parIdx - 1 ) + 1;
		int lastSpace = txt.find( QRegExp( QLatin1String("[\\s\\:]") ), firstSpace );
		if( lastSpace == -1 )
			lastSpace = txt.length();

		QString word = txt.mid( firstSpace, lastSpace - firstSpace );
		QString match;

		kDebug(14000) << k_funcinfo << word << " from '" << txt << "'" << endl;

		if ( word != m_lastMatch )
		{
			match = mComplete->makeCompletion( word );
			m_lastMatch.clear();
			parIdx -= word.length();
		}
		else
		{
			match = mComplete->nextMatch();
			parIdx -= m_lastMatch.length();
		}

		if ( !match.isNull() && !match.isEmpty() )
		{
			QString rightText = txt.right( txt.length() - lastSpace );

			if ( para == 0 && firstSpace == 0 && rightText[0] != QChar(':') )
			{
				rightText = match + QLatin1String(": ") + rightText;
				parIdx += 2;
			}
			else
				rightText = match + rightText;

			// insert *before* remove. this is becase Qt adds an extra blank line
			// if the rich text control becomes empty (if you remove the only para).
			// disable updates while we change the contents to eliminate flicker.
			textEdit()->setUpdatesEnabled( false );
			textEdit()->insertParagraph( txt.left(firstSpace) + rightText, para );
			textEdit()->removeParagraph( para + 1 );
			textEdit()->setCursorPosition( para, parIdx + match.length() );
			textEdit()->setUpdatesEnabled( true );
			// must call this rather than update because QTextEdit is broken :(
			textEdit()->updateContents();
			m_lastMatch = match;
		}
		else
		{
			kDebug(14000) << k_funcinfo << "No completions! Tried " << mComplete->items() << endl;
		}
	}
#endif
}

void ChatTextEditPart::slotPropertyChanged( Kopete::Contact*, const QString &key,
		const QVariant& oldValue, const QVariant &newValue  )
{
	if ( key == Kopete::Global::Properties::self()->nickName().key() )
	{
		mComplete->removeItem( oldValue.toString() );
		mComplete->addItem( newValue.toString() );
	}
}

void ChatTextEditPart::slotContactAdded( const Kopete::Contact *contact )
{
	connect( contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
	         this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;
	
	QString contactName = contact->getProperty(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->addItem( contactName );
}

void ChatTextEditPart::slotContactRemoved( const Kopete::Contact *contact )
{
	disconnect( contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
	            this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;
	
	QString contactName = contact->getProperty(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->removeItem( contactName );
}

bool ChatTextEditPart::canSend()
{
	int i;
	
	if ( !m_session ) return false;

	// can't send if there's nothing *to* send...
	if ( text().isEmpty() )
		return false;

	Kopete::ContactPtrList members = m_session->members();
	
	// if we can't send offline, make sure we have a reachable contact...
	if ( !( m_session->protocol()->capabilities() & Kopete::Protocol::CanSendOffline ) )
	{
		bool reachableContactFound = false;

		//TODO: does this perform badly in large / busy IRC channels? - no, doesn't seem to
		for( i = 0; i != members.size(); i++ )
		{
			if ( members[i]->isReachable() )
			{
				reachableContactFound = true;
				break;
			}
		}

		// no online contact found and can't send offline? can't send.
		if ( !reachableContactFound )
			return false;
	}

	return true;
}

void ChatTextEditPart::slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &newStatus, const Kopete::OnlineStatus &oldStatus )
{
	//FIXME: should use signal contact->isReachableChanged, but it doesn't exist ;(
	if ( ( oldStatus.status() == Kopete::OnlineStatus::Offline )
	  != ( newStatus.status() == Kopete::OnlineStatus::Offline ) )
	{
		emit canSendChanged( canSend() );
	}
}

void ChatTextEditPart::sendMessage()
{
	QString txt = this->text( Qt::PlainText );
	// avoid sending emtpy messages or enter keys (see bug 100334)
	if ( txt.isEmpty() || txt == "\n" )
		return;

	if ( m_lastMatch.isNull() && ( txt.indexOf( QRegExp( QLatin1String("^\\w+:\\s") ) ) > -1 ) )
	{ //no last match and it finds something of the form of "word:" at the start of a line
		QString search = txt.left( txt.indexOf(':') );
		if( !search.isEmpty() )
		{
			QString match = mComplete->makeCompletion( search );
			if( !match.isNull() )
				textEdit()->setText( txt.replace(0,search.length(),match) );
		}
	}

	if ( !m_lastMatch.isNull() )
	{
		//FIXME: what is the next line for?
		mComplete->addItem( m_lastMatch );
		m_lastMatch.clear();
	}

	slotStoppedTypingTimer();
	Kopete::Message sentMessage = contents();
	emit messageSent( sentMessage );
	historyList.prepend( this->text( Qt::PlainText) );
	historyPos = -1;
	clear();
	emit canSendChanged( false );
}

bool ChatTextEditPart::isTyping()
{
	QString txt = text( Qt::PlainText );

	//Make sure the message is empty. QString::isEmpty()
	//returns false if a message contains just whitespace
	//which is the reason why we strip the whitespace	
	return !txt.trimmed().isEmpty();
}

void ChatTextEditPart::slotTextChanged()
{
	if ( isTyping() )
	{
		// And they were previously typing
		if( !m_typingRepeatTimer->isActive() )
		{
			m_typingRepeatTimer->setSingleShot( false );
			m_typingRepeatTimer->start( 4000 );
			slotRepeatTypingTimer();
		}

		// Reset the stop timer again, regardless of status
		m_typingStopTimer->setSingleShot( true );
		m_typingStopTimer->start( 4500 );
	}

	emit canSendChanged( canSend() );
}

void ChatTextEditPart::historyUp()
{
	if ( historyList.empty() || historyPos == historyList.count() - 1 )
		return;
	
	QString text = this->text(Qt::PlainText);
	bool empty = text.trimmed().isEmpty();
	
	// got text? save it
	if ( !empty )
	{
		if ( historyPos == -1 )
		{
			historyList.prepend( text );
			historyPos = 0;
		}
		else
		{
			historyList[historyPos] = text;
		}
	}
	
	historyPos++;
	
	QString newText = historyList[historyPos];
// 	TextFormat format=textEdit()->textFormat();
// 	textEdit()->setTextFormat(AutoText); //workaround bug 115690
	textEdit()->setText( newText );
// 	textEdit()->setTextFormat(format);
	// TODO: Port to Qt4
	textEdit()->moveCursor( QTextEdit::MoveEnd );
}

void ChatTextEditPart::historyDown()
{
	if ( historyList.empty() || historyPos == -1 )
		return;
	
	QString text = this->text(Qt::PlainText);
	bool empty = text.trimmed().isEmpty();
	
	// got text? save it
	if ( !empty )
	{
		historyList[historyPos] = text;
	}
	
	historyPos--;
	
	QString newText = ( historyPos >= 0 ? historyList[historyPos] : QString::null );
	
	
// 	TextFormat format=textEdit()->textFormat();
// 	textEdit()->setTextFormat(AutoText); //workaround bug 115690
	textEdit()->setText( newText );
// 	textEdit()->setTextFormat(format);
	// TODO: Port to Qt4
	textEdit()->moveCursor( QTextEdit::MoveEnd, false );
}

void ChatTextEditPart::addText( const QString &text )
{
	if( Qt::mightBeRichText(text) )
	{
		textEdit()->insertHtml( text );
	}
	else
	{
		textEdit()->insertPlainText( text );
	}
}

void ChatTextEditPart::setContents( const Kopete::Message &message )
{
	textEdit()->setText( useRichText() ? message.escapedBody() : message.plainBody() );

	setFont( message.font() );
	setTextColor( message.foregroundColor() );
// 	setBackgroundColorColor( message.backgroundColor() );
}

Kopete::Message ChatTextEditPart::contents()
{
	Kopete::Message currentMsg( m_session->myself(), m_session->members() );
	currentMsg.setDirection( Kopete::Message::Outbound );
	useRichText() ? currentMsg.setHtmlBody( text() ) : currentMsg.setPlainBody( text() );
	
// 	currentMsg.setBackgroundColor( bgColor() );
	currentMsg.setForegroundColor( textColor() );
	currentMsg.setFont( font() );
	
	return currentMsg;
}

void ChatTextEditPart::slotRepeatTypingTimer()
{
	emit typing( true );
}

void ChatTextEditPart::slotStoppedTypingTimer()
{
	m_typingRepeatTimer->stop();
	m_typingStopTimer->stop();
	emit typing( false );
}

void ChatTextEditPart::setProtocolRichTextSupport()
{
	KRichTextEditPart::RichTextSupport richText;
	Kopete::Protocol::Capabilities protocolCaps = m_session->protocol()->capabilities();

	// Check for bold
	if( (protocolCaps & Kopete::Protocol::BaseBFormatting) || (protocolCaps & Kopete::Protocol::RichBFormatting) )
	{
		richText |= KRichTextEditPart::SupportBold;
	}
	// Check for italic
	if( (protocolCaps & Kopete::Protocol::BaseIFormatting) || (protocolCaps & Kopete::Protocol::RichIFormatting) )
	{
		richText |= KRichTextEditPart::SupportItalic;
	}
	// Check for underline
	if( (protocolCaps & Kopete::Protocol::BaseUFormatting) || (protocolCaps & Kopete::Protocol::RichUFormatting) )
	{
		richText |= KRichTextEditPart::SupportUnderline;
	}
	// Check for font support
	if( (protocolCaps & Kopete::Protocol::BaseFont) || (protocolCaps & Kopete::Protocol::RichFont) )
	{
		richText |= KRichTextEditPart::SupportFont;
	}
	// Check for text color support
	if( (protocolCaps & Kopete::Protocol::BaseFgColor) || (protocolCaps & Kopete::Protocol::RichFgColor) )
	{
		richText |= KRichTextEditPart::SupportTextColor;
	}
	// Check for alignment
	if( protocolCaps & Kopete::Protocol::Alignment )
	{
		richText |= KRichTextEditPart::SupportAlignment;
	}

	// Set rich text support in KRichTextEditPart
	setRichTextSupport( richText );
}

#include "chattexteditpart.moc"

// vim: set noet ts=4 sts=4 sw=4:
