/*
 Kopete Oscar Protocol
 icquserinfowidget.h - Display ICQ user info

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

#ifndef _ICQUSERINFOWIDGET_H_
#define _ICQUSERINFOWIDGET_H_

#include <kdialogbase.h>
#include <icquserinfo.h>

class KJanusWidget;
class ICQGeneralInfoWidget;
class ICQWorkInfoWidget;
class ICQOtherInfoWidget;
class ICQInterestInfoWidget;
class ICQContact;

class ICQUserInfoWidget : public KDialogBase
{
Q_OBJECT
public:
	ICQUserInfoWidget( QWidget* parent = 0, const char* name = 0 );
	void setContact( ICQContact* contact );
	
public slots:
	void fillBasicInfo( const ICQGeneralUserInfo& );
	void fillWorkInfo( const ICQWorkUserInfo& );
	void fillEmailInfo( const ICQEmailInfo& );
	void fillMoreInfo( const ICQMoreUserInfo& );
	void fillInterestInfo( const ICQInterestInfo& );

private:
	ICQGeneralInfoWidget* m_genInfoWidget;
	ICQWorkInfoWidget* m_workInfoWidget;
	ICQOtherInfoWidget* m_otherInfoWidget;
	ICQInterestInfoWidget * m_interestInfoWidget;
	KJanusWidget* m_janusWidget;
	ICQContact* m_contact;
	
};

#endif

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
