/*
    yahooauthreply.h - UI Page for Accepting/Rejecting an authorization request

    Copyright (c) 2006 by André Duffeck          <andre.duffeck@kdemail.net>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocale.h>

#include "yahooauthreplybase.h"
#include "yahooauthreply.h"

YahooAuthReply::YahooAuthReply(QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("Authorization Request - Yahoo"), KDialogBase::Ok | KDialogBase::Cancel,
              KDialogBase::Ok, true )
{
	mTheDialog = new YahooAuthReplyBase( this );
	setMainWidget( mTheDialog );
	setEscapeButton( Cancel );
}

YahooAuthReply::~YahooAuthReply()
{
	kdDebug(14180) << k_funcinfo << endl;
}	

bool YahooAuthReply::acceptAuth()
{
	return mTheDialog->rbGrant->isChecked(); 
}

QString YahooAuthReply::user()
{
	return m_User;
}

QString YahooAuthReply::reason()
{
	return mTheDialog->leReason->text();
}

void YahooAuthReply::setRequestReason( const QString &reason )
{
	mTheDialog->lblRequestReason->setText( reason );
}

void YahooAuthReply::setUser( const QString& user )
{
	m_User = user;
	updateReqLabel();
}

void YahooAuthReply::setName( const QString& name )
{
	m_Name = name;
	updateReqLabel();
}

void YahooAuthReply::updateReqLabel()
{
	QString nickandname;
	if( m_Name.isEmpty() )
		nickandname = QString("<b>%1</b>").arg(m_User);
	else
		nickandname = QString("<b>%1</b> (%2)").arg(m_User).arg(m_Name);
	mTheDialog->lblUserReq->setText( 
		i18n( "%1 requested authorization to add you to his/her contact list." ).arg( nickandname ) );
}

#include "yahooauthreply.moc"
