/*
 Kopete Yahoo Protocol
 yahoouserinfodialog.h - Display Yahoo user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 Andre Duffeck <mattr@kde.org>

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

#include "yahoouserinfodialog.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qobject.h>
#include <qtextcodec.h>

#include <kdatewidget.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <klocale.h>

#include "yahooworkinfowidget.h"
#include "yahoogeneralinfowidget.h"
#include "yahoootherinfowidget.h"


YahooUserInfoDialog::YahooUserInfoDialog( QWidget * parent, const char * name )
: KDialogBase( KDialogBase::IconList, 0,  parent, name, false, i18n( "Yahoo User Information" ), Ok )
{
	kDebug(14153) << k_funcinfo << "Creating new yahoo user info widget" << endl;
	
	QFrame* genInfo = addPage( i18n( "General Info" ),
	                                         i18n( "General Yahoo Information" ),
	                                         KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "identity" ), KIcon::Desktop ) );
	QVBoxLayout* genLayout = new QVBoxLayout( genInfo );
	m_genInfoWidget = new YahooGeneralInfoWidget( genInfo, "Basic Information" );
	genLayout->addWidget( m_genInfoWidget );
	
	QFrame* workInfo = addPage( i18n( "Work Info" ),
	                                          i18n( "Work Information" ),
	                                          KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "attach" ), KIcon::Desktop ) );
	QVBoxLayout* workLayout = new QVBoxLayout( workInfo );
	m_workInfoWidget = new YahooWorkInfoWidget( workInfo, "Work Information" );
	workLayout->addWidget( m_workInfoWidget );
	
	QFrame* otherInfo = addPage( i18n( "Other Info" ),
	                                           i18n( "Other Yahoo Information" ),
	                                           KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "email" ), KIcon::Desktop ) );
	QVBoxLayout* otherLayout = new QVBoxLayout( otherInfo );
	m_otherInfoWidget = new YahooOtherInfoWidget( otherInfo, "Other Information"  );
	otherLayout->addWidget( m_otherInfoWidget );
// 	
// 	QFrame* interestInfo = addPage( i18n( "Interest Info" ),
// 	                                           i18n( "Interest" ),
// 	                                           KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "email" ), KIcon::Desktop ) );
// 	QVBoxLayout* interestLayout = new QVBoxLayout( interestInfo );
// 	m_interestInfoWidget = new ICQInterestInfoWidget( interestInfo, "Other Information"  );
// 	interestLayout->addWidget( m_interestInfoWidget );

}

void YahooUserInfoDialog::setData( const YABEntry *yab )
{	
	m_genInfoWidget->firstNameEdit->setText( yab->firstName );
	m_genInfoWidget->secondNameEdit->setText( yab->secondName );
	m_genInfoWidget->lastNameEdit->setText( yab->lastName );
	m_genInfoWidget->nickNameEdit->setText( yab->nickName );
	m_genInfoWidget->yahooIdEdit->setText( yab->yahooId );
	m_genInfoWidget->titleEdit->setText( yab->title );
	m_genInfoWidget->birthdayEdit->setText( yab->birthday.toString() );
	m_genInfoWidget->anniversaryEdit->setText( yab->anniversary.toString() );
	
	m_genInfoWidget->addressEdit->setText( yab->privateAdress );
	m_genInfoWidget->cityEdit->setText( yab->privateCity );
	m_genInfoWidget->stateEdit->setText( yab->privateState );
	m_genInfoWidget->zipEdit->setText( yab->privateZIP );
	m_genInfoWidget->countryEdit->setText( yab->privateCountry );
	m_genInfoWidget->phoneEdit->setText( yab->privatePhone );
	m_genInfoWidget->cellEdit->setText( yab->phoneMobile );
	m_genInfoWidget->faxEdit->setText( yab->fax );
	m_genInfoWidget->emailEdit->setText( yab->email );
	m_genInfoWidget->emailEdit_2->setText( yab->altEmail1 );
	m_genInfoWidget->emailEdit_3->setText( yab->altEmail2 );
	m_genInfoWidget->homepageEdit->setText( yab->privateURL );
	
	m_workInfoWidget->phoneEdit->setText( yab->workPhone );
	m_workInfoWidget->addressEdit->setText( yab->workAdress );
	m_workInfoWidget->cityEdit->setText( yab->workCity );
	m_workInfoWidget->stateEdit->setText( yab->workState );
	m_workInfoWidget->zipEdit->setText( yab->workZIP );
	m_workInfoWidget->countryEdit->setText( yab->workCountry );
	m_workInfoWidget->companyEdit->setText( yab->corporation );
	m_workInfoWidget->homepageEdit->setText( yab->workURL );
	
	m_otherInfoWidget->commentsEdit->setText( yab->notes );
	m_otherInfoWidget->note1Edit->setText( yab->additional1 );
	m_otherInfoWidget->note2Edit->setText( yab->additional2 );
	m_otherInfoWidget->note3Edit->setText( yab->additional3 );
	m_otherInfoWidget->note4Edit->setText( yab->additional4 );
}

#include "yahoouserinfodialog.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

