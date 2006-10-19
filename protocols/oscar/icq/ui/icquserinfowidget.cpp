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
#include <QtGui/QStringListModel>

#include <kdatewidget.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>

#include "ui_icqgeneralinfo.h"
#include "icqcontact.h"
#include "icqprotocol.h"
#include "ui_icqhomeinfowidget.h"
#include "ui_icqworkinfowidget.h"
#include "ui_icqotherinfowidget.h"
#include "ui_icqinterestinfowidget.h"
#include "ui_icqorgaffinfowidget.h"


ICQUserInfoWidget::ICQUserInfoWidget( QWidget * parent, bool editable )
: KPageDialog( parent ), m_editable( editable )
{
	setFaceType( KPageDialog::List );
	setModal( false );
	setCaption( i18n( "ICQ User Information" ) );
	setDefaultButton( KDialog::Ok );
	
	kDebug(14153) << k_funcinfo << "Creating new icq user info widget" << endl;
	
	QWidget *genInfo = new QWidget(this);
	m_genInfoWidget = new Ui::ICQGeneralInfoWidget;
	m_genInfoWidget->setupUi( genInfo );
	KPageWidgetItem *genInfoItem = addPage( genInfo, i18n("General Info") );
	genInfoItem->setHeader( i18n("General ICQ Information") );
	genInfoItem->setIcon( KIcon("identity") );
	
	QWidget* homeInfo = new QWidget(this);
	m_homeInfoWidget = new Ui::ICQHomeInfoWidget;
	m_homeInfoWidget->setupUi( homeInfo );
	KPageWidgetItem *homeInfoItem = addPage( homeInfo, i18n("Home Info") );
	homeInfoItem->setHeader( i18n("Home Information") );
	homeInfoItem->setIcon( KIcon("gohome") );
	
	QWidget *workInfo = new QWidget(this);
	m_workInfoWidget = new Ui::ICQWorkInfoWidget;
	m_workInfoWidget->setupUi( workInfo );
	KPageWidgetItem *workInfoItem = addPage( workInfo, i18n("Work Info") );
	workInfoItem->setHeader( i18n( "Work Information" ) );
	workInfoItem->setIcon( KIcon("attach") );
	
	QWidget *otherInfo = new QWidget(this);
	m_otherInfoWidget = new Ui::ICQOtherInfoWidget();
	m_otherInfoWidget->setupUi( otherInfo );
	KPageWidgetItem *otherInfoItem = addPage( otherInfo, i18n("Other Info") );
	otherInfoItem->setHeader( i18n( "Other ICQ Information" ) );
	otherInfoItem->setIcon( KIcon("email") );
	
	QWidget *interestInfo = new QWidget(this);
	m_interestInfoWidget = new Ui::ICQInterestInfoWidget();
	m_interestInfoWidget->setupUi( interestInfo );
	KPageWidgetItem *interestInfoItem = addPage( interestInfo, i18n("Interest Info") );
	interestInfoItem->setHeader( i18n( "Interest Information" ) );
	interestInfoItem->setIcon( KIcon("email") );
	
	QWidget *orgAffInfo = new QWidget(this);
	m_orgAffInfoWidget = new Ui::ICQOrgAffInfoWidget();
	m_orgAffInfoWidget->setupUi( orgAffInfo );
	KPageWidgetItem *orgAffInfoItem = addPage( orgAffInfo, i18n("Org & Aff Info") );
	orgAffInfoItem->setHeader( i18n( "Organization & Affiliation Information" ) );
	orgAffInfoItem->setIcon( KIcon("kontact_contacts") );
	
	m_emailModel = new QStringListModel();
	m_otherInfoWidget->emailListView->setModel( m_emailModel );

	connect( m_genInfoWidget->birthdayYearSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDay()) );
	connect( m_genInfoWidget->birthdayMonthSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDay()) );

	connect( m_genInfoWidget->birthdayYearSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );
	connect( m_genInfoWidget->birthdayMonthSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );
	connect( m_genInfoWidget->birthdayDaySpin, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateAge()) );

	//ICQGeneralUserInfo
	m_genInfoWidget->nickNameEdit->setReadOnly( !m_editable );
	m_genInfoWidget->firstNameEdit->setReadOnly( !m_editable );
	m_genInfoWidget->lastNameEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->cityEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->stateEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->phoneEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->faxEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->addressEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->cellEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->zipEdit->setReadOnly( !m_editable );
	m_homeInfoWidget->emailEdit->setReadOnly( !m_editable );

	m_genInfoWidget->ageEdit->setReadOnly( !m_editable );
	m_genInfoWidget->birthdayDaySpin->setReadOnly( !m_editable );
	m_genInfoWidget->birthdayMonthSpin->setReadOnly( !m_editable );
	m_genInfoWidget->birthdayYearSpin->setReadOnly( !m_editable );
	m_homeInfoWidget->homepageEdit->setReadOnly( !m_editable );
	m_genInfoWidget->oCityEdit->setReadOnly( !m_editable );
	m_genInfoWidget->oStateEdit->setReadOnly( !m_editable );

	m_workInfoWidget->cityEdit->setReadOnly( !m_editable );
	m_workInfoWidget->stateEdit->setReadOnly( !m_editable );
	m_workInfoWidget->phoneEdit->setReadOnly( !m_editable );
	m_workInfoWidget->faxEdit->setReadOnly( !m_editable );
	m_workInfoWidget->addressEdit->setReadOnly( !m_editable );
	m_workInfoWidget->zipEdit->setReadOnly( !m_editable );
	m_workInfoWidget->companyEdit->setReadOnly( !m_editable );
	m_workInfoWidget->departmentEdit->setReadOnly( !m_editable );
	m_workInfoWidget->positionEdit->setReadOnly( !m_editable );
	m_workInfoWidget->homepageEdit->setReadOnly( !m_editable );
}

ICQUserInfoWidget::~ ICQUserInfoWidget()
{
	delete m_genInfoWidget;
	delete m_workInfoWidget;
	delete m_otherInfoWidget;
	delete m_interestInfoWidget;
	delete m_emailModel;
}

void ICQUserInfoWidget::setContact( ICQContact* contact )
{
	m_contact = contact;
	QObject::connect( contact, SIGNAL( haveBasicInfo( const ICQGeneralUserInfo& ) ),
	                  this, SLOT( fillBasicInfo( const ICQGeneralUserInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveWorkInfo( const ICQWorkUserInfo& ) ),
	                  this, SLOT( fillWorkInfo( const ICQWorkUserInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveEmailInfo( const ICQEmailInfo& ) ),
	                  this, SLOT( fillEmailInfo( const ICQEmailInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveNotesInfo( const ICQNotesInfo& ) ),
	                  this, SLOT( fillNotesInfo( const ICQNotesInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveMoreInfo( const ICQMoreUserInfo& ) ),
	                  this, SLOT( fillMoreInfo( const ICQMoreUserInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveInterestInfo( const ICQInterestInfo& ) ),
	                  this, SLOT( fillInterestInfo( const ICQInterestInfo& ) ) );
	QObject::connect( contact, SIGNAL( haveOrgAffInfo( const ICQOrgAffInfo& ) ),
	                  this, SLOT( fillOrgAffInfo( const ICQOrgAffInfo& ) ) );

	//Countries
	QMap<QString, int> sortedMap( reverseMap( static_cast<ICQProtocol*>( m_contact->protocol() )->countries() ) );
	QMapIterator<QString, int> it( sortedMap );

	while ( it.hasNext() )
	{
		it.next();
		m_homeInfoWidget->countryCombo->addItem( it.key(), it.value() );
		m_genInfoWidget->oCountryCombo->addItem( it.key(), it.value() );
		m_workInfoWidget->countryCombo->addItem( it.key(), it.value() );
	}

	//Languages
	sortedMap = reverseMap( static_cast<ICQProtocol*>( m_contact->protocol() )->languages() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->language1Combo->addItem( it.key(), it.value() );
		m_genInfoWidget->language2Combo->addItem( it.key(), it.value() );
		m_genInfoWidget->language3Combo->addItem( it.key(), it.value() );
	}

	//Genders
	sortedMap = reverseMap( static_cast<ICQProtocol*>( m_contact->protocol() )->genders() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->genderCombo->addItem( it.key(), it.value() );
	}

	//Maritals
	sortedMap = reverseMap( static_cast<ICQProtocol*>( m_contact->protocol() )->maritals() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_genInfoWidget->maritalCombo->addItem( it.key(), it.value() );
	}

	//Occupation
	sortedMap = reverseMap( static_cast<ICQProtocol*>( m_contact->protocol() )->occupations() );
	it = sortedMap;
	while ( it.hasNext() )
	{
		it.next();
		m_workInfoWidget->occupationCombo->addItem( it.key(), it.value() );
	}

	//Timezone
	QString timezone;
	for ( int zone = 24; zone >= -24; zone-- )
	{
		timezone = QString( "GTM %1%2:%3" )
			.arg( ( zone > 0 ) ? "-" : "" )
			.arg( qAbs( zone ) / 2, 2, 10, QLatin1Char('0') )
			.arg( ( qAbs( zone ) % 2 ) * 30, 2, 10, QLatin1Char('0')  );
		
		m_genInfoWidget->timezoneCombo->addItem( timezone, zone );
	}
}

QList<ICQInfoBase*> ICQUserInfoWidget::getInfoData() const
{
	QList<ICQInfoBase*> infoList;
	
	if ( !m_editable )
		return infoList;
	
	infoList.append( storeBasicInfo() );
	infoList.append( storeMoreInfo() );
	infoList.append( storeWorkInfo() );
	
	return infoList;
}

void ICQUserInfoWidget::fillBasicInfo( const ICQGeneralUserInfo& ui )
{
	QTextCodec* codec = m_contact->contactCodec();
	
	if ( m_editable )
		m_generalUserInfo = ui;
	
	m_genInfoWidget->uinEdit->setText( QString::number( ui.uin.get() ) );
	m_genInfoWidget->nickNameEdit->setText( codec->toUnicode( ui.nickName.get() ) );
	m_genInfoWidget->firstNameEdit->setText( codec->toUnicode( ui.firstName.get() ) );
	m_genInfoWidget->lastNameEdit->setText( codec->toUnicode( ui.lastName.get() ) );
	m_homeInfoWidget->emailEdit->setText( codec->toUnicode( ui.email.get() ) );
	m_homeInfoWidget->cityEdit->setText( codec->toUnicode( ui.city.get() ) );
	m_homeInfoWidget->stateEdit->setText( codec->toUnicode( ui.state.get() ) );
	m_homeInfoWidget->phoneEdit->setText( codec->toUnicode( ui.phoneNumber.get() ) );
	m_homeInfoWidget->faxEdit->setText( codec->toUnicode( ui.faxNumber.get() ) );
	m_homeInfoWidget->addressEdit->setText( codec->toUnicode( ui.address.get() ) );
	m_homeInfoWidget->cellEdit->setText( codec->toUnicode( ui.cellNumber.get() ) );
	m_homeInfoWidget->zipEdit->setText(  codec->toUnicode( ui.zip.get() ) );
	
	m_homeInfoWidget->countryCombo->setCurrentIndex( m_homeInfoWidget->countryCombo->findData( ui.country.get() ) );
	m_genInfoWidget->timezoneCombo->setCurrentIndex( m_genInfoWidget->timezoneCombo->findData( ui.timezone.get() ) );
}

void ICQUserInfoWidget::fillWorkInfo( const ICQWorkUserInfo& ui )
{
	QTextCodec* codec = m_contact->contactCodec();

	if ( m_editable )
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
	QTextCodec* codec = m_contact->contactCodec();
	
	QStringList emails;
	foreach( QByteArray email, info.emailList )
		emails << codec->toUnicode( email );
	
	m_emailModel->setStringList( emails );
}

void ICQUserInfoWidget::fillNotesInfo( const ICQNotesInfo& info )
{
	QTextCodec* codec = m_contact->contactCodec();
	
	m_otherInfoWidget->notesEdit->setText( codec->toUnicode( info.notes ) );
}

void ICQUserInfoWidget::fillInterestInfo( const ICQInterestInfo& info)
{
	QTextCodec* codec = m_contact->contactCodec();
	if (info.count>0) {
		QString topic = static_cast<ICQProtocol*>( m_contact->protocol() )->interests()[info.topics[0]];
		m_interestInfoWidget->topic1->setText( topic );
		m_interestInfoWidget->desc1->setText( codec->toUnicode( info.descriptions[0] ) );
	}
	if (info.count>1) {
		QString topic = static_cast<ICQProtocol*>( m_contact->protocol() )->interests()[info.topics[1]];
		m_interestInfoWidget->topic2->setText( topic );
		m_interestInfoWidget->desc2->setText( codec->toUnicode( info.descriptions[1] ) );
	}
	if (info.count>2) {
		QString topic = static_cast<ICQProtocol*>( m_contact->protocol() )->interests()[info.topics[2]];
		m_interestInfoWidget->topic3->setText( topic );
		m_interestInfoWidget->desc3->setText( codec->toUnicode( info.descriptions[2] ) );
	}
	if (info.count>3) {
		QString topic = static_cast<ICQProtocol*>( m_contact->protocol() )->interests()[info.topics[3]];
		m_interestInfoWidget->topic4->setText( topic );
		m_interestInfoWidget->desc4->setText( codec->toUnicode( info.descriptions[3] ) );
	}
}

void ICQUserInfoWidget::fillOrgAffInfo( const ICQOrgAffInfo& info)
{
	QTextCodec* codec = m_contact->contactCodec();
	
	ICQProtocol* p = static_cast<ICQProtocol*>( m_contact->protocol() );
	
	m_orgAffInfoWidget->org1KeywordEdit->setText( codec->toUnicode( info.org1Keyword ) );
	m_orgAffInfoWidget->org2KeywordEdit->setText( codec->toUnicode( info.org2Keyword ) );
	m_orgAffInfoWidget->org3KeywordEdit->setText( codec->toUnicode( info.org3Keyword ) );
	
	QString org1Category = p->organizations()[ info.org1Category ];
	m_orgAffInfoWidget->org1CategoryEdit->setText( org1Category );
	
	QString org2Category = p->organizations()[ info.org2Category ];
	m_orgAffInfoWidget->org2CategoryEdit->setText( org2Category );
	
	QString org3Category = p->organizations()[ info.org3Category ];
	m_orgAffInfoWidget->org3CategoryEdit->setText( org3Category );
	
	m_orgAffInfoWidget->aff1KeywordEdit->setText( codec->toUnicode( info.pastAff1Keyword ) );
	m_orgAffInfoWidget->aff2KeywordEdit->setText( codec->toUnicode( info.pastAff2Keyword ) );
	m_orgAffInfoWidget->aff3KeywordEdit->setText( codec->toUnicode( info.pastAff3Keyword ) );
	
	QString pastAff1Category = p->affiliations()[ info.pastAff1Category ];
	m_orgAffInfoWidget->aff1CategoryEdit->setText( pastAff1Category );
	
	QString pastAff2Category = p->affiliations()[ info.pastAff2Category ];
	m_orgAffInfoWidget->aff2CategoryEdit->setText( pastAff2Category );
	
	QString pastAff3Category = p->affiliations()[ info.pastAff3Category ];
	m_orgAffInfoWidget->aff3CategoryEdit->setText( pastAff3Category );
}

void ICQUserInfoWidget::fillMoreInfo( const ICQMoreUserInfo& ui )
{
	QTextCodec* codec = m_contact->contactCodec();

	if ( m_editable )
		m_moreUserInfo = ui;

	m_genInfoWidget->ageEdit->setText( QString::number( ui.age.get() ) );
	m_genInfoWidget->birthdayYearSpin->setValue( ui.birthdayYear.get() );
	m_genInfoWidget->birthdayMonthSpin->setValue( ui.birthdayMonth.get() );
	m_genInfoWidget->birthdayDaySpin->setValue( ui.birthdayDay.get() );
	m_genInfoWidget->genderCombo->setCurrentIndex( m_genInfoWidget->genderCombo->findData( ui.gender.get() ) );
	m_homeInfoWidget->homepageEdit->setText( codec->toUnicode( ui.homepage.get() ) );
	m_genInfoWidget->maritalCombo->setCurrentIndex( m_genInfoWidget->maritalCombo->findData( ui.marital.get() ) );
	m_genInfoWidget->oCityEdit->setText( codec->toUnicode( ui.ocity.get() ) );
	m_genInfoWidget->oStateEdit->setText( codec->toUnicode( ui.ostate.get() ) );
	m_genInfoWidget->oCountryCombo->setCurrentIndex( m_genInfoWidget->oCountryCombo->findData( ui.ocountry.get() ) );
	m_genInfoWidget->language1Combo->setCurrentIndex( m_genInfoWidget->language1Combo->findData( ui.lang1.get() ) );
	m_genInfoWidget->language2Combo->setCurrentIndex( m_genInfoWidget->language2Combo->findData( ui.lang2.get() ) );
	m_genInfoWidget->language3Combo->setCurrentIndex( m_genInfoWidget->language3Combo->findData( ui.lang3.get() ) );
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

ICQGeneralUserInfo* ICQUserInfoWidget::storeBasicInfo() const
{
	QTextCodec* codec = m_contact->contactCodec();
	ICQGeneralUserInfo* info = new ICQGeneralUserInfo( m_generalUserInfo );

	info->nickName.set( codec->fromUnicode( m_genInfoWidget->nickNameEdit->text() ) );
	info->firstName.set( codec->fromUnicode( m_genInfoWidget->firstNameEdit->text() ) );
	info->lastName.set( codec->fromUnicode( m_genInfoWidget->lastNameEdit->text() ) );
	info->email.set( codec->fromUnicode( m_homeInfoWidget->emailEdit->text() ) );
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
	QTextCodec* codec = m_contact->contactCodec();
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
	
	info->ocity.set( codec->fromUnicode( m_genInfoWidget->oCityEdit->text() ) );
	info->ostate.set( codec->fromUnicode( m_genInfoWidget->oStateEdit->text() ) );

	index = m_genInfoWidget->oCountryCombo->currentIndex();
	info->ocountry.set( m_genInfoWidget->oCountryCombo->itemData( index ).toInt() );

	index = m_genInfoWidget->language1Combo->currentIndex();
	info->lang1.set( m_genInfoWidget->language1Combo->itemData( index ).toInt() );

	index = m_genInfoWidget->language2Combo->currentIndex();
	info->lang2.set( m_genInfoWidget->language2Combo->itemData( index ).toInt() );

	index = m_genInfoWidget->language3Combo->currentIndex();
	info->lang3.set( m_genInfoWidget->language3Combo->itemData( index ).toInt() );

	return info;
}

ICQWorkUserInfo* ICQUserInfoWidget::storeWorkInfo() const
{
	QTextCodec* codec = m_contact->contactCodec();
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

#include "icquserinfowidget.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

