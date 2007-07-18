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

