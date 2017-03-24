/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either 
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public 
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KSPARSER_H
#define KSPARSER_H

#include <QColor>

/*
 * Helper class to parse IRC color/style codes and convert them to
 * richtext. The parser maintains an internal stack of the styles
 * applied because the IRC message could contain sequences as
 * (bold)Hello (red)World(endbold)! (blue)blue text
 * which needs to be converted to
 * <b>Hello </b><font color="red"><b>World</b>! </font><font color="blue">blue text</font>
 * to get correctly nested tags. (malte)
 */
namespace KSParser
{
	QString parse(QString);
	int colorForHTML(const QString &html);

	QColor ircColor(const QString &color);
	QColor ircColor(unsigned int color);
};

#endif

