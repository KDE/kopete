/*
  aimcontactbase.cpp  -  AIM Contact Base

  Copyright (c) 2003 by Will Stephenson
  Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/
#include "aimcontactbase.h"

#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>

#include <QtGui/QTextDocument>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextBlock>

#include "kopetechatsession.h"

#include "oscaraccount.h"

//liboscar
#include "oscarutils.h"

AIMContactBase::AIMContactBase( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon )
: OscarContact(account, name, parent, icon )
{	
	m_mobile = false;
	// Set the last autoresponse time to the current time yesterday
	m_lastAutoresponseTime = QDateTime::currentDateTime().addDays(-1);
}

AIMContactBase::~AIMContactBase()
{
}

void AIMContactBase::sendAutoResponse(Kopete::Message& msg)
{
	// The target time is 2 minutes later than the last message
	int delta = m_lastAutoresponseTime.secsTo( QDateTime::currentDateTime() );
	kDebug(OSCAR_GEN_DEBUG) << "Last autoresponse time: " << m_lastAutoresponseTime;
	kDebug(OSCAR_GEN_DEBUG) << "Current time: " << QDateTime::currentDateTime();
	kDebug(OSCAR_GEN_DEBUG) << "Difference: " << delta;
	// Check to see if we're past that time
	if(delta > 120)
	{
		kDebug(OSCAR_GEN_DEBUG) << "Sending auto response";
		
		// This code was yoinked straight from OscarContact::slotSendMsg()
		// If only that slot wasn't private, but I'm not gonna change it right now.
		Oscar::Message message;
		
		if ( m_details.hasCap( CAP_UTF8 ) )
		{
			message.setText( Oscar::Message::UCS2, msg.plainBody() );
		}
		else
		{
			QTextCodec* codec = contactCodec();
			message.setText( Oscar::Message::UserDefined, msg.plainBody(), codec );
		}
		
		message.setTimestamp( msg.timestamp() );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		message.setChannel( 0x01 );
		
		// isAuto defaults to false
		mAccount->engine()->sendMessage( message, true);
		kDebug(OSCAR_GEN_DEBUG) << "Sent auto response";
		manager(Kopete::Contact::CanCreate)->appendMessage(msg);
		manager(Kopete::Contact::CanCreate)->messageSucceeded();
		// Update the last autoresponse time
		m_lastAutoresponseTime = QDateTime::currentDateTime();
	}
	else
	{
		kDebug(OSCAR_GEN_DEBUG) << "Not enough time since last autoresponse, NOT sending";
	}
}

void AIMContactBase::slotSendMsg(Kopete::Message& message, Kopete::ChatSession *)
{
	Oscar::Message msg;
	QString s;
	
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;
	//okay, now we need to change the message.escapedBody from real HTML to aimhtml.
	//looking right now for docs on that "format".
	//looks like everything except for alignment codes comes in the format of spans
	
	//font-style:italic -> <i>
	//font-weight:600 -> <b> (anything > 400 should be <b>, 400 is not bold)
	//text-decoration:underline -> <u>
	//font-family: -> <font face="">
	//font-size:xxpt -> <font ptsize=xx>

	QTextDocument doc;
	doc.setHtml( message.escapedBody() );

	bool hasFontTag = false;
	QTextCharFormat defaultCharFormat;
	for ( QTextBlock it = doc.begin(); it != doc.end(); it = it.next() )
	{
		s += brMargin( it.blockFormat().topMargin(), defaultCharFormat.fontPointSize() );

		for ( QTextBlock::iterator it2 = it.begin(); !(it2.atEnd()); ++it2 )
		{
			QTextFragment currentFragment = it2.fragment();
			if ( currentFragment.isValid() )
			{
				QTextCharFormat format = currentFragment.charFormat();
				if ( format.fontFamily() != defaultCharFormat.fontFamily() ||
				     format.foreground() != defaultCharFormat.foreground() ||
				     aimFontSize(format.fontPointSize()) != aimFontSize(defaultCharFormat.fontPointSize()) )
				{
					if ( hasFontTag )
					{
						s += "</FONT>";
						hasFontTag = false;
					}
					
					QString fontTag;
					if ( !format.fontFamily().isEmpty() )
						fontTag += QString( " FACE=\"%1\"" ).arg( format.fontFamily() );
					if ( format.fontPointSize() > 0 )
						fontTag += QString( " SIZE=%1" ).arg( aimFontSize( format.fontPointSize() ) );
					if ( format.foreground().style() != Qt::NoBrush )
						fontTag += QString( " COLOR=%1" ).arg( format.foreground().color().name() );
					
					if ( !fontTag.isEmpty() )
					{
						s += QString("<FONT%1>").arg( fontTag );
						hasFontTag = true;
					}
				}
				
				if ( format.font().bold() != defaultCharFormat.font().bold() )
					s += ( format.font().bold() ) ? "<B>" : "</B>";
				if ( format.fontItalic() != defaultCharFormat.fontItalic() )
					s += ( format.hasProperty(QTextFormat::FontItalic) ) ? "<I>" : "</I>";
				if ( format.fontUnderline() != defaultCharFormat.fontUnderline() )
					s += ( format.hasProperty(QTextFormat::FontUnderline) ) ? "<U>" : "</U>";
				
				s += Qt::escape(currentFragment.text());
				defaultCharFormat = format;
			}
		}
		s += brMargin( it.blockFormat().bottomMargin(), defaultCharFormat.fontPointSize(), true );
	}

	s.replace( QChar::LineSeparator, "<BR>" );

	if ( s.endsWith( "<BR>" ) )
		s.chop(4);

	if ( hasFontTag )
		s += "</FONT>";
	if ( defaultCharFormat.font().bold() )
		s += "</B>";
	if ( defaultCharFormat.hasProperty( QTextFormat::FontItalic ) )
		s += "</I>";
	if ( defaultCharFormat.hasProperty( QTextFormat::FontUnderline ) )
		s += "</U>";

#if 0
	s=message.escapedBody();
	s.replace ( QRegExp( QString::fromLatin1("<span style=\"([^\"]*)\">([^<]*)</span>")),
	            QString::fromLatin1("<style>\\1;\"\\2</style>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-style:italic;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<i><style>\\1\\2\"\\3</style></i>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-weight:600;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<b><style>\\1\\2\"\\3</style></b>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)text-decoration:underline;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<u><style>\\1\\2\"\\3</style></u>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-family:([^;]*);([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<font face=\"\\2\"><style>\\1\\3\"\\4</style></font>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-size:([^p]*)pt;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<font ptsize=\"\\2\"><style>\\1\\3\"\\4</style></font>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)color:([^;]*);([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<font color=\"\\2\"><style>\\1\\3\"\\4</style></font>"));
	
	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("\\2"));
	
	//okay now change the <font ptsize="xx"> to <font size="xx">
	
	//0-9 are size 1
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"\\d\">")),
	            QString::fromLatin1("<font size=\"1\">"));
	//10-11 are size 2
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[01]\">")),
	            QString::fromLatin1("<font size=\"2\">"));
	//12-13 are size 3
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[23]\">")),
	            QString::fromLatin1("<font size=\"3\">"));
	//14-16 are size 4
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[456]\">")),
	            QString::fromLatin1("<font size=\"4\">"));
	//17-22 are size 5
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"(?:1[789]|2[012])\">")),
	            QString::fromLatin1("<font size=\"5\">"));
	//23-29 are size 6
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"2[3456789]\">")),QString::fromLatin1("<font size=\"6\">"));
	//30- (and any I missed) are size 7
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"[^\"]*\">")),QString::fromLatin1("<font size=\"7\">"));
	
	s.replace ( QRegExp ( QString::fromLatin1("<br[ /]*>")), QString::fromLatin1("<br>") );
#endif

	kDebug(OSCAR_GEN_DEBUG) << "sending " << s;
	
	// XXX Need to check for message size?
	
	// Allow UCS2 because official AIM client doesn't sets the CAP_UTF8 anymore!
	bool allowUCS2 = !isOnline() || !(m_details.userClass() & Oscar::CLASS_ICQ) || m_details.hasCap( CAP_UTF8 );
	msg.setText( Oscar::Message::encodingForText( s, allowUCS2 ), s, contactCodec() );
	
	msg.setId( message.id() );
	msg.setReceiver(mName);
	msg.setTimestamp(message.timestamp());
	msg.setChannel(0x01);
	
	mAccount->engine()->sendMessage(msg);
	
	message.setState( Kopete::Message::StateSending );
	// Show the message we just sent in the chat window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

int AIMContactBase::aimFontSize( int size ) const
{
	if ( size <= 0 )
		return 0;
	else if ( 1 <= size && size <= 9 )
		return 1;
	else if ( 10 <= size && size <= 11 )
		return 2;
	else if ( 12 <= size && size <= 13 )
		return 3;
	else if ( 14 <= size && size <= 16 )
		return 4;
	else if ( 17 <= size && size <= 22 )
		return 5;
	else if ( 23 <= size && size <= 29 )
		return 6;
	else
		return 7;
}

QString AIMContactBase::brMargin( int margin, int fontPointSize, bool endBlock ) const
{
	int brHeight = ( fontPointSize == 0 ) ? 12 : fontPointSize;
	int brCount = margin / brHeight;

	if ( brCount <= 0 )
		return ( endBlock ) ? "<BR>" : "";

	QString s;
	while ( brCount-- > 0 )
		s += "<BR>";

	return s;
}

#include "aimcontactbase.moc"
//kate: tab-width 4; indent-mode csands;
