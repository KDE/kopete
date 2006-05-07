/*
  oscaruserinfo.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

  Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "aimuserinfo.h"

#include "aimaccount.h"
#include "aimcontact.h"
#include "aimprotocol.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kdebug.h>
#include <kapplication.h>

#include <ktextedit.h>
#include <krun.h>

AIMUserInfoDialog::AIMUserInfoDialog( Kopete::Contact *c, AIMAccount *acc, bool modal,
                                      QWidget *parent, const char* name )
	: KDialogBase( parent, name, modal, i18n( "User Information on %1" )
	               .arg( c->property( Kopete::Global::Properties::self()->nickName() ).value().toString() ),
	               Cancel | Ok , Ok, true )
{
	kdDebug(14200) << k_funcinfo << "for contact '" << c->contactId() << "'" << endl;

	m_contact = c;
	mAccount = acc;

	mMainWidget = new AIMUserInfoWidget(this, "aimuserinfowidget");
	setMainWidget(mMainWidget);

	QObject::connect(this, SIGNAL(okClicked()), this, SLOT(slotSaveClicked()));
	QObject::connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUpdateClicked()));
	QObject::connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCloseClicked()));
	QObject::connect(c, SIGNAL(updatedProfile()), this, SLOT(slotUpdateProfile()));

	mMainWidget->txtScreenName->setText( c->contactId() );

	QString nickName = c->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if( nickName.isEmpty() )
		mMainWidget->txtNickName->setText( c->contactId() );
	else
		mMainWidget->txtNickName->setText( nickName );

	if(m_contact == mAccount->myself()) // edit own account profile
	{
		mMainWidget->lblWarnLevel->hide();
		mMainWidget->txtWarnLevel->hide();
		mMainWidget->lblIdleTime->hide();
		mMainWidget->txtIdleTime->hide();
		mMainWidget->lblOnlineSince->hide();
		mMainWidget->txtOnlineSince->hide();
		mMainWidget->txtAwayMessage->hide();
		mMainWidget->lblAwayMessage->hide();

		userInfoView=0L;
		mMainWidget->userInfoFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
		QVBoxLayout *l = new QVBoxLayout(mMainWidget->userInfoFrame);
		userInfoEdit = new KTextEdit(QString::null, QString::null,
			mMainWidget->userInfoFrame, "userInfoEdit");
		userInfoEdit->setTextFormat(PlainText);
		
		AIMMyselfContact* aimmc = dynamic_cast<AIMMyselfContact*>( c );
		if ( aimmc )
			userInfoEdit->setText( aimmc->userProfile() );
		else
			userInfoEdit->setText( QString::null );
		
		setButtonText(Ok, i18n("&Save Profile"));
		showButton(User1, false);
		l->addWidget(userInfoEdit);
	}
	else
	{
		userInfoEdit=0L;
		mMainWidget->userInfoFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
		QVBoxLayout *l = new QVBoxLayout(mMainWidget->userInfoFrame);
		userInfoView = new KTextBrowser(mMainWidget->userInfoFrame, "userInfoView");
		userInfoView->setTextFormat(AutoText);
		userInfoView->setNotifyClick(true);
		QObject::connect(
			userInfoView, SIGNAL(urlClick(const QString&)),
			this, SLOT(slotUrlClicked(const QString&)));
		QObject::connect(
			userInfoView, SIGNAL(mailClick(const QString&, const QString&)),
			this, SLOT(slotMailClicked(const QString&, const QString&)));
		showButton(Cancel, false);
		setButtonText(Ok, i18n("&Close"));
		setEscapeButton(Ok);
		l->addWidget(userInfoView);

		if(m_contact->isOnline())
		{
			// Update the user view to indicate that we're requesting the user's profile
			userInfoView->setText(i18n("Requesting User Profile, please wait..."));
		}
		QTimer::singleShot(0, this, SLOT(slotUpdateProfile()));
	}
}

AIMUserInfoDialog::~AIMUserInfoDialog()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
}

void AIMUserInfoDialog::slotUpdateClicked()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	QString newNick = mMainWidget->txtNickName->text();
	QString currentNick = m_contact->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( !newNick.isEmpty() && ( newNick != currentNick ) )
	{
		//m_contact->rename(newNick);
		//emit updateNickname(newNick);
		setCaption(i18n("User Information on %1").arg(newNick));
	}

}

void AIMUserInfoDialog::slotSaveClicked()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;

	if (userInfoEdit)
	{ // editable mode, set profile
		QString newNick = mMainWidget->txtNickName->text();
		QString currentNick = m_contact->property( Kopete::Global::Properties::self()->nickName() ).value().toString();
		if(!newNick.isEmpty() && ( newNick != currentNick ) )
		{
			//m_contact->rename(newNick);
			//emit updateNickname(newNick);
			setCaption(i18n("User Information on %1").arg(newNick));
		}

		mAccount->setUserProfile(userInfoEdit->text());
	}

	emit closing();
}

void AIMUserInfoDialog::slotCloseClicked()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	emit closing();
}

void AIMUserInfoDialog::slotUpdateProfile()
{
	kdDebug(14152) << k_funcinfo << "Got User Profile." << endl;
	AIMProtocol* p = static_cast<AIMProtocol*>( mAccount->protocol() );
	QString awayMessage = m_contact->property( p->awayMessage ).value().toString();
	mMainWidget->txtAwayMessage->setText( awayMessage );
	
	if ( awayMessage.isNull() )
	{
		mMainWidget->txtAwayMessage->hide();
		mMainWidget->lblAwayMessage->hide();
	}
	else
	{
		mMainWidget->txtAwayMessage->show();
		mMainWidget->lblAwayMessage->show();
	}
	
	QString onlineSince =  m_contact->property("onlineSince").value().toString();
	//QString onlineSince = m_details.onlineSinceTime().toString();
	mMainWidget->txtOnlineSince->setText( onlineSince );
	
	AIMContact* c = static_cast<AIMContact*>( m_contact );
	mMainWidget->txtIdleTime->setText(c->formattedIdleTime());
	mMainWidget->txtWarnLevel->setText(QString::number(c->warningLevel())); 
	
	QString contactProfile = m_contact->property( p->clientProfile ).value().toString();
	if ( contactProfile.isNull() )
	{
		contactProfile =
			i18n("<html><body><I>No user information provided</I></body></html>");
	}
	
	if(userInfoEdit)
	{
		userInfoEdit->setText(contactProfile);
	}
	else if(userInfoView)
	{
		userInfoView->setText(contactProfile);
	}
	
}

void AIMUserInfoDialog::slotUrlClicked(const QString &url)
{
	new KRun(KURL(url));
}

void AIMUserInfoDialog::slotMailClicked(const QString&, const QString &address)
{
	new KRun(KURL(address));
}

#include "aimuserinfo.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
