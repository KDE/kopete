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
#include <kicon.h>
#include <kparts/genericfactory.h>
#include <kstandardaction.h>
#include <ktextedit.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

typedef KParts::GenericFactory<KRichTextEditPart> KRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( librichtexteditpart, KRichTextEditPartFactory )

// FIXME: It is still needed with Qt4 ? Need to checkout.
class KopeteTextEdit : public KTextEdit
{
public:
    KopeteTextEdit( QWidget *parent )
     : KTextEdit( parent )
    {}

    bool event(QEvent *event)
    {
        // don't allow QTextEdit to override accels
        if ( event->type() == QEvent::AccelOverride )
            return QWidget::event(event);
        else
            return KTextEdit::event(event);
    }
};

class KRichTextEditPart::Private
{
public:
    Private()
     : editor(0), richTextEnabled(false), richTextSupport(KRichTextEditPart::DisableRichText),
       configWriteLock(false)
    {}

    KopeteTextEdit *editor;
    bool richTextEnabled;
    RichTextSupport richTextSupport;
    bool configWriteLock;

    KAction *checkSpelling;
    KToggleAction *enableRichText;

    KAction *actionTextColor;

    KToggleAction *action_bold;
    KToggleAction *action_italic;
    KToggleAction *action_underline;

    KFontAction *action_font;
    KFontSizeAction *action_font_size;

    KToggleAction *action_align_left;
    KToggleAction *action_align_right;
    KToggleAction *action_align_center;
    KToggleAction *action_align_justify;
};

KRichTextEditPart::KRichTextEditPart(QWidget *wparent, QObject*, const QStringList&)
 : KParts::ReadOnlyPart( wparent ), d(new Private)
{
    // we need an instance
    setInstance( KRichTextEditPartFactory::instance() );

    d->editor = new KopeteTextEdit( wparent );

    setWidget( d->editor );

    createActions();

    // TODO: Rename rc file
    setXMLFile( "kopeterichtexteditpartfull.rc" );

    //Set colors, font
    readConfig();
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

    // Spellchecking disabled when using rich text because the
    // text we were getting from widget was coloured HTML!
#ifdef __GNUC__
#warning Renable spellchecker (-DarkShock)
#endif
#if 0
    editor->setCheckSpellingEnabled( !richTextEnabled() );
    checkSpelling->setEnabled( !richTextEnabled() );
#endif

    //Enable / disable buttons
    updateActions();

    d->enableRichText->setEnabled( useRichText() );
    d->enableRichText->setChecked( useRichText() );
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

void KRichTextEditPart::reloadConfig()
{
    readConfig();
}

KAboutData *KRichTextEditPart::createAboutData()
{
    KAboutData *aboutData = new KAboutData("krichtexteditpart", I18N_NOOP("KRichTextEditPart"), "0.1",
                        I18N_NOOP("A simple rich text editor part"),
                        KAboutData::License_LGPL );
    aboutData->addAuthor("Richard J. Moore", 0, "rich@kde.org", "http://xmelegance.org/" );
    aboutData->addAuthor("Jason Keirstead", 0, "jason@keirstead.org", "http://www.keirstead.org/" );
    aboutData->addAuthor("Michaël Larouche", 0, "larouche@kde.org" "http://www.tehbisnatch.org/" );

    return aboutData;
}

void KRichTextEditPart::createActions()
{
    d->enableRichText = new KToggleAction( KIcon("pencil"), i18n("Enable &Rich Text"), this );
    actionCollection()->addAction( "enableRichText", d->enableRichText );
    d->enableRichText->setCheckedState( KGuiItem( i18n("Disable &Rich Text") ) );
    connect( d->enableRichText, SIGNAL(toggled(bool)),
            this, SLOT(setRichTextEnabled(bool)) );

    d->checkSpelling = new KAction( KIcon("spellcheck"), i18n("Check &Spelling"), actionCollection() );
    actionCollection()->addAction( "check_spelling", d->checkSpelling );
    connect( d->checkSpelling, SIGNAL(triggered(bool)), d->editor, SLOT(checkSpelling()) );

    //Foreground Color
    d->actionTextColor = new KAction( KIcon("color_line"), i18n("Text &Color..."), actionCollection() );
    actionCollection()->addAction( "format_textcolor", d->actionTextColor );
    connect( d->actionTextColor, SIGNAL(triggered(bool)), this, SLOT(setTextColor()) );

    //Font Family
    d->action_font = new KFontAction( i18n("&Font"), actionCollection() );
    actionCollection()->addAction( "format_font", d->action_font );
    connect( d->action_font, SIGNAL(activated(QString)), this, SLOT(setFont(QString)) );

    //Font Size
    d->action_font_size = new KFontSizeAction( i18n("Font &Size"), actionCollection() );
    actionCollection()->addAction( "format_font_size", d->action_font_size );
    connect( d->action_font_size, SIGNAL(fontSizeChanged(int)), this, SLOT( setFontSize(int) ) );

    //Formatting
    d->action_bold = new KToggleAction( KIcon("text_bold"), i18n("&Bold"), actionCollection() );
    actionCollection()->addAction( "format_bold", d->action_bold );
    d->action_bold->setShortcut( KShortcut(Qt::CTRL + Qt::Key_B) );
    connect( d->action_bold, SIGNAL(toggled(bool)), this, SLOT(setFontBold(bool)) );

    d->action_italic = new KToggleAction( KIcon("text_italic"), i18n("&Italic"), actionCollection() );
    actionCollection()->addAction( "format_italic", d->action_italic );
    d->action_italic->setShortcut( KShortcut(Qt::CTRL + Qt::Key_I) );
    connect(d->action_italic, SIGNAL(toggled(bool)),
        this, SLOT(setFontItalic(bool)) );

    d->action_underline = new KToggleAction( KIcon("text_under"), i18n("&Underline"), actionCollection() );
    actionCollection()->addAction( "format_underline", d->action_underline );
    d->action_underline->setShortcut( KShortcut(Qt::CTRL + Qt::Key_U) );
    connect( d->action_underline, SIGNAL(toggled(bool)),
        this, SLOT(setFontUnderline(bool)) );

    // TODO: Port to new Qt4 signals
    connect( d->editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
        this, SLOT( updateCharFormat() ) );
    updateCharFormat();
    connect( d->editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
        this, SLOT(updateFont()) );
    updateFont();

    //Alignment
    d->action_align_left = new KToggleAction( KIcon("text_left"), i18n("Align &Left"), actionCollection() );
    actionCollection()->addAction( "format_align_left", d->action_align_left );
    connect( d->action_align_left, SIGNAL(toggled(bool)),
        this, SLOT(setAlignLeft(bool)) );

    d->action_align_center = new KToggleAction( KIcon("text_center"), i18n("Align &Center"), actionCollection() );
    actionCollection()->addAction( "format_align_center", d->action_align_center );
    connect( d->action_align_center, SIGNAL(toggled(bool)),
        this, SLOT(setAlignCenter(bool)) );

    d->action_align_right = new KToggleAction( KIcon("text_right"), i18n("Align &Right"), actionCollection() );
    actionCollection()->addAction( "format_align_right", d->action_align_right );
    connect( d->action_align_right, SIGNAL(toggled(bool)),
        this, SLOT(setAlignRight(bool)) );

    d->action_align_justify = new KToggleAction( KIcon("text_block"), i18n("&Justify"), actionCollection() );
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
    d->action_bold->setChecked(  d->editor->fontWeight() >= QFont::Bold );
    d->action_italic->setChecked( d->editor->fontItalic() );
    d->action_underline->setChecked( d->editor->fontUnderline() );
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

void KRichTextEditPart::updateFont()
{
    if( d->editor->fontPointSize() > 0 )
        d->action_font_size->setFontSize( (int)d->editor->fontPointSize() );
    d->action_font->setFont( d->editor->fontFamily() );
}

void KRichTextEditPart::readConfig()
{
    // Don't update config untill we read whole config first
    d->configWriteLock = true;
    KConfig *config = KGlobal::config();
    config->setGroup("RichTextEditor");

    QColor standardColor = KGlobalSettings::textColor();
    QColor tmpColor;
    tmpColor = config->readEntry("TextColor", standardColor );
    kDebug() << k_funcinfo << "Text color: " << tmpColor.name() << endl;

    setTextColor( tmpColor );

    QFont tmpFont = KGlobalSettings::generalFont();
    setFont( config->readEntry("Font", tmpFont ) );

    int tmp = KGlobalSettings::generalFont().pixelSize();
    setFontSize( config->readEntry( "FontSize", tmp ) );

    d->action_bold->setChecked( config->readEntry( "FontBold", false ) );
    d->action_italic->setChecked( config->readEntry( "FontItalic", false ) );
    d->action_underline->setChecked( config->readEntry( "FontUnderline", false ) );

    switch( config->readEntry( "EditAlignment", int(Qt::AlignLeft) ) )
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
    d->configWriteLock = false;
}

void KRichTextEditPart::writeConfig()
{
    // If true we're still reading the conf write now, so don't write.
    if( d->configWriteLock ) return;

    KConfig *config = KGlobal::config();

    QFont currentFont = d->editor->currentFont();

    config->setGroup("RichTextEditor");
    config->writeEntry("Font", currentFont );
    config->writeEntry("FontSize", currentFont.pointSize() );
    config->writeEntry("FontBold", currentFont.bold() );
    config->writeEntry("FontItalic", currentFont.italic() );
    config->writeEntry("FontUnderline", currentFont.underline() );
    config->writeEntry("TextColor", d->editor->textColor() );
    config->writeEntry("EditAlignment", int(d->editor->alignment()) );
    config->sync();
}

void KRichTextEditPart::setTextColor()
{
    QColor currentTextColor = d->editor->textColor();

    int result = KColorDialog::getColor( currentTextColor, KGlobalSettings::textColor() , d->editor );
    if(!currentTextColor.isValid())
        currentTextColor = KGlobalSettings::textColor() ;
    if ( result != QDialog::Accepted  )
        return;

    setTextColor( currentTextColor );

    writeConfig();
}

void KRichTextEditPart::setTextColor(const QColor &newColor)
{
    if( d->richTextSupport & KRichTextEditPart::SupportTextColor )
    {
        d->editor->setTextColor( newColor );
    }

}

QColor KRichTextEditPart::textColor() const
{
    if( d->editor->textColor() == KGlobalSettings::textColor())
        return QColor();
    return d->editor->textColor();
}

void KRichTextEditPart::setFontSize(int size)
{
    if( size < 1 )
        return;

    if( d->richTextSupport & KRichTextEditPart::SupportFont )
    {
        d->editor->setFontPointSize( size );
        writeConfig();
    }
}

void KRichTextEditPart::setFont()
{
    QFont currentFont = d->editor->currentFont();
    KFontDialog::getFont( currentFont, false, d->editor );

    setFont( currentFont );
    writeConfig();
}

void KRichTextEditPart::setFont(const QFont &newFont)
{
    if( useRichText() )
    {
        d->editor->setCurrentFont( newFont );
    }
    updateFont();
}

void KRichTextEditPart::setFont(const QString &newFont)
{
    if( (d->richTextSupport & KRichTextEditPart::SupportFont) && useRichText() )
    {
        d->editor->setFontFamily( newFont );
    }

    updateFont();
    writeConfig();
}


void KRichTextEditPart::setFontBold(bool value)
{
    if( (d->richTextSupport & KRichTextEditPart::SupportFont) && useRichText() )
    {
        d->editor->setFontWeight( value ? QFont::Bold : QFont::Normal );
    }
    writeConfig();
}

void KRichTextEditPart::setFontItalic(bool value)
{
    if( (d->richTextSupport & KRichTextEditPart::SupportItalic) && useRichText() )
    {
        d->editor->setFontItalic(value);
    }
    writeConfig();
}

void KRichTextEditPart::setFontUnderline(bool value)
{
    if( (d->richTextSupport & KRichTextEditPart::SupportUnderline) && useRichText()  )
    {
        d->editor->setFontUnderline(value);
    }
    writeConfig();
}


void KRichTextEditPart::setAlignLeft( bool yes )
{
    if( yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
        d->editor->setAlignment( Qt::AlignLeft );

    writeConfig();
}

void KRichTextEditPart::setAlignRight( bool yes )
{
    if( yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
        d->editor->setAlignment( Qt::AlignRight );

    writeConfig();
}

void KRichTextEditPart::setAlignCenter( bool yes )
{
    if( yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
        d->editor->setAlignment( Qt::AlignCenter );
    writeConfig();
}

void KRichTextEditPart::setAlignJustify( bool yes )
{
    if( yes && useRichText() && (d->richTextSupport & KRichTextEditPart::SupportAlignment) )
        d->editor->setAlignment( Qt::AlignJustify );
    writeConfig();
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
