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
#include <kparts/genericfactory.h>
#include <private/qrichtext_p.h>

#include "krichtexteditpart.h"
#include "krichtexteditpart.moc"
#include "kopeteprotocol.h"

typedef KParts::GenericFactory<KopeteRichTextEditPart> KopeteRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkopeterichtexteditpart,  KopeteRichTextEditPartFactory )

class KopeteTextEdit : public KTextEdit
{
	public:
		KopeteTextEdit( QWidget *parent ) : KTextEdit( parent ) {};
		const QTextCursor * cursor() { return textCursor(); };
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
		m_capabilities & KopeteProtocol::RichFormatting ||
		m_capabilities & KopeteProtocol::Alignment ||
		m_capabilities & KopeteProtocol::RichFont ||
		m_capabilities & KopeteProtocol::RichColor
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
	if( enable )
	{
		editor->setTextFormat( Qt::RichText );
	}
	else
	{
		editor->setTextFormat( Qt::PlainText );
	}

	m_richTextEnabled = enable;
	emit toggleToolbar( buttonsEnabled() );

	// Spellchecking disabled when using rich text because the
	// text we were getting from widget was coloured HTML!
	editor->setCheckSpellingEnabled( !richTextEnabled() );
	checkSpelling->setEnabled( !richTextEnabled() );

	//Enable / disable buttons
	updateActions();
}

void KopeteRichTextEditPart::checkToolbarEnabled()
{
	emit toggleToolbar( buttonsEnabled() );
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
#if KDE_IS_VERSION(3,2,90)
	enableRichText->setCheckedState(i18n("Disable &Rich Text"));
#endif
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
		editor, SLOT( setPointSize(int) ) );

	//Formatting
	action_bold = new KToggleAction( i18n("&Bold"), "text_bold", CTRL+Key_B,
			ac, "format_bold" );
	connect( action_bold, SIGNAL( toggled(bool) ),
		editor, SLOT( setBold(bool) ) );

	action_italic = new KToggleAction( i18n("&Italic"), "text_italic", CTRL+Key_I,
			ac, "format_italic" );
	connect( action_italic, SIGNAL( toggled(bool) ),
		editor, SLOT( setItalic(bool) ) );

	action_underline = new KToggleAction( i18n("&Underline"), "text_under", CTRL+Key_U,
				ac, "format_underline" );
	connect( action_underline, SIGNAL( toggled(bool) ),
		editor, SLOT( setUnderline(bool) ) );

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
	bool enableFgColor = m_capabilities & KopeteProtocol::BaseFgColor || m_capabilities & KopeteProtocol::RichFgColor;
	bool enableBGColor = m_capabilities & KopeteProtocol::BaseBgColor || m_capabilities & KopeteProtocol::RichBgColor;
	bool activateAlignment = buttonsEnabled && ( m_capabilities & KopeteProtocol::Alignment );
	bool activateFont = m_capabilities & KopeteProtocol::BaseFont || m_capabilities & KopeteProtocol::RichFont;

	bool activateBFormat = m_capabilities & KopeteProtocol::BaseBFormatting || m_capabilities & KopeteProtocol::RichBFormatting;
	bool activateUFormat = m_capabilities & KopeteProtocol::BaseUFormatting || m_capabilities & KopeteProtocol::RichUFormatting;
	bool activateIFormat = m_capabilities & KopeteProtocol::BaseIFormatting || m_capabilities & KopeteProtocol::RichIFormatting;

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

	if( m_capabilities & KopeteProtocol::BaseBFormatting || m_capabilities & KopeteProtocol::RichBFormatting )
	{
		editor->setBold( action_bold->isChecked() );
	}
	if( m_capabilities & KopeteProtocol::BaseIFormatting || m_capabilities & KopeteProtocol::RichIFormatting )
	{
		editor->setItalic( action_italic->isChecked() );
	}
	if( m_capabilities & KopeteProtocol::BaseUFormatting || m_capabilities & KopeteProtocol::RichUFormatting )
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
	KConfig *config = KGlobal::config();
	config->setGroup("RichTextEditor");

	QColor tmpColor = KGlobalSettings::textColor();
	setFgColor( config->readColorEntry("FgColor", &tmpColor ) );

	tmpColor = KGlobalSettings::baseColor();
	setBgColor( config->readColorEntry("BgColor", &tmpColor ) );

	QFont tmpFont = KGlobalSettings::generalFont();
	setFont( config->readFontEntry("Font", &tmpFont ) );
}

void KopeteRichTextEditPart::writeConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup("RichTextEditor");
	config->writeEntry("Font", mFont );
	config->writeEntry("BgColor", mBgColor );
	config->writeEntry("FgColor", mFgColor );
	config->sync();
}

void KopeteRichTextEditPart::setFgColor()
{
	QColor col;

	int s = KColorDialog::getColor( col, editor->color(), editor );
	if ( s != QDialog::Accepted )
		return;

	setFgColor( col );

	writeConfig();
}

void KopeteRichTextEditPart::setFgColor( const QColor &newColor )
{
	mFgColor = newColor;

	if( !(m_capabilities & KopeteProtocol::RichColor) )
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

void KopeteRichTextEditPart::setBgColor()
{
	QColor col;

	int s = KColorDialog::getColor( col, mBgColor, editor );
	if ( s != QDialog::Accepted )
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

void KopeteRichTextEditPart::setFont()
{
	KFontDialog::getFont(mFont, false, editor);
	setFont(mFont);
	updateFont();
	writeConfig();
}

void KopeteRichTextEditPart::setFont( const QFont &newFont )
{
	mFont = newFont;
	editor->setFont(mFont);
}

void KopeteRichTextEditPart::setFont( const QString &newFont )
{
	mFont = QFont( newFont );
	editor->setFont(mFont);
	writeConfig();
}

void KopeteRichTextEditPart::setAlignLeft( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignLeft );
}

void KopeteRichTextEditPart::setAlignRight( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignRight );
}

void KopeteRichTextEditPart::setAlignCenter( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignCenter );
}

void KopeteRichTextEditPart::setAlignJustify( bool yes )
{
	if ( yes )
		editor->setAlignment( AlignJustify );
}

const QString KopeteRichTextEditPart::text( Qt::TextFormat fmt ) const
{
	if( fmt == editor->textFormat() || fmt != Qt::PlainText )
		return editor->text();
	else
		return editor->cursor()->document()->plainText();
}
