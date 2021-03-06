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
#include "ui_aiminfobase.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <qtimer.h>
#include <QDesktopServices>
#include <KLocalizedString>

#include <qtextbrowser.h>
#include <kdebug.h>

#include <ktextedit.h>
#include <krun.h>

AIMUserInfoDialog::AIMUserInfoDialog(AIMContact *c, AIMAccount *acc, QWidget *parent)
	: KDialog( parent )
{
	setCaption( i18n( "User Information on %1" ,
	                  c->displayName() ) );
	setButtons( KDialog::Cancel | KDialog::Ok );
	
	setDefaultButton(KDialog::Ok);
	showButtonSeparator(true);
	kDebug(14200) << "for contact '" << c->contactId() << "'";

	m_contact = c;
	mAccount = acc;

	QWidget* w = new QWidget( this );
	mMainWidget = new Ui::AIMUserInfoWidget();
	mMainWidget->setupUi( w );
	setMainWidget( w );

	QObject::connect(this, &AIMUserInfoDialog::okClicked, this, &AIMUserInfoDialog::slotSaveClicked);
	QObject::connect(this, &AIMUserInfoDialog::user1Clicked, this, &AIMUserInfoDialog::slotUpdateClicked);
	QObject::connect(this, &AIMUserInfoDialog::cancelClicked, this, &AIMUserInfoDialog::slotCloseClicked);
	QObject::connect(c, &AIMContact::updatedProfile, this, &AIMUserInfoDialog::slotUpdateProfile);
	QObject::connect(c, &AIMContact::statusMessageChanged, this, &AIMUserInfoDialog::slotUpdatedStatus);

	mMainWidget->txtScreenName->setText( c->contactId() );
	mMainWidget->txtNickName->setText( c->customName() );

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
		l->setContentsMargins( 0, 0, 0, 0 );
		userInfoEdit = new KTextEdit(QString(), mMainWidget->userInfoFrame);

		AIMMyselfContact* aimmc = dynamic_cast<AIMMyselfContact*>( c );
		if ( aimmc )
			userInfoEdit->setPlainText( aimmc->userProfile() );
		else
			userInfoEdit->setPlainText( QString() );

		setButtonText(Ok, i18n("&Save Profile"));
		showButton(User1, false);
		l->addWidget(userInfoEdit);
	}
	else
	{
		userInfoEdit=0L;
		mMainWidget->userInfoFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
		QVBoxLayout *l = new QVBoxLayout(mMainWidget->userInfoFrame);
		l->setContentsMargins( 0, 0, 0, 0 );
        userInfoView = new QTextBrowser(mMainWidget->userInfoFrame);
		userInfoView->setObjectName("userInfoView");
        userInfoView->setOpenLinks(true);
		QObject::connect(userInfoView, &QTextBrowser::anchorClicked, this, &AIMUserInfoDialog::slotUrlClicked);
		// This is not doing anything at present as in their slots they are unused. 
		// QObject::connect(userInfoView, SIGNAL(mailClick(QString,QString)), this, SLOT(slotMailClicked(QString,QString)));
		showButton(Cancel, false);
		setButtonText(Ok, i18n("&Close"));
		setEscapeButton(Ok);
		l->addWidget(userInfoView);

		if(m_contact->isOnline())
		{
			// Update the user view to indicate that we're requesting the user's profile
			userInfoView->setPlainText(i18n("Requesting User Profile, please wait..."));
		}
		QTimer::singleShot(0, this, &AIMUserInfoDialog::slotUpdateProfile);
	}
}

AIMUserInfoDialog::~AIMUserInfoDialog()
{
	delete mMainWidget;
	kDebug(14200) << "Called.";
}

void AIMUserInfoDialog::slotUpdateClicked()
{
	kDebug(14200) << "Called.";
	QString newNick = mMainWidget->txtNickName->text();
	QString currentNick = m_contact->displayName();
	if ( newNick != currentNick )
	{
		m_contact->setNickName(newNick);
		emit updateNickname(newNick);
		setCaption(i18n("User Information on %1", newNick));
	}

}

void AIMUserInfoDialog::slotSaveClicked()
{
	kDebug(14200) << "Called.";

	if (userInfoEdit)
	{ // editable mode, set profile
		QString newNick = mMainWidget->txtNickName->text();
		QString currentNick = m_contact->displayName();
		if(!newNick.isEmpty() && ( newNick != currentNick ) )
		{
			m_contact->setNickName(newNick);
			emit updateNickname(newNick);
			setCaption(i18n("User Information on %1", newNick));
		}

		mAccount->setUserProfile(userInfoEdit->toPlainText());
	}

	emit closing();
}

void AIMUserInfoDialog::slotCloseClicked()
{
	kDebug(14200) << "Called.";
	emit closing();
}

void AIMUserInfoDialog::slotUpdateProfile()
{
	kDebug(14152) << "Got User Profile.";
	AIMProtocol* p = static_cast<AIMProtocol*>( mAccount->protocol() );
	QString awayMessage = m_contact->property( p->statusMessage ).value().toString();
	mMainWidget->txtAwayMessage->setHtml( awayMessage );

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
		userInfoEdit->setPlainText(contactProfile);
	}
	else if(userInfoView)
	{
		userInfoView->setHtml(contactProfile);
	}

}

void AIMUserInfoDialog::slotUpdatedStatus(const Kopete::Contact* contact)
{
	Q_UNUSED(contact);
	slotUpdateProfile();
}

void AIMUserInfoDialog::slotUrlClicked(const QUrl &url)
{
	QDesktopServices::openUrl(url);
}

//KRun changed, so comment it so it compiles FIXME
void AIMUserInfoDialog::slotMailClicked(const QString&, const QString &address)
{
	Q_UNUSED(address);
	//new KRun(KUrl(address));
}

