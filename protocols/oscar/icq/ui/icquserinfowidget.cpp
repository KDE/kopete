/*
 Kopete Oscar Protocol
 icquserinfowidget.cpp - Display ICQ user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

 Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
*/

#include "icquserinfowidget.h"

#include <QtCore/QTextCodec>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QStandardItemModel>
#include <QtGui/QHeaderView>

#include <kdatewidget.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>

#include "ui_icqgeneralinfo.h"
#include "ui_icqhomeinfowidget.h"
#include "ui_icqworkinfowidget.h"
#include "ui_icqotherinfowidget.h"
#include "ui_icqinterestinfowidget.h"
#include "ui_icqorgaffinfowidget.h"

#include "oscarutils.h"
#include "icqcontact.h"
#include "icqaccount.h"
#include "icqprotocol.h"

#undef timezone

ICQUserInfoWidget::ICQUserInfoWidget( ICQAccount* account, const QString& contactId, QWidget * parent, bool ownInfo )
: KPageDialog( parent ), m_contact( 0 ), m_account( account ), m_contactId( contactId ), m_ownInfo( ownInfo )
{
	init();

	QObject::connect( m_account->engine(), SIGNAL(receivedIcqLongInfo(QString)),
	                  this, SLOT(receivedLongInfo(QString)) );

	m_genInfoWidget->uinEdit->setText( contactId );

	if ( m_account->isConnected() )
		m_account->engine()->requestFullInfo( m_contactId );
}

ICQUserInfoWidget::ICQUserInfoWidget( ICQContact* contact, QWidget * parent, bool ownInfo )
: KPageDialog( parent ), m_contact( contact ), m_account( static_cast<ICQAccount*>(contact->account()) ),
  m_contactId( contact->contactId() ), m_ownInfo( ownInfo )
{
	init();

	QObject::connect( contact, SIGNAL(haveBasicInfo(ICQGeneralUserInfo)),
	                  this, SLOT(fillBasicInfo(ICQGeneralUserInfo)) );
	QObject::connect( contact, SIGNAL(haveWorkInfo(ICQWorkUserInfo)),
	                  this, SLOT(fillWorkInfo(ICQWorkUserInfo)) );
	QObject::connect( contact, SIGNAL(haveEmailInfo(ICQEmailInfo)),
	                  this, SLOT(fillEmailInfo(ICQEmailInfo)) );
	QObject::connect( contact, SIGNAL(haveNotesInfo(ICQNotesInfo)),
	                  this, SLOT(fillNotesInfo(ICQNotesInfo)) );
	QObject::connect( contact, SIGNAL(haveMoreInfo(ICQMoreUserInfo)),
	                  this, SLOT(fillMoreInfo(ICQMoreUserInfo)) );
	QObject::connect( contact, SIGNAL(haveInterestInfo(ICQInterestInfo)),
	                  this, SLOT(fillInterestInfo(ICQInterestInfo)) );
	QObject::connect( contact, SIGNAL(haveOrgAffInfo(ICQOrgAffInfo)),
	                  this, SLOT(fillOrgAffInfo(ICQOrgAffInfo)) );

	ICQProtocol* icqProtocol = static_cast<ICQProtocol*>( m_contact->protocol() );

	m_genInfoWidget->uinEdit->setText( m_contact->contactId() );
	m_genInfoWidget->aliasEdit->setText( m_contact->ssiItem().alias() );
	m_genInfoWidget->ipEdit->setText( m_contact->property( icqProtocol->ipAddress ).value().toString() );

	if ( m_account->isConnected() )
		m_account->engine()->requestFullInfo( m_contactId );
}

void ICQUserInfoWidget::init()
{
	setFaceType( KPageDialog::List );
	setModal( false );
	setCaption( i18n( "ICQ User Information" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );

	if ( m_ownInfo )
		setDefaultButton( KDialog::Ok );
	else
		setDefaultButton( KDialog::Cancel );
	
	kDebug(OSCAR_ICQ_DEBUG) << "Creating new icq user info widget";
	
	QWidget *genInfo = new QWidget(this);
	m_genInfoWidget = new Ui::ICQGeneralInfoWidget;
	m_genInfoWidget->setupUi( genInfo );
	KPageWidgetItem *genInfoItem = addPage( genInfo, i18n("General Info") );
	genInfoItem->setHeader( i18n("General ICQ Information") );
	genInfoItem->setIcon( KIcon("user-identity") );
	
	QWidget* homeInfo = new QWidget(this);
	m_homeInfoWidget = new Ui::ICQHomeInfoWidget;
	m_homeInfoWidget->setupUi( homeInfo );
	KPageWidgetItem *homeInfoItem = addPage( homeInfo, i18n("Home Info") );
	homeInfoItem->setHeader( i18n("Home Information") );
	homeInfoItem->setIcon( KIcon("go-home") );
	
	QWidget *workInfo = new QWidget(this);
	m_workInfoWidget = new Ui::ICQWorkInfoWidget;
	m_workInfoWidget->setupUi( workInfo );
	KPageWidgetItem *workInfoItem = addPage( workInfo, i18n("Work Info") );
	workInfoItem->setHeader( i18n( "Work Information" ) );
	workInfoItem->setIcon( KIcon("applications-engineering") );
	
	QWidget *otherInfo = new QWidget(this);
	m_otherInfoWidget = new Ui::ICQOtherInfoWidget();
	m_otherInfoWidget->setupUi( otherInfo );
	KPageWidgetItem *otherInfoItem = addPage( otherInfo, i18n("Other Info") );
	otherInfoItem->setHeader( i18n( "Other ICQ Information" ) );
	otherInfoItem->setIcon( KIcon("internet-mail") );
	
	QWidget *interestInfo = new QWidget(this);
	m_interestInfoWidget = new Ui::ICQInterestInfoWidget();
	m_interestInfoWidget->setupUi( interestInfo );
	KPageWidgetItem *interestInfoItem = addPage( interestInfo, i18n("Interest Info") );
	interestInfoItem->setHeader( i18n( "Interest Information" ) );
	interestInfoItem->setIcon( KIcon("applications-games") );
	
	QWidget *orgAffInfo = new QWidget(this);
	m_orgAffInfoWidget = new Ui::ICQOrgAffInfoWidget();
	m_orgAffInfoWidget->setupUi( orgAffInfo );
	KPageWidgetItem *orgAffInfoItem = addPage( orgAffInfo, i18n("Org & Aff Info") );
	orgAffInfoItem->setHeader( i18n( "Organization & Affiliation Information" ) );
	orgAffInfoItem->setIcon( KIcon("preferences-web-browser-identification") );
	
	m_emailModel = new QStandardItemModel();
	QStandardItem *modelItem = new QStandardItem( i18n( "Type" ) );
	m_emailModel->setHorizontalHeaderItem( 0, modelItem );
	modelItem = new QStandardItem( i18n( "Publish Email/Email" ) );
	m_emailModel->setHorizontalHeaderItem( 1, modelItem );

	m_otherInfoWidget->emailTableView->setModel( m_emailModel );
	m_otherInfoWidget->emailTableView->horizontalHeader()->setStretchLastSection( true );
	m_otherInfoWidget->emailTableView->setSelectionMode( QAbstractItemView::SingleSelection );

	connect( m_genInfoWidget->birthdayYearSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDay()) );
	connect( m_genInfoWidget->birthdayMonthSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDay()) );

	connect( m_genInfoWidget->birthdayYearSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );
	connect( m_genInfoWidget->birthdayMonthSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );
	connect( m_genInfoWidget->birthdayDaySpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );

	connect( m_orgAffInfoWidget->org1CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrg1CategoryChanged(int)) );
	connect( m_orgAffInfoWidget->org2CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrg2CategoryChanged(int)) );
	connect( m_orgAffInfoWidget->org3CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrg3CategoryChanged(int)) );

	connect( m_orgAffInfoWidget->aff1CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAff1CategoryChanged(int)) );
	connect( m_orgAffInfoWidget->aff2CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAff2CategoryChanged(int)) );
	connect( m_orgAffInfoWidget->aff3CategoryCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAff3CategoryChanged(int)) );

	connect( m_interestInfoWidget->topic1Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterestTopic1Changed(int)) );
	connect( m_interestInfoWidget->topic2Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterestTopic2Changed(int)) );
	connect( m_interestInfoWidget->topic3Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterestTopic3Changed(int)) );
	connect( m_interestInfoWidget->topic4Combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterestTopic4Changed(int)) );

	if ( m_ownInfo )
	{
		connect( m_otherInfoWidget->addEmailButton, SIGNAL(clicked(bool)), this, SLOT(slotAddEmail()) );
		connect( m_otherInfoWidget->removeEmailButton, SIGNAL(clicked(bool)), this, SLOT(slotRemoveEmail()) );
		connect( m_otherInfoWidget->upEmailButton, SIGNAL(clicked(bool)), this, SLOT(slotUpEmail()) );
		connect( m_otherInfoWidget->downEmailButton, SIGNAL(clicked(bool)), this, SLOT(slotDownEmail()) );
		connect( m_otherInfoWidget->emailTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		         this, SLOT(slotEmailSelectionChanged(QItemSelection)) );
	}

	m_genInfoWidget->aliasEdit->setReadOnly( m_ownInfo );

	//ICQGeneralUserInfo
	m_genInfoWidget->nickNameEdit->setReadOnly( !m_ownInfo );
	m_genInfoWidget->firstNameEdit->setReadOnly( !m_ownInfo );
	m_genInfoWidget->lastNameEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->cityEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->stateEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->phoneEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->faxEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->addressEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->cellEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->zipEdit->setReadOnly( !m_ownInfo );

	m_genInfoWidget->ageEdit->setReadOnly( !m_ownInfo );
	m_genInfoWidget->birthdayDaySpin->setReadOnly( !m_ownInfo );
	m_genInfoWidget->birthdayMonthSpin->setReadOnly( !m_ownInfo );
	m_genInfoWidget->birthdayYearSpin->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->homepageEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->oCityEdit->setReadOnly( !m_ownInfo );
	m_homeInfoWidget->oStateEdit->setReadOnly( !m_ownInfo );

	m_workInfoWidget->cityEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->stateEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->phoneEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->faxEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->addressEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->zipEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->companyEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->departmentEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->positionEdit->setReadOnly( !m_ownInfo );
	m_workInfoWidget->homepageEdit->setReadOnly( !m_ownInfo );

	m_orgAffInfoWidget->org1KeywordEdit->setReadOnly( !m_ownInfo );
	m_orgAffInfoWidget->org2KeywordEdit->setReadOnly( !m_ownInfo );
	m_orgAffInfoWidget->org3KeywordEdit->setReadOnly( !m_ownInfo );
	m_orgAffInfoWidget->aff1KeywordEdit->setReadOnly( !m_ownInfo );
	m_orgAffInfoWidget->aff2KeywordEdit->setReadOnly( !m_ownInfo );
	m_orgAffInfoWidget->aff3KeywordEdit->setReadOnly( !m_ownInfo );

	m_interestInfoWidget->desc1->setReadOnly( !m_ownInfo );
	m_interestInfoWidget->desc2->setReadOnly( !m_ownInfo );
	m_interestInfoWidget->desc3->setReadOnly( !m_ownInfo );
	m_interestInfoWidget->desc4->setReadOnly( !m_ownInfo );

	m_otherInfoWidget->notesEdit->setReadOnly( !m_ownInfo );

	m_otherInfoWidget->addEmailButton->setEnabled( m_ownInfo );
	m_otherInfoWidget->removeEmailButton->setEnabled( false );
	m_otherInfoWidget->upEmailButton->setEnabled( false );
	m_otherInfoWidget->downEmailButton->setEnabled( false );

	if ( !m_ownInfo )
	{
		m_homeInfoWidget->countryCombo->setReadOnly( true );
		m_homeInfoWidget->oCountryCombo->setReadOnly( true );
		m_workInfoWidget->countryCombo->setReadOnly( true );
		m_genInfoWidget->language1Combo->setReadOnly( true );
		m_genInfoWidget->language2Combo->setReadOnly( true );
		m_genInfoWidget->language3Combo->setReadOnly( true );
		m_genInfoWidget->genderCombo->setReadOnly( true );
		m_genInfoWidget->maritalCombo->setReadOnly( true );
		m_workInfoWidget->occupationCombo->setReadOnly( true );
		m_orgAffInfoWidget->org1CategoryCombo->setReadOnly( true );
		m_orgAffInfoWidget->org2CategoryCombo->setReadOnly( true );
		m_orgAffInfoWidget->org3CategoryCombo->setReadOnly( true );
		m_orgAffInfoWidget->aff1CategoryCombo->setReadOnly( true );
		m_orgAffInfoWidget->aff2CategoryCombo->setReadOnly( true );
		m_orgAffInfoWidget->aff3CategoryCombo->setReadOnly( true );
		m_interestInfoWidget->topic1Combo->setReadOnly( true );
		m_interestInfoWidget->topic2Combo->setReadOnly( true );
		m_interestInfoWidget->topic3Combo->setReadOnly( true );
		m_interestInfoWidget->topic4Combo->setReadOnly( true );
		m_genInfoWidget->timezoneCombo->setReadOnly( true );
	}

	ICQProtocol* icqProtocol = static_cast<ICQProtocol*>( m_account->protocol() );

	//Countries
	QMap<QString, int> sortedMap( reverseMap( icqProtocol->countries() ) );
	QMapIterator<QString, int> it( sortedMap );

	while ( it.hasNext() )
	{
		it.next();
		m_homeInfoWidget->countryCombo->addItem( it.key(), it.value() );
		m_homeInfoWidget->oCountryCombo->addItem( it.key(), it.value() );
		m_workInfoWidget->countryCombo->addItem( it.key(), it.value() );
	}

	//Languages
	sortedMap = reverseMap( icqProtocol->languages() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->language1Combo->addItem( it.key(), it.value() );
		m_genInfoWidget->language2Combo->addItem( it.key(), it.value() );
		m_genInfoWidget->language3Combo->addItem( it.key(), it.value() );
	}

	//Genders
	sortedMap = reverseMap( icqProtocol->genders() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->genderCombo->addItem( it.key(), it.value() );
	}

	//Maritals
	sortedMap = reverseMap( icqProtocol->maritals() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->maritalCombo->addItem( it.key(), it.value() );
	}

	//Occupations
	sortedMap = reverseMap( icqProtocol->occupations() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_workInfoWidget->occupationCombo->addItem( it.key(), it.value() );
	}

	//Organizations
	sortedMap = reverseMap( icqProtocol->organizations() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_orgAffInfoWidget->org1CategoryCombo->addItem( it.key(), it.value() );
		m_orgAffInfoWidget->org2CategoryCombo->addItem( it.key(), it.value() );
		m_orgAffInfoWidget->org3CategoryCombo->addItem( it.key(), it.value() );
	}

	//Affiliations
	sortedMap = reverseMap( icqProtocol->affiliations() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_orgAffInfoWidget->aff1CategoryCombo->addItem( it.key(), it.value() );
		m_orgAffInfoWidget->aff2CategoryCombo->addItem( it.key(), it.value() );
		m_orgAffInfoWidget->aff3CategoryCombo->addItem( it.key(), it.value() );
	}

	//Interests
	sortedMap = reverseMap( icqProtocol->interests() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_interestInfoWidget->topic1Combo->addItem( it.key(), it.value() );
		m_interestInfoWidget->topic2Combo->addItem( it.key(), it.value() );
		m_interestInfoWidget->topic3Combo->addItem( it.key(), it.value() );
		m_interestInfoWidget->topic4Combo->addItem( it.key(), it.value() );
	}

	//Timezone
	QString timezone;
	for ( int zone = 24; zone >= -24; zone-- )
	{
		timezone = QString( "GMT %1%2:%3" )
			.arg( ( zone > 0 ) ? "-" : "" )
			.arg( qAbs( zone ) / 2, 2, 10, QLatin1Char('0') )
			.arg( ( qAbs( zone ) % 2 ) * 30, 2, 10, QLatin1Char('0')  );

		m_genInfoWidget->timezoneCombo->addItem( timezone, zone );
	}
}

ICQUserInfoWidget::~ ICQUserInfoWidget()
{
	delete m_genInfoWidget;
	delete m_homeInfoWidget;
	delete m_workInfoWidget;
	delete m_otherInfoWidget;
	delete m_interestInfoWidget;
	delete m_orgAffInfoWidget;
	delete m_emailModel;
}

QList<ICQInfoBase*> ICQUserInfoWidget::getInfoData() const
{
	QList<ICQInfoBase*> infoList;
	
	if ( !m_ownInfo )
		return infoList;
	
	infoList.append( storeBasicInfo() );
	infoList.append( storeMoreInfo() );
	infoList.append( storeWorkInfo() );
	infoList.append( storeOrgAffInfo() );
	infoList.append( storeInterestInfo() );
	infoList.append( storeNotesInfo() );
	infoList.append( storeEmailInfo() );
	
	return infoList;
}

QString ICQUserInfoWidget::getAlias() const
{
	return m_genInfoWidget->aliasEdit->text();
}

void ICQUserInfoWidget::fillBasicInfo( const ICQGeneralUserInfo& ui )
{
	QTextCodec* codec = getTextCodec();
	
	if ( m_ownInfo )
		m_generalUserInfo = ui;
	
	m_genInfoWidget->nickNameEdit->setText( codec->toUnicode( ui.nickName.get() ) );
	m_genInfoWidget->firstNameEdit->setText( codec->toUnicode( ui.firstName.get() ) );
	m_genInfoWidget->lastNameEdit->setText( codec->toUnicode( ui.lastName.get() ) );
	m_homeInfoWidget->cityEdit->setText( codec->toUnicode( ui.city.get() ) );
	m_homeInfoWidget->stateEdit->setText( codec->toUnicode( ui.state.get() ) );
	m_homeInfoWidget->phoneEdit->setText( codec->toUnicode( ui.phoneNumber.get() ) );
	m_homeInfoWidget->faxEdit->setText( codec->toUnicode( ui.faxNumber.get() ) );
	m_homeInfoWidget->addressEdit->setText( codec->toUnicode( ui.address.get() ) );
	m_homeInfoWidget->cellEdit->setText( codec->toUnicode( ui.cellNumber.get() ) );
	m_homeInfoWidget->zipEdit->setText(  codec->toUnicode( ui.zip.get() ) );
	
	m_homeInfoWidget->countryCombo->setCurrentIndex( m_homeInfoWidget->countryCombo->findData( ui.country.get() ) );
	m_genInfoWidget->timezoneCombo->setCurrentIndex( m_genInfoWidget->timezoneCombo->findData( ui.timezone.get() ) );

	if ( !ui.email.get().isEmpty() )
	{
		QList<QStandardItem *> items;
		QStandardItem *modelItem;

		modelItem = new QStandardItem( i18nc("Primary email address", "Primary") );
		modelItem->setEditable( false );
		modelItem->setSelectable( false );
		items.append( modelItem );

		modelItem = new QStandardItem( codec->toUnicode( ui.email.get() ) );
		modelItem->setEditable( m_ownInfo );
		modelItem->setCheckable( true );
		modelItem->setCheckState( ( ui.publishEmail.get() ) ? Qt::Checked : Qt::Unchecked );
		items.append( modelItem );

		m_emailModel->insertRow( 0, items );
	}
}

void ICQUserInfoWidget::fillWorkInfo( const ICQWorkUserInfo& ui )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_workUserInfo = ui;

	m_workInfoWidget->cityEdit->setText( codec->toUnicode( ui.city.get() ) );
	m_workInfoWidget->stateEdit->setText( codec->toUnicode( ui.state.get() ) );
	m_workInfoWidget->phoneEdit->setText( codec->toUnicode( ui.phone.get() ) );
	m_workInfoWidget->faxEdit->setText( codec->toUnicode( ui.fax.get() ) );
	m_workInfoWidget->addressEdit->setText( codec->toUnicode( ui.address.get() ) );
	m_workInfoWidget->zipEdit->setText( codec->toUnicode( ui.zip.get() ) );
	m_workInfoWidget->companyEdit->setText( codec->toUnicode( ui.company.get() ) );
	m_workInfoWidget->departmentEdit->setText( codec->toUnicode( ui.department.get() ) );
	m_workInfoWidget->positionEdit->setText( codec->toUnicode( ui.position.get() ) );
	m_workInfoWidget->homepageEdit->setText( codec->toUnicode( ui.homepage.get() ) );

	m_workInfoWidget->countryCombo->setCurrentIndex( m_workInfoWidget->countryCombo->findData( ui.country.get() ) );
	m_workInfoWidget->occupationCombo->setCurrentIndex( m_workInfoWidget->occupationCombo->findData( ui.occupation.get() ) );
}

void ICQUserInfoWidget::fillEmailInfo( const ICQEmailInfo& info )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_emailInfo = info;

	int size = info.emailList.get().size();
	for ( int i = 0; i < size; i++ )
	{
		int row = m_emailModel->rowCount();

		ICQEmailInfo::EmailItem item = info.emailList.get().at(i);
		QStandardItem *modelItem = new QStandardItem( i18nc("Other email address", "More")  );
		modelItem->setEditable( false );
		modelItem->setSelectable( false );
		m_emailModel->setItem( row, 0, modelItem );
		modelItem = new QStandardItem( codec->toUnicode( item.email ) );
		modelItem->setEditable( m_ownInfo );
		modelItem->setCheckable( true );
		modelItem->setCheckState( ( item.publish ) ? Qt::Checked : Qt::Unchecked );
		m_emailModel->setItem( row, 1, modelItem );
	}
}

void ICQUserInfoWidget::fillNotesInfo( const ICQNotesInfo& info )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_notesInfo = info;

	m_otherInfoWidget->notesEdit->setPlainText( codec->toUnicode( info.notes.get() ) );
}

void ICQUserInfoWidget::fillInterestInfo( const ICQInterestInfo& info )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_interestInfo = info;

	int index = m_interestInfoWidget->topic1Combo->findData( info.topics[0].get() );
	m_interestInfoWidget->topic1Combo->setCurrentIndex( index );
	m_interestInfoWidget->desc1->setText( codec->toUnicode( info.descriptions[0].get() ) );

	index = m_interestInfoWidget->topic2Combo->findData( info.topics[1].get() );
	m_interestInfoWidget->topic2Combo->setCurrentIndex( index );
	m_interestInfoWidget->desc2->setText( codec->toUnicode( info.descriptions[1].get() ) );

	index = m_interestInfoWidget->topic3Combo->findData( info.topics[2].get() );
	m_interestInfoWidget->topic3Combo->setCurrentIndex( index );
	m_interestInfoWidget->desc3->setText( codec->toUnicode( info.descriptions[2].get() ) );

	index = m_interestInfoWidget->topic4Combo->findData( info.topics[3].get() );
	m_interestInfoWidget->topic4Combo->setCurrentIndex( index );
	m_interestInfoWidget->desc4->setText( codec->toUnicode( info.descriptions[3].get() ) );
}

void ICQUserInfoWidget::fillOrgAffInfo( const ICQOrgAffInfo& info )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_orgAffUserInfo = info;

	m_orgAffInfoWidget->org1KeywordEdit->setText( codec->toUnicode( info.org1Keyword.get() ) );
	m_orgAffInfoWidget->org2KeywordEdit->setText( codec->toUnicode( info.org2Keyword.get() ) );
	m_orgAffInfoWidget->org3KeywordEdit->setText( codec->toUnicode( info.org3Keyword.get() ) );

	int index = m_orgAffInfoWidget->org1CategoryCombo->findData( info.org1Category.get() );
	m_orgAffInfoWidget->org1CategoryCombo->setCurrentIndex( index );

	index = m_orgAffInfoWidget->org2CategoryCombo->findData( info.org2Category.get() );
	m_orgAffInfoWidget->org2CategoryCombo->setCurrentIndex( index );

	index = m_orgAffInfoWidget->org3CategoryCombo->findData( info.org3Category.get() );
	m_orgAffInfoWidget->org3CategoryCombo->setCurrentIndex( index );

	m_orgAffInfoWidget->aff1KeywordEdit->setText( codec->toUnicode( info.pastAff1Keyword.get() ) );
	m_orgAffInfoWidget->aff2KeywordEdit->setText( codec->toUnicode( info.pastAff2Keyword.get() ) );
	m_orgAffInfoWidget->aff3KeywordEdit->setText( codec->toUnicode( info.pastAff3Keyword.get() ) );

	index = m_orgAffInfoWidget->aff1CategoryCombo->findData( info.pastAff1Category.get() );
	m_orgAffInfoWidget->aff1CategoryCombo->setCurrentIndex( index );

	index = m_orgAffInfoWidget->aff2CategoryCombo->findData( info.pastAff2Category.get() );
	m_orgAffInfoWidget->aff2CategoryCombo->setCurrentIndex( index );

	index = m_orgAffInfoWidget->aff3CategoryCombo->findData( info.pastAff3Category.get() );
	m_orgAffInfoWidget->aff3CategoryCombo->setCurrentIndex( index );
}

void ICQUserInfoWidget::fillMoreInfo( const ICQMoreUserInfo& ui )
{
	QTextCodec* codec = getTextCodec();

	if ( m_ownInfo )
		m_moreUserInfo = ui;

	m_genInfoWidget->ageEdit->setText( QString::number( ui.age.get() ) );
	m_genInfoWidget->birthdayYearSpin->setValue( ui.birthdayYear.get() );
	m_genInfoWidget->birthdayMonthSpin->setValue( ui.birthdayMonth.get() );
	m_genInfoWidget->birthdayDaySpin->setValue( ui.birthdayDay.get() );
	m_genInfoWidget->genderCombo->setCurrentIndex( m_genInfoWidget->genderCombo->findData( ui.gender.get() ) );
	m_homeInfoWidget->homepageEdit->setText( codec->toUnicode( ui.homepage.get() ) );
	m_genInfoWidget->maritalCombo->setCurrentIndex( m_genInfoWidget->maritalCombo->findData( ui.marital.get() ) );
	m_homeInfoWidget->oCityEdit->setText( codec->toUnicode( ui.ocity.get() ) );
	m_homeInfoWidget->oStateEdit->setText( codec->toUnicode( ui.ostate.get() ) );
	m_homeInfoWidget->oCountryCombo->setCurrentIndex( m_homeInfoWidget->oCountryCombo->findData( ui.ocountry.get() ) );
	m_genInfoWidget->language1Combo->setCurrentIndex( m_genInfoWidget->language1Combo->findData( ui.lang1.get() ) );
	m_genInfoWidget->language2Combo->setCurrentIndex( m_genInfoWidget->language2Combo->findData( ui.lang2.get() ) );
	m_genInfoWidget->language3Combo->setCurrentIndex( m_genInfoWidget->language3Combo->findData( ui.lang3.get() ) );
	m_otherInfoWidget->sendInfoCheck->setChecked( ui.sendInfo.get() );
}

void ICQUserInfoWidget::receivedLongInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( m_contactId ) )
		return;

	kDebug(OSCAR_ICQ_DEBUG) << "received long info from engine";	
	fillBasicInfo( m_account->engine()->getGeneralInfo( contact ) );
	fillWorkInfo( m_account->engine()->getWorkInfo( contact ) );
	fillEmailInfo( m_account->engine()->getEmailInfo( contact ) );
	fillNotesInfo( m_account->engine()->getNotesInfo( contact ) );
	fillMoreInfo( m_account->engine()->getMoreInfo( contact ) );
	fillInterestInfo( m_account->engine()->getInterestInfo( contact ) );
	fillOrgAffInfo( m_account->engine()->getOrgAffInfo( contact ) );
}

void ICQUserInfoWidget::slotUpdateDay()
{
	int year = m_genInfoWidget->birthdayYearSpin->value();
	int month = m_genInfoWidget->birthdayMonthSpin->value();
	QDate date( year, month, 1 );
	
	if ( date.isValid() )
		m_genInfoWidget->birthdayDaySpin->setMaximum( date.daysInMonth() );
	else
		m_genInfoWidget->birthdayDaySpin->setMaximum( 31 );
}

void ICQUserInfoWidget::slotUpdateAge()
{
	QDate now = QDate::currentDate();
	int year = m_genInfoWidget->birthdayYearSpin->value();
	int month = m_genInfoWidget->birthdayMonthSpin->value();
	int day = m_genInfoWidget->birthdayDaySpin->value();

	int age = 0;

	if ( year > 0 )
	{
		age = now.year() - year;
		if ( month > now.month() )
		{
			age--;
		}
		else if ( month == now.month() )
		{
			if ( day > now.day() )
			{
				age--;
			}
		}
	}

	m_genInfoWidget->ageEdit->setText( QString::number( age ) );
}

void ICQUserInfoWidget::slotOrg1CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->org1CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->org1KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotOrg2CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->org2CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->org2KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotOrg3CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->org3CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->org3KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotAff1CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->aff1CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->aff1KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotAff2CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->aff2CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->aff2KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotAff3CategoryChanged( int index )
{
	bool enable = !( m_orgAffInfoWidget->aff3CategoryCombo->itemData( index ).toInt() == 0 );
	m_orgAffInfoWidget->aff3KeywordEdit->setEnabled( enable );
}

void ICQUserInfoWidget::slotInterestTopic1Changed( int index )
{
	bool enable = !( m_interestInfoWidget->topic1Combo->itemData( index ).toInt() == 0 );
	m_interestInfoWidget->desc1->setEnabled( enable );
}

void ICQUserInfoWidget::slotInterestTopic2Changed( int index )
{
	bool enable = !( m_interestInfoWidget->topic2Combo->itemData( index ).toInt() == 0 );
	m_interestInfoWidget->desc2->setEnabled( enable );
}

void ICQUserInfoWidget::slotInterestTopic3Changed( int index )
{
	bool enable = !( m_interestInfoWidget->topic3Combo->itemData( index ).toInt() == 0 );
	m_interestInfoWidget->desc3->setEnabled( enable );
}

void ICQUserInfoWidget::slotInterestTopic4Changed( int index )
{
	bool enable = !( m_interestInfoWidget->topic4Combo->itemData( index ).toInt() == 0 );
	m_interestInfoWidget->desc4->setEnabled( enable );
}

void ICQUserInfoWidget::slotAddEmail()
{
	QItemSelectionModel* selectionModel = m_otherInfoWidget->emailTableView->selectionModel();
	QModelIndexList indexes = selectionModel->selectedIndexes();
	int row = ( indexes.count() > 0 ) ? indexes.at( 0 ).row() + 1 : m_emailModel->rowCount();

	QList<QStandardItem *> items;
	QStandardItem *modelItem;

	modelItem = new QStandardItem( ( row == 0 ) ? i18nc("Primary email address", "Primary") : i18nc("Other email address", "More") );
	modelItem->setEditable( false );
	modelItem->setSelectable( false );
	items.append( modelItem );

	modelItem = new QStandardItem();
	modelItem->setEditable( m_ownInfo );
	modelItem->setCheckable( true );
	modelItem->setCheckState( Qt::Unchecked );
	items.append( modelItem );

	m_emailModel->insertRow( row, items );
	QModelIndex idx = m_emailModel->index( row, 1 );
	selectionModel->select( idx, QItemSelectionModel::SelectCurrent );

	if ( row == 0 && m_emailModel->rowCount() > 1 )
		m_emailModel->item( 1, 0 )->setText( i18nc("Other email address", "More") );
}

void ICQUserInfoWidget::slotRemoveEmail()
{
	QItemSelectionModel* selectionModel = m_otherInfoWidget->emailTableView->selectionModel();
	QModelIndexList indexes = selectionModel->selectedIndexes();
	
	if ( indexes.count() > 0 )
	{
		int row = indexes.at( 0 ).row();
		m_emailModel->removeRow( row );

		if ( row == 0 && m_emailModel->rowCount() > 0 )
			m_emailModel->item( 0, 0 )->setText( i18nc("Primary email address", "Primary") );

		QModelIndex idx = m_emailModel->index( ( row > 0 ) ? row - 1 : row , 1 );
		selectionModel->select( idx, QItemSelectionModel::SelectCurrent );
	}
}

void ICQUserInfoWidget::slotUpEmail()
{
	QItemSelectionModel* selectionModel = m_otherInfoWidget->emailTableView->selectionModel();
	QModelIndexList indexes = selectionModel->selectedIndexes();

	if ( indexes.count() > 0 )
	{
		int row = indexes.at( 0 ).row();
		if ( row > 0 )
		{
			swapEmails( row-1, row );

			QModelIndex idx = m_emailModel->index( row - 1, 1 );
			selectionModel->select( idx, QItemSelectionModel::SelectCurrent );
		}
	}
}

void ICQUserInfoWidget::slotDownEmail()
{
	QItemSelectionModel* selectionModel = m_otherInfoWidget->emailTableView->selectionModel();
	QModelIndexList indexes = selectionModel->selectedIndexes();

	if ( indexes.count() > 0 )
	{
		int row = indexes.at( 0 ).row();
		if ( row < m_emailModel->rowCount() - 1 )
		{
			swapEmails( row, row + 1 );

			QModelIndex idx = m_emailModel->index( row + 1, 1 );
			selectionModel->select( idx, QItemSelectionModel::SelectCurrent );
		}
	}
}

void ICQUserInfoWidget::slotEmailSelectionChanged( const QItemSelection& selected )
{
	QModelIndexList indexes = selected.indexes();
	if ( indexes.count() > 0 )
	{
		int row = indexes.at( 0 ).row();
		m_otherInfoWidget->upEmailButton->setEnabled( (row > 0) );
		m_otherInfoWidget->downEmailButton->setEnabled( (row < m_emailModel->rowCount()-1 ) );
		m_otherInfoWidget->removeEmailButton->setEnabled( true );
	}
	else
	{
		m_otherInfoWidget->removeEmailButton->setEnabled( false );
		m_otherInfoWidget->upEmailButton->setEnabled( false );
		m_otherInfoWidget->downEmailButton->setEnabled( false );
	}
}

void ICQUserInfoWidget::swapEmails( int r1, int r2 )
{
	if ( r1 > r2 )
		qSwap( r1, r2 );

	QList<QStandardItem *> rowItems1 = m_emailModel->takeRow( r1 );
	QList<QStandardItem *> rowItems2 = m_emailModel->takeRow( r2-1 );

	rowItems1.at( 0 )->setText( ( r2 == 0 ) ? i18nc("Primary email address", "Primary") : i18nc("Other email address", "More") );
	rowItems2.at( 0 )->setText( ( r1 == 0 ) ? i18nc("Primary email address", "Primary") : i18nc("Other email address", "More") );
	m_emailModel->insertRow( r1, rowItems2 );
	m_emailModel->insertRow( r2, rowItems1 );
}

ICQGeneralUserInfo* ICQUserInfoWidget::storeBasicInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQGeneralUserInfo* info = new ICQGeneralUserInfo( m_generalUserInfo );

	//Email is stored in storeEmailInfo(), because if we change primary email we have to update all emails.
	info->nickName.set( codec->fromUnicode( m_genInfoWidget->nickNameEdit->text() ) );
	info->firstName.set( codec->fromUnicode( m_genInfoWidget->firstNameEdit->text() ) );
	info->lastName.set( codec->fromUnicode( m_genInfoWidget->lastNameEdit->text() ) );
	info->city.set( codec->fromUnicode( m_homeInfoWidget->cityEdit->text() ) );
	info->state.set( codec->fromUnicode( m_homeInfoWidget->stateEdit->text() ) );
	info->phoneNumber.set( codec->fromUnicode( m_homeInfoWidget->phoneEdit->text() ) );
	info->faxNumber.set( codec->fromUnicode( m_homeInfoWidget->faxEdit->text() ) );
	info->address.set( codec->fromUnicode( m_homeInfoWidget->addressEdit->text() ) );
	info->cellNumber.set( codec->fromUnicode( m_homeInfoWidget->cellEdit->text() ) );
	info->zip.set( codec->fromUnicode( m_homeInfoWidget->zipEdit->text() ) );

	int index = m_homeInfoWidget->countryCombo->currentIndex();
	info->country.set( m_homeInfoWidget->countryCombo->itemData( index ).toInt() );

	index = m_genInfoWidget->timezoneCombo->currentIndex();
	info->timezone.set( m_genInfoWidget->timezoneCombo->itemData( index ).toInt() );
	
	return info;
}

ICQMoreUserInfo* ICQUserInfoWidget::storeMoreInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQMoreUserInfo* info = new ICQMoreUserInfo( m_moreUserInfo );

	info->age.set( m_genInfoWidget->ageEdit->text().toInt() );
	info->birthdayYear.set( m_genInfoWidget->birthdayYearSpin->value() );
	info->birthdayMonth.set( m_genInfoWidget->birthdayMonthSpin->value() );
	info->birthdayDay.set( m_genInfoWidget->birthdayDaySpin->value() );

	int index = m_genInfoWidget->genderCombo->currentIndex();
	info->gender.set( m_genInfoWidget->genderCombo->itemData( index ).toInt() );

	info->homepage.set( codec->fromUnicode( m_homeInfoWidget->homepageEdit->text() ) );

	index = m_genInfoWidget->maritalCombo->currentIndex();
	info->marital.set( m_genInfoWidget->maritalCombo->itemData( index ).toInt() );
	
	info->ocity.set( codec->fromUnicode( m_homeInfoWidget->oCityEdit->text() ) );
	info->ostate.set( codec->fromUnicode( m_homeInfoWidget->oStateEdit->text() ) );

	index = m_homeInfoWidget->oCountryCombo->currentIndex();
	info->ocountry.set( m_homeInfoWidget->oCountryCombo->itemData( index ).toInt() );

	index = m_genInfoWidget->language1Combo->currentIndex();
	info->lang1.set( m_genInfoWidget->language1Combo->itemData( index ).toInt() );

	index = m_genInfoWidget->language2Combo->currentIndex();
	info->lang2.set( m_genInfoWidget->language2Combo->itemData( index ).toInt() );

	index = m_genInfoWidget->language3Combo->currentIndex();
	info->lang3.set( m_genInfoWidget->language3Combo->itemData( index ).toInt() );

	info->sendInfo.set( m_otherInfoWidget->sendInfoCheck->isChecked() );

	return info;
}

ICQWorkUserInfo* ICQUserInfoWidget::storeWorkInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQWorkUserInfo* info = new ICQWorkUserInfo( m_workUserInfo );

	info->city.set( codec->fromUnicode( m_workInfoWidget->cityEdit->text() ) );
	info->state.set( codec->fromUnicode( m_workInfoWidget->stateEdit->text() ) );
	info->phone.set( codec->fromUnicode( m_workInfoWidget->phoneEdit->text() ) );
	info->fax.set( codec->fromUnicode( m_workInfoWidget->faxEdit->text() ) );
	info->address.set( codec->fromUnicode( m_workInfoWidget->addressEdit->text() ) );
	info->zip.set( codec->fromUnicode( m_workInfoWidget->zipEdit->text() ) );
	info->company.set( codec->fromUnicode( m_workInfoWidget->companyEdit->text() ) );
	info->department.set( codec->fromUnicode( m_workInfoWidget->departmentEdit->text() ) );
	info->position.set( codec->fromUnicode( m_workInfoWidget->positionEdit->text() ) );
	info->homepage.set( codec->fromUnicode( m_workInfoWidget->homepageEdit->text() ) );

	int index = m_workInfoWidget->countryCombo->currentIndex();
	info->country.set( m_workInfoWidget->countryCombo->itemData( index ).toInt() );

	index = m_workInfoWidget->occupationCombo->currentIndex();
	info->occupation.set( m_workInfoWidget->occupationCombo->itemData( index ).toInt() );

	return info;
}

ICQOrgAffInfo* ICQUserInfoWidget::storeOrgAffInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQOrgAffInfo* info = new ICQOrgAffInfo( m_orgAffUserInfo );

	info->org1Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->org1KeywordEdit->text() ) );
	info->org2Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->org2KeywordEdit->text() ) );
	info->org3Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->org3KeywordEdit->text() ) );

	int index = m_orgAffInfoWidget->org1CategoryCombo->currentIndex();
	info->org1Category.set( m_orgAffInfoWidget->org1CategoryCombo->itemData( index ).toInt() );

	index = m_orgAffInfoWidget->org2CategoryCombo->currentIndex();
	info->org2Category.set( m_orgAffInfoWidget->org2CategoryCombo->itemData( index ).toInt() );

	index = m_orgAffInfoWidget->org3CategoryCombo->currentIndex();
	info->org3Category.set( m_orgAffInfoWidget->org3CategoryCombo->itemData( index ).toInt() );

	info->pastAff1Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->aff1KeywordEdit->text() ) );
	info->pastAff2Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->aff2KeywordEdit->text() ) );
	info->pastAff3Keyword.set( codec->fromUnicode( m_orgAffInfoWidget->aff3KeywordEdit->text() ) );

	index = m_orgAffInfoWidget->aff1CategoryCombo->currentIndex();
	info->pastAff1Category.set( m_orgAffInfoWidget->aff1CategoryCombo->itemData( index ).toInt() );

	index = m_orgAffInfoWidget->aff2CategoryCombo->currentIndex();
	info->pastAff2Category.set( m_orgAffInfoWidget->aff2CategoryCombo->itemData( index ).toInt() );
	
	index = m_orgAffInfoWidget->aff3CategoryCombo->currentIndex();
	info->pastAff3Category.set( m_orgAffInfoWidget->aff3CategoryCombo->itemData( index ).toInt() );
	
	return info;
}

ICQInterestInfo* ICQUserInfoWidget::storeInterestInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQInterestInfo* info = new ICQInterestInfo( m_interestInfo );

	int index = m_interestInfoWidget->topic1Combo->currentIndex();
	info->topics[0].set( m_interestInfoWidget->topic1Combo->itemData( index ).toInt() );
	info->descriptions[0].set( codec->fromUnicode( m_interestInfoWidget->desc1->text() ) );

	index = m_interestInfoWidget->topic2Combo->currentIndex();
	info->topics[1].set( m_interestInfoWidget->topic2Combo->itemData( index ).toInt() );
	info->descriptions[1].set( codec->fromUnicode( m_interestInfoWidget->desc2->text() ) );

	index = m_interestInfoWidget->topic3Combo->currentIndex();
	info->topics[2].set( m_interestInfoWidget->topic3Combo->itemData( index ).toInt() );
	info->descriptions[2].set( codec->fromUnicode( m_interestInfoWidget->desc3->text() ) );

	index = m_interestInfoWidget->topic4Combo->currentIndex();
	info->topics[3].set( m_interestInfoWidget->topic4Combo->itemData( index ).toInt() );
	info->descriptions[3].set( codec->fromUnicode( m_interestInfoWidget->desc4->text() ) );

	return info;
}

ICQNotesInfo* ICQUserInfoWidget::storeNotesInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQNotesInfo* info = new ICQNotesInfo( m_notesInfo );

	info->notes.set( codec->fromUnicode( m_otherInfoWidget->notesEdit->toPlainText() ) );

	return info;
}

ICQEmailInfo* ICQUserInfoWidget::storeEmailInfo() const
{
	QTextCodec* codec = getTextCodec();
	ICQEmailInfo* info = new ICQEmailInfo( m_emailInfo );

	//Prepend primary email to emails
	QList<ICQEmailInfo::EmailItem> emails = info->emailList.get();
	if ( !m_generalUserInfo.email.get().isEmpty() )
	{
		ICQEmailInfo::EmailItem item;
		item.email = m_generalUserInfo.email.get();
		item.publish = m_generalUserInfo.publishEmail.get();
		emails.prepend( item );
	}
	info->emailList = emails;

	//Store emails
	emails.clear();

	int size = m_emailModel->rowCount();
	for ( int i = 0; i < size; i++ )
	{
		QStandardItem *modelItem = m_emailModel->item( i, 1 );

		ICQEmailInfo::EmailItem item;
		item.email = codec->fromUnicode( modelItem->text() );
		item.publish = ( i > 0 ) ? ( modelItem->checkState() == Qt::Checked ) : false;
		emails.append( item );
	}

	if ( emails.count() == 0 )
	{	// Delete all emails
		ICQEmailInfo::EmailItem item;
		item.publish = false;
		emails.append( item );
	}
	info->emailList.set( emails );

	return info;
}

QMap<QString, int> ICQUserInfoWidget::reverseMap( const QMap<int, QString>& map ) const
{
	QMap<QString, int> revMap;
	QMapIterator<int, QString> it( map );

	while ( it.hasNext() )
	{
		it.next();
		revMap.insert( it.value(), it.key() );
	}

	return revMap;
}

QTextCodec* ICQUserInfoWidget::getTextCodec() const
{
	return (m_contact) ? m_contact->contactCodec() : m_account->defaultCodec();
}

#include "icquserinfowidget.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

