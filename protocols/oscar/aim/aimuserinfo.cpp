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
#include <qpushbutton.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <kdebug.h>
#include <kapplication.h>

AIMUserInfo::AIMUserInfo(const QString name, const QString nick,
						 OscarAccount *account, AIMContact *contact)
	: AIMUserInfoBase()
{
// 	QMimeSourceFactory::defaultFactory()->addFilePath(
// 		kapp->dirs()->findDirs("data","kopete/")[0]);
// 	QMimeSourceFactory::defaultFactory()->addFilePath(
// 		kapp->dirs()->findDirs("data","kopete/pics/")[0]);
	m_name = name;
	m_nick = nick;
	m_account = account;
	// Set the caption
	setCaption( i18n("User Information on %1").arg(name) );

	// Save
	QObject::connect(cmdSave, SIGNAL(clicked()),
					 this, SLOT(slotSaveClicked()));

	// Close
	QObject::connect(cmdClose, SIGNAL(clicked()),
					 this, SLOT(slotCloseClicked()));

	// Engine got user profile
	QObject::connect(m_account->getEngine(),
					 SIGNAL(gotUserProfile(const UserInfo &, const QString)),
					 this, SLOT(slotSearchFound(const UserInfo &, const QString)));

	screenNameLabel->setText(name);
	if (nick.isEmpty()){
		nickNameLE->setText(name);
	} else {
		nickNameLE->setText(nick);
	}

	// If we're conneced
	if( m_account->isConnected() )
	{  // And our buddy is not offline
		if(contact->onlineStatus() != AIMProtocol::protocol()->statusOffline)
		{  // Update the user view to indicate that we're requesting the user's profile
			userInfoView->setText(i18n("Requesting User Profile, please wait"));
			// Ask the engine for the profile
			m_account->getEngine()->sendUserProfileRequest(name);
		}
	}
}

// This constructor is called when we want to edit
// our own profile
AIMUserInfo::AIMUserInfo(const QString name, const QString nick,
						 OscarAccount *account, const QString &profile)
	: AIMUserInfoBase()
{
	QMimeSourceFactory::defaultFactory()->addFilePath(
		kapp->dirs()->findDirs("data","kopete/")[0]);
	QMimeSourceFactory::defaultFactory()->addFilePath(
		kapp->dirs()->findDirs("data","kopete/pics/")[0]);

	m_name = name;
	m_nick = nick;
	setCaption( i18n("User Information on %1").arg(name) );

	m_account = account;

	// Save
	QObject::connect(cmdSave, SIGNAL(clicked()),
					 this, SLOT(slotSaveClicked()));

	// Close
	QObject::connect(cmdClose, SIGNAL(clicked()),
					 this, SLOT(slotCloseClicked()));

	screenNameLabel->setText(name);
	if (nick.isEmpty()){
		nickNameLE->setText(name);
	} else {
		nickNameLE->setText(nick);
	}
	cmdSave->setText("&Save Profile");
	userInfoView->setReadOnly(false);
	userInfoView->setTextFormat(PlainText);
	userInfoView->setText(profile);
}

void AIMUserInfo::slotSaveClicked()
{
	emit updateNickname(nickNameLE->text());
	setCaption( i18n("User Information on %1").arg(nickNameLE->text()) );

	// If the user view is not read only, then it's editable, and we should
	// save it if the user requests to do so
	if (!userInfoView->isReadOnly())
	{  // Tell the engine to set my profile
		m_account->getEngine()->setMyProfile(userInfoView->text());
	}
}

void AIMUserInfo::slotCloseClicked()
{
	delete this;
}

void AIMUserInfo::slotSearchFound(const UserInfo &/*u*/, const QString profile)
{
	kdDebug(14190) << k_funcinfo << "Got User Profile: " << endl
				   << profile << endl;
	userInfoView->setText(profile);
	//disconnect so we can have more than one user profile window open with
	//different users' info in them
	QObject::disconnect(m_account->getEngine(),
						SIGNAL(gotUserProfile(const UserInfo &, const QString)),
						this, SLOT(slotSearchFound(const UserInfo &, const QString)));
}

#include "aimuserinfo.moc"
