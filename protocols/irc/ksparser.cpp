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

#include <kopetenotifyclient.h>
#include <kdebug.h>
#include <qstylesheet.h>
#include "ksparser.h"

KSParser KSParser::m_parser;

const QColor KSParser::IRC_Colors[17]=
{
	Qt::white,
	Qt::black,
	Qt::darkBlue,
	Qt::darkGreen,
	Qt::red,
	Qt::darkRed,
	Qt::darkMagenta,
	Qt::darkYellow,
	Qt::yellow,
	Qt::green,
	Qt::darkCyan,
	Qt::cyan,
	Qt::blue,
	Qt::magenta,
	Qt::darkGray,
	Qt::gray,
	QColor() // default invalid color, must be the last
};

KSParser::KSParser()
{
	kdDebug(14120) << k_funcinfo << endl;
}

KSParser::~KSParser()
{
	kdDebug(14120) << k_funcinfo << endl;
}

/* NOTE: If thread corruption are seen simply ad a qlock here */
QCString KSParser::parse(const QCString &message)
{
	return m_parser._parse( QString::fromLatin1(message) ).latin1();
}

QString KSParser::_parse(const QString &message)
{
	QString res;
	m_tags.clear();
	m_attributes.clear();

	for(uint i=0; i<message.length(); i++)
	{
		switch( message[i].latin1() )
		{
		case 3:		//Color code
			if(message[++i].digitValue()>-1)
			{
				QString tagStyle;
				QString fgColor = QString( message[i] );
				if( message[++i].digitValue() > -1 )
					fgColor += message[i];

				QString bgColor;
				if( message[i] == ',' )
				{
					bgColor = QString( message[i] );
					if( message[++i].digitValue() > -1 )
						bgColor += message[i];
				}

				QColor c = ircColor(fgColor.toInt());
				if ( c.isValid() )
					tagStyle += QString::fromLatin1("color:%1;").arg(c.name());

				if(!bgColor.isEmpty())
				{
					c = ircColor(bgColor.toInt());
					if(c.isValid())
						tagStyle += QString::fromLatin1("background-color:%1;").arg(c.name());
				}

				if(!tagStyle.isEmpty())
					res += pushTag(QString::fromLatin1("span"), QString::fromLatin1("style=\"%1\"").arg( tagStyle ) );

				i--;
			}
			else
				res += popTag( QString::fromLatin1("span") );
			break;
		case 2:		//Bold
			res += toggleTag("b");
			break;
		case 6:		//Invert Colors
			break;
		case 7:		//System bell
			KNotifyClient::beep( QString::fromLatin1("IRC beep event received in a message") );
			break;
		case '\t':
			res += QString::fromLatin1("&nbsp;&nbsp;&nbsp;&nbsp;");
			break;
		case '\n':
			res += QString::fromLatin1("<br/>");
			break;
		case 15:	//Plain Text, close all tags
			res.append( popAll() );
			break;
		case 31: 	//Underline
			res += toggleTag("u");
			break;
		case '<':
			res += QString::fromLatin1("&lt;");
			break;
		case '>':
			res += QString::fromLatin1("&gt;");
			break;
		default:
			res += QStyleSheet::escape(message[i]);
		}
	}
	res.append( popAll() );
	return res;
}

QString KSParser::pushTag(const QString &tag, const QString &attributes)
{
	QString res;
	m_tags.push(tag);
	if(!m_attributes.contains(tag))
		m_attributes.insert(tag, attributes);
	else if(!attributes.isEmpty())
		m_attributes.replace(tag, attributes);
	res.append("<" + tag);
	if(!m_attributes[tag].isEmpty())
		res.append(" " + m_attributes[tag]);
	return res + ">";
}

QString KSParser::popTag(const QString &tag)
{
	if (!m_tags.contains(tag))
		return QString::null;

	QString res;
	QValueStack<QString> savedTags;
	while(m_tags.top() != tag)
	{
		savedTags.push(m_tags.pop());
		res.append("</" + savedTags.top() + ">");
	}
	res.append("</" + m_tags.pop() + ">");
	m_attributes.remove(tag);
	while(!savedTags.isEmpty())
		res.append(pushTag(savedTags.pop()));
	return res;
}

QString KSParser::toggleTag(const QString &tag)
{
	return m_attributes.contains(tag)?popTag(tag):pushTag(tag);
}

QString KSParser::popAll()
{
	QString res;
	while(!m_tags.isEmpty())
		res.append("</" + m_tags.pop() + ">");
	m_attributes.clear();
	return res;
}

int KSParser::colorForHTML(const QString &html)
{
	QColor color(html);
	for(uint i=0; i<sizeof(IRC_Colors)/sizeof(QColor); i++)
	{
		if(IRC_Colors[i] == color)
			return i;
	}
	return -1;
}
