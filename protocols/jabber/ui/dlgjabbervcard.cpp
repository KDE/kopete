/* Jabber vCard dialog stuff, blah blah licensed under the GNU General
 * Public License, blah blah Daniel Stone <dstone@kde.org> blah. */

#include <klocale.h>
#include "dlgjabbervcard.h"

#include <qvariant.h>
#include <kurllabel.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a dlgJabberVCard which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
dlgJabberVCard::dlgJabberVCard( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "dlgJabberVCard" );
    resize( 400, 422 ); 
    setCaption( tr2i18n( "dlgJabbervCard" ) );
    dlgJabberVCardLayout = new QGridLayout( this, 1, 1, 5, 1, "dlgJabberVCardLayout"); 

    GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setTitle( tr2i18n( "User Information" ) );
    GroupBox1->setColumnLayout(0, Qt::Vertical );
    GroupBox1->layout()->setSpacing( 6 );
    GroupBox1->layout()->setMargin( 11 );
    GroupBox1Layout = new QGridLayout( GroupBox1->layout() );
    GroupBox1Layout->setAlignment( Qt::AlignTop );

    TextLabel2 = new QLabel( GroupBox1, "TextLabel2" );
    TextLabel2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel2->sizePolicy().hasHeightForWidth() ) );
    TextLabel2->setText( tr2i18n( "Nickname:" ) );

    GroupBox1Layout->addMultiCellWidget( TextLabel2, 0, 0, 0, 1 );

    nickNameLabel = new QLabel( GroupBox1, "nickNameLabel" );
    nickNameLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0, nickNameLabel->sizePolicy().hasHeightForWidth() ) );
    nickNameLabel->setFrameShape(QLabel::StyledPanel);
    nickNameLabel->setFrameShadow(QLabel::Sunken);
    nickNameLabel->setText(QString::null);

    GroupBox1Layout->addMultiCellWidget( nickNameLabel, 0, 0, 2, 5 );

    TextLabel3 = new QLabel( GroupBox1, "TextLabel3" );
    TextLabel3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel3->sizePolicy().hasHeightForWidth() ) );
    TextLabel3->setText( tr2i18n( "Jabber ID:" ) );

    GroupBox1Layout->addWidget( TextLabel3, 0, 6 );

    JabberIDLabel = new QLabel( GroupBox1, "JabberIDLabel" );
    JabberIDLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, JabberIDLabel->sizePolicy().hasHeightForWidth() ) );
    JabberIDLabel->setFrameShape( QLabel::StyledPanel );
    JabberIDLabel->setFrameShadow( QLabel::Sunken );
    JabberIDLabel->setText( QString::null );

    GroupBox1Layout->addWidget( JabberIDLabel, 0, 7 );

    homepageLabel = new KURLLabel( GroupBox1, "homepageLabel" );

    GroupBox1Layout->addMultiCellWidget( homepageLabel, 2, 2, 2, 7 );

    TextLabel29 = new QLabel( GroupBox1, "TextLabel29" );
    TextLabel29->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel29->sizePolicy().hasHeightForWidth() ) );
    TextLabel29->setText( tr2i18n( "Gender:" ) );

    GroupBox1Layout->addWidget( TextLabel29, 3, 3 );

    TextLabel26 = new QLabel( GroupBox1, "TextLabel26" );
    TextLabel26->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel26->sizePolicy().hasHeightForWidth() ) );
    TextLabel26->setText( tr2i18n( "Homepage:" ) );

    GroupBox1Layout->addMultiCellWidget( TextLabel26, 2, 2, 0, 1 );

    TextLabel31 = new QLabel( GroupBox1, "TextLabel31" );
    TextLabel31->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel31->sizePolicy().hasHeightForWidth() ) );
    TextLabel31->setText( tr2i18n( "Birthday:" ) );

    GroupBox1Layout->addWidget( TextLabel31, 3, 6 );

    birthdayLabel = new QLabel( GroupBox1, "birthdayLabel" );
    birthdayLabel->setFrameShape( QLabel::StyledPanel );
    birthdayLabel->setFrameShadow( QLabel::Sunken );
    birthdayLabel->setText( QString::null );

    GroupBox1Layout->addWidget( birthdayLabel, 3, 7 );

    fullNameLabel = new QLabel( GroupBox1, "fullNameLabel" );
    fullNameLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, 0, 0, fullNameLabel->sizePolicy().hasHeightForWidth() ) );
    fullNameLabel->setFrameShape( QLabel::StyledPanel );
    fullNameLabel->setFrameShadow( QLabel::Sunken );
    fullNameLabel->setText( QString::null );

    GroupBox1Layout->addMultiCellWidget( fullNameLabel, 1, 1, 1, 3 );

    TextLabel7 = new QLabel( GroupBox1, "TextLabel7" );
    TextLabel7->setText( tr2i18n( "Name:" ) );

    GroupBox1Layout->addWidget( TextLabel7, 1, 0 );

    dlgJabberVCardLayout->addMultiCellWidget( GroupBox1, 0, 0, 0, 2 );

    cmdClose = new QPushButton( this, "cmdClose" );
    cmdClose->setText( tr2i18n( "&Close" ) );
    cmdClose->setDefault( TRUE );

    dlgJabberVCardLayout->addWidget( cmdClose, 2, 2 );

    cmdSave = new QPushButton( this, "cmdSave" );
    cmdSave->setText( tr2i18n( "&Save Nickname" ) );

    dlgJabberVCardLayout->addWidget( cmdSave, 2, 0 );

    GroupBox3 = new QGroupBox( this, "GroupBox3" );
    GroupBox3->setTitle( tr2i18n( "Contact Information" ) );
    GroupBox3->setColumnLayout(0, Qt::Vertical );
    GroupBox3->layout()->setSpacing( 6 );
    GroupBox3->layout()->setMargin( 11 );
    GroupBox3Layout = new QGridLayout( GroupBox3->layout() );
    GroupBox3Layout->setAlignment( Qt::AlignTop );

    Layout10 = new QGridLayout( 0, 1, 1, 0, 3, "Layout10"); 

    TextLabel18 = new QLabel( GroupBox3, "TextLabel18" );
    TextLabel18->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)4, 0, 0, TextLabel18->sizePolicy().hasHeightForWidth() ) );
    TextLabel18->setText( tr2i18n( "Address:" ) );

    Layout10->addWidget( TextLabel18, 0, 0 );

    addressLabel = new QLabel( GroupBox3, "addressLabel" );
    addressLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, addressLabel->sizePolicy().hasHeightForWidth() ) );
    addressLabel->setFrameShape( QLabel::StyledPanel );
    addressLabel->setFrameShadow( QLabel::Sunken );
    addressLabel->setText( QString::null );
    addressLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignLeft ) );

    Layout10->addWidget( addressLabel, 1, 0 );

    GroupBox3Layout->addLayout( Layout10, 1, 0 );

    Layout3 = new QGridLayout( 0, 1, 1, 0, 3, "Layout3"); 

    TextLabel14 = new QLabel( GroupBox3, "TextLabel14" );
    TextLabel14->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel14->sizePolicy().hasHeightForWidth() ) );
    TextLabel14->setText( tr2i18n( "Phone:" ) );

    Layout3->addWidget( TextLabel14, 0, 0 );

    phoneNumberLabel = new QLabel( GroupBox3, "phoneNumberLabel" );
    phoneNumberLabel->setFrameShape( QLabel::StyledPanel );
    phoneNumberLabel->setFrameShadow( QLabel::Sunken );
    phoneNumberLabel->setText( QString::null );

    Layout3->addWidget( phoneNumberLabel, 0, 1 );

    GroupBox3Layout->addMultiCellLayout( Layout3, 2, 2, 0, 1 );

    Layout5 = new QGridLayout( 0, 1, 1, 0, 6, "Layout5"); 

    TextLabel12 = new QLabel( GroupBox3, "TextLabel12" );
    TextLabel12->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel12->sizePolicy().hasHeightForWidth() ) );
    TextLabel12->setText( tr2i18n( "Email Address:" ) );

    Layout5->addWidget( TextLabel12, 0, 0 );

    emailAddressLabel = new KURLLabel( GroupBox3, "emailAddressLabel" );
    emailAddressLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, emailAddressLabel->sizePolicy().hasHeightForWidth() ) );
    emailAddressLabel->setText( QString::null );
    emailAddressLabel->setProperty( "url", QString::null );

    Layout5->addWidget( emailAddressLabel, 0, 1 );

    GroupBox3Layout->addMultiCellLayout( Layout5, 0, 0, 0, 1 );

    Layout1 = new QGridLayout( 0, 1, 1, 0, 3, "Layout1"); 

    cityLabel = new QLabel( GroupBox3, "cityLabel" );
    cityLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, cityLabel->sizePolicy().hasHeightForWidth() ) );
    cityLabel->setFrameShape( QLabel::StyledPanel );
    cityLabel->setFrameShadow( QLabel::Sunken );
    cityLabel->setText( QString::null );

    Layout1->addWidget( cityLabel, 0, 1 );

    TextLabel22 = new QLabel( GroupBox3, "TextLabel22" );
    TextLabel22->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel22->sizePolicy().hasHeightForWidth() ) );
    TextLabel22->setText( tr2i18n( "State:" ) );

    Layout1->addWidget( TextLabel22, 1, 0 );

    TextLabel20 = new QLabel( GroupBox3, "TextLabel20" );
    TextLabel20->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel20->sizePolicy().hasHeightForWidth() ) );
    TextLabel20->setText( tr2i18n( "City:" ) );

    Layout1->addWidget( TextLabel20, 0, 0 );

    countryLabel = new QLabel( GroupBox3, "countryLabel" );
    countryLabel->setFrameShape( QLabel::StyledPanel );
    countryLabel->setFrameShadow( QLabel::Sunken );
    countryLabel->setText( QString::null );

    Layout1->addWidget( countryLabel, 2, 1 );

    TextLabel24 = new QLabel( GroupBox3, "TextLabel24" );
    TextLabel24->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, TextLabel24->sizePolicy().hasHeightForWidth() ) );
    TextLabel24->setText( tr2i18n( "Country:" ) );

    Layout1->addWidget( TextLabel24, 2, 0 );

    stateLabel = new QLabel( GroupBox3, "stateLabel" );
    stateLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, stateLabel->sizePolicy().hasHeightForWidth() ) );
    stateLabel->setFrameShape( QLabel::StyledPanel );
    stateLabel->setFrameShadow( QLabel::Sunken );
    stateLabel->setText( QString::null );

    Layout1->addWidget( stateLabel, 1, 1 );

    GroupBox3Layout->addLayout( Layout1, 1, 1 );

    dlgJabberVCardLayout->addMultiCellWidget( GroupBox3, 1, 1, 0, 2 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    dlgJabberVCardLayout->addItem( spacer, 2, 1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
dlgJabberVCard::~dlgJabberVCard()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "dlgjabbervcard.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

