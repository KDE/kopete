#include "ui_messengergeneralinfo.h"

MessengerUserInfoWidget::MessengerUserInfoWidget( QWidget * parent)
: KPageDialog( parent )
{
	setFaceType( KPageDialog::List );
	setModal( false );
	setCaption( i18n( "Messenger User Information" ) );
	setDefaultButton( KDialog::Ok );
	
	kDebug(14153) << k_funcinfo << "Creating new icq user info widget" << endl;

	//general
	QWidget *genInfo = new QWidget(this);
	m_genInfoWidget = new Ui::MessengerGeneralInfoWidget;
	m_genInfoWidget->setupUi( genInfo );
	KPageWidgetItem *genInfoItem = addPage( genInfo, i18n("General Info") );
	genInfoItem->setHeader( i18n("General Information") );
	genInfoItem->setIcon( KIcon("identity") );

	//contact
	QWidget* contactInfo = new QWidget(this);
	m_contactInfoWidget = new Ui::MessengerContactInfoWidget;
	m_contactInfoWidget->setupUi( contactInfo );
	KPageWidgetItem *contactInfoItem = addPage( contactInfo, i18n("Contact Info") );
	contactInfoItem->setHeader( i18n("Contact Information") );
	contactInfoItem->setIcon( KIcon("go-home") );

	//personal
	QWidget *personalInfo = new QWidget(this);
	m_personalInfoWidget = new Ui::MessengerPersonalInfoWidget();
	m_personalInfoWidget->setupUi( personalInfo );
	KPageWidgetItem *otherInfoItem = addPage( otherInfo, i18n("Personal Info") );
	otherInfoItem->setHeader( i18n( "Perosnal Information" ) );
	otherInfoItem->setIcon( KIcon("email") );

	//work
	QWidget *workInfo = new QWidget(this);
	m_workInfoWidget = new Ui::MessengerWorkInfoWidget;
	m_workInfoWidget->setupUi( workInfo );
	KPageWidgetItem *workInfoItem = addPage( workInfo, i18n("Work Info") );
	workInfoItem->setHeader( i18n( "Work Information" ) );
	workInfoItem->setIcon( KIcon("attach") );

	//notes
	QWidget *notesInfo = new QWidget(this);
	m_notesInfoWidget = new Ui::MessengerNotesInfoWidget;
	m_notesInfoWidget->setupUi( notesInfo );
	KPageWidgetItem *notesInfoItem = addPage( notesInfo, i18n("Notes Info") );
	notesInfoItem->setHeader( i18n( "Notes Information" ) );
	notesInfoItem->setIcon( KIcon("attach") );

	connect( m_genInfoWidget->nickNameLineEdit, SIGNAL(textChanged(const QString &)), 
			this, SLOT(slotUpdateNickName(const QString &)) );
	connect( m_contactInfoWidget->nickNameEdit, SIGNAL(textChanged(const QString &)),
		   	this, SLOT(slotUpdateNickName(const QString&)) );

	//set live id non editable
	m_genInfoWidget->idLineEdit->setReadOnly(true);
}

MessengerUserInfoWidget::~MessengerUserInfoWidget()
{
	delete m_genInfoWidget;
	delete m_contactInfoWidget;
	delete m_personalInfoWidget;
	delete m_workInfoWidget;
	delete m_notesInfoWidget;
}

void MessengerUserInfoWidget::setContact( MessengerContact* contact )
{
	MessengerProtocol* messengerProtocol = static_cast<MessengerProtocol*>( m_contact->protocol() );

	//Countries
	messengerProtocol->fillComboFromTable( m_genInfoWidget->mobileCountryComboBox, messengerProtocol->countries() );
	//Birthday Months
	messengerProtocol->fillComboFromTable( m_personalInfoWidget->birthdayMonthComboBox, messengerProtocol->months() );
	//Birthday Days
	messengerProtocol->fillComboFromTable( m_personalInfoWidget->birthdayDayComboBox, messengerProtocol->days() );
	//Anniversary Months
	messengerProtocol->fillComboFromTable( m_personalInfoWidget->anniversaryMonthComboBox, messengerProtocol->months() );
	//Anniversary Days
	messengerProtocol->fillComboFromTable( m_personalInfoWidget->anniversaryDayComboBox, messengerProtocol->days() );
}

void MessengerUserInfoWidget::slotUpdateDay()
{
	int year = m_personalInfoWidget->birthdayYearSpin->value();
	int month = m_personalInfoWidget->birthdayMonthComboBox->value();
	QDate date( year, month, 1 );
	
	if ( date.isValid() )
		m_genInfoWidget->birthdayDaySpin->setMaximum( date.daysInMonth() );
	else
		m_genInfoWidget->birthdayDaySpin->setMaximum( 31 );
}

void MessengerUserInfoWidget::slotUpdateNickName(const QString & text)
{
	m_genInfoWidget->nickNameLineEdit->setText(text);
	m_contactInfoWidget->nickNameEdit->setText(text);
}

void MessengerUserInfoWidget::setUserInfo(MessengerContactInfo * contactInfo)
{

	m_contactInfo	= contactInfo;

	//General Page
	//cann't edit
	m_genInfoWidget->idLineEdit->setText(contactInfo->m_generalUserInfo->email);
	m_genInfoWidget->nickNameLineEdit->setText(contactInfo->m_annotationUserInfo->NickName);
	//setup mobile phone
	//TODO
	m_genInfoWidget->mobilePhoneEdit->setText(m_generalUserInfo->ContactPhoneMobile);
	//setup group
	//groupCombo->
	
	//Contact Page
	m_contactInfoWidget->lastNameEdit->setText(contactInfo->m_generalUserInfo->lastName);
	m_contactInfoWidget->firstNameEdit->setText(contactInfo->m_generalUserInfo->firstName);
	m_contactInfoWidget->nickNameEdit->setText(contactInfo->m_annotationUserInfo->NickName);
	m_contactInfoWidget->homePhoneLineEdit->setText(contactInfo->m_phoneUserInfo->ContactPhonePersonal);
	m_contactInfoWidget->personalEmailLineEdit->setText(contactInfo->m_emailUserInfo->ContactEmailPersonal);
	m_contactInfoWidget->workPhoneLineEdit->setText(contactInfo->m_phoneUserInfo->ContactPhoneBusiness);
	m_contactInfoWidget->workEmailLineEdit->setText(contactInfo->m_emailUserInfo->ContactEmailBusiness);
	m_contactInfoWidget->otherPhoneLineEdit->setText(contactInfo->m_phoneUserInfo->ContactPhoneOther);
	m_contactInfoWidget->otherEmailLineEdit->setText(contactInfo->m_emailUserInfo->ContactEmailOther);
	m_contactInfoWidget->otherIMLineEdit->setText(contactInfo->m_emailUserInfo->ContactEmailMessenger);
	//Primary Email address
//	m_contactInfoWidget->primaryEmailComboBox->;
	
	//personal Page
	m_personalInfoWidget->countryEdit->setText(contactInfo->m_locationUserInfo->PersonalCountry);
	m_personalInfoWidget->stateEdit->setText(contactInfo->m_locationUserInfo->PersonalState);
	m_personalInfoWidget->cityEdit->setText(contactInfo->m_locationUserInfo->PersonalCity);
	m_personalInfoWidget->streetEdit->setText(contactInfo->m_locationUserInfo->PersonalStreet);
	m_personalInfoWidget->zipEdit->setText(contactInfo->m_locationUserInfo->PersonalPostalCode);
	m_personalInfoWidget->homePhoneEdit->setText(contactInfo->m_phoneUserInfo->ContactPhonePersonal);
	m_personalInfoWidget->otherPhoneEdit->setText(contactInfo->m_phoneUserInfo->ContactPhoneOther);
	m_personalInfoWidget->personalEmailLineEdit->setText(contactInfo->m_emailUserInfo->ContactEmailPersonal);
	m_personalInfoWidget->websiteEdit->setText(contactInfo->m_websiteUserInfo->ContactWebSitePersonal);
	//birthday
	m_personalInfoWidget->birthdayYearSpin->setText(contactInfo->m_generalUserInfo->birthdate.year());
	m_personalInfoWidget->birthdayMonthComboBox->setText(contactInfo->m_generalUserInfo->birthdate.month());
	m_personalInfoWidget->birthdayDayComboBox->setText(contactInfo->m_generalUserInfo->birthdate.day());
	m_personalInfoWidget->spouseEdit->setText(contactInfo->m_annotationUserInfo->Spouse);
	//anniversary
	m_personalInfoWidget->YearSpin->setText(contactInfo->m_generalUserInfo->anniversary.year());
	m_personalInfoWidget->birthdayMonthComboBox->setText(contactInfo->m_generalUserInfo->anniversary.month());
	m_personalInfoWidget->birthdayDayComboBox->setText(contactInfo->m_generalUserInfo->anniversary.day());

	//Work Page
	m_workInfoWidget->companyEdit->setText(contactInfo->m_locationUserInfo->BusinessName);
	m_workInfoWidget->jobEdit->setText(contactInfo->m_annotationUserInfo->JobTitle);
	m_workInfoWidget->countryEdit->setText(contactInfo->m_locationUserInfo->BusinessCountry);
	m_workInfoWidget->stateEdit->setText(contactInfo->m_locationUserInfo->BusinessState);
	m_workInfoWidget->cityEdit->setText(contactInfo->m_locationUserInfo->BusinessCity);
	m_workInfoWidget->streetEdit->setText(contactInfo->m_locationUserInfo->BusinessStreet);
	m_workInfoWidget->zipEdit->setText(contactInfo->m_locationUserInfo->BusinessPostalCode);
	m_workInfoWidget->workphoneEdit->setText(contactInfo->m_phoneUserInfo->ContactPhoneBusiness);
	m_workInfoWidget->workEmailEdit->setText(contactInfo->m_emailUserInfo->ContactEmailBusiness);
	m_workInfoWidget->homeFaxEdit->setText(contactInfo->m_phoneUserInfo->ContactPhoneFax);
	m_workInfoWidget->workMobileEdit->setText(contactInfo->m_phoneUserInfo->ContactPhonePager);
	m_workInfoWidget->workWebsiteEdit->setText(contactInfo->m_websiteUserInfo->ContactWebSiteBusiness);

	//Comment Page
	m_notesInfoWidget->notesTextEdit->setText(contactInfo->m_generalUserInfo->comment);
}

void MessengerUserInfoWidget::setUserInfo(MessengerContactInfo * contactInfo)
{
	m_contactInfo->m_generalUserInfo->email = m_genInfoWidget->idLineEdit->text();
	m_contactInfo->m_annotationUserInfo->NickName = m_genInfoWidget->nickNameLineEdit->text();
	m_contactInfo->m_generalUserInfo->ContactPhoneMobile = m_generalUserInfoWidget->mobilePhoneEdit->text();

	//Contact Page
	m_contactInfo->m_generalUserInfo->lastName = m_contactInfoWidget->lastNameEdit->text();
	m_contactInfo->m_generalUserInfo->firstName = m_contactInfoWidget->firstNameEdit->text();
	m_contactInfo->m_annotationUserInfo->NickName = m_contactInfoWidget->nickNameEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhonePersonal = m_contactInfoWidget->homePhoneLineEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailPersonal = m_contactInfoWidget->personalEmailLineEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhoneBusiness = m_contactInfoWidget->workPhoneLineEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailBusiness = m_contactInfoWidget->workEmailLineEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhoneOther =	m_contactInfoWidget->otherPhoneLineEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailOther = m_contactInfoWidget->otherEmailLineEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailMessenger =	m_contactInfoWidget->otherIMLineEdit->text();
	//Primary Email address
//	m_contactInfoWidget->primaryEmailComboBox->;
	
	//personal Page
	m_contactInfo->m_locationUserInfo->PersonalCountry = m_personalInfoWidget->countryEdit->text();
	m_contactInfo->m_locationUserInfo->PersonalState = m_personalInfoWidget->stateEdit->text();
	m_contactInfo->m_locationUserInfo->PersonalCity = m_personalInfoWidget->cityEdit->text();
	m_contactInfo->m_locationUserInfo->PersonalStreet =	m_personalInfoWidget->streetEdit->text();
	m_contactInfo->m_locationUserInfo->PersonalPostalCode = m_personalInfoWidget->zipEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhonePersonal = m_personalInfoWidget->homePhoneEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhoneOther = m_personalInfoWidget->otherPhoneEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailPersonal = m_personalInfoWidget->personalEmailLineEdit->text();
	m_contactInfo->m_websiteUserInfo->ContactWebSitePersonal = m_personalInfoWidget->websiteEdit->text();
	//birthday
	m_contactInfo->m_generalUserInfo->birthdate = QDate(
			m_personalInfoWidget->birthdayYearSpin->text().toInt(),
			m_personalInfoWidget->birthdayMonthComboBox->text().toInt(),
			m_personalInfoWidget->birthdayDayComboBox->text().toInt());

	m_contactInfo->m_annotationUserInfo->Spouse = m_personalInfoWidget->spouseEdit->text();
	//anniversary
	m_contactInfo->m_generalUserInfo->anniversary = QDate(
			m_personalInfoWidget->anniversaryYearSpin->text().toInt(),
			m_personalInfoWidget->anniversaryMonthComboBox->text().toInt(),
			m_personalInfoWidget->anniversaryDayComboBox->text().toInt());

	//Work Page
	m_contactInfo->m_locationUserInfo->BusinessName = m_workInfoWidget->companyEdit->text();
	m_contactInfo->m_annotationUserInfo->JobTitle = m_workInfoWidget->jobEdit->text();
	m_contactInfo->m_locationUserInfo->BusinessCountry = m_workInfoWidget->countryEdit->text();
	m_contactInfo->m_locationUserInfo->BusinessState = m_workInfoWidget->stateEdit->text();
	m_contactInfo->m_locationUserInfo->BusinessCity = m_workInfoWidget->cityEdit->text();
	m_contactInfo->m_locationUserInfo->BusinessStreet = m_workInfoWidget->streetEdit->text();
	m_contactInfo->m_locationUserInfo->BusinessPostalCode = m_workInfoWidget->zipEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhoneBusiness = m_workInfoWidget->workphoneEdit->text();
	m_contactInfo->m_emailUserInfo->ContactEmailBusiness = m_workInfoWidget->workEmailEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhoneFax = m_workInfoWidget->homeFaxEdit->text();
	m_contactInfo->m_phoneUserInfo->ContactPhonePager = m_workInfoWidget->workMobileEdit->text();
	m_contactInfo->m_websiteUserInfo->ContactWebSiteBusiness = m_workInfoWidget->workWebsiteEdit->text();

	//comment
	m_contactInfo->m_generalUserInfo->comment = m_notesInfoWidget->notesTextEdit->text();
}

