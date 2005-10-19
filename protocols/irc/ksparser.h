/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/
#ifndef __ksparser_h__
#define __ksparser_h__

#include <qcolor.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstack.h>

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
public:
	static QCString parse(const QCString &);
	static int colorForHTML( const QString &html );

	static QColor ircColor(const QString &color);
	static QColor ircColor(unsigned int color);
};

#endif


