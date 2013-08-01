/*
    chattexteditpart.cpp - Chat Text Edit Part

    Copyright (c) 2008      by Benson Tsai           <btsai@vrwarp.com>
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
#include "kopeteappearancesettings.h"
#include "kopetechatwindowsettings.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kcompletion.h>
#include <kdebug.h>
#include <kfontaction.h>
#include <kfontdialog.h>
#include <kfontsizeaction.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>
#include <kicon.h>
#include <kparts/genericfactory.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kxmlguifactory.h>


// Qt includes
#include <QtCore/QTimer>
#include <QtCore/QRegExp>
#include <QtCore/QEvent>
#include <QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextCharFormat>


typedef KParts::GenericFactory<ChatTextEditPart> ChatTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( librichtexteditpart, ChatTextEditPartFactory )

ChatTextEditPart::ChatTextEditPart( Kopete::ChatSession *session, QWidget *parent)
	: KParts::ReadOnlyPart( parent ), m_session(session)
{
	init(session, parent);
}

ChatTextEditPart::ChatTextEditPart(QWidget *parent, QObject*, const QStringList&)
	: KParts::ReadOnlyPart( parent ), m_session()
{
	init(m_session, parent);
}

void ChatTextEditPart::init( Kopete::ChatSession *session, QWidget *parent)
{
	// we need an instance
	setComponentData( ChatTextEditPartFactory::componentData() );
	
	editor = new KopeteRichTextWidget(parent, m_session->protocol()->capabilities(), actionCollection());
	setWidget( editor );

	// TODO: Rename rc file
	setXMLFile( "kopeterichtexteditpart/kopeterichtexteditpartfull.rc" );
	
	historyPos = -1;

	mComplete = new KCompletion();
	mComplete->setIgnoreCase( true );
	mComplete->setOrder( KCompletion::Weighted );

	// set params on the edit widget
	textEdit()->setMinimumSize( QSize( 75, 20 ) );

	// some signals and slots connections
	connect( textEdit(), SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );

	// timers for typing notifications
	m_typingRepeatTimer = new QTimer(this);
	m_typingRepeatTimer->setObjectName("m_typingRepeatTimer");
	m_typingStopTimer   = new QTimer(this);
	m_typingStopTimer->setObjectName("m_typingStopTimer");

	connect( m_typingRepeatTimer, SIGNAL(timeout()), this, SLOT(slotRepeatTypingTimer()) );
	connect( m_typingStopTimer,   SIGNAL(timeout()), this, SLOT(slotStoppedTypingTimer()) );

	connect( session, SIGNAL(contactAdded(const Kopete::Contact*,bool)),
	         this, SLOT(slotContactAdded(const Kopete::Contact*)) );
	connect( session, SIGNAL(contactRemoved(const Kopete::Contact*,QString,Qt::TextFormat,bool)),
	         this, SLOT(slotContactRemoved(const Kopete::Contact*)) );
	connect( session, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
	         this, SLOT(slotContactStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );

	connect( Kopete::AppearanceSettings::self(), SIGNAL(appearanceChanged()),
	         this, SLOT(slotAppearanceChanged()) );

	connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()),
	         this, SLOT(slotAppearanceChanged()) );

	connect( editor, SIGNAL(richTextSupportChanged()), this, SLOT (slotRichTextSupportChanged()) );

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

void ChatTextEditPart::slotDisplayNameChanged( const QString &oldName, const QString &newName )
{
	mComplete->removeItem( oldName );
	mComplete->addItem( newName );
}

void ChatTextEditPart::slotContactAdded( const Kopete::Contact *contact )
{
	connect( contact, SIGNAL(displayNameChanged(QString,QString)),
	         this, SLOT(slotDisplayNameChanged(QString,QString)) );

	mComplete->addItem( contact->displayName() );
}

void ChatTextEditPart::slotContactRemoved( const Kopete::Contact *contact )
{
	disconnect( contact, SIGNAL(displayNameChanged(QString,QString)),
	            this, SLOT(slotDisplayNameChanged(QString,QString)) );

	mComplete->removeItem( contact->displayName() );
}

bool ChatTextEditPart::canSend()
{
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
		for( int i = 0; i != members.size(); ++i )
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
	historyList.prepend( this->text( Qt::AutoText) );
	historyPos = -1;
	textEdit()->moveCursor(QTextCursor::End);
	textEdit()->clear();
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
		text = this->text(Qt::AutoText);
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
	textEdit()->setTextOrHtml( newText );
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
		text = this->text(Qt::AutoText);
		historyList[historyPos] = text;
	}

	historyPos--;

	QString newText = ( historyPos >= 0 ? historyList[historyPos] : QString() );


	textEdit()->setTextOrHtml( newText );
	textEdit()->moveCursor( QTextCursor::End );
}

void ChatTextEditPart::addText( const QString &text )
{
	if( Qt::mightBeRichText(text) )
	{
		if ( textEdit()->isRichTextEnabled() )
		{
			textEdit()->insertHtml( text );
		}
		else
		{
			QTextDocument doc;
			doc.setHtml( text );
			textEdit()->insertPlainText( doc.toPlainText() );
		}
	}
	else
	{
		textEdit()->insertPlainText( text );
	}
}

void ChatTextEditPart::setContents( const Kopete::Message &message )
{
	if ( isRichTextEnabled() )
		textEdit()->setHtml ( message.escapedBody() );
	else
		textEdit()->setPlainText ( message.plainBody() );
	textEdit()->moveCursor ( QTextCursor::End );
}

Kopete::Message ChatTextEditPart::contents()
{
	Kopete::Message currentMsg( m_session->myself(), m_session->members() );
	currentMsg.setDirection( Kopete::Message::Outbound );

	if (isRichTextEnabled())
	{
		currentMsg.setHtmlBody(text());
	
		Kopete::Protocol::Capabilities protocolCaps = m_session->protocol()->capabilities();
	
		// I hate base *only* support, *waves fist at MSN*
		if (protocolCaps & Kopete::Protocol::BaseFormatting)
		{
			currentMsg.setFont(textEdit()->currentRichFormat().font());
		}
	
		if (protocolCaps & Kopete::Protocol::BaseFgColor)
		{
			currentMsg.setForegroundColor(textEdit()->currentRichFormat().foreground().color());
		}
	
		if (protocolCaps & Kopete::Protocol::BaseBgColor)
		{
			currentMsg.setBackgroundColor(textEdit()->currentRichFormat().background().color());
		}
	}
	else
	{
		currentMsg.setPlainBody(text());
	}

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

	QFont font = ( settings->chatFontSelection() == 1 ) ? settings->chatFont() : KGlobalSettings::generalFont();
	QTextCharFormat format;
	format.setFont(font);
	format.setBackground(settings->chatBackgroundColor());
	format.setForeground(settings->chatTextColor());

	editor->setDefaultPlainCharFormat(format);
	editor->setDefaultRichCharFormat(format);
}

void ChatTextEditPart::slotRichTextSupportChanged()
{
	KXMLGUIFactory * f = factory();
	if (f)
	{
		f->removeClient(this);
		f->addClient(this);
	}
}

KopeteRichTextWidget *ChatTextEditPart::textEdit()
{
    return editor;
}

void ChatTextEditPart::setCheckSpellingEnabled( bool enabled )
{
    editor->setCheckSpellingEnabled( enabled );
}

bool ChatTextEditPart::checkSpellingEnabled() const
{
    return editor->checkSpellingEnabled();
}

void ChatTextEditPart::checkToolbarEnabled()
{
	emit toolbarToggled( isRichTextEnabled() );
}

KAboutData *ChatTextEditPart::createAboutData()
{
    KAboutData *aboutData = new KAboutData("krichtexteditpart", 0, ki18n("Chat Text Edit Part"), "0.1",
                        ki18n("A simple rich text editor part"),
                        KAboutData::License_LGPL );
    aboutData->addAuthor(ki18n("Richard J. Moore"), KLocalizedString(), "rich@kde.org", "http://xmelegance.org/" );
    aboutData->addAuthor(ki18n("Jason Keirstead"), KLocalizedString(), "jason@keirstead.org", "http://www.keirstead.org/" );
    aboutData->addAuthor(ki18n("Michaël Larouche"), KLocalizedString(), "larouche@kde.org" "http://www.tehbisnatch.org/" );
    aboutData->addAuthor(ki18n("Benson Tsai"), KLocalizedString(), "btsai@vrwarp.com" "http://www.vrwarp.com/" );

    return aboutData;
}

void ChatTextEditPart::readConfig( KConfigGroup& config )
{
    kDebug() << "Loading config";

    QTextCharFormat format = editor->defaultRichFormat();

    QFont font = config.readEntry( "TextFont", format.font() );
    QColor fg = config.readEntry( "TextFgColor", format.foreground().color() );
    QColor bg = config.readEntry( "TextBgColor", format.background().color() );

	QTextCharFormat desiredFormat = editor->currentRichFormat();
    desiredFormat.setFont(font);
    desiredFormat.setForeground(fg);
    desiredFormat.setBackground(bg);
    editor->setCurrentRichCharFormat(desiredFormat);

    textEdit()->setAlignment(static_cast<Qt::AlignmentFlag>(config.readEntry( "EditAlignment", int(Qt::AlignLeft) )));
}

void ChatTextEditPart::writeConfig( KConfigGroup& config )
{
    kDebug() << "Saving config";

	config.writeEntry( "TextFont", editor->currentRichFormat().font() );
	config.writeEntry( "TextFgColor", editor->currentRichFormat().foreground().color() );
	config.writeEntry( "TextBgColor", editor->currentRichFormat().background().color() );
    config.writeEntry( "EditAlignment", int(editor->alignment()) );
}

void ChatTextEditPart::resetConfig( KConfigGroup& config )
{
    kDebug() << "Setting default font style";

    editor->slotResetFontAndColor();

    //action_align_left->trigger();

    config.deleteEntry( "TextFont" );
    config.deleteEntry( "TextFg" );
    config.deleteEntry( "TextBg" );
    config.deleteEntry( "EditAlignment" );
}

QString ChatTextEditPart::text( Qt::TextFormat format ) const
{
    if( (format == Qt::RichText || format == Qt::AutoText) && isRichTextEnabled() )
        return editor->toHtml();
    else
        return editor->toPlainText();
}

bool ChatTextEditPart::isRichTextEnabled() const
{
	return editor->isRichTextEnabled();
}

#include "chattexteditpart.moc"

// vim: set noet ts=4 sts=4 sw=4:
