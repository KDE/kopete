/* This file is part of ksirc
   Copyright (c) 2001 Malte Starostik <malte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
Color parser code courtesy of ksirc <http://www.kde.org>
Modified by Jason Keirstead <jason@keirstead.org>
*/

// $Id$

#include <qstring.h>
#include <qcolor.h>
#include <qregexp.h>
#include <knotifyclient.h>
#include <kdebug.h>
#include "ksparser.h"

QString KSParser::parse( const QString &message )
{
	QString res;
	m_tags.clear();
	m_attributes.clear();

	for (uint i = 0; i < message.length(); i++)
	{
		QChar ch = message[i];

		switch( ((int)ch) )
		{
			case 3:
			{
				//Color code
				if ( message[++i].digitValue() > -1 )
				{
					QString tagStyle;
					QString fgColor = QString( message[i] );
					while( message[++i].digitValue() > -1 )
						fgColor += message[i];

					QString bgColor;
					if( message[i] == ',' )
					{
						while( message[++i].digitValue() > -1 )
							bgColor += message[i];
					}

					QColor c = ircColor( fgColor.toInt() );
					if ( c.isValid() )
						tagStyle += QString::fromLatin1( "color:%1;" ).arg( c.name() );

					if( !bgColor.isEmpty() )
					{
						c = ircColor( bgColor.toInt() );
						if ( c.isValid() )
							tagStyle += QString::fromLatin1( "background-color:%1;" ).arg( c.name() );
					}

					if( !tagStyle.isEmpty() )
						res += pushTag(  QString::fromLatin1("span"), QString::fromLatin1("style=\"%1\"").arg( tagStyle ) );

					i--;
				}
				else
					res += popTag( QString::fromLatin1("span") );
				break;
			}
			case 2:
				//Bold
				res += toggleTag("b");
				break;
			case 6:
				//Invert Colors
				break;
			case 7:
				//System bell
				KNotifyClient::beep( QString::fromLatin1("IRC beep event recieved in a message") );
				break;

			case 15:
				//Plain Text, close all tags
				res.append( popAll() );
				break;

			case 31:
				//Underline
				res += toggleTag("u");
				break;

			default:
				res += ch;
		}
	}

	res.append( popAll() );

	return res;
}

QString KSParser::pushTag(const QString &tag, const QString &attributes)
{
    QString res;
    m_tags.push(tag);
    if (!m_attributes.contains(tag))
        m_attributes.insert(tag, attributes);
    else if (!attributes.isEmpty())
        m_attributes.replace(tag, attributes);
    res.append("<" + tag);
    if (!m_attributes[tag].isEmpty())
        res.append(" " + m_attributes[tag]);
    return res + ">";
}

QString KSParser::popTag(const QString &tag)
{
    if (!m_tags.contains(tag))
        return QString::null;

    QString res;
    QValueStack<QString> savedTags;
    while (m_tags.top() != tag)
    {
        savedTags.push(m_tags.pop());
        res.append("</" + savedTags.top() + ">");
    }
    res.append("</" + m_tags.pop() + ">");
    m_attributes.remove(tag);
    while (!savedTags.isEmpty())
        res.append(pushTag(savedTags.pop()));
    return res;
}

QString KSParser::toggleTag(const QString &tag)
{
    return m_attributes.contains(tag) ? popTag(tag) : pushTag(tag);
}

QString KSParser::popAll()
{
    QString res;
    while (!m_tags.isEmpty())
        res.append("</" + m_tags.pop() + ">");
    m_attributes.clear();
    return res;
}

QColor KSParser::ircColor(int code)
{
	switch( code )
	{
		case 0:
			return Qt::white;
		case 1:
			return Qt::black;
		case 2:
			return Qt::darkBlue;
		case 3:
			return Qt::darkGreen;
		case 4:
			return Qt::red;
		case 5:
			return Qt::darkRed;
		case 6:
			return Qt::darkMagenta;
		case 7:
			return Qt::darkYellow;
		case 8:
			return Qt::yellow;
		case 9:
			return Qt::green;
		case 10:
			return Qt::darkCyan;
		case 11:
			return Qt::cyan;
		case 12:
			return Qt::blue;
		case 13:
			return Qt::magenta;
		case 14:
			return Qt::darkGray;
		case 15:
			return Qt::gray;
		default:
			return QColor();
	}
}

int KSParser::colorForHTML( const QString &html )
{
	QColor color;
	color.setNamedColor( html );

	for( int i=0; i < 16; i++ )
	{
		if( ircColor( i ) == color )
			return i;
	}

	return -1;
}
