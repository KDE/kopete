#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './addscriptdialog.ui'
**
** Created: Thu Jan 30 20:02:38 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "addscriptdialog.h"

#include <klineedit.h>
#include <kurlrequester.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kmessagebox.h>

/* 
 *  Constructs a AddScriptDialog as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AddScriptDialog::AddScriptDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    if ( !name )
	setName( "AddScriptDialog" );
    AddScriptDialogLayout = new QGridLayout( this, 1, 1, 10, 10, "AddScriptDialogLayout"); 

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    buttonOk = new KPushButton( this, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new KPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    AddScriptDialogLayout->addMultiCellLayout( Layout1, 3, 3, 0, 1 );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setAlignment( int( QLabel::AlignBottom ) );
    AddScriptDialogLayout->addWidget( textLabel1, 0, 0 );
    
    kLineEdit1 = new KLineEdit( this, "kLineEdit1" );
    AddScriptDialogLayout->addWidget( kLineEdit1, 0, 1 );
    
    textLabel2 = new QLabel( this, "textLabel2" );
    textLabel2->setAlignment( int( QLabel::AlignBottom ) );
    AddScriptDialogLayout->addWidget( textLabel2, 1, 0 );
    
    kLineEdit2 = new KLineEdit( this, "kLineEdit2" );
    AddScriptDialogLayout->addWidget( kLineEdit2, 1, 1 );
  
    textLabel3 = new QLabel( this, "textLabel3" );
    textLabel3->setAlignment( int( QLabel::AlignBottom ) );
    AddScriptDialogLayout->addWidget( textLabel3, 2, 0 );

    kURLRequester1 = new KURLRequester( this, "kURLRequester1" );
    kURLRequester1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, kURLRequester1->sizePolicy().hasHeightForWidth() ) );
    kURLRequester1->setFilter( QString::fromLatin1("*.pl|") + i18n("Perl Scripts") + QString::fromLatin1("\n*.*|") + i18n("All Files") );
    AddScriptDialogLayout->addWidget( kURLRequester1, 2, 1 );
    
    QWidget::setTabOrder( kLineEdit1, kLineEdit2 );
    QWidget::setTabOrder( kLineEdit2, kURLRequester1->lineEdit() );
    kLineEdit1->setFocus();    
    
    languageChange();
    
    resize( QSize(455, 149).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AddScriptDialog::~AddScriptDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void AddScriptDialog::accept()
{
	if( kURLRequester1->url().stripWhiteSpace().isEmpty() || kLineEdit1->text().stripWhiteSpace().isEmpty() || kLineEdit2->text().stripWhiteSpace().isEmpty() )
	{
		KMessageBox::sorry( this, i18n("Please fill out all form fields."), i18n("Missing Fields") );
	}
	else
	{
		emit( scriptChosen( kURLRequester1->url(), kLineEdit1->text().stripWhiteSpace(), kLineEdit2->text().stripWhiteSpace() ) );
		deleteLater();
	}
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AddScriptDialog::languageChange()
{
    setCaption( tr2i18n( "Add Script" ) );
    buttonOk->setGuiItem( KStdGuiItem::ok() );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setGuiItem( KStdGuiItem::cancel() );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    textLabel1->setText( tr2i18n( "Name:" ) );
    textLabel2->setText( tr2i18n( "Description:" ) );
    textLabel3->setText( tr2i18n( "Script Path:" ) );
}

#include "addscriptdialog.moc"
