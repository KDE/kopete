/*
    rtf2html.h  -  A simple RTF Parser

    Copyright (c) 2002 by Vladimir Shutoff   <vovan@shutoff.ru>             (original code)
    Copyright (c) 2004 by Thiago S. Barcelos <barcelos@ime.usp.br>          (Kopete port)
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef RTF2HTML_H
#define RTF2HTML_H

#include <qstring.h>
#include <stdio.h>

#include <qtextcodec.h>
#include <qcolor.h>
#include <qregexp.h>
#include <kdebug.h>

#include <vector>
#include <stack>
#include <string>
#include <stdarg.h>

using namespace std;

struct FontDef
{
    int		charset;
    string	taggedName;
    string	nonTaggedName;
};

class RTF2HTML;

enum TagEnum
{
    TAG_ALL = 0,
    TAG_FONT_SIZE,
    TAG_FONT_COLOR,
    TAG_FONT_FAMILY,
    TAG_BG_COLOR,
    TAG_BOLD,
    TAG_ITALIC,
    TAG_UNDERLINE
};

class ParStyle
{
public:
    ParStyle() { dir = DirLTR; }
    void clearFormatting();

public:
    enum {DirLTR, DirRTL} dir;
};

class Level
{
public:
    Level(RTF2HTML *_p);
    Level(const Level&);
    void setText(const char* str);
    void setFontTbl() { m_bFontTbl = true; }
    void setColors() { m_bColors = true; resetColors(); }
    void setRed(unsigned char val) { setColor(val, &m_nRed); }
    void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
    void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
    void setFont(unsigned nFont);
    void setEncoding(unsigned nFont);
    void setFontName();
    void setFontColor(unsigned short color);
    void setFontBgColor(unsigned short color);
    void setFontSizeHalfPoints(unsigned short sizeInHalfPoints);
    void setFontSize(unsigned short sizeInPoints);
    void setBold(bool);
    void setItalic(bool);
    void setUnderline(bool);
    void startParagraph();
    bool isParagraphOpen() const;
    void clearParagraphFormatting();
    void setParagraphDirLTR();
    void setParagraphDirRTL();
    void addLineBreak();
    void flush();
    void reset();
    void resetTag(TagEnum tag);
protected:
    string text;
    void Init();
    RTF2HTML *p;
    void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
    void setColor(unsigned char val, unsigned char *p)
    { *p = val; m_bColorInit=true; }

    // Marks the position in m_tags where this level begun.
    unsigned m_nTagsStartPos;

    // True when parsing the fonts table
    bool m_bFontTbl;
    // True when parsing the colors table.
    bool m_bColors;
    // True when inside a 'fname' block.
    bool m_bFontName;
    // False until we get the tagged font name.
    bool m_bTaggedFontNameOk;

    unsigned char m_nRed;
    unsigned char m_nGreen;
    unsigned char m_nBlue;
    bool m_bColorInit;
    unsigned m_nFont; // 1-based
    unsigned m_nEncoding;
    unsigned m_nFontColor; // 1-based
    unsigned m_nFontSize;
    unsigned m_nFontBgColor; // 1-based
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
};

class OutTag
{
public:
    OutTag(TagEnum _tag, unsigned _param) : tag(_tag), param(_param) {}
    TagEnum tag;
    unsigned param;
};

enum quoteMode
{
    quoteHTML,
    quoteXML,
    quoteNOBR
};

class RTF2HTML
{
    friend class Level;

public:
    RTF2HTML();
    QString Parse(const char *rtf, const char *encoding);

    // Paragraph-specific functions:

    QString quoteString(const QString &_str, quoteMode mode = quoteHTML);
    // Appends a string with formatting into the paragraph buffer.
    void PrintUnquoted(const char *str, ...);
    // Quotes and appends a string to the paragraph buffer.
    void PrintQuoted(const QString &str);
    // Writes down the tags from oTags into the paragraph buffer.
    void FlushOutTags();
    // Retrieves the top not-yet-written tag.
    OutTag* getTopOutTag(TagEnum tagType);
    // Writes down the paragraph buffer and resets the paragraph state.
    void FlushParagraph();

    // Document-wide functions:

    void PutTag(TagEnum n)
    {
       tags.push(n);
    }

protected:

// Paragraph members

    // True if the paragraph was opened explicitly.
    bool bExplicitParagraph;
    // The paragraph's HTML buffer.
    QString sParagraph;
    // Defines the paragraph's formatting.
    ParStyle parStyle;
    // Tags which weren't yet printed out.
    vector<OutTag> oTags;

// Document members

    // The document HTML buffer.
    QString s;
    // Fonts table.
    vector<FontDef> fonts;
    // Colors table.
    vector<QColor> colors;
    // Stack of tags (across all levels, not just current level)
    stack<TagEnum> tags;

// RTF parser internals

    const char *rtf_ptr;
    const char *encoding;
    Level cur_level;
    stack<Level> levels;
};

#endif
