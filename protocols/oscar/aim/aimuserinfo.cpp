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

#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kdebug.h>
#include <kapplication.h>

#include <ktextedit.h>
#include <khtmlview.h>
#include <khtml_part.h>

AIMUserInfoDialog::AIMUserInfoDialog(AIMContact *c, AIMAccount *acc, bool modal,
	QWidget *parent, const char* name)
	: KDialogBase(parent, name, modal, i18n("User Information on %1").arg(c->displayName()), Close | User1, Close, true,
		i18n("&Save Nickname"))
{
	kdDebug(14200) << k_funcinfo << "for contact '" << c->displayName() << "'" << endl;

	mContact = c;
	mAccount = acc;

	mMainWidget = new AIMUserInfoWidget(this, "aimuserinfowidget");
	setMainWidget(mMainWidget);

	QObject::connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSaveClicked()));
	QObject::connect(this, SIGNAL(closeClicked()), this, SLOT(slotCloseClicked()));
	QObject::connect(mContact, SIGNAL(updatedProfile()), this, SLOT(slotUpdateProfile()));

	mMainWidget->txtScreenName->setText(c->contactName());

	if(mContact->displayName().isEmpty())
		mMainWidget->txtNickName->setText(mContact->contactName());
	else
		mMainWidget->txtNickName->setText(mContact->displayName());

	if(mContact == mAccount->myself()) // edit own account profile
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
		mMainWidget->htmlFrame->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
		QVBoxLayout *l = new QVBoxLayout(mMainWidget->htmlFrame);
		userInfoEdit = new KTextEdit(QString::null, QString::null,
			mMainWidget->htmlFrame, "userInfoEdit");
		userInfoEdit->setTextFormat(PlainText);
		userInfoEdit->setText(mContact->userProfile());
		setButtonText(User1, "&Save Profile");
		l->addWidget(userInfoEdit);
	}
	else
	{
		userInfoEdit=0L;
		mMainWidget->htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
		QVBoxLayout *l = new QVBoxLayout(mMainWidget->htmlFrame);
		userInfoView = new KHTMLPart(mMainWidget->htmlFrame, "preview");
		userInfoView->setJScriptEnabled(false);
		userInfoView->setJavaEnabled(false);
		userInfoView->setPluginsEnabled(false);
		userInfoView->setMetaRefreshEnabled(false);
		KHTMLView *htmlWidget = userInfoView->view();
		htmlWidget->setMarginWidth(4);
		htmlWidget->setMarginHeight(4);
		htmlWidget->setFocusPolicy(NoFocus);
		htmlWidget->setSizePolicy(
			QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		l->addWidget(htmlWidget);

		if(mAccount->isConnected())
		{  // And our buddy is not offline
			if(mContact->onlineStatus() != AIMProtocol::protocol()->statusOffline)
			{
				// Update the user view to indicate that we're requesting the user's profile
				userInfoView->begin();
				userInfoView->write(i18n("Requesting User Profile, please wait"));
				userInfoView->end();
				// Ask the engine for the profile
				mAccount->engine()->sendUserProfileRequest(mContact->contactName());
			}
		}
	}
}

AIMUserInfoDialog::~AIMUserInfoDialog()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
}

void AIMUserInfoDialog::slotSaveClicked()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	QString newNick = mMainWidget->txtNickName->text();
	if(!newNick.isEmpty() && (newNick != mContact->displayName()))
	{
		mContact->rename(newNick);
//		emit updateNickname(newNick);
		setCaption(i18n("User Information on %1").arg(newNick));
	}

	if (userInfoEdit) // editable mode, set profile
		mAccount->setUserProfile(userInfoEdit->text());
}

void AIMUserInfoDialog::slotCloseClicked()
{
	kdDebug(14200) << k_funcinfo << "Called." << endl;
	emit closing();
}

void AIMUserInfoDialog::slotUpdateProfile()
{
	kdDebug(14190) << k_funcinfo << "Got User Profile." << endl;

	QObject::disconnect(mContact, SIGNAL(updatedProfile()), this, SLOT(slotUpdateProfile()));

	QDateTime qdt;
	qdt.setTime_t(static_cast<uint>(mContact->userInfo().onlinesince));
	mMainWidget->txtOnlineSince->setText(qdt.toString());
	mMainWidget->txtIdleTime->setText(QString::number(mContact->userInfo().idletime));
	mMainWidget->txtAwayMessage->setText(mContact->awayMessage());
	mMainWidget->txtWarnLevel->setText(QString::number(mContact->userInfo().evil));

	QString contactProfile = mContact->userProfile();
	if(contactProfile.isNull())
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
		userInfoView->begin();
		userInfoView->write(contactProfile);
		userInfoView->end();
	}
}

#include "aimuserinfo.moc"
