/* 
   Copyright (C) 2009 Benson Tsai <btsai@vrwarp.com>
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

#include "kopeterichtextwidget.h"

// KDE includes
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <klocalizedstring.h>
#include <ktoggleaction.h>

// Qt includes
#include <QUrl>
#include <QtCore/QEvent>
#include <QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextCharFormat>
#include <QTextDocumentFragment>

// TODO: Add i18n context

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KopeteRichTextWidget::Private
{
public:
    Private(KopeteRichTextWidget *parent)
            : q(parent), defaultFormat(), desiredFormat(), empty(true), updating(false),
              checkSpelling(0), toggleRichText(0), reset(0), protocolCaps()
    {
        desiredFormat.setBackground(QColor("white"));
        desiredFormat.setForeground(QColor("black"));

        defaultFormat = desiredFormat;
    }

    KopeteRichTextWidget *q;

    QTextCharFormat defaultFormat;
    QTextCharFormat desiredFormat;

    bool empty;
    bool updating;

    KToggleAction* checkSpelling;
    KToggleAction* toggleRichText;
    KAction* reset;

    Kopete::Protocol::Capabilities protocolCaps;

    void setProtocolRichTextSupport();
    KopeteRichTextWidget::RichTextSupport getProtocolRichTextSupport();
    void mergeAll(const QTextCharFormat& format);
};
//@endcond

KopeteRichTextWidget::KopeteRichTextWidget(QWidget* parent, Kopete::Protocol::Capabilities protocolCaps)
        : KRichTextWidget(parent),
        d(new Private(this))
{
    connect(this, SIGNAL(textChanged()),
            this, SLOT(updateTextFormat()) );

    d->protocolCaps = protocolCaps;

    d->setProtocolRichTextSupport();
}

bool KopeteRichTextWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
            if (keyEvent->key() ==  Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                // Enter is the default shortcut for sending a message,
                // therefore it should not be handled by a textedit
                return QWidget::event(event);
            }
            if (keyEvent->matches(QKeySequence::Copy) && !textCursor().hasSelection())
            {
                // The copy shortcut has to be handled outside of
                // the textedit because otherwise you cannot use it
                // to copy a selection in the chatmessagepart
                // see bug: #163535
                return QWidget::event(event);
            }
            if ((keyEvent->matches(QKeySequence::MoveToPreviousPage) || keyEvent->matches(QKeySequence::MoveToNextPage))
                    && document()->isEmpty())
            {
                // Allow to scroll the chat if the user has not entered
                // some text in the KRichTextEditPart.
                return QWidget::event(event);
            }
        }
    }
    return KRichTextWidget::event(event);
}

void KopeteRichTextWidget::createActions(KActionCollection *actionCollection)
{
    if (!d->checkSpelling)
    {
        d->checkSpelling = new KToggleAction(KIcon("tools-check-spelling"), i18n("Automatic Spell Checking"), actionCollection);
        actionCollection->addAction("enable_auto_spell_check", d->checkSpelling);
        d->checkSpelling->setChecked(true);
        connect(d->checkSpelling, SIGNAL(toggled(bool)), this, SLOT(setCheckSpellingEnabled(bool)));
    }

    if (!d->toggleRichText)
    {
        d->toggleRichText = new KToggleAction(KIcon("draw-freehand"), i18n("Enable &Rich Text"), actionCollection);
        actionCollection->addAction("enable_richtext", d->toggleRichText);
        connect(d->toggleRichText, SIGNAL(toggled(bool)), this, SLOT(setRichTextEnabled(bool)));
    }

    if (!d->reset)
    {
        d->reset = new KAction(KIcon("format-stroke-color"), i18n("Reset Font And Color"), actionCollection);
        actionCollection->addAction("format_font_and_color_reset", d->reset);
        connect(d->reset, SIGNAL(triggered(bool)), this, SLOT(slotResetFontAndColor()));
    }

    KRichTextWidget::createActions(actionCollection);
}

void KopeteRichTextWidget::setRichTextEnabled(bool enable)
{
    if (enable)
    {
        setRichTextSupport(d->getProtocolRichTextSupport());
        enableRichTextMode();
    }
    else
    {
        setRichTextSupport(KopeteRichTextWidget::DisableRichText);
        switchToPlainText();
    }

    d->toggleRichText->setChecked(enable);

    emit richTextSupportChanged();
}

void KopeteRichTextWidget::slotResetFontAndColor()
{
    setCurrentCharFormat(d->defaultFormat);
}

void KopeteRichTextWidget::setTextBackgroundColor(const QColor &color)
{
    d->desiredFormat.setBackground(color);

    if (d->protocolCaps & Kopete::Protocol::BaseBgColor)
    {
        QTextCharFormat format;
        format.setBackground(color);
        d->mergeAll(format);

        QPalette palette = this->palette();
        palette.setColor(QPalette::Active, QPalette::Base, color);
        palette.setColor(QPalette::Inactive, QPalette::Base, color);
        this->setPalette(palette);
    }
    else
    {
        KRichTextWidget::setTextBackgroundColor(color);
    }
}

void KopeteRichTextWidget::setTextForegroundColor(const QColor &color)
{
    d->desiredFormat.setForeground(color);

    if (d->protocolCaps & Kopete::Protocol::BaseFgColor)
    {
        QTextCharFormat format;
        format.setForeground(color);
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setTextForegroundColor(color);
    }
}

void KopeteRichTextWidget::setFontFamily(QString family)
{
    d->desiredFormat.setFontFamily(family);

    if (d->protocolCaps & Kopete::Protocol::BaseFont)
    {
        QTextCharFormat format;
        format.setFontFamily(family);
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setFontFamily(family);
    }
}

void KopeteRichTextWidget::setFontSize(int size)
{
    d->desiredFormat.setFontPointSize(size);

    if (d->protocolCaps & Kopete::Protocol::BaseFont)
    {
        QTextCharFormat format;
        format.setFontPointSize(size);
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setFontSize(size);
    }
}

void KopeteRichTextWidget::setTextBold(bool bold)
{
    QFont font = d->desiredFormat.font();
    font.setBold(bold);
    d->desiredFormat.setFont(font);

    if (d->protocolCaps & Kopete::Protocol::BaseBFormatting)
    {
        QTextCharFormat format;
        format.setFontWeight(d->desiredFormat.fontWeight());
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setTextBold(bold);
    }
}

void KopeteRichTextWidget::setTextItalic(bool italic)
{
    d->desiredFormat.setFontItalic(italic);

    if (d->protocolCaps & Kopete::Protocol::BaseIFormatting)
    {
        QTextCharFormat format;
        format.setFontItalic(italic);
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setTextItalic(italic);
    }
}

void KopeteRichTextWidget::setTextUnderline(bool underline)
{
    d->desiredFormat.setFontUnderline(underline);

    if (d->protocolCaps & Kopete::Protocol::BaseUFormatting)
    {
        QTextCharFormat format;
        format.setFontItalic(underline);
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setTextUnderline(underline);
    }
}

void KopeteRichTextWidget::setTextStrikeOut(bool)
{
    kDebug() << "Strikeout not supported!";
}

void KopeteRichTextWidget::setCurrentCharFormat(const QTextCharFormat & format)
{
    d->desiredFormat = format;
    KRichTextWidget::setCurrentCharFormat(format);

    if (d->protocolCaps & (Kopete::Protocol::BaseFormatting | Kopete::Protocol::BaseColor))
    {
        d->mergeAll(format);
    }
}

QTextCharFormat KopeteRichTextWidget::currentCharFormat() const
{
    return d->desiredFormat;
}

void KopeteRichTextWidget::updateTextFormat()
{
    if (d->updating)
        return;

    bool empty = document()->isEmpty();
    if (!empty && d->empty)
    {
        d->updating = true;
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(d->desiredFormat);
        mergeCurrentCharFormat(d->desiredFormat);
        cursor.endEditBlock();
        d->updating = false;
    }

    d->empty = empty;
}

void KopeteRichTextWidget::insertFromMimeData(const QMimeData * source)
{
    if (source->hasUrls())
    {
        QList<QUrl> urls = source->urls();
        if (urls.size() > 0)
        {
            textCursor().insertText(urls[0].toString());
            return;
        }
    }

    // If HTML then you need to unset d->empty to make sure rich text gets through correctly
    if (source->hasHtml())
    {
        d->empty = d->empty && source->html().isEmpty();
        // double check to make sure we aren't pasting empty space
        if (d->empty)
        {
            QTextDocumentFragment frag = QTextDocumentFragment::fromHtml(source->html());
            d->empty = frag.toPlainText().trimmed().isEmpty();
        }
    }

    KRichTextWidget::insertFromMimeData(source);
}

void KopeteRichTextWidget::setDefaultCharFormat(const QTextCharFormat& format)
{
    d->defaultFormat = format;

    setCurrentCharFormat(format);

    // set background color to match
    QPalette palette = this->palette();
    palette.setColor(QPalette::Active, QPalette::Base, format.background().color());
    palette.setColor(QPalette::Inactive, QPalette::Base, format.background().color());
    this->setPalette(palette);
}

QTextCharFormat KopeteRichTextWidget::defaultFormat() const
{
    return d->defaultFormat;
}

KopeteRichTextWidget::RichTextSupport KopeteRichTextWidget::Private::getProtocolRichTextSupport()
{
    KopeteRichTextWidget::RichTextSupport richText = 0;

    // Check for bold
    if ((protocolCaps & Kopete::Protocol::BaseBFormatting) || (protocolCaps & Kopete::Protocol::RichBFormatting))
    {
        richText |= KopeteRichTextWidget::SupportBold;
    }
    // Check for italic
    if ((protocolCaps & Kopete::Protocol::BaseIFormatting) || (protocolCaps & Kopete::Protocol::RichIFormatting))
    {
        richText |= KopeteRichTextWidget::SupportItalic;
    }
    // Check for underline
    if ((protocolCaps & Kopete::Protocol::BaseUFormatting) || (protocolCaps & Kopete::Protocol::RichUFormatting))
    {
        richText |= KopeteRichTextWidget::SupportUnderline;
    }
    // Check for font support
    if ((protocolCaps & Kopete::Protocol::BaseFont) || (protocolCaps & Kopete::Protocol::RichFont))
    {
        richText |= KopeteRichTextWidget::SupportFontFamily | KopeteRichTextWidget::SupportFontSize;
    }
    // Check for text color support
    if ((protocolCaps & Kopete::Protocol::BaseFgColor) || (protocolCaps & Kopete::Protocol::RichFgColor))
    {
        richText |= KopeteRichTextWidget::SupportTextForegroundColor;
    }
    // Check for background color support
    if ((protocolCaps & Kopete::Protocol::BaseBgColor) || (protocolCaps & Kopete::Protocol::RichBgColor))
    {
        richText |= KopeteRichTextWidget::SupportTextBackgroundColor;
    }
    // Check for alignment
    if (protocolCaps & Kopete::Protocol::Alignment)
    {
        richText |= KopeteRichTextWidget::SupportAlignment;
    }

    return richText;
}

void KopeteRichTextWidget::Private::setProtocolRichTextSupport()
{
    KopeteRichTextWidget::RichTextSupport richText = getProtocolRichTextSupport();

    // Set editor support
    q->setRichTextSupport(richText);

    // Set the toggles if possible
    if (toggleRichText)
    {
        toggleRichText->setEnabled(richText != 0);
        if (q->textMode() == KopeteRichTextWidget::Rich)
        {
            toggleRichText->setChecked(true);
        }
        else
        {
            toggleRichText->setChecked(false);
        }
    }
}

void KopeteRichTextWidget::Private::mergeAll(const QTextCharFormat& format)
{
    QTextCursor cursor = q->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.mergeCharFormat(format);
    q->mergeCurrentCharFormat(format);
    cursor.endEditBlock();
}

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
#include "kopeterichtextwidget.moc"
