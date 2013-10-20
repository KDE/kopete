/*
   Copyright (C) 2009 Roman Jarosz <kedgedev@gmail.com>
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
#include <kcolorscheme.h>
#include <KConfigGroup>

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
    Private(KopeteRichTextWidget *parent, KActionCollection *ac, Kopete::Protocol::Capabilities caps)
        : q(parent), actionCollection(ac), protocolCaps(caps), resettingCharFormat(false), empty(true), updating(false), changingTextMode(false),
        checkSpelling(0), toggleRichText(0), reset(0)
    {
    }

    KopeteRichTextWidget *q;
    KActionCollection *actionCollection;

    QList<QKeySequence> sendKeySequenceList;

    const Kopete::Protocol::Capabilities protocolCaps;

    QTextCharFormat defaultPlainFormat;
    QTextCharFormat defaultRichFormat;
    QTextCharFormat currentRichFormat;

    QTextCharFormat lastCharFormat;
    bool resettingCharFormat;

    bool empty;
    bool updating;

    bool changingTextMode;

    KToggleAction* autoResize;
    KToggleAction* checkSpelling;
    KToggleAction* toggleRichText;
    KAction* reset;

    void mergeAll(const QTextCharFormat& format);
};
//@endcond

KopeteRichTextWidget::KopeteRichTextWidget(QWidget* parent, Kopete::Protocol::Capabilities protocolCaps, KActionCollection *actionCollection)
        : KRichTextWidget(parent),
        d(new Private(this, actionCollection, protocolCaps))
{
    connect(this, SIGNAL(textModeChanged(KRichTextEdit::Mode)),
            this, SLOT(slotTextModeChanged(KRichTextEdit::Mode)));

    // Default plaintext setup
    setRichTextSupport(KopeteRichTextWidget::DisableRichText);
    d->changingTextMode = true;
    switchToPlainText();
    d->changingTextMode = false;
    createActions(d->actionCollection);
    setCurrentPlainCharFormat(d->defaultPlainFormat);

    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(updateCharFormat(QTextCharFormat)));

    connect(this, SIGNAL(textChanged()),
            this, SLOT(updateTextFormat()));
}

KopeteRichTextWidget::~KopeteRichTextWidget()
{
    KConfigGroup configGroupMode( KGlobal::config(), QLatin1String( "KopeteChatWindowGroupMode" ));
    KConfigGroup configIndividual( KGlobal::config(), QLatin1String( "KopeteChatWindowIndividualMode" ));
    configGroupMode.writeEntry( "AutoResize", d->autoResize->isChecked());
    configIndividual.writeEntry( "AutoResize", d->autoResize->isChecked());
    delete d;
}

void KopeteRichTextWidget::setTextOrHtml(const QString &text)
{
    if (Qt::mightBeRichText(text))
    {
        if (isRichTextEnabled())
            setHtml(text);
        else
        {
            QTextDocument doc;
            doc.setHtml(text);
            setPlainText(doc.toPlainText());
        }
    }
    else
    {
        setPlainText(text);
    }
}

void KopeteRichTextWidget::slotCheckSpellingChanged(bool b)
{
    setCheckSpellingEnabled(b);
}

void KopeteRichTextWidget::slotDocumentSizeUpdated()
{
    int currentFontHeight = QFontMetrics(font()).height();
    int difference = document()->size().toSize().height() - size().height() + currentFontHeight;
    emit documentSizeUpdated(difference);
}

void KopeteRichTextWidget::slotEnableAutoResize(bool enable)
{
    if (enable)
    {
        connect(this, SIGNAL(textChanged()),
                this, SLOT(slotDocumentSizeUpdated()));
    }
    else
    {
        disconnect(this, SLOT(slotDocumentSizeUpdated()));
    }
}

void KopeteRichTextWidget::createActions(KActionCollection *actionCollection)
{
    KConfigGroup config(KGlobal::config(), QLatin1String("KopeteChatWindowIndividualMode"));

    bool autoResizeEnabled = config.readEntry("AutoResize", true);
    d->autoResize = new KToggleAction( i18n("Input auto-resize"), actionCollection );
    connect( d->autoResize, SIGNAL(toggled(bool)), this, SLOT(slotEnableAutoResize(bool)) );
    d->autoResize->setChecked(autoResizeEnabled);
    slotEnableAutoResize(autoResizeEnabled);
    actionCollection->addAction( "enable_autoresize", d->autoResize );

    if (!d->checkSpelling)
    {
        d->checkSpelling = new KToggleAction(KIcon("tools-check-spelling"), i18n("Automatic Spell Checking"), actionCollection);
        actionCollection->addAction("enable_auto_spell_check", d->checkSpelling);
        d->checkSpelling->setChecked(true);
        connect(d->checkSpelling, SIGNAL(toggled(bool)), this, SLOT(slotCheckSpellingChanged(bool)));
    }

    bool richTextSupport = (getProtocolRichTextSupport() != KopeteRichTextWidget::DisableRichText);
    if (!d->toggleRichText && richTextSupport)
    {
        d->toggleRichText = new KToggleAction(KIcon("draw-freehand"), i18n("Enable &Rich Text"), actionCollection);
        actionCollection->addAction("enable_richtext", d->toggleRichText);
        d->toggleRichText->setChecked(isRichTextEnabled());
        connect(d->toggleRichText, SIGNAL(toggled(bool)), this, SLOT(setRichTextEnabled(bool)));
    }
    else if (d->toggleRichText && !richTextSupport)
    {
        actionCollection->removeAction(d->toggleRichText);
        d->toggleRichText = 0;
    }

    if (!d->reset && isRichTextEnabled())
    {
        d->reset = new KAction(KIcon("format-stroke-color"), i18n("Reset Font And Color"), actionCollection);
        actionCollection->addAction("format_font_and_color_reset", d->reset);
        connect(d->reset, SIGNAL(triggered(bool)), this, SLOT(slotResetFontAndColor()));
    }
    else if (d->reset && !isRichTextEnabled())
    {
        actionCollection->removeAction(d->reset);
        d->reset = 0;
    }

    KRichTextWidget::createActions(actionCollection);

    // FIXME: Really ugly hack, but we reset format in updateCharFormat and if we don't disconnect this
    //        then actions will have old values and not the reset.
    disconnect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
               this, SLOT(_k_updateCharFormatActions(QTextCharFormat)));
}

void KopeteRichTextWidget::setRichTextEnabled(bool enable)
{
    if (isRichTextEnabled() == enable)
        return;

    KopeteRichTextWidget::RichTextSupport richText = getProtocolRichTextSupport();
    if (enable && richText != KopeteRichTextWidget::DisableRichText)
    {
        setRichTextSupport(richText);
        d->changingTextMode = true;
        enableRichTextMode();
        d->changingTextMode = false;
        createActions(d->actionCollection);
        setCurrentRichCharFormat(d->currentRichFormat);
    }
    else
    {
        setRichTextSupport(KopeteRichTextWidget::DisableRichText);
        d->changingTextMode = true;
        switchToPlainText();
        d->changingTextMode = false;
        createActions(d->actionCollection);
        setCurrentPlainCharFormat(d->defaultPlainFormat);
    }

    if (d->toggleRichText)
        d->toggleRichText->setChecked(isRichTextEnabled());

    if (d->reset)
        d->reset->setEnabled(isRichTextEnabled());

    emit richTextSupportChanged();
}

void KopeteRichTextWidget::slotResetFontAndColor()
{
    setCurrentRichCharFormat(d->defaultRichFormat);
}

void KopeteRichTextWidget::setFontFamily(QString family)
{
    d->currentRichFormat.setFontFamily(family);
    if (!isRichTextEnabled())
        return;

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
    d->currentRichFormat.setFontPointSize(size);
    if (!isRichTextEnabled())
        return;

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
    QFont font = d->currentRichFormat.font();
    font.setBold(bold);
    d->currentRichFormat.setFont(font);

    if (!isRichTextEnabled())
        return;

    if (d->protocolCaps & Kopete::Protocol::BaseBFormatting)
    {
        QTextCharFormat format;
        format.setFontWeight(d->currentRichFormat.fontWeight());
        d->mergeAll(format);
    }
    else
    {
        KRichTextWidget::setTextBold(bold);
    }
}

void KopeteRichTextWidget::setTextItalic(bool italic)
{
    d->currentRichFormat.setFontItalic(italic);
    if (!isRichTextEnabled())
        return;

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
    d->currentRichFormat.setFontUnderline(underline);
    if (!isRichTextEnabled())
        return;

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
    d->lastCharFormat = format;
    KRichTextWidget::setCurrentCharFormat(format);
}

void KopeteRichTextWidget::updateCharFormat(const QTextCharFormat & f)
{
    // TODO: This should go to KRichTextWidget or KRichTextEdit
    if (d->resettingCharFormat)
        return;

    if (f != QTextCharFormat() || !document()->isEmpty())
    {
        d->lastCharFormat = f;
        bool bOpaque = d->lastCharFormat.foreground().isOpaque();
        bool fOpaque = d->lastCharFormat.background().isOpaque();

        if (!fOpaque)
            d->lastCharFormat.setForeground(palette().color(QPalette::Active, QPalette::Text));
        if (!bOpaque)
            d->lastCharFormat.setBackground(palette().color(QPalette::Active, QPalette::Base));

        if (!fOpaque || !bOpaque)
        {
            d->resettingCharFormat = true;
            KRichTextWidget::setCurrentCharFormat(d->lastCharFormat);
            d->resettingCharFormat = false;
        }

        if (isRichTextEnabled() && d->currentRichFormat != d->lastCharFormat)
        {
            d->currentRichFormat = d->lastCharFormat;

            if (d->protocolCaps & Kopete::Protocol::BaseBgColor)
            {
                QPalette palette = this->palette();
                palette.setColor(QPalette::Active, QPalette::Base, d->lastCharFormat.background().color());
                palette.setColor(QPalette::Inactive, QPalette::Base, d->lastCharFormat.background().color());
                this->setPalette(palette);
            }
        }
    }
    else
    {
        d->resettingCharFormat = true;
        KRichTextWidget::setCurrentCharFormat(d->lastCharFormat);
        d->resettingCharFormat = false;
    }
    updateActionStates();
}

void KopeteRichTextWidget::updateTextFormat()
{
    if (d->updating || !isRichTextEnabled())
        return;

    bool empty = document()->isEmpty();
    if (!empty && d->empty)
    {
        d->updating = true;
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.select(QTextCursor::Document);
        cursor.mergeCharFormat(d->currentRichFormat);
        mergeCurrentCharFormat(d->currentRichFormat);
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

bool KopeteRichTextWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
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
    if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyRelease || event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent)
        {
            QKeySequence keyEventSequance(keyEvent->modifiers() + keyEvent->key());
            foreach(const QKeySequence& sendKeySequence, d->sendKeySequenceList)
            {
                if (keyEventSequance.matches(sendKeySequence))
                {
                    // Don't handle the shortcut for sending text in the textedit
                    return false;// QWidget::event(event);
                }
            }
        }
    }
    return KRichTextWidget::event(event);
}

void KopeteRichTextWidget::setSendKeySequenceList(const QList<QKeySequence>& keySequenceList)
{
        d->sendKeySequenceList = keySequenceList;
}

void KopeteRichTextWidget::setDefaultPlainCharFormat(const QTextCharFormat& format)
{
    d->defaultPlainFormat = format;
    setCurrentPlainCharFormat(d->defaultPlainFormat);
}

void KopeteRichTextWidget::setCurrentPlainCharFormat(const QTextCharFormat & format)
{
    if (isRichTextEnabled())
        return;

    setCurrentCharFormat(format);
    d->mergeAll(format);

    // set background color to match
    QPalette palette = this->palette();
    palette.setColor(QPalette::Active, QPalette::Base, format.background().color());
    palette.setColor(QPalette::Inactive, QPalette::Base, format.background().color());
    this->setPalette(palette);
}

void KopeteRichTextWidget::setDefaultRichCharFormat(const QTextCharFormat& format)
{
    bool usingDefaultFormat = (d->defaultRichFormat == d->currentRichFormat);

    d->defaultRichFormat = format;
    if (usingDefaultFormat)
        setCurrentRichCharFormat(d->defaultRichFormat);
}

void KopeteRichTextWidget::setCurrentRichCharFormat(const QTextCharFormat & format)
{
    d->currentRichFormat = format;

    if (isRichTextEnabled())
    {
        setCurrentCharFormat(format);
        if (d->protocolCaps & (Kopete::Protocol::BaseFormatting | Kopete::Protocol::BaseColor))
        {
            d->mergeAll(format);
        }

        QColor color;
        if (d->protocolCaps & Kopete::Protocol::BaseBgColor)
            color = format.background().color();
        else
            color = KColorScheme(QPalette::Active, KColorScheme::View).background().color();

        QPalette palette = this->palette();
        palette.setColor(QPalette::Active, QPalette::Base, color);
        palette.setColor(QPalette::Inactive, QPalette::Base, color);
        this->setPalette(palette);
    }
}

QTextCharFormat KopeteRichTextWidget::defaultPlainFormat() const
{
    return d->defaultPlainFormat;
}

QTextCharFormat KopeteRichTextWidget::defaultRichFormat() const
{
    return d->defaultRichFormat;
}

QTextCharFormat KopeteRichTextWidget::currentRichFormat() const
{
    return d->currentRichFormat;
}

bool KopeteRichTextWidget::isRichTextEnabled() const
{
    return (textMode() == KopeteRichTextWidget::Rich);
}

void KopeteRichTextWidget::slotTextModeChanged(KRichTextEdit::Mode)
{
    if (d->changingTextMode == false)
    {
        kWarning() << "Unexpected text mode change!!!";
        kWarning() << kBacktrace();
    }
}

KopeteRichTextWidget::RichTextSupport KopeteRichTextWidget::getProtocolRichTextSupport() const
{
    KopeteRichTextWidget::RichTextSupport richText = KopeteRichTextWidget::DisableRichText;

    // Check for bold
    if ((d->protocolCaps & Kopete::Protocol::BaseBFormatting) || (d->protocolCaps & Kopete::Protocol::RichBFormatting))
    {
        richText |= KopeteRichTextWidget::SupportBold;
    }
    // Check for italic
    if ((d->protocolCaps & Kopete::Protocol::BaseIFormatting) || (d->protocolCaps & Kopete::Protocol::RichIFormatting))
    {
        richText |= KopeteRichTextWidget::SupportItalic;
    }
    // Check for underline
    if ((d->protocolCaps & Kopete::Protocol::BaseUFormatting) || (d->protocolCaps & Kopete::Protocol::RichUFormatting))
    {
        richText |= KopeteRichTextWidget::SupportUnderline;
    }
    // Check for font support
    if ((d->protocolCaps & Kopete::Protocol::BaseFont) || (d->protocolCaps & Kopete::Protocol::RichFont))
    {
        richText |= KopeteRichTextWidget::SupportFontFamily | KopeteRichTextWidget::SupportFontSize;
    }
    // Check for text color support
    if ((d->protocolCaps & Kopete::Protocol::BaseFgColor) || (d->protocolCaps & Kopete::Protocol::RichFgColor))
    {
        richText |= KopeteRichTextWidget::SupportTextForegroundColor;
    }
    // Check for background color support
    if ((d->protocolCaps & Kopete::Protocol::BaseBgColor) || (d->protocolCaps & Kopete::Protocol::RichBgColor))
    {
        richText |= KopeteRichTextWidget::SupportTextBackgroundColor;
    }
    // Check for alignment
    if (d->protocolCaps & Kopete::Protocol::Alignment)
    {
        richText |= KopeteRichTextWidget::SupportAlignment;
    }

    return richText;
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
