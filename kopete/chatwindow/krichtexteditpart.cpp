#include <ktextedit.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <kparts/genericfactory.h>
#include <private/qrichtext_p.h>

#include "krichtexteditpart.h"
#include "krichtexteditpart.moc"

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

KopeteRichTextEditPart::KopeteRichTextEditPart( QWidget *parent, const char *name, bool supportsRichText )
  : KParts::ReadOnlyPart( parent, name ? name : "rich_text_part" )
{
	// we need an instance
	setInstance( KopeteRichTextEditPartFactory::instance() );

	editor = new KopeteTextEdit( parent );
	editor->setReadOnly( false );

	simpleMode = !supportsRichText;
	setWidget( editor );
	createActions();

	if( simpleMode ) //Only modidy bg and fg color and font
	{
		editor->setTextFormat( Qt::PlainText );
		setXMLFile( "kopeterichtexteditpartsimple.rc" );
	}
	else //Modify rich text
	{
		editor->setTextFormat( Qt::RichText );
		setXMLFile( "kopeterichtexteditpartfull.rc" );
	}

	#if KDE_IS_VERSION( 3, 1, 90 )
		editor->setCheckSpellingEnabled( false );
	#endif
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
	(void) new KAction( i18n("Text &Color..."), "color_line", 0,
		this, SLOT( setFgColor() ),
		ac, "format_color" );

	(void) new KAction( i18n("Background Co&lor..."), "color_fill", 0,
		this, SLOT( setBgColor() ),
		ac, "format_bgcolor" );

	action_font = new KFontAction( i18n("&Font"), 0,
			ac, "format_font" );
	connect( action_font, SIGNAL( activated( const QString & ) ),
		this, SLOT( setFont( const QString & ) ) );

	action_font_size = new KFontSizeAction( i18n("Font &Size"), 0,
			ac, "format_font_size" );
	connect( action_font_size, SIGNAL( fontSizeChanged(int) ),
		editor, SLOT( setPointSize(int) ) );

	if( !simpleMode )
	{
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

		connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
			this, SLOT( updateCharFmt() ) );
		connect( editor, SIGNAL( cursorPositionChanged( int,int ) ),
			this, SLOT( updateAligment() ) );

		updateCharFmt();
		updateAligment();
	}

	connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
		this, SLOT( updateFont() ) );

	updateFont();
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

	if( !simpleMode )
	{
		editor->setBold( action_bold->isChecked() );
		editor->setItalic( action_italic->isChecked() );
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

void KopeteRichTextEditPart::setFgColor()
{
	QColor col;

	int s = KColorDialog::getColor( col, editor->color(), editor );
	if ( s != QDialog::Accepted )
		return;

	setFgColor( col );
}

void KopeteRichTextEditPart::setFgColor( const QColor &newColor )
{
	mFgColor = newColor;

	if( simpleMode )
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
}

void KopeteRichTextEditPart::setFont( const QFont &newFont )
{
	mFont = newFont;
	editor->setFont(mFont);
}

void KopeteRichTextEditPart::setFont( const QString &newFont )
{
	editor->setFamily(newFont);
	mFont = editor->currentFont();
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
