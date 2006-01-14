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

#include <knotifyclient.h>
#include <kdebug.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qstylesheet.h>
#include "ksparser.h"
#include <stdlib.h>

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

const QRegExp KSParser::sm_colorsModeRegexp("(\\d{1,2})(?:,(\\d{1,2}))?");

template <typename _TYPE_>
	inline void swap(_TYPE_ &o1, _TYPE_ &o2)
{
	_TYPE_ tmp = o1;
	o1 = o2;
	o2 = tmp;
}

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
	return m_parser._parse(message);
}

QCString KSParser::_parse(const QCString &message)
{
	QCString data( message.size() * 2 );
	QBuffer buff( data );
	buff.open( IO_WriteOnly );

	m_tags.clear();
	m_attributes.clear();

	QRegExp colorsModeRegexp(sm_colorsModeRegexp);

	// should be set to the current default colors ....
	QColor fgColor; /*KopeteMesage::fg().name()*/
	QColor bgColor; /*KopeteMesage::bg().name()*/

	uint chars = 0;
	for(uint i = 0; i < message.length(); ++i)
	{
		const QChar &cur = message[i];
		QString toAppend;

		switch (cur)
		{
			case 0x02:	//Bold: ^B
				toAppend= toggleTag("b");
				break;
			case 0x03:	//Color code: ^C
				if (colorsModeRegexp.search(message, i+1) == (int)i+1)
				{
					i += colorsModeRegexp.matchedLength(); // + 1 will be added by ++
					QString tagStyle;

					fgColor = ircColor(colorsModeRegexp.cap(1));
					bgColor = ircColor(colorsModeRegexp.cap(2));

					toAppend = pushColorTag(fgColor, bgColor);
				}
				else
				{
					toAppend = popTag(QString::fromLatin1("span"));
				}
				break;
			case 0x07:	//System bell: ^G
				KNotifyClient::beep( QString::fromLatin1("IRC beep event received in a message") );
				break;
			case '\t':	// 0x09
				toAppend = QString::fromLatin1("&nbsp;&nbsp;&nbsp;&nbsp;");
				break;
			case '\n':	// 0x0D
				toAppend= QString::fromLatin1("<br/>");
				break;
			case 0x0D:	// Italics: ^N
				toAppend = toggleTag("i");
				break;
			case 0x0F:	//Plain Text, close all tags: ^O
				toAppend = popAll();
				break;
	//		case 0x12:	// Reverse original text colors: ^R
	//			break;
			case 0x16:	//Invert Colors: ^V
				swap(fgColor, bgColor);
				toAppend = pushColorTag(fgColor, bgColor);
				break;
			case 0x1F:	//Underline
				toAppend = toggleTag("u");
				break;
			case '<':
				toAppend = QString::fromLatin1("&lt;");
				break;
			case '>':
				toAppend = QString::fromLatin1("&gt;");
				break;
			default:
				if (cur < QChar(' ')) // search for control characters
					toAppend = QString::fromLatin1("&lt;%1&gt;").arg(cur, 2, 16).upper();
				else
					toAppend = QStyleSheet::escape(cur);
		}

		chars += toAppend.length();
		buff.writeBlock( toAppend.latin1(), toAppend.length() );
	}

	QString toAppend = popAll();
	chars += toAppend.length();
	buff.writeBlock( toAppend.latin1(), toAppend.length() );

	// Make sure we have enough room for NULL character.
	if (data.size() < chars+1)
		data.resize(chars+1);

	data[chars] = '\0';

	return data;
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

QString KSParser::pushColorTag(const QColor &fgColor, const QColor &bgColor)
{
	QString tagStyle;

	if (fgColor.isValid())
		tagStyle += QString::fromLatin1("color:%1;").arg(fgColor.name());
	if (bgColor.isValid())
		tagStyle += QString::fromLatin1("background-color:%1;").arg(bgColor.name());

	if(!tagStyle.isEmpty())
		tagStyle = QString::fromLatin1("style=\"%1\"").arg(tagStyle);

	return pushTag(QString::fromLatin1("span"), tagStyle);;
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

QColor KSParser::ircColor(const QString &color)
{
	bool success;
	unsigned int intColor = color.toUInt(&success);

	if (success)
		return ircColor(intColor);
	else
		return QColor();
}

QColor KSParser::ircColor(unsigned int color)
{
	unsigned int maxcolor = sizeof(IRC_Colors)/sizeof(QColor);
	return color<=maxcolor?IRC_Colors[color]:IRC_Colors[maxcolor];
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
