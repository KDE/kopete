/*
 Kopete Oscar Protocol
 icquserinfowidget.cpp - Display ICQ user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>

 Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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


ICQUserInfoWidget::ICQUserInfoWidget( QWidget * parent )
: KPageDialog( parent )
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
}

void ICQUserInfoWidget::fillBasicInfo( const ICQGeneralUserInfo& ui )
{
	QTextCodec* codec = m_contact->contactCodec();
	m_genInfoWidget->uinEdit->setText( m_contact->contactId() );
	m_genInfoWidget->nickNameEdit->setText( codec->toUnicode( ui.nickname ) );
	m_genInfoWidget->fullNameEdit->setText( codec->toUnicode( ui.firstName ) + ' ' + codec->toUnicode( ui.lastName ) );
	m_genInfoWidget->ipEdit->setText( m_contact->property( "ipAddress" ).value().toString() );
	m_homeInfoWidget->emailEdit->setText( codec->toUnicode( ui.email ) );
	m_homeInfoWidget->cityEdit->setText( codec->toUnicode( ui.city ) );
	m_homeInfoWidget->stateEdit->setText( codec->toUnicode( ui.state ) );
	m_homeInfoWidget->phoneEdit->setText( codec->toUnicode( ui.phoneNumber ) );
	m_homeInfoWidget->faxEdit->setText( codec->toUnicode( ui.faxNumber ) );
	m_homeInfoWidget->addressEdit->setText( codec->toUnicode( ui.address ) );
	m_homeInfoWidget->cellEdit->setText( codec->toUnicode( ui.cellNumber ) );
	m_homeInfoWidget->zipEdit->setText(  codec->toUnicode( ui.zip ) );
	
	QString country = static_cast<ICQProtocol*>( m_contact->protocol() )->countries()[ui.country];
	m_homeInfoWidget->countryEdit->setText( country );
	
	QString timezone = QString( "GTM %1%2:%3" )
		.arg( ( ui.timezone > 0 ) ? "-" : "" )
		.arg( qAbs( ui.timezone ) / 2, 2, 10, QLatin1Char('0') )
		.arg( ( qAbs( ui.timezone ) % 2 ) * 30, 2, 10, QLatin1Char('0')  );
	m_genInfoWidget->timezoneEdit->setText( timezone );
}

void ICQUserInfoWidget::fillWorkInfo( const ICQWorkUserInfo& ui )
{
	QTextCodec* codec = m_contact->contactCodec();
	m_workInfoWidget->cityEdit->setText( codec->toUnicode( ui.city ) );
	m_workInfoWidget->stateEdit->setText( codec->toUnicode( ui.state ) );
	m_workInfoWidget->phoneEdit->setText( codec->toUnicode( ui.phone ) );
	m_workInfoWidget->faxEdit->setText( codec->toUnicode( ui.fax ) );
	m_workInfoWidget->addressEdit->setText( codec->toUnicode( ui.address ) );
	m_workInfoWidget->zipEdit->setText( codec->toUnicode( ui.zip ) );
	m_workInfoWidget->companyEdit->setText( codec->toUnicode( ui.company ) );
	m_workInfoWidget->departmentEdit->setText( codec->toUnicode( ui.department ) );
	m_workInfoWidget->positionEdit->setText( codec->toUnicode( ui.position ) );
	m_workInfoWidget->homepageEdit->setText( codec->toUnicode( ui.homepage ) );

	ICQProtocol* p = static_cast<ICQProtocol*>( m_contact->protocol() );
	QString country = p->countries()[ui.country];
	m_workInfoWidget->countryEdit->setText( country );
	
	QString occupation = p->occupations()[ui.occupation];
	m_workInfoWidget->occupationEdit->setText( occupation );
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
	m_genInfoWidget->ageSpinBox->setValue( ui.age );
	if ( ui.birthday.isValid() )
		m_genInfoWidget->birthday->setText( KGlobal::locale()->formatDate( ui.birthday,true ) );
		
	QString gender = static_cast<ICQProtocol*>( m_contact->protocol() )->genders()[ui.gender];
	m_genInfoWidget->genderEdit->setText( gender );
	m_homeInfoWidget->homepageEdit->setText( codec->toUnicode( ui.homepage ) );

	QString ms = static_cast<ICQProtocol*>( m_contact->protocol() )->maritals()[ui.marital];
	m_genInfoWidget->marital->setText( ms );

	m_genInfoWidget->oCityEdit->setText( codec->toUnicode( ui.ocity) );
	m_genInfoWidget->oStateEdit->setText( codec->toUnicode( ui.ostate) );
	
	ICQProtocol* p = static_cast<ICQProtocol*>( m_contact->protocol() );
	
	QString ocountry = p->countries()[ui.ocountry];
	m_genInfoWidget->oCountryEdit->setText( ocountry );
	
	QString lang1 = p->languages()[ui.lang1];
	m_genInfoWidget->language1Edit->setText( lang1 );
	
	QString lang2 = p->languages()[ui.lang2];
	m_genInfoWidget->language2Edit->setText( lang2 );
	
	QString lang3 = p->languages()[ui.lang3];
	m_genInfoWidget->language3Edit->setText( lang3 );
}


#include "icquserinfowidget.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

