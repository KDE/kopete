/*
    historylogger.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historylogger.h"

#include <qdom.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>
#include <qptrlist.h>


#include <kdebug.h>
#include <klocale.h>
//#include <kiconloader.h>
#include <kstandarddirs.h>


#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopetemessage.h"

#define CBUFLENGTH 512 // buffer length for fgets() //FIXME: i don't like this

HistoryLogger::HistoryLogger( KopeteContact *c , QObject *parent , const char *name )
 : QObject(parent, name)
{
	m_hideOutgoing=false;
	m_reversed=false;
	m_currentPos=-1;

	QString logFileName = c->protocol()->pluginId() + QString::fromLatin1( "/" ) +
		c->contactId().replace( QRegExp( QString::fromLatin1( "[./~]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( ".log" );

	m_logFileName = locateLocal( "data", QString::fromLatin1( "kopete/" ) + logFileName ) ;


	xmllist = new QDomDocument( QString::fromLatin1( "History" ) );

	messages = countMessages();
}


HistoryLogger::~HistoryLogger()
{
}


void HistoryLogger::readLog(int msgStart , int msgCount)
{
	//From the kopete 0.6 kopetehistorywidget
    //  by Richard Stellingwerff <remenic@linuxfromscratch.org>

	if(messages==0)
		return; //no messages

	QDomElement msgelement;
	QDomNode node;

	KopeteMessage::MessageDirection dir;

	QString body, date, nick;
	QString buffer, msgBlock;

	char cbuf[CBUFLENGTH]; // buffer for the log file

	int currentPos=0;
	int startPos;
	int endPos;

	if (  m_reversed )
	{
		startPos = messages - (msgCount + msgStart);
		endPos = messages - msgStart;
	}
	else
	{
		startPos = msgStart;
		endPos = msgStart + msgCount;
	}

	if (startPos < 0)
		startPos = 0;

	if (endPos > messages)
		endPos = messages;

	m_currentPos=startPos;

//	kdDebug(14010) << k_funcinfo << "messages=" <<messages << " msgStar=" <<msgStart << " msgCount=" << msgCount  << " startPos=" << startPos << " endPod=" << endPos<< endl;

//	int steps = msgCount / 10; // split the progressbar into 10 parts.
//	int sPos = 0;

//	if (steps < 0) steps = 1;



//	mProgress->setTotalSteps(10);

	// show the user that it's being processed (i know, this method is ugly)
//	mProgress->setProgress(1);

	//Loop through the logfiles in the list, process each one
//	for ( QStringList::Iterator it = logFileNames.begin(); it != logFileNames.end(); ++it )
//	{
		QString logFileName = m_logFileName;//*it;

		// open the file
		FILE *f = fopen(QFile::encodeName(logFileName), "r");
		if(f==NULL)
			return;

		// find the first message to display ( this needs to be faster! )
		while ( !feof( f ) && currentPos != startPos)
		{
			fgets(cbuf, CBUFLENGTH, f);

			buffer = QString::fromUtf8(cbuf);

			while ( strchr(cbuf, '\n') == NULL && !feof(f) ) // Keep reading till we got one line.
			{
				fgets( cbuf, CBUFLENGTH, f );
				buffer += QString::fromUtf8(cbuf);
			}

			if( buffer.startsWith( QString::fromLatin1( "<message " ) ) && ( !m_hideOutgoing ||buffer.contains( QString::fromLatin1( "direction=\"inbound\"" ) ) ) )
				currentPos++;
		}

		currentPos++;

		// create a new <message> block
		while ( ! feof( f ) && currentPos <= endPos)
		{
			fgets(cbuf, CBUFLENGTH, f);
			buffer = QString::fromUtf8(cbuf);

			while ( strchr(cbuf, '\n') == NULL && !feof(f) )
			{
				fgets( cbuf, CBUFLENGTH, f );
				buffer += QString::fromUtf8(cbuf);
			}

			if( buffer.startsWith( QString::fromLatin1( "<message " ) ) )
			{
				msgBlock = buffer;

				// find the end of the message block
				while( !feof( f ) && buffer != QString::fromLatin1( "</message>\n" ) /*strcmp("</message>\n", cbuf )*/ )
				{
					fgets(cbuf, CBUFLENGTH, f);
					buffer = QString::fromUtf8(cbuf);

					while ( strchr(cbuf, '\n') == NULL && !feof(f) )
					{
						fgets( cbuf, 511, f );
						buffer += QString::fromUtf8(cbuf);
					}

					msgBlock.append(buffer);
				}

				// now let's work on this new block
				xmllist->setContent(msgBlock, false);
				msgelement = xmllist->documentElement();
				node = msgelement.firstChild();

				if( msgelement.attribute( QString::fromLatin1( "direction" ) ) == QString::fromLatin1( "inbound" ) )
					dir = KopeteMessage::Inbound;
				else
				{
					dir = KopeteMessage::Outbound;

					// skip outbound messages if only incoming messages want to be seen
					if (m_hideOutgoing)
						continue;
				}

				// Read all the elements.
				QString tagname;
				QDomElement element;

				while ( ! node.isNull() )
				{

					if ( node.isElement() )
					{
						element = node.toElement();
						tagname = element.tagName();

						if( dir == KopeteMessage::Inbound && tagname == QString::fromLatin1( "srcnick" ) )
							nick = element.text();
						else if( dir == KopeteMessage::Outbound && tagname == QString::fromLatin1( "destnick" ) )
							nick = element.text();
						else if( tagname == QString::fromLatin1( "date" ) )
							date = element.text();
						else if( tagname == QString::fromLatin1( "body" ) )
							body = element.text().stripWhiteSpace();
					}

					node = node.nextSibling();
				}

				// add the message to the history view
				kdDebug(14010) << k_funcinfo << "post message " << body << endl;
				emit addMessage(dir, nick, date, body);
				currentPos++;

				// update the progress bar
//				sPos++;
//				mProgress->setProgress( sPos / steps );
			}
		}

		fclose( f );
//	}

//	mProgress->reset();

}

int HistoryLogger::countMessages()
{
	//From the kopete 0.6 kopetehistorywidget
    //  by Richard Stellingwerff <remenic@linuxfromscratch.org>

	QString buffer;
	char cbuf[CBUFLENGTH];
	int n = 0;

//	for ( QStringList::Iterator it = logFileNames.begin(); it != logFileNames.end(); ++it )
//	{
		//Check if thie file exists
		FILE *f = fopen(QFile::encodeName(m_logFileName), "r");

		// FIXME: fopen instead of QFile ??????? - Martijn
		if (f == NULL)
		{
			// Oops, no file
		}
		else
		{
			while ( !feof(f) )
			{
				fgets(cbuf, CBUFLENGTH, f);
				buffer = cbuf;

				while ( strchr(cbuf, '\n') == NULL && !feof( f ) )
				{
					fgets(cbuf, CBUFLENGTH, f);
					buffer += QString::fromUtf8( cbuf );
				}

				if ( /*buffer.startsWith("<message ")*/ !strncmp("<message ", cbuf, 9 ) && (!m_hideOutgoing ||  buffer.contains( QString::fromLatin1( "inbound" ) )  ))
					n++;
				else
					continue;
			}

			// Close file only when it exists
			fclose(f);
		}
//	}

	return n;
}

void HistoryLogger::setReversed(bool b)
{
	m_reversed=b;
}

void HistoryLogger::setHideOutgoing(bool b)
{
	m_hideOutgoing=b;
	messages=countMessages();
}


#include "historylogger.moc"
