#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './perlscriptprefsbase.ui'
**
** Created: Thu Jan 30 16:51:04 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "perlscriptprefsbase.h"

#include <qvariant.h>
#include <klistview.h>
#include <ktrader.h>
#include <klibloader.h>
#include <qhbox.h>
#include <kpushbutton.h>
#include <qheader.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kstdaction.h>
#include <kactioncollection.h>

#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
/* 
 *  Constructs a PerlScriptPrefsUI as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
PerlScriptPrefsUI::PerlScriptPrefsUI( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
	if ( !name )
		setName( "PerlScriptPrefsUI" );
	
	QVBoxLayout *v = new QVBoxLayout( this, 4 );
	
	scriptView = new KListView( this, "scriptView" );
	scriptView->setAllColumnsShowFocus( true );
	scriptView->addColumn( i18n("Name") );
	scriptView->addColumn( i18n("Description") );
	scriptView->addColumn( i18n("Path") );
	scriptView->setMaximumHeight( 140 );
	v->addWidget( scriptView );
	
	QHBoxLayout *h = new QHBoxLayout( this, 4 );
	h->insertStretch(0);
	removeButton = new KPushButton( this, "removeButton" );
	removeButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	h->addWidget( removeButton );
	
	addButton = new KPushButton( this, "addButton" );
	addButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	h->addWidget( addButton );
	v->addLayout( h );
	
	QHBox *f = new QHBox( this );
	f->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
	KService::Ptr service = *offers.begin();
	KLibFactory *factory = KLibLoader::self()->factory( service->library() );
	editDocument = static_cast<KTextEditor::Document *>( factory->create( f, 0, "KTextEditor::Document" ) );
	editArea = editDocument->createView( f, 0 );
	setHighlight();
	//v->addWidget( editArea );
	v->addWidget( f );
	
	cp = KTextEditor::clipboardInterface( editArea );	
	
	KActionCollection *coll = new KActionCollection(this);

	KStdAction::cut( this, SLOT(slotCut()), coll);
	KStdAction::copy( this, SLOT(slotCopy()), coll);
	KStdAction::paste( this, SLOT(slotPaste()), coll);
	
	QHBoxLayout *h2 = new QHBoxLayout( this, 4 );
	h2->insertStretch(0);
	saveButton = new KPushButton( this, "saveButton" );
	saveButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	h2->addWidget( saveButton );
	v->addLayout(h2);
	
	languageChange();
}

/*
 *  Destroys the object and frees any allocated resources
 */
PerlScriptPrefsUI::~PerlScriptPrefsUI()
{
    // no need to delete child widgets, Qt does it all for us
}

void PerlScriptPrefsUI::setHighlight()
{
	KTextEditor::HighlightingInterface *hi = KTextEditor::highlightingInterface( editDocument );
	int count = hi->hlModeCount();
	for( int i=0; i < count; i++ )
	{
		if( hi->hlModeName(i) == QString::fromLatin1("Perl") )
		{
			hi->setHlMode(i);
			break;
		}
	}
}

void PerlScriptPrefsUI::slotCut()
{
	cp->cut();
}

void PerlScriptPrefsUI::slotCopy()
{
	cp->copy();
}

void PerlScriptPrefsUI::slotPaste()
{
	cp->paste();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PerlScriptPrefsUI::languageChange()
{
    saveButton->setText( tr2i18n( "Save Changes" ) );
    addButton->setText( tr2i18n( "Add Script" ) );
    removeButton->setText( tr2i18n( "Remove Script" ) );
}

#include "perlscriptprefsbase.moc"
