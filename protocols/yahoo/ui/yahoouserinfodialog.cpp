/*
 Kopete Yahoo Protocol
 yahoouserinfodialog.h - Display Yahoo user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 Andre Duffeck <andre@duffeck.de>

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
#include "yahoocontact.h"

YahooUserInfoDialog::YahooUserInfoDialog( YahooContact *c, QWidget * parent, const char * name )
: KDialogBase( KDialogBase::IconList, 0,  parent, name, false, i18n( "Yahoo User Information" ), User2|User1|Cancel, Cancel, false, i18n("Save and Close"), i18n("Merge with existing entry") )
{
	kdDebug(14180) << k_funcinfo << "Creating new yahoo user info widget" << endl;
	m_contact = c;
	showButton( User2, false );
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
	
	QObject::connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSaveAndCloseClicked()));
}

void YahooUserInfoDialog::setAccountConnected( bool isOnline )
{
	enableButton( User1, isOnline );
	enableButton( User2, isOnline );
}

void YahooUserInfoDialog::slotSaveAndCloseClicked()
{
	YABEntry entry;
	entry.yahooId = m_yab.yahooId;
	entry.YABId = m_yab.YABId;
	entry.firstName = m_genInfoWidget->firstNameEdit->text();
	entry.secondName = m_genInfoWidget->secondNameEdit->text();
	entry.lastName = m_genInfoWidget->lastNameEdit->text();
	entry.nickName = m_genInfoWidget->nickNameEdit->text();
	entry.email = m_genInfoWidget->emailEdit->text();
	entry.privatePhone = m_genInfoWidget->phoneEdit->text();
	entry.workPhone = m_workInfoWidget->phoneEdit->text();
	entry.pager = m_genInfoWidget->pagerEdit->text();
	entry.fax = m_genInfoWidget->faxEdit->text();
	entry.phoneMobile = m_genInfoWidget->cellEdit->text();
	entry.additionalNumber = m_genInfoWidget->additionalEdit->text();
	entry.altEmail1 = m_genInfoWidget->emailEdit_2->text();
	entry.altEmail2 = m_genInfoWidget->emailEdit_3->text();
	entry.privateURL = m_genInfoWidget->homepageEdit->text();
	entry.title = m_genInfoWidget->titleEdit->text();
	entry.corporation = m_workInfoWidget->companyEdit->text();
	entry.workAdress = m_workInfoWidget->addressEdit->text();
	entry.workCity = m_workInfoWidget->cityEdit->text();
	entry.workState = m_workInfoWidget->stateEdit->text();
	entry.workZIP = m_workInfoWidget->zipEdit->text();
	entry.workCountry = m_workInfoWidget->countryEdit->text();
	entry.workURL = m_workInfoWidget->homepageEdit->text();
	entry.privateAdress = m_genInfoWidget->addressEdit->text();
	entry.privateCity = m_genInfoWidget->cityEdit->text();
	entry.privateState = m_genInfoWidget->stateEdit->text();
	entry.privateZIP = m_genInfoWidget->zipEdit->text();
	entry.privateCountry = m_genInfoWidget->countryEdit->text();
	QString bi = m_genInfoWidget->birthdayEdit->text();
	entry.birthday = QDate( bi.section("/",2,2).toInt(), bi.section("/",1,1).toInt(), bi.section("/",0,0).toInt() );
	QString an = m_genInfoWidget->anniversaryEdit->text();
	entry.anniversary = QDate( an.section("/",2,2).toInt(), an.section("/",1,1).toInt(), an.section("/",0,0).toInt() );
	entry.additional1 = m_otherInfoWidget->note1Edit->text();
	entry.additional2 = m_otherInfoWidget->note2Edit->text();
	entry.additional3 = m_otherInfoWidget->note3Edit->text();
	entry.additional4 = m_otherInfoWidget->note4Edit->text();
	entry.notes = m_otherInfoWidget->commentsEdit->text();
// 	entry.imAIM = m_genInfoWidget->firstNameEdit->text();
// 	entry.imGoogleTalk = m_genInfoWidget->firstNameEdit->text();
// 	entry.imICQ = m_genInfoWidget->firstNameEdit->text();
// 	entry.imIRC = m_genInfoWidget->firstNameEdit->text();
// 	entry.imMSN = m_genInfoWidget->firstNameEdit->text();
// 	entry.imQQ = m_genInfoWidget->firstNameEdit->text();
// 	entry.imSkype = m_genInfoWidget->firstNameEdit->text();
		
	emit saveYABEntry( entry );
	
	QDialog::accept();
}

void YahooUserInfoDialog::slotUser2()
{
	if( m_contact )
	{
		YABEntry entry;
		const YABEntry *oldEntry = m_contact->yabEntry();

		entry.yahooId = m_yab.yahooId;
		entry.YABId = m_yab.YABId;
		entry.firstName = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->firstName : m_genInfoWidget->firstNameEdit->text();
		entry.secondName = m_genInfoWidget->secondNameEdit->text().isEmpty() ? oldEntry->secondName : m_genInfoWidget->secondNameEdit->text();
		entry.lastName = m_genInfoWidget->lastNameEdit->text().isEmpty() ? oldEntry->lastName : m_genInfoWidget->lastNameEdit->text();
		entry.nickName = m_genInfoWidget->nickNameEdit->text().isEmpty() ? oldEntry->nickName : m_genInfoWidget->nickNameEdit->text();
		entry.email = m_genInfoWidget->emailEdit->text().isEmpty() ? oldEntry->email : m_genInfoWidget->emailEdit->text();
		entry.privatePhone = m_genInfoWidget->phoneEdit->text().isEmpty() ? oldEntry->privatePhone : m_genInfoWidget->phoneEdit->text();
		entry.workPhone = m_workInfoWidget->phoneEdit->text().isEmpty() ? oldEntry->workPhone : m_workInfoWidget->phoneEdit->text();
		entry.pager = m_genInfoWidget->pagerEdit->text().isEmpty() ? oldEntry->pager : m_genInfoWidget->pagerEdit->text();
		entry.fax = m_genInfoWidget->faxEdit->text().isEmpty() ? oldEntry->fax : m_genInfoWidget->faxEdit->text();
		entry.phoneMobile = m_genInfoWidget->cellEdit->text().isEmpty() ? oldEntry->phoneMobile : m_genInfoWidget->cellEdit->text();
		entry.additionalNumber = m_genInfoWidget->additionalEdit->text().isEmpty() ? oldEntry->additionalNumber : m_genInfoWidget->additionalEdit->text();
		entry.altEmail1 = m_genInfoWidget->emailEdit_2->text().isEmpty() ? oldEntry->altEmail1 : m_genInfoWidget->emailEdit_2->text();
		entry.altEmail2 = m_genInfoWidget->emailEdit_3->text().isEmpty() ? oldEntry->altEmail2 : m_genInfoWidget->emailEdit_3->text();
		entry.privateURL = m_genInfoWidget->homepageEdit->text().isEmpty() ? oldEntry->privateURL : m_genInfoWidget->homepageEdit->text();
		entry.title = m_genInfoWidget->titleEdit->text().isEmpty() ? oldEntry->title : m_genInfoWidget->titleEdit->text();
		entry.corporation = m_workInfoWidget->companyEdit->text().isEmpty() ? oldEntry->corporation : m_workInfoWidget->companyEdit->text();
		entry.workAdress = m_workInfoWidget->addressEdit->text().isEmpty() ? oldEntry->workAdress : m_workInfoWidget->addressEdit->text();
		entry.workCity = m_workInfoWidget->cityEdit->text().isEmpty() ? oldEntry->workCity : m_workInfoWidget->cityEdit->text();
		entry.workState = m_workInfoWidget->stateEdit->text().isEmpty() ? oldEntry->workState : m_workInfoWidget->stateEdit->text();
		entry.workZIP = m_workInfoWidget->zipEdit->text().isEmpty() ? oldEntry->workZIP : m_workInfoWidget->zipEdit->text();
		entry.workCountry = m_workInfoWidget->countryEdit->text().isEmpty() ? oldEntry->workCountry : m_workInfoWidget->countryEdit->text();
		entry.workURL = m_workInfoWidget->homepageEdit->text().isEmpty() ? oldEntry->workURL : m_workInfoWidget->homepageEdit->text();
		entry.privateAdress = m_genInfoWidget->addressEdit->text().isEmpty() ? oldEntry->privateAdress : m_genInfoWidget->addressEdit->text();
		entry.privateCity = m_genInfoWidget->cityEdit->text().isEmpty() ? oldEntry->privateCity : m_genInfoWidget->cityEdit->text();
		entry.privateState = m_genInfoWidget->stateEdit->text().isEmpty() ? oldEntry->privateState : m_genInfoWidget->stateEdit->text();
		entry.privateZIP = m_genInfoWidget->zipEdit->text().isEmpty() ? oldEntry->privateZIP : m_genInfoWidget->zipEdit->text();
		entry.privateCountry = m_genInfoWidget->countryEdit->text().isEmpty() ? oldEntry->privateCountry : m_genInfoWidget->countryEdit->text();
		
		if( m_genInfoWidget->birthdayEdit->text().isEmpty() )
			entry.birthday = oldEntry->birthday;
		else
		{
			QString bi = m_genInfoWidget->birthdayEdit->text();
			entry.birthday = QDate( bi.section("/",2,2).toInt(), bi.section("/",1,1).toInt(), bi.section("/",0,0).toInt() );
		}
		
		if( m_genInfoWidget->anniversaryEdit->text().isEmpty() )
			entry.anniversary = oldEntry->anniversary;
		else
		{
			QString an = m_genInfoWidget->anniversaryEdit->text();
			entry.anniversary = QDate( an.section("/",2,2).toInt(), an.section("/",1,1).toInt(), an.section("/",0,0).toInt() );
		}
		
		entry.additional1 = m_otherInfoWidget->note1Edit->text().isEmpty() ? oldEntry->additional1 : m_otherInfoWidget->note1Edit->text();
		entry.additional2 = m_otherInfoWidget->note2Edit->text().isEmpty() ? oldEntry->additional2 : m_otherInfoWidget->note2Edit->text();
		entry.additional3 = m_otherInfoWidget->note3Edit->text().isEmpty() ? oldEntry->additional3 : m_otherInfoWidget->note3Edit->text();
		entry.additional4 = m_otherInfoWidget->note4Edit->text().isEmpty() ? oldEntry->additional4 : m_otherInfoWidget->note4Edit->text();
		entry.notes = m_otherInfoWidget->commentsEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imAIM = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imGoogleTalk = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imICQ = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imIRC = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imMSN = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imQQ = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
	// 	entry.imSkype = m_genInfoWidget->firstNameEdit->text().isEmpty() ? oldEntry->notes : m_otherInfoWidget->commentsEdit->text();
		
		emit saveYABEntry( entry );
	}
	
	QDialog::accept();
}

void YahooUserInfoDialog::setData( const YABEntry &yab )
{	
	m_yab = yab;
	
	if( m_yab.source == YABEntry::SourceContact )
	{
		showButton( User2, true );
		setButtonText( User1, i18n("Replace existing entry") );
	}
	
	m_genInfoWidget->firstNameEdit->setText( yab.firstName );
	m_genInfoWidget->secondNameEdit->setText( yab.secondName );
	m_genInfoWidget->lastNameEdit->setText( yab.lastName );
	m_genInfoWidget->nickNameEdit->setText( yab.nickName );
	m_genInfoWidget->yahooIdEdit->setText( yab.yahooId );
	m_genInfoWidget->titleEdit->setText( yab.title );
	
	if( yab.birthday.isValid() )
		m_genInfoWidget->birthdayEdit->setText( QString("%1/%2/%3").arg( yab.birthday.day() ).arg( yab.birthday.month() ).arg( yab.birthday.year() ));
	if( yab.anniversary.isValid() )
	m_genInfoWidget->anniversaryEdit->setText( QString("%1/%2/%3").arg( yab.anniversary.day() ).arg( yab.anniversary.month() ).arg( yab.anniversary.year() ));
	
	m_genInfoWidget->addressEdit->setText( yab.privateAdress );
	m_genInfoWidget->cityEdit->setText( yab.privateCity );
	m_genInfoWidget->stateEdit->setText( yab.privateState );
	m_genInfoWidget->zipEdit->setText( yab.privateZIP );
	m_genInfoWidget->countryEdit->setText( yab.privateCountry );
	m_genInfoWidget->phoneEdit->setText( yab.privatePhone );
	m_genInfoWidget->cellEdit->setText( yab.phoneMobile );
	m_genInfoWidget->faxEdit->setText( yab.fax );
	m_genInfoWidget->pagerEdit->setText( yab.pager );
	m_genInfoWidget->emailEdit->setText( yab.email );
	m_genInfoWidget->emailEdit_2->setText( yab.altEmail1 );
	m_genInfoWidget->emailEdit_3->setText( yab.altEmail2 );
	m_genInfoWidget->homepageEdit->setText( yab.privateURL );
	m_genInfoWidget->additionalEdit->setText( yab.additionalNumber );
	
	m_workInfoWidget->phoneEdit->setText( yab.workPhone );
	m_workInfoWidget->addressEdit->setText( yab.workAdress );
	m_workInfoWidget->cityEdit->setText( yab.workCity );
	m_workInfoWidget->stateEdit->setText( yab.workState );
	m_workInfoWidget->zipEdit->setText( yab.workZIP );
	m_workInfoWidget->countryEdit->setText( yab.workCountry );
	m_workInfoWidget->companyEdit->setText( yab.corporation );
	m_workInfoWidget->homepageEdit->setText( yab.workURL );
	
	m_otherInfoWidget->commentsEdit->setText( yab.notes );
	m_otherInfoWidget->note1Edit->setText( yab.additional1 );
	m_otherInfoWidget->note2Edit->setText( yab.additional2 );
	m_otherInfoWidget->note3Edit->setText( yab.additional3 );
	m_otherInfoWidget->note4Edit->setText( yab.additional4 );
}

#include "yahoouserinfodialog.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;

