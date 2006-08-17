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

#include "kopetechatsession.h"
#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"
#include "kopeteglobal.h"
#include "kopeteprefs.h"

#include <kcompletion.h>
#include <kdebug.h>
#include <ktextedit.h>
#include <ksyntaxhighlighter.h>

#include <qtimer.h>
#include <qregexp.h>

ChatTextEditPart::ChatTextEditPart( Kopete::ChatSession *session, QWidget *parent, const char *name )
	: KopeteRichTextEditPart( parent, name, session->protocol()->capabilities() ), m_session(session)
{
	historyPos = -1;
	
	toggleAutoSpellCheck(KopetePrefs::prefs()->spellCheck());
	
	mComplete = new KCompletion();
	mComplete->setIgnoreCase( true );
	mComplete->setOrder( KCompletion::Weighted );
	
	// set params on the edit widget
	edit()->setMinimumSize( QSize( 75, 20 ) );
	edit()->setWordWrap( QTextEdit::WidgetWidth );
	edit()->setWrapPolicy( QTextEdit::AtWhiteSpace );
	edit()->setAutoFormatting( QTextEdit::AutoNone );

	// some signals and slots connections
	connect( edit(), SIGNAL( textChanged()), this, SLOT( slotTextChanged() ) );

	// timers for typing notifications
	m_typingRepeatTimer = new QTimer(this, "m_typingRepeatTimer");
	m_typingStopTimer   = new QTimer(this, "m_typingStopTimer");

	connect( m_typingRepeatTimer, SIGNAL( timeout() ), this, SLOT( slotRepeatTypingTimer() ) );
	connect( m_typingStopTimer,   SIGNAL( timeout() ), this, SLOT( slotStoppedTypingTimer() ) );

	connect( session, SIGNAL( contactAdded(const Kopete::Contact*, bool) ),
	         this, SLOT( slotContactAdded(const Kopete::Contact*) ) );
	connect( session, SIGNAL( contactRemoved(const Kopete::Contact*, const QString&, Kopete::Message::MessageFormat, bool) ),
	         this, SLOT( slotContactRemoved(const Kopete::Contact*) ) );
	connect( session, SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus & , const Kopete::OnlineStatus &) ),
	         this, SLOT( slotContactStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ) );
	
	slotContactAdded( session->myself() );
	for ( QPtrListIterator<Kopete::Contact> it( session->members() ); it.current(); ++it )
		slotContactAdded( *it );
}

ChatTextEditPart::~ChatTextEditPart()
{
	delete mComplete;
}

KTextEdit *ChatTextEditPart::edit()
{
	return static_cast<KTextEdit*>(widget());
}

void ChatTextEditPart::toggleAutoSpellCheck( bool enabled )
{
	if ( richTextEnabled() )
		enabled = false;

	m_autoSpellCheckEnabled = enabled;
	if ( spellHighlighter() )
	{
		spellHighlighter()->setAutomatic( enabled );
		spellHighlighter()->setActive( enabled );
	}
	edit()->setCheckSpellingEnabled( enabled );
}

bool ChatTextEditPart::autoSpellCheckEnabled() const
{
	return m_autoSpellCheckEnabled;
}

KDictSpellingHighlighter* ChatTextEditPart::spellHighlighter()
{
	QSyntaxHighlighter *qsh = edit()->syntaxHighlighter();
	KDictSpellingHighlighter* kdsh = dynamic_cast<KDictSpellingHighlighter*>( qsh );
	return kdsh;
}

// NAUGHTY, BAD AND WRONG! (but needed to fix nick complete bugs)
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

void ChatTextEditPart::complete()
{
	int para = 1, parIdx = 1;
	edit()->getCursorPosition( &para, &parIdx);

	// FIXME: strips out all formatting
	QString txt = static_cast<EvilTextEdit*>(edit())->plainText( para );

	if ( parIdx > 0 )
	{
		int firstSpace = txt.findRev( QRegExp( QString::fromLatin1("\\s\\S+") ), parIdx - 1 ) + 1;
		int lastSpace = txt.find( QRegExp( QString::fromLatin1("[\\s\\:]") ), firstSpace );
		if( lastSpace == -1 )
			lastSpace = txt.length();

		QString word = txt.mid( firstSpace, lastSpace - firstSpace );
		QString match;

		kdDebug(14000) << k_funcinfo << word << " from '" << txt << "'" << endl;

		if ( word != m_lastMatch )
		{
			match = mComplete->makeCompletion( word );
			m_lastMatch = QString::null;
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
				rightText = match + QString::fromLatin1(": ") + rightText;
				parIdx += 2;
			}
			else
				rightText = match + rightText;

			// insert *before* remove. this is becase Qt adds an extra blank line
			// if the rich text control becomes empty (if you remove the only para).
			// disable updates while we change the contents to eliminate flicker.
			edit()->setUpdatesEnabled( false );
			edit()->insertParagraph( txt.left(firstSpace) + rightText, para );
			edit()->removeParagraph( para + 1 );
			edit()->setCursorPosition( para, parIdx + match.length() );
			edit()->setUpdatesEnabled( true );
			// must call this rather than update because QTextEdit is broken :(
			edit()->updateContents();
			m_lastMatch = match;
		}
		else
		{
			kdDebug(14000) << k_funcinfo << "No completions! Tried " << mComplete->items() << endl;
		}
	}
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
	
	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->addItem( contactName );
}

void ChatTextEditPart::slotContactRemoved( const Kopete::Contact *contact )
{
	disconnect( contact, SIGNAL( propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
	            this, SLOT( slotPropertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ) ) ;
	
	QString contactName = contact->property(Kopete::Global::Properties::self()->nickName()).value().toString();
	mComplete->removeItem( contactName );
}

bool ChatTextEditPart::canSend()
{
	if ( !m_session ) return false;

	// can't send if there's nothing *to* send...
	if ( edit()->text().isEmpty() )
		return false;

	Kopete::ContactPtrList members = m_session->members();
	
	// if we can't send offline, make sure we have a reachable contact...
	if ( !( m_session->protocol()->capabilities() & Kopete::Protocol::CanSendOffline ) )
	{
		bool reachableContactFound = false;

		//TODO: does this perform badly in large / busy IRC channels? - no, doesn't seem to
		QPtrListIterator<Kopete::Contact> it ( members );
		for( ; it.current(); ++it )
		{
			if ( (*it)->isReachable() )
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
	QString txt = text( Qt::PlainText );
	// avoid sending emtpy messages or enter keys (see bug 100334)
	if ( txt.isEmpty() || txt == "\n" )
		return;

	if ( m_lastMatch.isNull() && ( txt.find( QRegExp( QString::fromLatin1("^\\w+:\\s") ) ) > -1 ) )
	{ //no last match and it finds something of the form of "word:" at the start of a line
		QString search = txt.left( txt.find(':') );
		if( !search.isEmpty() )
		{
			QString match = mComplete->makeCompletion( search );
			if( !match.isNull() )
				edit()->setText( txt.replace(0,search.length(),match) );
		}
	}

	if ( !m_lastMatch.isNull() )
	{
		//FIXME: what is the next line for?
		mComplete->addItem( m_lastMatch );
		m_lastMatch = QString::null;
	}

	slotStoppedTypingTimer();
	Kopete::Message sentMessage = contents();
	emit messageSent( sentMessage );
	historyList.prepend( edit()->text() );
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
	return !txt.stripWhiteSpace().isEmpty();
}

void ChatTextEditPart::slotTextChanged()
{
	if ( isTyping() )
	{
		// And they were previously typing
		if( !m_typingRepeatTimer->isActive() )
		{
			m_typingRepeatTimer->start( 4000, false );
			slotRepeatTypingTimer();
		}

		// Reset the stop timer again, regardless of status
		m_typingStopTimer->start( 4500, true );
	}

	emit canSendChanged( canSend() );
}

void ChatTextEditPart::historyUp()
{
	if ( historyList.empty() || historyPos == historyList.count() - 1 )
		return;
	
	QString text = edit()->text();
	bool empty = text.stripWhiteSpace().isEmpty();
	
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
	TextFormat format=edit()->textFormat();
	edit()->setTextFormat(AutoText); //workaround bug 115690
	edit()->setText( newText );
	edit()->setTextFormat(format);
	edit()->moveCursor( QTextEdit::MoveEnd, false );
}

void ChatTextEditPart::historyDown()
{
	if ( historyList.empty() || historyPos == -1 )
		return;
	
	QString text = edit()->text();
	bool empty = text.stripWhiteSpace().isEmpty();
	
	// got text? save it
	if ( !empty )
	{
		historyList[historyPos] = text;
	}
	
	historyPos--;
	
	QString newText = ( historyPos >= 0 ? historyList[historyPos] : QString::null );
	
	
	TextFormat format=edit()->textFormat();
	edit()->setTextFormat(AutoText); //workaround bug 115690
	edit()->setText( newText );
	edit()->setTextFormat(format);
	
	
	
	edit()->moveCursor( QTextEdit::MoveEnd, false );
}

void ChatTextEditPart::addText( const QString &text )
{
	edit()->insert( text );
}

void ChatTextEditPart::setContents( const Kopete::Message &message )
{
	edit()->setText( richTextEnabled() ? message.escapedBody() : message.plainBody() );

	setFont( message.font() );
	setFgColor( message.fg() );
	setBgColor( message.bg() );
}

Kopete::Message ChatTextEditPart::contents()
{
	Kopete::Message currentMsg( m_session->myself(), m_session->members(), edit()->text(),
	                            Kopete::Message::Outbound, richTextEnabled() ?
	                            Kopete::Message::RichText : Kopete::Message::PlainText );
	
	currentMsg.setBg( bgColor() );
	currentMsg.setFg( fgColor() );
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

#include "chattexteditpart.moc"

// vim: set noet ts=4 sts=4 sw=4:

