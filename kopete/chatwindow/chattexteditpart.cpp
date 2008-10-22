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

#include <QtCore/QTimer>
#include <QtCore/QRegExp>

ChatTextEditPart::ChatTextEditPart( Kopete::ChatSession *session, QWidget *parent )
	: KRichTextEditPart(parent, 0, QStringList()), m_session(session)
{
	// Set rich support in the part
	setProtocolRichTextSupport();

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

	connect( Kopete::AppearanceSettings::self(), SIGNAL( appearanceChanged() ),
	         this, SLOT( slotAppearanceChanged() ) );

	connect( KGlobalSettings::self(), SIGNAL( kdisplayFontChanged() ),
	         this, SLOT( slotAppearanceChanged() ) );

	slotAppearanceChanged();

	slotContactAdded( session->myself() );

	foreach( Kopete::Contact *contact, session->members() )
		slotContactAdded( contact );
}

ChatTextEditPart::~ChatTextEditPart()
{
	delete mComplete;
}

void ChatTextEditPart::complete()
{
	QTextCursor textCursor = textEdit()->textCursor();
	QTextBlock block = textCursor.block();

	QString txt = block.text();
	const int blockLength = block.length() - 1; // block.length includes the '\n'
	const int blockPosition = block.position();
    int cursorPos = textCursor.position() - blockPosition;

	// TODO replace with textCursor.movePosition(QTextCursor::PreviousWord)?
	const int startPos = txt.lastIndexOf( QRegExp( QLatin1String("\\s\\S+") ), cursorPos - 1 ) + 1;
	int endPos = txt.indexOf( QRegExp( QLatin1String("[\\s\\:]") ), startPos );
	if( endPos == -1 )
	{
		endPos = blockLength;
	}
	const QString word = txt.mid( startPos, endPos - startPos );

	if ( endPos < txt.length() && txt[endPos] == ':') {
		// Eat ':' and ' ' too, if they are there, so that after pressing Tab
		// we are on the right side of them again.
		++endPos;
		if ( endPos < txt.length() && txt[endPos] == ' ') {
			++endPos;
		}
	}

	//kDebug(14000) << word << "from" << txt
	//			  << "cursor pos=" << cursorPos
	//			  << "start pos=" << startPos << "end pos=" << endPos;

	QString match;
	if ( word != m_lastMatch )
	{
		match = mComplete->makeCompletion( word );
		m_lastMatch.clear();
	}
	else
	{
		match = mComplete->nextMatch();
	}

	if ( !match.isEmpty() )
	{
		m_lastMatch = match;

		if ( textCursor.blockNumber() == 0 && startPos == 0 )
		{
			match += QLatin1String(": ");
		}

		//kDebug(14000) << "Selecting from position" << cursorPos << "to position" << endPos;
		// Select the text to remove
		textCursor.setPosition( startPos + blockPosition );
		textCursor.setPosition( endPos + blockPosition, QTextCursor::KeepAnchor );
		//kDebug(14000) << "replacing selection:" << textCursor.selectedText() << "with match:" << match;
		// Type the text to replace it
		textCursor.insertText( match );
		textEdit()->setTextCursor( textCursor );
	}
	else
	{
		//kDebug(14000) << "No completions! Tried" << mComplete->items();
	}
}

void ChatTextEditPart::slotPropertyChanged( Kopete::PropertyContainer*, const QString &key,
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
	connect( contact, SIGNAL( propertyChanged( Kopete::PropertyContainer *, const QString &, const QVariant &, const QVariant & ) ),
	         this, SLOT( slotPropertyChanged( Kopete::PropertyContainer *, const QString &, const QVariant &, const QVariant & ) ) ) ;

	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->addItem( contactName );
}

void ChatTextEditPart::slotContactRemoved( const Kopete::Contact *contact )
{
	disconnect( contact, SIGNAL( propertyChanged( Kopete::PropertyContainer *, const QString &, const QVariant &, const QVariant & ) ),
	            this, SLOT( slotPropertyChanged( Kopete::PropertyContainer *, const QString &, const QVariant &, const QVariant & ) ) ) ;

	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->removeItem( contactName );
}

bool ChatTextEditPart::canSend()
{
	int i;

	if ( !m_session ) return false;

	// can't send if there's nothing *to* send...
	if ( text(Qt::PlainText).isEmpty() )
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
	textEdit()->moveCursor( QTextCursor::End );
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

	QString newText = ( historyPos >= 0 ? historyList[historyPos] : QString() );


// 	TextFormat format=textEdit()->textFormat();
// 	textEdit()->setTextFormat(AutoText); //workaround bug 115690
	textEdit()->setText( newText );
// 	textEdit()->setTextFormat(format);
	textEdit()->moveCursor( QTextCursor::End );
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
	if ( useRichText() )
		textEdit()->setHtml ( message.escapedBody() );
	else
		textEdit()->setPlainText ( message.plainBody() );
	textEdit()->moveCursor ( QTextCursor::End );

// 	setFont( message.font() );
// 	setTextColor( message.foregroundColor() );
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

void ChatTextEditPart::slotAppearanceChanged()
{
	Kopete::AppearanceSettings *settings = Kopete::AppearanceSettings::self();

	setDefualtTextColor( settings->chatTextColor() );
	setDefualtFont( ( settings->chatFontSelection() == 1 ) ? settings->chatFont() : KGlobalSettings::generalFont() );
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
