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
#include "krichtexteditpart.h"

// Qt includes
#include <QtCore/QEvent>
#include <QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextCharFormat>

// KDE includes
#include <kaction.h>
#include <kactionmenu.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontaction.h>
#include <kfontdialog.h>
#include <kfontsizeaction.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>
#include <kicon.h>
#include <kparts/genericfactory.h>
#include <kstandardaction.h>
#include <ktextedit.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

typedef KParts::GenericFactory<KRichTextEditPart> KRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( librichtexteditpart, KRichTextEditPartFactory )

// Still needed for Qt4 to be able to use the return key as shortcut for sending
class KopeteTextEdit : public KTextEdit
{
public:
    KopeteTextEdit( QWidget *parent )
     : KTextEdit( parent )
    {}

    bool event(QEvent *event)
    {
        if ( event->type() == QEvent::ShortcutOverride )
        {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent)
            {
                if( keyEvent->key() ==  Qt::Key_Return || keyEvent->key() == Qt::Key_Enter )
                {
                    // Enter is the default shortcut for sending a message,
                    // therefore it should not be handled by a textedit
                    return QWidget::event(event);
                }
                if ( keyEvent->matches(QKeySequence::Copy) && !textCursor().hasSelection() )
                {
                    // The copy shortcut has to be handled outside of
                    // the textedit because otherwise you cannot use it 
                    // to copy a selection in the chatmessagepart
                    // see bug: #163535
                    return QWidget::event(event);
                }
                if ( (keyEvent->matches(QKeySequence::MoveToPreviousPage) || keyEvent->matches(QKeySequence::MoveToNextPage))
                     && document()->isEmpty() )
                {
                    // Allow to scroll the chat if the user has not entered
                    // some text in the KRichTextEditPart.
                    return QWidget::event(event);
                }
            }
        }
        return KTextEdit::event(event);
    }
};

class KRichTextEditPart::Private
{
public:
    Private()
     : editor(0), richTextEnabled(false), richTextSupport(KRichTextEditPart::DisableRichText),
        configDirty(false), usingDefault(true), defualtFont(KGlobalSettings::generalFont()),
        updating(false)
    {}

    KopeteTextEdit *editor;
    bool richTextEnabled;
    RichTextSupport richTextSupport;

    KAction *checkSpelling;
    KToggleAction *enableRichText;

    KAction *actionTextColor;

    bool configDirty;
    bool usingDefault;
    QColor defualtTextColor;
    QFont defualtFont;
    QColor desiredTextColor;
    QFont desiredFont;

    KToggleAction *action_bold;
    KToggleAction *action_italic;
    KToggleAction *action_underline;

    KFontAction *action_font;
    KFontSizeAction *action_font_size;

    KToggleAction *action_align_left;
    KToggleAction *action_align_right;
    KToggleAction *action_align_center;
    KToggleAction *action_align_justify;

    bool updating;
};

KRichTextEditPart::KRichTextEditPart(QWidget *wparent, QObject*, const QStringList&)
 : KParts::ReadOnlyPart( wparent ), d(new Private)
{
    // we need an instance
    setComponentData( KRichTextEditPartFactory::componentData() );

    d->editor = new KopeteTextEdit( wparent );
    setWidget( d->editor );

    // FIXME: The actions aren't available anywhere!!!
    createActions();

    // TODO: Rename rc file
    setXMLFile( "kopeterichtexteditpart/kopeterichtexteditpartfull.rc" );
}

KRichTextEditPart::~KRichTextEditPart()
{
    delete d;
}

KTextEdit *KRichTextEditPart::textEdit()
{
    return static_cast<KTextEdit*>(d->editor);
}

QFont KRichTextEditPart::font() const
{
    return d->editor->currentFont();
}

void KRichTextEditPart::clear()
{
    d->editor->clear();
}

bool KRichTextEditPart::isRichTextEnabled() const
{
    return d->richTextEnabled;
}

bool KRichTextEditPart::isRichTextAvailable() const
{
    return (d->richTextSupport & FormattingSupport ||
            d->richTextSupport & SupportAlignment ||
            d->richTextSupport & SupportFont ||
            d->richTextSupport & SupportTextColor);
}

void KRichTextEditPart::setDefualtFont( const QFont& font )
{
    d->defualtFont = font;
    if ( d->usingDefault && !d->configDirty )
    {
        d->desiredFont = d->defualtFont;
        updateCharFormat();
    }
}

void KRichTextEditPart::setDefualtTextColor( const QColor& textColor )
{
    d->defualtTextColor = textColor;
    if ( d->usingDefault && !d->configDirty )
    {
        d->desiredTextColor = d->defualtTextColor;
        updateCharFormat();
    }
}

void KRichTextEditPart::setCheckSpellingEnabled( bool enabled )
{
    d->editor->setCheckSpellingEnabled( enabled );
}

bool KRichTextEditPart::checkSpellingEnabled() const
{
    return d->editor->checkSpellingEnabled();
}

bool KRichTextEditPart::useRichText() const
{
    return isRichTextEnabled() && isRichTextAvailable();
}

void KRichTextEditPart::setRichTextEnabled( bool enable )
{
    d->richTextEnabled = enable;
    d->editor->setAcceptRichText( useRichText() );

    // Emit toolbarToggled signal
    checkToolbarEnabled();

    //Enable / disable buttons
    updateActions();

    d->enableRichText->setEnabled( useRichText() );
    d->enableRichText->setChecked( useRichText() );

    emit richTextChanged();
}

void KRichTextEditPart::setRichTextSupport(const KRichTextEditPart::RichTextSupport &support)
{
    d->richTextSupport = support;

    // Update to new settings
    setRichTextEnabled( isRichTextEnabled() );
}

void KRichTextEditPart::checkToolbarEnabled()
{
    emit toolbarToggled( useRichText() );
}

KAboutData *KRichTextEditPart::createAboutData()
{
    KAboutData *aboutData = new KAboutData("krichtexteditpart", 0, ki18n("KRichTextEditPart"), "0.1",
                        ki18n("A simple rich text editor part"),
                        KAboutData::License_LGPL );
    aboutData->addAuthor(ki18n("Richard J. Moore"), KLocalizedString(), "rich@kde.org", "http://xmelegance.org/" );
    aboutData->addAuthor(ki18n("Jason Keirstead"), KLocalizedString(), "jason@keirstead.org", "http://www.keirstead.org/" );
    aboutData->addAuthor(ki18n("Michaël Larouche"), KLocalizedString(), "larouche@kde.org" "http://www.tehbisnatch.org/" );

    return aboutData;
}

void KRichTextEditPart::createActions()
{
    d->enableRichText = new KToggleAction( KIcon("draw-freehand"), i18n("Enable &Rich Text"), this );
    actionCollection()->addAction( "enableRichText", d->enableRichText );
    d->enableRichText->setCheckedState( KGuiItem( i18n("Disable &Rich Text") ) );
    connect( d->enableRichText, SIGNAL(toggled(bool)),
            this, SLOT(setRichTextEnabled(bool)) );

    d->checkSpelling = new KAction( KIcon("tools-check-spelling"), i18n("Check &Spelling"), actionCollection() );
    actionCollection()->addAction( "check_spelling", d->checkSpelling );
    connect( d->checkSpelling, SIGNAL(triggered(bool)), d->editor, SLOT(checkSpelling()) );

    //Foreground Color
    d->actionTextColor = new KAction( KIcon("color_line"), i18n("Text &Color..."), actionCollection() );
    actionCollection()->addAction( "format_textcolor", d->actionTextColor );
    connect( d->actionTextColor, SIGNAL(triggered(bool)), this, SLOT(setTextColor()) );

    //Font Family
    d->action_font = new KFontAction( i18n("&Font"), actionCollection() );
    actionCollection()->addAction( "format_font", d->action_font );
    connect( d->action_font, SIGNAL(triggered(QString)), this, SLOT(setFont(QString)) );

    //Font Size
    d->action_font_size = new KFontSizeAction( i18n("Font &Size"), actionCollection() );
    actionCollection()->addAction( "format_font_size", d->action_font_size );
    connect( d->action_font_size, SIGNAL(fontSizeChanged(int)), this, SLOT( setFontSize(int) ) );

    //Formatting
    d->action_bold = new KToggleAction( KIcon("format-text-bold"), i18n("&Bold"), actionCollection() );
    actionCollection()->addAction( "format_bold", d->action_bold );
    d->action_bold->setShortcut( KShortcut(Qt::CTRL + Qt::Key_B) );
    connect( d->action_bold, SIGNAL(toggled(bool)), this, SLOT(setFontBold(bool)) );

    d->action_italic = new KToggleAction( KIcon("format-text-italic"), i18n("&Italic"), actionCollection() );
    actionCollection()->addAction( "format_italic", d->action_italic );
    d->action_italic->setShortcut( KShortcut(Qt::CTRL + Qt::Key_I) );
    connect(d->action_italic, SIGNAL(toggled(bool)),
        this, SLOT(setFontItalic(bool)) );

    d->action_underline = new KToggleAction( KIcon("format-text-underline"), i18n("&Underline"), actionCollection() );
    actionCollection()->addAction( "format_underline", d->action_underline );
    d->action_underline->setShortcut( KShortcut(Qt::CTRL + Qt::Key_U) );
    connect( d->action_underline, SIGNAL(toggled(bool)),
        this, SLOT(setFontUnderline(bool)) );

    // TODO: Port to new Qt4 signals. These are qt4 signals no?
    connect( d->editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
        this, SLOT( updateCharFormat() ) );
    updateCharFormat();

    //Alignment
    d->action_align_left = new KToggleAction( KIcon("text-left"), i18n("Align &Left"), actionCollection() );
    actionCollection()->addAction( "format_align_left", d->action_align_left );
    connect( d->action_align_left, SIGNAL(toggled(bool)),
        this, SLOT(setAlignLeft(bool)) );

    d->action_align_center = new KToggleAction( KIcon("text-center"), i18n("Align &Center"), actionCollection() );
    actionCollection()->addAction( "format_align_center", d->action_align_center );
    connect( d->action_align_center, SIGNAL(toggled(bool)),
        this, SLOT(setAlignCenter(bool)) );

    d->action_align_right = new KToggleAction( KIcon("text-right"), i18n("Align &Right"), actionCollection() );
    actionCollection()->addAction( "format_align_right", d->action_align_right );
    connect( d->action_align_right, SIGNAL(toggled(bool)),
        this, SLOT(setAlignRight(bool)) );

    d->action_align_justify = new KToggleAction( KIcon("format-justify-fill"), i18n("&Justify"), actionCollection() );
    actionCollection()->addAction( "format_align_justify", d->action_align_justify );
    connect( d->action_align_justify, SIGNAL(toggled(bool)),
        this, SLOT(setAlignJustify(bool)) );

    QActionGroup *alignmentGroup = new QActionGroup(this);
    alignmentGroup->addAction(d->action_align_left);
    alignmentGroup->addAction(d->action_align_center);
    alignmentGroup->addAction(d->action_align_right);
    alignmentGroup->addAction(d->action_align_justify);

    connect( d->editor, SIGNAL(cursorPositionChanged()),
        this, SLOT(updateAligment()) );

    updateAligment();
}

void KRichTextEditPart::updateActions()
{
    bool useRichText = this->useRichText();

    bool activateAlignment = useRichText && ( d->richTextSupport & KRichTextEditPart::SupportAlignment );
    bool activateFont = (d->richTextSupport & KRichTextEditPart::SupportFont);

    d->actionTextColor->setEnabled( useRichText && (d->richTextSupport & KRichTextEditPart::SupportTextColor) );

    d->action_font->setEnabled( useRichText && activateFont);
    d->action_font_size->setEnabled( useRichText && activateFont );

    d->action_bold->setEnabled( useRichText && (d->richTextSupport & KRichTextEditPart::SupportBold) );
    d->action_italic->setEnabled( useRichText && (d->richTextSupport & KRichTextEditPart::SupportItalic) );
    d->action_underline->setEnabled( useRichText && (d->richTextSupport & KRichTextEditPart::SupportUnderline) );

    d->action_align_left->setEnabled( activateAlignment );
    d->action_align_center->setEnabled( activateAlignment );
    d->action_align_right->setEnabled( activateAlignment );
    d->action_align_justify->setEnabled( activateAlignment );
}

void KRichTextEditPart::updateCharFormat()
{
    d->updating = true;
    if( d->editor->fontPointSize() > 0 )
        d->action_font_size->setFontSize( (int)d->editor->fontPointSize() );
    d->action_font->setFont( d->editor->fontFamily() );

    //this "workarounds" KTextEdit edit, to keep the font even when you delete all typed chars
    d->editor->setTextColor( d->desiredTextColor );
    if( useRichText() )
    {
        d->editor->setCurrentFont( d->desiredFont );
        d->action_bold->setChecked( d->desiredFont.bold() );
        d->action_italic->setChecked( d->desiredFont.italic() );
        d->action_underline->setChecked( d->desiredFont.underline() );
    }
    d->updating = false;
}

void KRichTextEditPart::updateAligment()
{
    Qt::Alignment align = d->editor->alignment();

    switch ( align )
    {
        case Qt::AlignRight:
            d->action_align_right->setChecked( true );
            break;
        case Qt::AlignCenter:
            d->action_align_center->setChecked( true );
            break;
        case Qt::AlignLeft:
            d->action_align_left->setChecked( true );
            break;
        case Qt::AlignJustify:
            d->action_align_justify->setChecked( true );
            break;
        default:
            break;
    }
}

void KRichTextEditPart::readConfig( KConfigGroup& config )
{
    kDebug() << "Loading config";

    d->usingDefault = ( !config.hasKey("TextColor") && !config.hasKey("Font") );
    d->desiredTextColor = config.readEntry( "TextColor", d->defualtTextColor );
    d->desiredFont = config.readEntry("Font", d->defualtFont );
    updateCharFormat();

    switch( config.readEntry( "EditAlignment", int(Qt::AlignLeft) ) )
    {
        case Qt::AlignLeft:
            d->action_align_left->trigger();
            break;
        case Qt::AlignCenter:
            d->action_align_center->trigger();
            break;
        case Qt::AlignRight:
            d->action_align_right->trigger();
            break;
        case Qt::AlignJustify:
            d->action_align_justify->trigger();
            break;
    }

    // Clean dirty flag.
    d->configDirty = false;
}

void KRichTextEditPart::writeConfig( KConfigGroup& config )
{
    if ( d->configDirty == false )
        return;

    kDebug() << "Saving config";

    QFont currentFont = d->editor->currentFont();
    config.writeEntry( "Font", currentFont );
    config.writeEntry( "TextColor", d->editor->textColor() );
    config.writeEntry( "EditAlignment", int(d->editor->alignment()) );
    d->usingDefault = false;

    // Clean dirty flag.
    d->configDirty = false;
}

void KRichTextEditPart::resetConfig( KConfigGroup& config )
{
    kDebug() << "Setting default font style";

    d->desiredTextColor = d->defualtTextColor;
    d->desiredFont = d->defualtFont;
    updateCharFormat();

    d->action_align_left->trigger();

    config.deleteEntry( "Font" );
    config.deleteEntry( "TextColor" );
    config.deleteEntry( "EditAlignment" );

    d->usingDefault = true;
    // Set dirty flag.
    d->configDirty = false;
}

void KRichTextEditPart::setTextColor()
{
    QColor currentTextColor = d->desiredTextColor;

    int result = KColorDialog::getColor( currentTextColor, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() , d->editor );
    if(!currentTextColor.isValid())
        currentTextColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() ;
    if ( result != QDialog::Accepted  )
        return;

    setTextColor( currentTextColor );
}

void KRichTextEditPart::setTextColor(const QColor &newColor)
{
    if( !d->updating && d->richTextSupport & KRichTextEditPart::SupportTextColor )
    {
        d->desiredTextColor = newColor;
        d->configDirty = true;
        updateCharFormat();
    }
}

QColor KRichTextEditPart::textColor() const
{
    if( d->editor->textColor() == KColorScheme(QPalette::Active, KColorScheme::View).foreground().color() )
        return QColor();
    return d->editor->textColor();
}

void KRichTextEditPart::setFontSize(int size)
{
    if( size < 1 )
        return;

    if( !d->updating && d->richTextSupport & KRichTextEditPart::SupportFont )
    {
        d->desiredFont.setPointSize( size );
        d->configDirty = true;
        updateCharFormat();
    }
}

void KRichTextEditPart::setFont()
{
    QFont currentFont = d->desiredFont;
    KFontDialog::getFont( currentFont, false, d->editor );
    setFont( currentFont );
}

void KRichTextEditPart::setFont(const QFont &newFont)
{
    if( !d->updating && useRichText() )
    {
        d->desiredFont = newFont;
        d->configDirty = true;
        updateCharFormat();
    }
}

void KRichTextEditPart::setFont(const QString &newFont)
{
    if( !d->updating && (d->richTextSupport & KRichTextEditPart::SupportFont) && useRichText() )
    {
        d->desiredFont.setFamily( newFont );
        d->configDirty = true;
        updateCharFormat();
    }
}


void KRichTextEditPart::setFontBold(bool value)
{
    if( !d->updating && (d->richTextSupport & KRichTextEditPart::SupportFont) && useRichText() )
    {
        d->desiredFont.setBold( value );
        d->configDirty = true;
        updateCharFormat();
    }
}

void KRichTextEditPart::setFontItalic(bool value)
{
    if( !d->updating && (d->richTextSupport & KRichTextEditPart::SupportItalic) && useRichText() )
    {
        d->desiredFont.setItalic( value );
        d->configDirty = true;
        updateCharFormat();
    }
}

void KRichTextEditPart::setFontUnderline(bool value)
{
    if( !d->updating && (d->richTextSupport & KRichTextEditPart::SupportUnderline) && useRichText()  )
    {
        d->desiredFont.setUnderline( value );
        d->configDirty = true;
        updateCharFormat();
    }
}


void KRichTextEditPart::setAlignLeft( bool yes )
{
    if( !d->updating && yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
    {
        d->editor->setAlignment( Qt::AlignLeft );
        d->configDirty = true;
    }
}

void KRichTextEditPart::setAlignRight( bool yes )
{
    if( !d->updating && yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
    {
        d->editor->setAlignment( Qt::AlignRight );
        d->configDirty = true;
    }
}

void KRichTextEditPart::setAlignCenter( bool yes )
{
    if( !d->updating && yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
    {
        d->editor->setAlignment( Qt::AlignCenter );
        d->configDirty = true;
    }
}

void KRichTextEditPart::setAlignJustify( bool yes )
{
    if( !d->updating && yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
    {
        d->editor->setAlignment( Qt::AlignJustify );
        d->configDirty = true;
    }
}

QString KRichTextEditPart::text( Qt::TextFormat format ) const
{
    if( (format == Qt::RichText || format == Qt::AutoText) && useRichText() )
        return d->editor->toHtml();
    else
        return d->editor->toPlainText();
}

#include "krichtexteditpart.moc"

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
