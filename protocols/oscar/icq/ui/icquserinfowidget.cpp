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

#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qobject.h>

#include <kdatewidget.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <klocale.h>

#include "icqgeneralinfo.h"
#include "icqcontact.h"
#include "icqprotocol.h"
#include "icqworkinfowidget.h"
#include "icqotherinfowidget.h"


ICQUserInfoWidget::ICQUserInfoWidget( QWidget * parent, const char * name )
: KDialogBase( KDialogBase::IconList, 0,  parent, name, false, i18n( "ICQ User Information" ), Ok )
{
	kdDebug(14153) << k_funcinfo << "Creating new icq user info widget" << endl;
	
	QFrame* genInfo = addPage( i18n( "General Info" ),
	                                         i18n( "General ICQ Information" ),
	                                         KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "identity" ), KIcon::Desktop ) );
	QVBoxLayout* genLayout = new QVBoxLayout( genInfo );
	m_genInfoWidget = new ICQGeneralInfoWidget( genInfo, "Basic Information" );
	genLayout->addWidget( m_genInfoWidget );
	
	QFrame* workInfo = addPage( i18n( "Work Info" ),
	                                          i18n( "Work Information" ),
	                                          KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "attach" ), KIcon::Desktop ) );
	QVBoxLayout* workLayout = new QVBoxLayout( workInfo );
	m_workInfoWidget = new ICQWorkInfoWidget( workInfo, "Work Information" );
	workLayout->addWidget( m_workInfoWidget );
	
	QFrame* otherInfo = addPage( i18n( "Other Info" ),
	                                           i18n( "Other ICQ Information" ),
	                                           KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "email" ), KIcon::Desktop ) );
	QVBoxLayout* otherLayout = new QVBoxLayout( otherInfo );
	m_otherInfoWidget = new ICQOtherInfoWidget( otherInfo, "Other Information"  );
	otherLayout->addWidget( m_otherInfoWidget );
	
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
	QObject::connect( contact, SIGNAL( haveMoreInfo( const ICQMoreUserInfo& ) ),
	                  this, SLOT( fillMoreInfo( const ICQMoreUserInfo& ) ) );
}

void ICQUserInfoWidget::fillBasicInfo( const ICQGeneralUserInfo& ui )
{
	m_genInfoWidget->uinEdit->setText( m_contact->contactId() );
	m_genInfoWidget->nickNameEdit->setText( ui.nickname );
	m_genInfoWidget->fullNameEdit->setText( ui.firstName + " " + ui.lastName );
	m_genInfoWidget->ipEdit->setText( QString::fromLatin1( "0.0.0.0" ) );
	m_genInfoWidget->emailEdit->setText( ui.email );
	m_genInfoWidget->cityEdit->setText( ui.city );
	m_genInfoWidget->stateEdit->setText( ui.state );
	m_genInfoWidget->phoneEdit->setText( ui.phoneNumber );
	m_genInfoWidget->faxEdit->setText( ui.faxNumber );
	m_genInfoWidget->addressEdit->setText( ui.address );
	m_genInfoWidget->cellEdit->setText( ui.cellNumber );
	m_genInfoWidget->zipEdit->setText(  ui.zip );
	
	QString country = static_cast<ICQProtocol*>( m_contact->protocol() )->countries()[ui.country];
	m_genInfoWidget->countryEdit->setText( country );
}

void ICQUserInfoWidget::fillWorkInfo( const ICQWorkUserInfo& ui )
{
	m_workInfoWidget->cityEdit->setText( ui.city );
	m_workInfoWidget->stateEdit->setText( ui.state );
	m_workInfoWidget->phoneEdit->setText( ui.phone );
	m_workInfoWidget->faxEdit->setText( ui.fax );
	m_workInfoWidget->addressEdit->setText( ui.address );
	m_workInfoWidget->zipEdit->setText( ui.zip );
	m_workInfoWidget->companyEdit->setText( ui.company );
	m_workInfoWidget->departmentEdit->setText( ui.department );
	m_workInfoWidget->positionEdit->setText( ui.position );
	m_workInfoWidget->homepageEdit->setText( ui.homepage );

	ICQProtocol* p = static_cast<ICQProtocol*>( m_contact->protocol() );
	QString country = p->countries()[ui.country];
	m_workInfoWidget->countryEdit->setText( country );
	
	//TODO handle the occupation
}

void ICQUserInfoWidget::fillEmailInfo( const ICQEmailInfo& )
{
}

void ICQUserInfoWidget::fillMoreInfo( const ICQMoreUserInfo& ui )
{
	m_genInfoWidget->ageSpinBox->setValue( ui.age );
	if ( ui.birthday.isValid() )
		m_genInfoWidget->birthday->setText( KGlobal::locale()->formatDate( ui.birthday,true ) );
		
	QString gender = static_cast<ICQProtocol*>( m_contact->protocol() )->genders()[ui.gender];
	m_genInfoWidget->genderEdit->setText( gender );
	m_genInfoWidget->homepageEdit->setText( ui.homepage );
	//TODO languages	
}


#include "icquserinfowidget.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

