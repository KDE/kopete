#include <ktextedit.h>
#include <kaction.h>
#include <kstdaction.h>
#include <ktoggleaction.h>
#include <kfontaction.h>
#include <kfontsizeaction.h>
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
#include <kaction.h>
#include <ktoggleaction.h>
#include <kfontsizeaction.h>
#include <kactionmenu.h>
#include <kfontaction.h>
#include <kicon.h>
//#include <private/q3richtext_p.h>
//#include <Q3RichText>
#include <QTextCursor>

#include "krichtexteditpart.h"
#include "krichtexteditpart.moc"
#include "kopeteprotocol.h"
#include "kopeteappearancesettings.h"

typedef KParts::GenericFactory<KopeteRichTextEditPart> KopeteRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkopeterichtexteditpart, KopeteRichTextEditPartFactory )
			

class KopeteTextEdit : public KTextEdit
{
public:
	KopeteTextEdit( QWidget *parent ) : KTextEdit( parent ) {}
//	const Q3TextCursor * cursor() { return textCursor(); }
	bool event(QEvent *event)
	{
		// don't allow QTextEdit to override accels
		if ( event->type() == QEvent::AccelOverride )
			return QWidget::event(event);
		else
			return KTextEdit::event(event);
	}
};



KopeteRichTextEditPart::KopeteRichTextEditPart( QWidget *wparent, QObject*, const QStringList& )
	: KParts::ReadOnlyPart( wparent )
{
	KopeteRichTextEditPart::KopeteRichTextEditPart( wparent, false );
}

KTextEdit* KopeteRichTextEditPart::editorWidget()
{
	return editor;
}

KopeteRichTextEditPart::KopeteRichTextEditPart( QWidget *parent, int capabilities )
  : KParts::ReadOnlyPart( parent),
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
#warning spellchecker crash in kde4
#if 0	
	editor->setCheckSpellingEnabled( !richTextEnabled() );
	checkSpelling->setEnabled( !richTextEnabled() );
#endif
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
	enableRichText = new KToggleAction( KIcon("pencil"), i18n("Enable &Rich Text"), ac, "enableRichText" );
	enableRichText->setCheckedState( KGuiItem( i18n("Disable &Rich Text") ) );
	connect( enableRichText, SIGNAL( toggled(bool) ),
			this, SLOT( slotSetRichTextEnabled(bool) ) );

	checkSpelling = new KAction( KIcon("spellcheck"), i18n("Check &Spelling"), ac, "check_spelling" );
	connect( checkSpelling, SIGNAL( triggered(bool) ), editor, SLOT( checkSpelling() ) );

	//Fg Color
	actionFgColor = new KAction( KIcon("color_line"), i18n("Text &Color..."), ac, "format_color" );
	connect( actionFgColor, SIGNAL( triggered(bool) ), this, SLOT( setFgColor() ) );

	//BG Color
	actionBgColor = new KAction( KIcon("color_fill"), i18n("Background Co&lor..."), ac, "format_bgcolor" );
	connect( actionBgColor, SIGNAL( triggered(bool) ), this, SLOT( setBgColor() ) );

	//Font Family
	action_font = new KFontAction( i18n("&Font"), ac, "format_font" );
	connect( action_font, SIGNAL( activated( const QString & ) ),
		this, SLOT( setFont( const QString & ) ) );

	//Font Size
	action_font_size = new KFontSizeAction( i18n("Font &Size"), ac, "format_font_size" );
	connect( action_font_size, SIGNAL( fontSizeChanged(int) ),
		this, SLOT( setFontSize(int) ) );

	//Formatting
	action_bold = new KToggleAction( KIcon("text_bold"), i18n("&Bold"), ac, "format_bold" );
	action_bold->setShortcut( KShortcut(Qt::CTRL + Qt::Key_B) );
	connect( action_bold, SIGNAL( toggled(bool) ),
		this, SLOT( setBold(bool) ) );

	action_italic = new KToggleAction( KIcon("text_italic"), i18n("&Italic"), ac, "format_italic" );
	action_italic->setShortcut( KShortcut(Qt::CTRL + Qt::Key_I) );
	connect( action_italic, SIGNAL( toggled(bool) ),
		this, SLOT( setItalic(bool) ) );

	action_underline = new KToggleAction( KIcon("text_under"), i18n("&Underline"), ac, "format_underline" );
	action_underline->setShortcut( KShortcut(Qt::CTRL + Qt::Key_U) );
	connect( action_underline, SIGNAL( toggled(bool) ),
		this, SLOT( setUnderline(bool) ) );

	connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
		this, SLOT( updateCharFmt() ) );
	updateCharFmt();

	connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
		this, SLOT( updateFont() ) );
	updateFont();

	//Alignment
	action_align_left = new KToggleAction( KIcon("text_left"), i18n("Align &Left"), ac, "format_align_left" );
	connect( action_align_left, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignLeft(bool) ) );

	action_align_center = new KToggleAction( KIcon("text_center"), i18n("Align &Center"), ac, "format_align_center" );
	connect( action_align_center, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignCenter(bool) ) );

	action_align_right = new KToggleAction( KIcon("text_right"), i18n("Align &Right"), ac, "format_align_right" );
	connect( action_align_right, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignRight(bool) ) );

	action_align_justify = new KToggleAction( KIcon("text_block"), i18n("&Justify"), ac, "format_align_justify" );
	connect( action_align_justify, SIGNAL( toggled(bool) ),
		this, SLOT( setAlignJustify(bool) ) );

	QActionGroup *alignmentGroup = new QActionGroup(this);
	alignmentGroup->addAction(action_align_left);
	alignmentGroup->addAction(action_align_center);
	alignmentGroup->addAction(action_align_right);
	alignmentGroup->addAction(action_align_justify);

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
		case Qt::AlignRight:
			action_align_right->setChecked( true );
			break;
		case Qt::AlignCenter:
			action_align_center->setChecked( true );
			break;
		case Qt::AlignLeft:
			action_align_left->setChecked( true );
			break;
		case Qt::AlignJustify:
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
	setFgColor( config->readEntry("FgColor", tmpColor ) );

	tmpColor = KGlobalSettings::baseColor();
	setBgColor( config->readEntry("BgColor", tmpColor ) );

	QFont tmpFont = Kopete::AppearanceSettings::self()->chatFont();
	setFont( config->readEntry("Font", tmpFont ) );

	int tmp = KGlobalSettings::generalFont().pixelSize();
	setFontSize( config->readEntry( "FontSize", tmp ) );

	action_bold->setChecked( config->readEntry( "FontBold", false ) );
	action_italic->setChecked( config->readEntry( "FontItalic", false ) );
	action_underline->setChecked( config->readEntry( "FontUnderline", false ) );

#warning Find a what to port "config->readNumEntry( "EditAlignment", Qt::AlignLeft )".
	switch( config->readEntry( "EditAlignment", int(Qt::AlignLeft) ) )
	{
		case Qt::AlignLeft:
			action_align_left->trigger();
		break;
		case Qt::AlignCenter:
			action_align_center->trigger();
		break;
		case Qt::AlignRight:
			action_align_right->trigger();
		break;
		case Qt::AlignJustify:
			action_align_justify->trigger();
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
	config->writeEntry("EditAlignment", QVariant(editor->alignment()) );
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
		pal.setColor(QPalette::Active, QPalette::Text, mFgColor );
		pal.setColor(QPalette::Inactive, QPalette::Text, mFgColor );

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

QColor KopeteRichTextEditPart::bgColor()
{
	if( mBgColor == KGlobalSettings::baseColor())
		return QColor();
	return mBgColor;
}

void KopeteRichTextEditPart::setBgColor( const QColor &newColor )
{
	mBgColor = newColor;

	QPalette pal = editor->palette();
	pal.setColor(QPalette::Active, QPalette::Base, mBgColor );
	pal.setColor(QPalette::Inactive, QPalette::Base, mBgColor );
	pal.setColor(QPalette::Disabled, QPalette::Base, mBgColor );

	if ( pal == QApplication::palette( editor ) )
		editor->unsetPalette();
	else
		editor->setPalette(pal);
}

void KopeteRichTextEditPart::setFontSize( int size )
{
	if( size < 1 )
		return;
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
		editor->setAlignment( Qt::AlignLeft );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignRight( bool yes )
{
	if ( yes )
		editor->setAlignment( Qt::AlignRight );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignCenter( bool yes )
{
	if ( yes )
		editor->setAlignment( Qt::AlignCenter );
	writeConfig();
}

void KopeteRichTextEditPart::setAlignJustify( bool yes )
{
	if ( yes )
		editor->setAlignment( Qt::AlignJustify );
	writeConfig();
}

QString KopeteRichTextEditPart::text( Qt::TextFormat fmt ) const
{
#warning  Commented to make it compile with qt4
//	if( fmt == editor->textFormat() || fmt != Qt::PlainText )
		return editor->text();
//	else
//		return editor->cursor()->document()->plainText();
}

