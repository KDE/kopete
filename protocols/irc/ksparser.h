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
#include <qvaluestack.h>

/*
 * Helper class to parse IRC color/style codes and convert them to
 * richtext. The parser maintains an internal stack of the styles
 * applied because the IRC message could contain sequences as
 * (bold)Hello (red)World(endbold)! (blue)blue text
 * which needs to be converted to
 * <b>Hello </b><font color="red"><b>World</b>! </font><font color="blue">blue text</font>
 * to get correctly nested tags. (malte)
 */
class KSParser
{
public:
	static QCString parse(const QCString &);
	static int colorForHTML( const QString &html );

	static QColor ircColor(const QString &color);
	static QColor ircColor(unsigned int color);

	~KSParser();
private:
	KSParser();

	QCString _parse(const QCString &);
	QString pushTag(const QString &, const QString & = QString::null);
	QString pushColorTag(const QColor &fgColor, const QColor &bgColor);
	QString popTag(const QString &);
	QString toggleTag(const QString &);
	QString popAll();

private:
	static KSParser m_parser;
	static const QColor IRC_Colors[17];
	static const QRegExp sm_colorsModeRegexp;

	QValueStack<QString> m_tags;
	QMap<QString, QString> m_attributes;
};

#endif


