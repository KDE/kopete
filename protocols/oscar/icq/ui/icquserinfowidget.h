/*
 Kopete Oscar Protocol
 icquserinfowidget.h - Display ICQ user info

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 Roman Jarosz <kedgedev@centrum.cz>

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

#ifndef _ICQUSERINFOWIDGET_H_
#define _ICQUSERINFOWIDGET_H_

#include <kpagedialog.h>
#include <icquserinfo.h>

class QStringListModel;

namespace Ui
{
	class ICQGeneralInfoWidget;
	class ICQHomeInfoWidget;
	class ICQWorkInfoWidget;
	class ICQOtherInfoWidget;
	class ICQInterestInfoWidget;
	class ICQOrgAffInfoWidget;
}
class ICQContact;

class ICQUserInfoWidget : public KPageDialog
{
Q_OBJECT
public:
	ICQUserInfoWidget( QWidget* parent = 0, bool editable = false );
	~ICQUserInfoWidget();
	void setContact( ICQContact* contact );
	
	QList<ICQInfoBase*> getInfoData() const;

public slots:
	void fillBasicInfo( const ICQGeneralUserInfo& );
	void fillWorkInfo( const ICQWorkUserInfo& );
	void fillEmailInfo( const ICQEmailInfo& );
	void fillNotesInfo( const ICQNotesInfo& );
	void fillMoreInfo( const ICQMoreUserInfo& );
	void fillInterestInfo( const ICQInterestInfo& );
	void fillOrgAffInfo( const ICQOrgAffInfo& info);

private slots:
	void slotUpdateDay();
	void slotUpdateAge();

	void slotOrg1CategoryChanged( int index );
	void slotOrg2CategoryChanged( int index );
	void slotOrg3CategoryChanged( int index );
	void slotAff1CategoryChanged( int index );
	void slotAff2CategoryChanged( int index );
	void slotAff3CategoryChanged( int index );
	
private:
	ICQGeneralUserInfo* storeBasicInfo() const;
	ICQMoreUserInfo* storeMoreInfo() const;
	ICQWorkUserInfo* storeWorkInfo() const;
	ICQOrgAffInfo* storeOrgAffInfo() const;

	QMap<QString, int> reverseMap( const QMap<int, QString>& map ) const;

	Ui::ICQGeneralInfoWidget* m_genInfoWidget;
	Ui::ICQWorkInfoWidget* m_workInfoWidget;
	Ui::ICQHomeInfoWidget* m_homeInfoWidget;
	Ui::ICQOtherInfoWidget* m_otherInfoWidget;
	Ui::ICQInterestInfoWidget * m_interestInfoWidget;
	Ui::ICQOrgAffInfoWidget* m_orgAffInfoWidget;
	ICQContact* m_contact;
	
	QStringListModel* m_emailModel;
	bool m_editable;

	ICQGeneralUserInfo m_generalUserInfo;
	ICQMoreUserInfo m_moreUserInfo;
	ICQWorkUserInfo m_workUserInfo;
	ICQOrgAffInfo m_orgAffUserInfo;
};

#endif

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
