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

#include "ksparser.h"

#include <knotifyclient.h>
//#include <kdebug.h>

#include <QBuffer>
#include <QList>
#include <QMap>
#include <QTextDocument> // for Qt::escape

typedef struct
{
        QList<QString> tags; // stack
        QMap<QString, QString> attributes;
} ParserData;

static const QColor s_IRC_Colors[17]=
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

static const QRegExp s_colorsModeRegexp("(\\d{1,2})(?:,(\\d{1,2}))?");

template <typename _TYPE_>
	inline void swap(_TYPE_ &o1, _TYPE_ &o2)
{
	_TYPE_ tmp = o1;
	o1 = o2;
	o2 = tmp;
}

static QString pushTag(ParserData *d, const QString &tag, const QString &attributes = QString::null)
{
	QString res;
	d->tags.append(tag);
	if(!d->attributes.contains(tag))
		d->attributes.insert(tag, attributes);
	else if(!d->attributes.isEmpty())
		d->attributes.replace(tag, attributes);
	res.append("<" + tag);
	if(!d->attributes[tag].isEmpty())
		res.append(" " + d->attributes[tag]);
	return res + ">";
}

static QString pushColorTag(ParserData *d, const QColor &fgColor, const QColor &bgColor)
{
	QString tagStyle;

	if (fgColor.isValid())
		tagStyle += QString::fromLatin1("color:%1;").arg(fgColor.name());
	if (bgColor.isValid())
		tagStyle += QString::fromLatin1("background-color:%1;").arg(bgColor.name());

	if(!tagStyle.isEmpty())
		tagStyle = QString::fromLatin1("style=\"%1\"").arg(tagStyle);

	return pushTag(d, QString::fromLatin1("span"), tagStyle);;
}

static QString popTag(ParserData *d, const QString &tag)
{
	if (!d->tags.contains(tag))
		return QString::null;
/*
	QString res;
	QList<QString> savedTags;

	while(d->tags.top() != tag)
	{
		savedTags.push(d->tags.pop_back());
		res.append("</" + savedTags.top() + ">");
	}
	res.append("</" + d->tags.pop_back() + ">");
	d->attributes.remove(tag);
	while(!savedTags.isEmpty())
		res.append(pushTag(savedTags.pop_back()));
	return res;
*/
	return QString::null;
}

static QString popAll(ParserData *d)
{
	QString res;
	while(!d->tags.isEmpty())
		res.append("</" + d->tags.takeLast() + ">");
	d->attributes.clear();
	return res;
}

static QString toggleTag(ParserData *d, const QString &tag)
{
        return d->attributes.contains(tag)?popTag(d, tag):pushTag(d, tag);
}

QString KSParser::parse(QString message)
{
	QString ret;
	ParserData d;

	QRegExp colorsModeRegexp(s_colorsModeRegexp);

	// should be set to the current default colors ....
	QColor fgColor; /*KopeteMesage::fg().name()*/
	QColor bgColor; /*KopeteMesage::bg().name()*/

	message = Qt::escape(message);
	for(uint i = 0; i<message.length(); ++i)
	{
		QChar car = message[i];
		switch (car.unicode())
		{
		case 0x02:	//Bold: ^B
			ret += toggleTag(&d, "b");
			break;
		case 0x03:	//Color code: ^C
			if (colorsModeRegexp.search(message, i+1) == (int)i+1)
			{
				i += colorsModeRegexp.matchedLength(); // + 1 will be added by ++
				QString tagStyle;

				fgColor = ircColor(colorsModeRegexp.cap(1));
				bgColor = ircColor(colorsModeRegexp.cap(2));

				ret += pushColorTag(&d, fgColor, bgColor);
			}
			else
			{
				ret += popTag(&d, QString::fromLatin1("span"));
			}
			break;
		case 0x07:	//System bell: ^G
			KNotifyClient::beep( QString::fromLatin1("IRC beep event received in a message") );
			break;
		case '\t':	// 0x09
			ret += QString::fromLatin1("&nbsp;&nbsp;&nbsp;&nbsp;");
			break;
		case '\n':	// 0x0D
			ret += QString::fromLatin1("<br/>");
			break;
		case 0x0D:	// Italics: ^N
			ret += toggleTag(&d, "i");
			break;
		case 0x0F:	//Plain Text, close all tags: ^O
			ret += popAll(&d);
			break;
//		case 0x12:	// Reverse original text colors: ^R
//			break;
		case 0x16:	//Invert Colors: ^V
			swap(fgColor, bgColor);
			ret += pushColorTag(&d, fgColor, bgColor);
			break;
		case 0x1F:	//Underline
			ret += toggleTag(&d, "u");
			break;
		case '<':
			ret += QString::fromLatin1("&lt;");
			break;
		case '>':
			ret += QString::fromLatin1("&gt;");
			break;
		default:
			if (car < QChar(' ')) // search for control characters
				ret += QString::fromLatin1("&lt;%1&gt;").arg(car, 2, 16).upper();
			else
				ret += car;
		}
	}

	ret += popAll(&d);

	return ret;
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
	unsigned int maxcolor = sizeof(s_IRC_Colors)/sizeof(QColor);
	return color<=maxcolor?s_IRC_Colors[color]:s_IRC_Colors[maxcolor];
}

int KSParser::colorForHTML(const QString &html)
{
	QColor color(html);
	for(uint i=0; i<sizeof(s_IRC_Colors)/sizeof(QColor); i++)
	{
		if(s_IRC_Colors[i] == color)
			return i;
	}
	return -1;
}

