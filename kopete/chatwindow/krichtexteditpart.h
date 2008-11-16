/* This file is part of the KDE libraries
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
#ifndef KRICHTEXTEDITPART_H
#define KRICHTEXTEDITPART_H

#include <kparts/part.h>

#include <QtGui/QFont>
#include <QtGui/QColor>
#include <QtCore/QFlags>

// TODO: Use kdelibs export
#include <kopete_export.h>

class KAboutData;
class KTextEdit;
class KConfigGroup;

/**
 * @brief Simple WYSIWYG rich text editor part.
 *
 * Technicaly it just a wrapper around KTextEdit with a toolbar of actions.
 * The action toolbar adds buttons to set text bold, italic 
 * or underline, set font size and font familly and set text color.
 *
 * @author Michaël Larouche <larouche@kde.org>
 * @author Richard Moore <rich@kde.org>
 */
class KRICHTEXTEDITPART_EXPORT KRichTextEditPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    enum RichTextSupportValues
    {
        DisableRichText = 0,
        SupportBold,
        SupportItalic,
        SupportUnderline,
        SupportAlignment,
        SupportFont,
        SupportTextColor,

        FormattingSupport = SupportBold | SupportItalic | SupportUnderline,

        FullSupport = FormattingSupport | SupportAlignment | SupportFont | SupportTextColor
    };
    Q_DECLARE_FLAGS(RichTextSupport, RichTextSupportValues)

    KRichTextEditPart(QWidget *parent, QObject *, const QStringList &);

    ~KRichTextEditPart();

    /**
     * @brief Get the text in the editor in the given format.
     * By default if return the text using the most appropriate format.
     *
     * @param format A value in Qt::TextFormat enum.
     *
     * @return text using the given format
     */
    QString text( Qt::TextFormat format = Qt::AutoText ) const;

    /**
     * @brief Get the font currently used by the editor.
     */
    QFont font() const;
    /**
     * @brief Get the current text color
     */
    QColor textColor() const;

    /**
     * @brief Clear text inside the editor
     */
    void clear();

    /**
     * @brief Is rich text is currently enabled
     */
    bool isRichTextEnabled() const;

    bool isRichTextAvailable() const;

    void setDefualtFont( const QFont& font );

    void setDefualtTextColor( const QColor& textColor );

    /** 
     * Enable or Disable the automatic spell checking
     * @param enabled the state that auto spell checking should beee
     */
    void setCheckSpellingEnabled( bool enabled );

    /**
     * Get the state of auto spell checking
     * @return true if auto spell checking is turned on, false otherwise
     */
    bool checkSpellingEnabled() const;

    static KAboutData *createAboutData();

    /**
     * @brief Disable file open, because it's not used by this part.
     */
    virtual bool openFile() { return false; }

    void setRichTextSupport(const KRichTextEditPart::RichTextSupport &support);
    RichTextSupport richTextSupport() const;

    /**
     * @brief Get the inside KTextEdit
     * @return instance of KTextEdit
     */
    KTextEdit *textEdit();

public slots:
    void setTextColor();
    void setTextColor( const QColor & );

    void setFont();
    void setFont(const QFont &font);
    void setFont(const QString &familyName);
    void setFontSize(int size);
    void setFontUnderline(bool value);
    void setFontBold(bool value);
    void setFontItalic(bool value);

    void setAlignLeft(bool yes);
    void setAlignRight(bool yes);
    void setAlignCenter(bool yes);
    void setAlignJustify(bool yes);

    void checkToolbarEnabled();
    void setRichTextEnabled(bool enable);

signals:
    void toolbarToggled(bool enabled);
    void richTextChanged();

protected:
    /**
     * @brief Create the required actions for the part.
     */
    virtual void createActions();

    bool useRichText() const;

protected slots:
    void updateActions();

    void updateCharFormat();
    void updateAligment();

public:
    void readConfig( KConfigGroup& config );
    void resetConfig( KConfigGroup& config );
    void writeConfig( KConfigGroup& config );

private:
    class Private;
    Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KRichTextEditPart::RichTextSupport)

#endif

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
