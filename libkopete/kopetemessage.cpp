/*
    kopetemessage.cpp  -  Base class for Kopete messages

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2006 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2006-2007 by Charles Connell        <charles@connells.org>
    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2008      by Roman Jarosz           <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemessage.h"

#include <stdlib.h>

#include <QtCore/QDateTime>
#include <QtCore/QLatin1String>
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtCore/QTextCodec>
#include <QtGui/QTextDocument>
#include <QtGui/QColor>

#include <kdebug.h>
#include <kstringhandler.h>

#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetechatsession.h"
#include "kopetecontact.h"
#include "kopeteemoticons.h"


namespace Kopete
{

class Message::Private
	: public QSharedData
{
public:
	Private() //assign next message id, it can't be changed later
		: id(nextId++), direction(Internal), format(Qt::PlainText), type(TypeNormal), importance(Normal), state(StateUnknown),
		  delayed(false), formattingOverride(false), forceHtml(false), isRightToLeft(false),
		  timeStamp( QDateTime::currentDateTime() ), body(new QTextDocument), parsedBodyDirty(true), escapedBodyDirty(true),
		  fileTransfer(0)
	{}
	Private (const Private &other);
	~Private();

	const uint id;
	QPointer<Contact> from;
	ContactPtrList to;
	QPointer<ChatSession> manager;

	MessageDirection direction;
	Qt::TextFormat format;
	MessageType type;
	QString requestedPlugin;
	MessageImportance importance;
	MessageState state;
	bool delayed;
	bool formattingOverride, forceHtml;
	bool isRightToLeft;
	QDateTime timeStamp;
	QFont font;
	QStringList classes;

	QColor foregroundColor;
	QColor backgroundColor;
	QString subject;

	QTextDocument* body;
	mutable QString parsedBody;
	mutable bool parsedBodyDirty;
	mutable QString escapedBody;
	mutable bool escapedBodyDirty;

	class FileTransferInfo
	{
	public:
		FileTransferInfo() : disabled(false), fileSize(0)
		{}

		bool disabled;
		QString fileName;
		unsigned long fileSize;
		QPixmap filePreview;
	};
	FileTransferInfo* fileTransfer;

	static uint nextId;
};

// Start with 1 as 0 is reserved for invalid id;
uint Message::Private::nextId = 1;

Message::Private::Private (const Message::Private &other)
	: QSharedData (other), id(other.id)
{
	from = other.from;
	to = other.to;
	manager = other.manager;

	direction = other.direction;
	format = other.format;
	type = other.type;
	requestedPlugin = other.requestedPlugin;
	importance = other.importance;
	state = other.state;
	delayed = other.delayed;
	formattingOverride = other.formattingOverride;
	isRightToLeft = other.isRightToLeft;
	timeStamp = other.timeStamp;
	font = other.font;
	classes = other.classes;

	foregroundColor = other.foregroundColor;
	backgroundColor = other.backgroundColor;
	subject = other.subject;

	body = other.body->clone();
	parsedBody = other.parsedBody;
	parsedBodyDirty = other.parsedBodyDirty;
	escapedBody = other.escapedBody;
	escapedBodyDirty = other.escapedBodyDirty;

	if ( other.fileTransfer )
		fileTransfer = new FileTransferInfo( *other.fileTransfer );
	else
		fileTransfer = 0;
}

Message::Private::~Private ()
{
	delete fileTransfer;

	delete body;
}

Message::Message()
 : d( new Private )
{
}

Message::Message( const Contact *fromKC, const Contact *toKC )
 : d(new Private)
{
	d->from = const_cast<Contact*>(fromKC);
	QList<Contact *> contacts;
	contacts << const_cast<Contact*>(toKC);

	d->to = contacts;
}

Message::Message( const Contact *fromKC, const QList<Contact*> &toKC )
 : d( new Private )
{
	d->from = const_cast<Contact*>(fromKC);
	d->to = toKC;
}

Message::Message( const Message &other )
	: d(other.d)
{
}

Message& Message::operator=( const Message &other )
{
	d = other.d;
	return *this;
}

Message::~Message()
{
}

uint Message::id() const
{
	return d->id;
}

uint Message::nextId()
{
	return Message::Private::nextId++;
}

void Message::setBackgroundOverride( bool enabled )
{
	setFormattingOverride(enabled);
}

void Message::setForegroundOverride( bool enabled )
{
	setFormattingOverride(enabled);
}

void Message::setRichTextOverride( bool enabled )
{
	setFormattingOverride(enabled);
}

void Message::setFormattingOverride( bool enabled )
{
	d->formattingOverride = enabled;
	d->parsedBodyDirty=true;
	d->escapedBodyDirty=true;
}

void Message::setForegroundColor( const QColor &color )
{
	d->foregroundColor=color;
}

void Message::setBackgroundColor( const QColor &color )
{
	d->backgroundColor=color;
}

void Message::setFont( const QFont &font )
{
	d->font = font;
}

void Message::setPlainBody (const QString &body)
{
	doSetBody (body, Qt::PlainText);
}

void Message::setHtmlBody (const QString &body)
{
	doSetBody (body, Qt::RichText);
}

void Message::setForcedHtmlBody( const QString &body)
{
	setHtmlBody(body);
	d->forceHtml = true;
}

void Message::doSetBody (QString body, Qt::TextFormat f)
{
	// Remove ObjectReplacementCharacter because otherwise html text will be empty
	if ( body.contains( QChar( QChar::ObjectReplacementCharacter ) ) )
		body.replace( QChar( QChar::ObjectReplacementCharacter ), QChar( ' ' ) );

	if (f == Qt::PlainText)
		d->body->setPlainText(body);
	else
		d->body->setHtml(body);
	d->format = f;
	d->isRightToLeft = d->body->toPlainText().isRightToLeft();
	d->escapedBodyDirty = true;
	d->parsedBodyDirty = true;
}

void Message::setBody (const QTextDocument *_body)
{
	doSetBody (_body, Qt::RichText);
}

void Message::doSetBody (const QTextDocument *body, Qt::TextFormat f)
{
	delete d->body;
	d->body = body->clone();          // delete the old body and replace it with a *copy* of the new one
	d->format = f;
	d->isRightToLeft = d->body->toPlainText().isRightToLeft();
	d->escapedBodyDirty = true;
	d->parsedBodyDirty = true;
}

void Message::setImportance(Message::MessageImportance i)
{
	d->importance = i;
}

QString Message::unescape( const QString &xml )
{
	QString data = xml;

	// Remove linebreak and multiple spaces. First return nbsp's to normal spaces :)
	data = data.simplified();

	int pos;
	while ( ( pos = data.indexOf( '<' ) ) != -1 )
	{
		int endPos = data.indexOf( '>', pos + 1 );
		if( endPos == -1 )
			break;    // No more complete elements left

		// Take the part between < and >, and extract the element name from that
		int matchWidth = endPos - pos + 1;
		const QString match = data.mid( pos + 1, matchWidth - 2 ).simplified();
		int elemEndPos = match.indexOf( ' ' );
		const QString elem = ( elemEndPos == -1 ? match.toLower() : match.left( elemEndPos ).toLower() );
		if ( elem == QLatin1String( "img" ) )
		{
			// Replace smileys with their original text'
			const QString attrTitle  = QLatin1String( "title=\"" );
			int titlePos    = match.indexOf( attrTitle, elemEndPos );
			int titleEndPos = match.indexOf( '"',       titlePos + attrTitle.length() );
			if( titlePos == -1 || titleEndPos == -1 )
			{
				// Not a smiley but a normal <img>
				// Don't update pos, we restart at this position :)
				data.remove( pos, matchWidth );
			}
			else
			{
				QString orig = match.mid( titlePos + attrTitle.length(),
					titleEndPos - titlePos - attrTitle.length() );
				data.replace( pos, matchWidth, orig );
				pos += orig.length();
			}
		}
		else if ( elem == QLatin1String( "/p" ) || elem == QLatin1String( "/div" ) ||
			elem == QLatin1String( "br" ) )
		{
			// Replace paragraph, div and line breaks with a newline
			data.replace( pos, matchWidth, '\n' );
			pos++;
		}
		else
		{
			// Remove all other elements entirely
			// Don't update pos, we restart at this position :)
			data.remove( pos, matchWidth );
		}
	}

	// Replace stuff starting with '&'
	data.replace( QLatin1String( "&gt;" ), QLatin1String( ">" ) );
	data.replace( QLatin1String( "&lt;" ), QLatin1String( "<" ) );
	data.replace( QLatin1String( "&quot;" ), QLatin1String( "\"" ) );
	data.replace( QLatin1String( "&nbsp;" ), QLatin1String( " " ) );
	data.replace( QLatin1String( "&amp;" ), QLatin1String( "&" ) );
	data.replace( QLatin1String( "&#160;" ), QLatin1String( " " ) );  //this one is used in jabber:  note, we should escape all &#xx;

	return data;
}

QString Message::escape( const QString &text )
{
	QString html = Qt::escape( text );
 	//Replace carriage returns inside the text
	html.replace( QLatin1Char( '\n' ), QLatin1String( "<br />" ) );
	//Replace a tab with 4 spaces
	html.replace( QLatin1Char( '\t' ), QLatin1String( "&nbsp;&nbsp;&nbsp;&nbsp;" ) );

	//Replace multiple spaces with &nbsp;
	//do not replace every space so we break the linebreak
	html.replace( QRegExp( QLatin1String( "\\s\\s" ) ), QLatin1String( "&nbsp; " ) );

	return html;
}

QString Message::plainBody() const
{
	// Remove ObjectReplacementCharacter which can be there if html text contains img tag.
	QString plainText = d->body->toPlainText();
	plainText.replace( QChar( QChar::ObjectReplacementCharacter ), QChar( ' ' ) );
	return plainText;
}

QString Message::escapedBody() const
{
//	kDebug(14010) << escapedBody() << " " << d->richTextOverride;

//	the escaped body is cached because QRegExp is very expensive, so it shouldn't be used any more than necessary
	if (!d->escapedBodyDirty)
		return d->escapedBody;
	else {
		QString html;
		if ( d->format == Qt::PlainText || (d->formattingOverride && !d->forceHtml))
			html = Qt::convertFromPlainText( d->body->toPlainText(), Qt::WhiteSpaceNormal );
		else
			html = d->body->toHtml();

//		all this regex business is to take off the outer HTML document provided by QTextDocument
//		remove the head
		QRegExp badStuff ("<![^<>]*>|<head[^<>]*>.*</head[^<>]*>|</?html[^<>]*>|</?body[^<>]*>");
		html = html.remove (badStuff);
//		remove newlines that may be present, since they end up being displayed in the chat window. real newlines are represented with <br>, so we know \n's are meaningless
		html = html.remove ('\n');
		d->escapedBody = html;
		d->escapedBodyDirty = false;
		return html;
	}
}

QString Message::parsedBody() const
{
	//kDebug(14000) << "messageformat: " << d->format;
	if ( !d->parsedBodyDirty )
		return d->parsedBody;
	
	d->parsedBody = Kopete::Emoticons::parseEmoticons(parseLinks(escapedBody(), Qt::RichText));
	d->parsedBodyDirty = false;
	return d->parsedBody;
}

static QString makeRegExp( const char *pattern )
{
	const QString urlChar = QLatin1String( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString boundaryStart = QString( "(^|[^%1])(" ).arg( urlChar );
	const QString boundaryEnd = QString( ")([^%1]|$)" ).arg( urlChar );

	return boundaryStart + QLatin1String(pattern) + boundaryEnd;
}

const QStringList Message::regexpPatterns()
{
	const QString name = QLatin1String( "[\\w\\+\\-=_\\.]+" );
	const QString userAndPassword = QString( "(?:%1(?::%1)?\\@)" ).arg( name );
	const QString urlChar = QLatin1String( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString urlSection = QString( "[%1]+" ).arg( urlChar );
	const QString domain = QLatin1String( "[\\-\\w_]+(?:\\.[\\-\\w_]+)+" );
	QStringList patternList;
	patternList << makeRegExp("\\w+://%1?\\w%2").arg( userAndPassword, urlSection )
	            << makeRegExp("%1?www\\.%2%3").arg( userAndPassword, domain, urlSection )
	            << makeRegExp("%1@%2").arg( name, domain );
	return patternList;
}

QString Message::parseLinks( const QString &message, Qt::TextFormat format )
{
	if ( format & Qt::RichText )
	{
		// < in HTML *always* means start-of-tag
		QStringList entries = message.split( QChar('<'), QString::KeepEmptyParts );

		QStringList::Iterator it = entries.begin();

		// first one is different: it doesn't start with an HTML tag.
		if ( it != entries.end() )
		{
			*it = parseLinks( *it, Qt::PlainText );
			++it;
		}

		for ( ; it != entries.end(); ++it )
		{
			QString curr = *it;
			// > in HTML means start-of-tag if and only if it's the first one after a <
			int tagclose = curr.indexOf( QChar('>') );
			// no >: the HTML is broken, but we can cope
			if ( tagclose == -1 )
				continue;
			QString tag = curr.left( tagclose + 1 );
			QString body = curr.mid( tagclose + 1 );
			*it = tag + parseLinks( body, Qt::PlainText );
		}
		return entries.join(QLatin1String("<"));
	}

	QString result = message;

	// common subpatterns - may not contain matching parens!
	const QString name = QLatin1String( "[\\w\\+\\-=_\\.]+" );
	const QString userAndPassword = QString( "(?:%1(?::%1)?\\@)" ).arg( name );
	const QString urlChar = QLatin1String( "\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*\\(\\)" );
	const QString urlSection = QString( "[%1]+" ).arg( urlChar );
	const QString domain = QLatin1String( "[\\-\\w_]+(?:\\.[\\-\\w_]+)+" );

	//Replace http/https/ftp links:
	// Replace (stuff)://[user:password@](linkstuff) with a link
	result.replace(
		QRegExp( makeRegExp("\\w+://%1?\\w%2").arg( userAndPassword, urlSection ) ),
		QLatin1String("\\1<a href=\"\\2\" title=\"\\2\">\\2</a>\\3" ) );

	// Replace www.X.Y(linkstuff) with a http: link
	result.replace(
		QRegExp( makeRegExp("%1?www\\.%2%3").arg( userAndPassword, domain, urlSection ) ),
		QLatin1String("\\1<a href=\"http://\\2\" title=\"http://\\2\">\\2</a>\\3" ) );

	//Replace Email Links
	// Replace user@domain with a mailto: link
	result.replace(
		QRegExp( makeRegExp("%1@%2").arg( name, domain ) ),
		QLatin1String("\\1<a href=\"mailto:\\2\" title=\"mailto:\\2\">\\2</a>\\3") );

	//Workaround for Bug 85061: Highlighted URLs adds a ' ' after the URL itself
	// the trailing  &nbsp; is included in the url.
	result.replace( QRegExp( QLatin1String("(<a href=\"[^\"]+)(&nbsp;)(\")")  ) , QLatin1String("\\1\\3") );

	return result;
}



QDateTime Message::timestamp() const
{
	return d->timeStamp;
}

void Message::setTimestamp(const QDateTime &timestamp)
{
	d->timeStamp = timestamp;
}

const Contact *Message::from() const
{
	return d->from;
}

QList<Contact*> Message::to() const
{
	return d->to;
}

Message::MessageType Message::type() const
{
	return d->type;
}

void Message::setType(MessageType type)
{
	d->type = type;
}

QString Message::requestedPlugin() const
{
	return d->requestedPlugin;
}

void Message::setRequestedPlugin(const QString &requestedPlugin)
{
	d->requestedPlugin = requestedPlugin;
}

QColor Message::foregroundColor() const
{
	return d->foregroundColor;
}

QColor Message::backgroundColor() const
{
	return d->backgroundColor;
}

bool Message::isRightToLeft() const
{
	return d->isRightToLeft;
}

QFont Message::font() const
{
	return d->font;
}

QString Message::subject() const
{
	return d->subject;
}

void Message::setSubject(const QString &subject)
{
	d->subject = subject;
}

const QTextDocument *Message::body() const
{
	return d->body;
}

Qt::TextFormat Message::format() const
{
	return d->format;
}

Message::MessageDirection Message::direction() const
{
	return d->direction;
}

void Message::setDirection(MessageDirection direction)
{
	d->direction = direction;
}

Message::MessageImportance Message::importance() const
{
	return d->importance;
}

Message::MessageState Message::state() const
{
	return d->state;
}

void Message::setState(MessageState state)
{
	d->state = state;
}

ChatSession *Message::manager() const
{
	return d->manager;
}

void Message::setManager(ChatSession *kmm)
{
	d->manager=kmm;
}

QString Message::getHtmlStyleAttribute() const
{
	QString styleAttribute;

	styleAttribute = QString::fromUtf8("style=\"");

	if( !d->formattingOverride)
	{
		// Affect foreground(color) and background color to message.
		// we only do this if the formatting won't get stripped anyway
		if( d->foregroundColor.isValid() )
		{
			styleAttribute += QString::fromUtf8("color: %1; ").arg(d->foregroundColor.name());
		}
		if( d->backgroundColor.isValid() )
		{
			styleAttribute += QString::fromUtf8("background-color: %1; ").arg(d->backgroundColor.name());
		}

		// Affect font parameters.
		if( d->font!=QFont() )
		{
			QString fontstr;
			if(!d->font.family().isNull())
				fontstr+=QLatin1String("font-family: ")+d->font.family()+QLatin1String("; ");
			if(d->font.italic())
				fontstr+=QLatin1String("font-style: italic; ");
			if(d->font.strikeOut())
				fontstr+=QLatin1String("text-decoration: line-through; ");
			if(d->font.underline())
				fontstr+=QLatin1String("text-decoration: underline; ");
			if(d->font.bold())
				fontstr+=QLatin1String("font-weight: bold;");

			styleAttribute += fontstr;
		}
	}

	styleAttribute += QString::fromUtf8("\"");

	return styleAttribute;
}

void Message::setFileTransferDisabled( bool disabled )
{
	if ( !d->fileTransfer )
		d->fileTransfer = new Message::Private::FileTransferInfo();

	d->fileTransfer->disabled = disabled;
}

bool Message::fileTransferDisabled() const
{
	return ( d->fileTransfer ) ? d->fileTransfer->disabled : false;
}

void Message::setFileName( const QString &fileName )
{
	if ( !d->fileTransfer )
		d->fileTransfer = new Message::Private::FileTransferInfo();

	d->fileTransfer->fileName = fileName;
}

QString Message::fileName() const
{
	return ( d->fileTransfer ) ? d->fileTransfer->fileName : QString();
}

void Message::setFileSize( unsigned long size )
{
	if ( !d->fileTransfer )
		d->fileTransfer = new Message::Private::FileTransferInfo();

	d->fileTransfer->fileSize = size;
}

unsigned long Message::fileSize() const
{
	return ( d->fileTransfer ) ? d->fileTransfer->fileSize : 0;
}

void Message::setFilePreview( const QPixmap &preview )
{
	if ( !d->fileTransfer )
		d->fileTransfer = new Message::Private::FileTransferInfo();

	d->fileTransfer->filePreview = preview;
}

QPixmap Message::filePreview() const
{
	return ( d->fileTransfer ) ? d->fileTransfer->filePreview : QPixmap();
}

// prime candidate for removal
#if 0
QString Message::decodeString( const QByteArray &message, const QTextCodec *providedCodec, bool *success )
{
	/*
	Note to everyone. This function is not the most efficient, that is for sure.
	However, it *is* the only way we can be guaranteed that a given string is
	decoded properly.
	*/

	if( success )
		*success = true;

	// Avoid heavy codec tests on empty message.
	if( message.isEmpty() )
            return QString::fromAscii( message );

	//Check first 128 chars
	int charsToCheck = message.length();
	charsToCheck = 128 > charsToCheck ? charsToCheck : 128;

#ifdef __GNUC__
	#warning Rewrite the following code: heuristicContentMatch() do not existe anymore.
#endif
	//They are providing a possible codec. Check if it is valid
//	if( providedCodec && providedCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		return providedCodec->toUnicode( message );
	}

        //NOTE see KEncodingDetector@kdecore
	//Check if it is UTF
	if( KStringHandler::isUtf8(message) )
	{
		//We have a UTF string almost for sure. At least we know it will be decoded.
		return QString::fromUtf8( message );
	}
/*
	//Try codecForContent - exact match
	QTextCodec *testCodec = QTextCodec::codecForContent(message, charsToCheck);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		return testCodec->toUnicode( message );
	}

	kWarning(14000) << "Unable to decode string using provided codec(s), taking best guesses!";
	if( success )
		*success = false;

	//We don't have any clues here.

	//Try local codec
	testCodec = QTextCodec::codecForLocale();
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kDebug(14000) << "Using locale's codec";
		return testCodec->toUnicode( message );
	}

	//Try latin1 codec
	testCodec = QTextCodec::codecForMib(4);
	if( testCodec && testCodec->heuristicContentMatch( message, charsToCheck ) >= charsToCheck )
	{
		//All chars decodable.
		kDebug(14000) << "Using latin1";
		return testCodec->toUnicode( message );
	}

	kDebug(14000) << "Using latin1 and cleaning string";
	//No codec decoded. Just decode latin1, and clean out any junk.
	QString result = QLatin1String( message );
	const uint length = message.length();
	for( uint i = 0; i < length; ++i )
	{
		if( !result[i].isPrint() )
			result[i] = '?';
	}

	return result;
*/
	return QString();
}
#endif

QStringList Message::classes() const
{
	return d->classes;
}

void Message::addClass(const QString & classe)
{
	d->classes.append(classe);
}

void Message::setClasses(const QStringList & classes)
{
	d->classes = classes;
}

}

bool Kopete::Message::delayed() const
{
	return d->delayed;
}

void Kopete::Message::setDelayed(bool delay)
{
	d->delayed = delay;
}
