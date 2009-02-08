/* 
   Copyright 2009 Benson Tsai <btsai@vrwarp.com>
   Copyright (C) 2006 Michaël Larouche <larouche@kde.org>
   Copyright (C) 2003 Richard Moore <rich@kde.org>
   Copyright (c) 2003-2005 Jason Keirstead <jason@keirstead.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KOPETERICHTEXTWIDGET_H
#define KOPETERICHTEXTWIDGET_H

#include "kopeteprotocol.h"

#include <krichtextwidget.h>

/**
 * @brief A KopeteRichTextWidget with overidden behaviors
 *
 * This class overrides the default behavior of fonts when cleared, pasted, etc
 * to match the expected behavior of the user.
 *
 * @author Benson Tsai <btsai@vrwarp.com>
 *
 * @since 4.2
 */
class KDEUI_EXPORT KopeteRichTextWidget : public KRichTextWidget
{
    Q_OBJECT
public:
    explicit KopeteRichTextWidget(QWidget *parent, Kopete::Protocol::Capabilities protocolCaps);

    bool event(QEvent *event);

    void setCurrentCharFormat(const QTextCharFormat & format);

    QTextCharFormat currentCharFormat() const;

    virtual void createActions(KActionCollection *actionCollection);

    void setDefaultCharFormat( const QTextCharFormat& format );

    QTextCharFormat defaultFormat() const;


public Q_SLOTS:
    /**
    * enable/disable rich text support
    * @param enable
    */
    void setRichTextEnabled( bool enable );

    void setTextBackgroundColor(const QColor &color);
    void setTextForegroundColor(const QColor &color);
    void setFontFamily(QString family);
    void setFontSize(int size);
    void setTextBold(bool bold);
    void setTextItalic(bool italic);
    void setTextUnderline(bool underline);
    void setTextStrikeOut(bool strikeout);

    void slotResetFontAndColor();

signals:
    void richTextSupportChanged();

protected:
    virtual void insertFromMimeData(const QMimeData * source);

protected slots:
    void updateTextFormat();


private:
    //@cond PRIVATE
    class Private;
    friend class Private;
    Private *const d;
    //@endcond
};

#endif

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
