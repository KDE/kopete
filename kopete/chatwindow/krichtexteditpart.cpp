#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>

#include <ktextedit.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kparts/genericfactory.h>
#include <kspell.h>
#include <kprinter.h>
#include <kreplace.h>

#include "krichtexteditpart.h"
#include "krichtexteditpart.moc"

struct Data
{
    KFind *find;
    KReplace *replace;
};

typedef KParts::GenericFactory<KRichTextEditPart> KRichTextEditPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkrichtexteditpart,  KRichTextEditPartFactory )

KRichTextEditPart::KRichTextEditPart( QWidget *wparent, const char *wname,
				      QObject *parent, const char *name,
				      const QStringList &/*args*/ )
  : KParts::ReadWritePart( parent, name ? name : "rich_text_part" )
{
    // we need an instance
    setInstance( KRichTextEditPartFactory::instance() );

    editor = new KTextEdit( wparent, wname ? wname : "edit_widget" );
    editor->setReadOnly( false );
    editor->setTextFormat( Qt::RichText );
    setWidget( editor );

    d = new Data;

    createActions();

    setXMLFile( "krichtexteditpartui.rc" );
    setReadWrite( true );
}


KRichTextEditPart::~KRichTextEditPart()
{
    delete d;
}

KAboutData *KRichTextEditPart::createAboutData()
{
    // the non-i18n name here must be the same as the directory in
    // which the part's rc file is installed ('partrcdir' in the
    // Makefile)
    KAboutData *aboutData = new KAboutData("krichtexteditpart", I18N_NOOP("KRichTextEditPart"), "0.1",
					   I18N_NOOP("A simple rich text editor part"),
					   KAboutData::License_LGPL );
    aboutData->addAuthor("Richard J. Moore", 0, "rich@kde.org", "http://xmelegance.org/" );
    return aboutData;
}

void KRichTextEditPart::setReadWrite( bool rw )
{
    editor->setReadOnly( !rw );
    if ( rw ) {
	// Handle modified
    }
    else {
	// Handle modified
    }

    ReadWritePart::setReadWrite( rw );
}

bool KRichTextEditPart::openFile()
{
    QFile file( filename() );
    if ( file.open( IO_ReadOnly ) && file.isDirectAccess() ) {
	QTextStream ts( &file );

	QString s = ts.read();
	editor->setText( s );
	editor->setTextFormat( Qt::RichText );

	return true;
    }

    return false;
}

bool KRichTextEditPart::saveFile()
{
    if ( !isReadWrite() )
	return false;

    // Do save...
    QFile file( filename() );
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream ts( &file );
	ts << editor->text();

	return true;
    }

    return false;
}

bool KRichTextEditPart::print()
{
    KPrinter printer;
    printer.setFullPage(true);

    if ( printer.setup( editor ) )
    {
        QPainter p( &printer );
        QPaintDeviceMetrics metrics(p.device());
        int dpix = metrics.logicalDpiX();
        int dpiy = metrics.logicalDpiY();
        const int margin = 72; // pt
        QRect body(margin*dpix/72, margin*dpiy/72,
            metrics.width()-margin*dpix/72*2,
            metrics.height()-margin*dpiy/72*2 );
        QSimpleRichText richText( editor->text(), QFont(), editor->context(), editor->styleSheet(),
                    editor->mimeSourceFactory(), body.height() );
        richText.setWidth( &p, body.width() );
        QRect view( body );
        int page = 1;
        while (true)
        {
            richText.draw( &p, body.left(), body.top(), view, editor->colorGroup() );
            view.moveBy( 0, body.height() );
            p.translate( 0 , -body.height() );
            p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
                    view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
            if ( view.top()  >= richText.height() )
                break;
            printer.newPage();
            page++;
        }
        return true;
    }

    return false;
}

//
// Define Actions
//

void KRichTextEditPart::createActions()
{
    createActions( actionCollection() );
}

void KRichTextEditPart::createActions( KActionCollection *ac )
{
    //
    // File Actions
    //
    (void) KStdAction::open( this, SLOT( open() ), ac );
    (void) KStdAction::openRecent( this, SLOT( openURL( const KURL &) ), ac );
    (void) KStdAction::save( this, SLOT( save() ), ac );
    (void) KStdAction::saveAs( this, SLOT( saveAs() ), ac );

    //
    // Edit Actions
    //
    KAction *action_undo = KStdAction::undo( editor, SLOT( undo() ), ac );
    action_undo->setEnabled( false );
    connect( editor, SIGNAL( undoAvailable(bool) ),
         action_undo, SLOT( setEnabled(bool) ) );

    KAction *action_redo = KStdAction::redo( editor, SLOT( redo() ), ac );
    action_redo->setEnabled( false );
    connect( editor, SIGNAL( redoAvailable(bool) ),
         action_redo, SLOT( setEnabled(bool) ) );

    KAction *action_cut = KStdAction::cut( editor, SLOT( cut() ), ac );
    action_cut->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ),
         action_cut, SLOT( setEnabled(bool) ) );

    KAction *action_copy = KStdAction::copy( editor, SLOT( copy() ), ac );
    action_copy->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ),
         action_copy, SLOT( setEnabled(bool) ) );

    (void) KStdAction::print( this, SLOT( print() ), ac );

    (void) KStdAction::paste( editor, SLOT( paste() ), ac );
    (void) new KAction( i18n( "C&lear" ), 0,
            editor, SLOT( removeSelectedText() ),
            ac, "edit_clear" );

    (void) KStdAction::selectAll( editor, SLOT( selectAll() ), ac );

    //
    // View Actions
    //
    (void) KStdAction::zoomIn( editor, SLOT( zoomIn() ), ac );
    (void) KStdAction::zoomOut( editor, SLOT( zoomOut() ), ac );

    //
    // Character Formatting
    //
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

    (void) new KAction( i18n("Text &Color..."), "colorpicker", 0,
            this, SLOT( formatColor() ),
            ac, "format_color" );

    //
    // Font
    //
    action_font = new KFontAction( i18n("&Font"), 0,
                   ac, "format_font" );
    connect( action_font, SIGNAL( activated( const QString & ) ),
         editor, SLOT( setFamily( const QString & ) ) );


    action_font_size = new KFontSizeAction( i18n("Font &Size"), 0,
                        ac, "format_font_size" );
    connect( action_font_size, SIGNAL( fontSizeChanged(int) ),
         editor, SLOT( setPointSize(int) ) );

    //
    // Alignment
    //
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

    //
    // Tools
    //
    (void) KStdAction::spelling( this, SLOT( checkSpelling() ), ac );

    //
    // Setup enable/disable
    //
    updateActions();

    connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
         this, SLOT( updateFont() ) );
    connect( editor, SIGNAL( currentFontChanged( const QFont & ) ),
         this, SLOT( updateCharFmt() ) );
    connect( editor, SIGNAL( cursorPositionChanged( int,int ) ),
         this, SLOT( updateAligment() ) );
}

//
// Enable/disable actions
//

void KRichTextEditPart::updateActions()
{
    updateCharFmt();
    updateAligment();
    updateFont();
}

void KRichTextEditPart::updateCharFmt()
{
    action_bold->setChecked( editor->bold() );
    action_italic->setChecked( editor->italic() );
    action_underline->setChecked( editor->underline() );
}

void KRichTextEditPart::updateAligment()
{
    int align = editor->alignment();

    switch ( align ) {
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

void KRichTextEditPart::updateFont()
{
    if ( editor->pointSize() > 0 )
    action_font_size->setFontSize( editor->pointSize() );
    action_font->setFont( editor->family() );
}

//
// Formatting actions
//

void KRichTextEditPart::formatColor()
{
    QColor col;

    int s = KColorDialog::getColor( col, editor->color(), editor );
    if ( s != QDialog::Accepted )
    return;

    editor->setColor( col );
}

void KRichTextEditPart::setAlignLeft( bool yes )
{
    if ( yes )
    editor->setAlignment( AlignLeft );
}

void KRichTextEditPart::setAlignRight( bool yes )
{
    if ( yes )
    editor->setAlignment( AlignRight );
}

void KRichTextEditPart::setAlignCenter( bool yes )
{
    if ( yes )
    editor->setAlignment( AlignCenter );
}

void KRichTextEditPart::setAlignJustify( bool yes )
{
    if ( yes )
    editor->setAlignment( AlignJustify );
}

//
// Content Actions
//

bool KRichTextEditPart::open()
{
    KURL url = KFileDialog::getOpenURL();
    if ( url.isEmpty() )
    return false;

    return openURL( url );
}

bool KRichTextEditPart::saveAs()
{
    KURL url = KFileDialog::getSaveURL();
    if ( url.isEmpty() )
    return false;

    return KParts::ReadWritePart::saveAs( url );
}

void KRichTextEditPart::checkSpelling()
{
    QString s;
    if ( editor->hasSelectedText() )
    s = editor->selectedText();
    else
    s = editor->text();

    KSpell::modalCheck( s );
}


// Local Variables:
// c-basic-offset: 4
// End:



