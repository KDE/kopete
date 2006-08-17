#include <ktextedit.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kglobalsettings.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qevent.h>
#include <kparts/genericfactory.h>
#include <private/qrichtext_p.h>

#include "krichtexteditpart.h"
#include "krichtexteditpart.moc"
#include "kopeteprotocol.h"
#include "kopeteprefs.h"

typedef KParts::GenericFactory<KopeteRichTextEditPart> KopeteRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkopeterichtexteditpart, KopeteRichTextEditPartFactory )

class KopeteTextEdit : public KTextEdit
{
public:
	KopeteTextEdit( QWidget *parent ) : KTextEdit( parent ) {}
	const QTextCursor * cursor() { return textCursor(); }
	bool event(QEvent *event)
	{
		// don't allow QTextEdit to override accels
		if ( event->type() == QEvent::AccelOverride )
			return QWidget::event(event);
		else
			return KTextEdit::event(event);
	}
};

KopeteRichTextEditPart::KopeteRichTextEditPart( QWidget *wparent, const char *wname, QObject*, const char*, const QStringList& )
	: KParts::ReadOnlyPart( wparent, wname ? wname : "rich_text_part" )
{
	KopeteRichTextEditPart::KopeteRichTextEditPart( wparent, wname, false );
}

KopeteRichTextEditPart::KopeteRichTextEditPart( QWidget *parent, const char *name, int capabilities )
  : KParts::ReadOnlyPart( parent, name ? name : "rich_text_part" ),
	m_capabilities( capabilities ),
	m_richTextEnabled( true )
{
	// we need an instance
	setInstance( KopeteRichTextEditPartFactory::instance() );

	editor = new KopeteTextEdit( parent );
	editor->setReadOnly( false );

	setWidget( editor );

	m_richTextAvailable = (
		m_capabilities & Kopete::Protocol::RichFormatting ||
		m_capabilities & Kopete::Protocol::Alignment ||
		m_capabilities & Kopete::Protocol::RichFont ||
		m_capabilities & Kopete::Protocol::RichColor
	);

	createActions();

	setXMLFile( "kopeterichtexteditpartfull.rc" );
	enableRichText->setEnabled( m_richTextAvailable );
	enableRichText->setChecked( m_richTextAvailable );
	slotSetRichTextEnabled( m_richTextAvailable );

	//Set colors, font
	readConfig();
}

void KopeteRichTextEditPart::slotSetRichTextEnabled( bool enable )
{
	m_richTextEnabled = enable && m_richTextAvailable;

	if( m_richTextEnabled )
	{
		editor->setTextFormat( Qt::RichText );
	}
	else
	{
		editor->setTextFormat( Qt::PlainText );
	}

	emit toggleToolbar( buttonsEnabled() );

	// Spellchecking disabled when using rich text because the
	// text we were getting from widget was coloured HTML!
	editor->setCheckSpellingEnabled( !m_richTextEnabled );
	checkSpelling->setEnabled( !m_richTextEnabled );

	//Enable / disable buttons
	updateActions();
	enableRichText->setChecked( m_richTextEnabled );
}

void KopeteRichTextEditPart::checkToolbarEnabled()
{
	emit toggleToolbar( buttonsEnabled() );
}

void KopeteRichTextEditPart::reloadConfig()
{
	readConfig();
}

void KopeteRichTextEditPart::createActions()
{
	createActions( actionCollection() );
}

KAboutData *KopeteRichTextEditPart::createAboutData()
{
	KAboutData *aboutData = new KAboutData("kopeterichtexteditpart", I18N_NOOP("KopeteRichTextEditPart"), "0.1",
						I18N_NOOP("A simple rich text editor part for Kopete"),
						KAboutData::License_LGPL );
	aboutData->addAuthor("Richard J. Moore", 0, "rich@kde.org", "http://xmelegance.org/" );
	aboutData->addAuthor("Jason Keirstead", 0, "jason@keirstead.org", "http://www.keirstead.org/" );
	return aboutData;
}

void KopeteRichTextEditPart::createActions( KActionCollection *ac )
{
	enableRichText = new KToggleAction(i18n("Enable &Rich Text"), "pencil", 0,
				ac, "enableRichText" );
	enableRichText->setCheckedState(i18n("Disable &Rich Text"));
	connect( enableRichText, SIGNAL( toggled(bool) ),
			this, SLOT( slotSetRichTextEnabled(bool) ) );

	checkSpelling = new KAction( i18n("Check &Spelling"), "spellcheck", 0,
				editor, SLOT( checkSpelling() ), ac, "check_spelling" );

	//Fg Color
	actionFgColor = new KAction( i18n("Text &Color..."), "color_line", 0,
		this, SLOT( setFgColor() ),
		ac, "format_color" );

	//BG Color
	actionBgColor = new KAction( i18n("Background Co&lor..."), "color_fill", 0,
		this, SLOT( setBgColor() ),
		ac, "format_bgcolor" );

	//Font Family
	action_font = new KFontAction( i18n("&Font"), 0,
			ac, "format_font" );
	connect( action_font, SIGNAL( activated( const QString & ) ),
		this, SLOT( setFont( const QString & ) ) );

	//Font Size
	action_font_size = new KFontSizeAction( i18n("Font &Size"), 0,
			ac, "format_font_size" );
	connect( action_font_size, SIGNAL( fontSizeChanged(int) ),
		this, SLOT( setFontSize(int) ) );

	//Formatting
	action_bold = new KToggleAction( i18n("&Bold"), "text_bold", CTRL+Key_B,
			ac, "format_bold" );
	connect( action_bold, SIGNAL( toggled(bool) ),
		this, SLOT( setBold(bool) ) );

	action_italic = new KToggleAction( i18n("&Italic"), "text_italic", CTRL+Key_I,
			ac, "format_italic" );
	connect( action_italic, SIGNAL( toggled(bool) ),
		this, SLOT( setItalic(bool) ) );

	action_underline = new KToggleAction( i18n("&Underline"), "text_under", CTRL+Key_U,
				ac, "format_underline" );
	connect( action_underline, SIGNAL( toggled(bool) ),
		this, SLOT( setUnderline(bool) ) );

	connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
		this, SLOT( updateCharFmt() ) );
	updateCharFmt();

	connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
		this, SLOT( updateFont() ) );
	updateFont();

	//Alignment
	action_align_left = new KToggleAction( i18n("Align &Left"), "text_left", 0,
			ac, "format_align_left" );
	connect( action_align_left, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignLeft(bool) ) );

	action_align_center = new KToggleAction( i18n("Align &Center"), "text_center", 0,
			ac, "format_align_center" );
	connect( action_align_center, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignCenter(bool) ) );

	action_align_right = new KToggleAction( i18n("Align &Right"), "text_right", 0,
			ac, "format_align_right" );
	connect( action_align_right, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignRight(bool) ) );

	action_align_justify = new KToggleAction( i18n("&Justify"), "text_block", 0,
			ac, "format_align_justify" );
	connect( action_align_justify, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignJustify(bool) ) );

	action_align_left->setExclusiveGroup( "alignment" );
	action_align_center->setExclusiveGroup( "alignment" );
	action_align_right->setExclusiveGroup( "alignment" );
	action_align_justify->setExclusiveGroup( "alignment" );

	connect( editor, SIGNAL( cursorPositionChanged( int,int ) ),
		this, SLOT( updateAligment() ) );

	updateAligment();
}

void KopeteRichTextEditPart::updateActions()
{
	bool buttonsEnabled = this->buttonsEnabled();
	bool enableFgColor = m_capabilities & Kopete::Protocol::BaseFgColor || m_capabilities & Kopete::Protocol::RichFgColor;
	bool enableBGColor = m_capabilities & Kopete::Protocol::BaseBgColor || m_capabilities & Kopete::Protocol::RichBgColor;
	bool activateAlignment = buttonsEnabled && ( m_capabilities & Kopete::Protocol::Alignment );
	bool activateFont = m_capabilities & Kopete::Protocol::BaseFont || m_capabilities & Kopete::Protocol::RichFont;

	bool activateBFormat = m_capabilities & Kopete::Protocol::BaseBFormatting || m_capabilities & Kopete::Protocol::RichBFormatting;
	bool activateUFormat = m_capabilities & Kopete::Protocol::BaseUFormatting || m_capabilities & Kopete::Protocol::RichUFormatting;
	bool activateIFormat = m_capabilities & Kopete::Protocol::BaseIFormatting || m_capabilities & Kopete::Protocol::RichIFormatting;

	actionFgColor->setEnabled( buttonsEnabled && enableFgColor );
	actionBgColor->setEnabled( buttonsEnabled && enableBGColor );

	action_font->setEnabled( buttonsEnabled && activateFont );
	action_font_size->setEnabled( buttonsEnabled && activateFont );

	action_bold->setEnabled( buttonsEnabled && activateBFormat );
	action_italic->setEnabled( buttonsEnabled && activateIFormat );
	action_underline->setEnabled( buttonsEnabled && activateUFormat );

	action_align_left->setEnabled( activateAlignment );
	action_align_center->setEnabled( activateAlignment );
	action_align_right->setEnabled( activateAlignment );
	action_align_justify->setEnabled( activateAlignment );
}

void KopeteRichTextEditPart::updateCharFmt()
{
	action_bold->setChecked( editor->bold() );
	action_italic->setChecked( editor->italic() );
	action_underline->setChecked( editor->underline() );
}

void KopeteRichTextEditPart::clear()
{
	editor->setText( QString::null );
	setFont( mFont );
	setFgColor( mFgColor );

	if( m_capabilities & Kopete::Protocol::BaseBFormatting || m_capabilities & Kopete::Protocol::RichBFormatting )
	{
		editor->setBold( action_bold->isChecked() );
	}
	if( m_capabilities & Kopete::Protocol::BaseIFormatting || m_capabilities & Kopete::Protocol::RichIFormatting )
	{
		editor->setItalic( action_italic->isChecked() );
	}
	if( m_capabilities & Kopete::Protocol::BaseUFormatting || m_capabilities & Kopete::Protocol::RichUFormatting )
	{
		editor->setUnderline( action_underline->isChecked() );
	}
}

void KopeteRichTextEditPart::updateAligment()
{
	int align = editor->alignment();

	switch ( align )
	{
		case AlignRight:
			action_align_right->setChecked( true );
			break;
		case AlignCenter:
			action_align_center->setChecked( true );
			break;
		case AlignLeft:
			action_align_left->setChecked( true );
			break;
		case AlignJustify:
			action_align_justify->setChecked( true );
			break;
		default:
			break;
	}
}

void KopeteRichTextEditPart::updateFont()
{
	if ( editor->pointSize() > 0 )
		action_font_size->setFontSize( editor->pointSize() );
	action_font->setFont( editor->family() );
}

void KopeteRichTextEditPart::readConfig()
{
	// Don't update config untill we read whole config first
	m_configWriteLock = true;
	KConfig *config = KGlobal::config();
	config->setGroup("RichTextEditor");

	QColor tmpColor = KGlobalSettings::textColor();
	setFgColor( config->readColorEntry("FgColor", &tmpColor ) );

	tmpColor = KGlobalSettings::baseColor();
	setBgColor( config->readColorEntry("BgColor", &tmpColor ) );

	QFont tmpFont = KopetePrefs::prefs()->fontFace();
	setFont( config->readFontEntry("Font", &tmpFont ) );

	int tmp = KGlobalSettings::generalFont().pixelSize();
	setFontSize( config->readNumEntry( "FontSize", tmp ) );

	action_bold->setChecked( config->readBoolEntry( "FontBold" ) );
	action_italic->setChecked( config->readBoolEntry( "FontItalic" ) );
	action_underline->setChecked( config->readBoolEntry( "FontUnderline" ) );

	switch( config->readNumEntry( "EditAlignment", AlignLeft ) )
	{
		case AlignLeft:
			action_align_left->activate();
		break;
		case AlignCenter:
			action_align_center->activate();
		break;
		case AlignRight:
			action_align_right->activate();
		break;
		case AlignJustify:
			action_align_justify->activate();
		break;
	}
	m_configWriteLock = false;
}

void KopeteRichTextEditPart::writeConfig()
{
	// If true we're still reading the conf write now, so don't write.
	if( m_configWriteLock ) return;

	KConfig *config = KGlobal::config();
	config->setGroup("RichTextEditor");
	config->writeEntry("Font", mFont );
	config->writeEntry("FontSize", mFont.pointSize() );
	config->writeEntry("FontBold", mFont.bold() );
	config->writeEntry("FontItalic", mFont.italic() );
	config->writeEntry("FontUnderline", mFont.underline() );
	config->writeEntry("BgColor", mBgColor );
	config->writeEntry("FgColor", mFgColor );
	config->writeEntry("EditAlignment", editor->alignment() );
	config->sync();
}

void KopeteRichTextEditPart::setFgColor()
{
	QColor col=editor->color();

	int s = KColorDialog::getColor( col, KGlobalSettings::textColor() , editor );
	if(!col.isValid())
		col= KGlobalSettings::textColor() ;
	if ( s != QDialog::Accepted  )
		return;

	setFgColor( col );

	writeConfig();
}

void KopeteRichTextEditPart::setFgColor( const QColor &newColor )
{
	mFgColor = newColor;

	if( !(m_capabilities & Kopete::Protocol::RichColor) )
	{
		QPalette pal = editor->palette();
		pal.setColor(QPalette::Active, QColorGroup::Text, mFgColor );
		pal.setColor(QPalette::Inactive, QColorGroup::Text, mFgColor );

		if ( pal == QApplication::palette( editor ) )
			editor->unsetPalette();
		else
			editor->setPalette(pal);
	}

	editor->setColor( mFgColor );
}

QColor KopeteRichTextEditPart::fgColor()
{
	if( mFgColor == KGlobalSettings::textColor())
		return QColor();
	return mFgColor;
}

void KopeteRichTextEditPart::setBgColor()
{
	QColor col=mBgColor;

	int s = KColorDialog::getColor( col, KGlobalSettings::baseColor(), editor );
	if(!col.isValid())
	{
		col=KGlobalSettings::baseColor();
	}

	if ( s != QDialog::Accepted  )
		return;

	setBgColor( col );

	writeConfig();
}

void KopeteRichTextEditPart::setBgColor( const QColor &newColor )
{
	mBgColor = newColor;

	QPalette pal = editor->palette();
	pal.setColor(QPalette::Active, QColorGroup::Base, mBgColor );
	pal.setColor(QPalette::Inactive, QColorGroup::Base, mBgColor );
	pal.setColor(QPalette::Disabled, QColorGroup::Base, mBgColor );

	if ( pal == QApplication::palette( editor ) )
		editor->unsetPalette();
	else
		editor->setPalette(pal);
}

QColor KopeteRichTextEditPart::bgColor()
{
	if( mBgColor == KGlobalSettings::baseColor())
		return QColor();
	return mBgColor;
}

void KopeteRichTextEditPart::setFontSize( int size )
{
	mFont.setPointSize( size );
	if( m_capabilities & Kopete::Protocol::RichFont )
		editor->setPointSize( size );
	else if( m_capabilities & Kopete::Protocol::BaseFont)
		editor->setFont( mFont );
	writeConfig();
}

void KopeteRichTextEditPart::setFont()
{
	KFontDialog::getFont(mFont, false, editor);
	setFont(mFont);
	writeConfig();
}

void KopeteRichTextEditPart::setFont( const QFont &newFont )
{
	mFont = newFont;
	editor->setFont(mFont);
	updateFont();
}

void KopeteRichTextEditPart::setFont( const QString &newFont )
{
	mFont.setFamily( newFont );
	if( m_capabilities & Kopete::Protocol::RichFont)
		editor->setFamily( newFont );
	else if( m_capabilities & Kopete::Protocol::BaseFont)
		editor->setFont( mFont );
	updateFont();
	writeConfig();
}


void KopeteRichTextEditPart::setBold( bool b )
{
	mFont.setBold(b);
	if( m_capabilities & Kopete::Protocol::RichBFormatting || m_capabilities & Kopete::Protocol::BaseBFormatting )
	{
		if( m_richTextEnabled )
			editor->setBold(b);
		else
			editor->setFont(mFont);
	}
	writeConfig();
}

void KopeteRichTextEditPart::setItalic( bool b )
{
	mFont.setItalic( b );
	if( m_capabilities & Kopete::Protocol::RichIFormatting ||  m_capabilities & Kopete::Protocol::BaseIFormatting )
	{
		if(m_richTextEnabled)
			editor->setItalic(b);
		else
			editor->setFont(mFont);
	}
	writeConfig();
}

void KopeteRichTextEditPart::setUnderline( bool b )
{
	mFont.setUnderline( b );
	if( m_capabilities & Kopete::Protocol::RichUFormatting ||  m_capabilities & Kopete::Protocol::BaseUFormatting  )
	{
		if(m_richTextEnabled)
			editor->setUnderline(b);
		else
			editor->setFont(mFont);
	}
	writeConfig();
}


void KopeteRichTextEditPart::setAlignLeft( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignLeft );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignRight( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignRight );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignCenter( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignCenter );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignJustify( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignJustify );
	writeConfig();
}

QString KopeteRichTextEditPart::text( Qt::TextFormat fmt ) const
{
	if( fmt == editor->textFormat() || fmt != Qt::PlainText )
		return editor->text();
	else
		return editor->cursor()->document()->plainText();
}

