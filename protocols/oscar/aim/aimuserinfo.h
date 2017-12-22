/*
    oscaruserinfo.h  -  Oscar Protocol Plugin

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

#ifndef AIMUSERINFO_H
#define AIMUSERINFO_H

#include <kdialog.h>

namespace Kopete { class Contact; }
namespace Ui { class AIMUserInfoWidget; }
class QTextBrowser;
class KTextEdit;
class AIMAccount;

class AIMUserInfoDialog : public KDialog
{
	Q_OBJECT
	public:
		AIMUserInfoDialog(Kopete::Contact *c, AIMAccount *acc, QWidget *parent = nullptr);
		~AIMUserInfoDialog();

	private:
		AIMAccount *mAccount;
		Kopete::Contact* m_contact;
		Ui::AIMUserInfoWidget *mMainWidget;
        QTextBrowser *userInfoView;
		KTextEdit *userInfoEdit;

	private Q_SLOTS:
		void slotSaveClicked();
		void slotCloseClicked();
		void slotUpdateClicked();
		void slotUpdateProfile();
		void slotUrlClicked(const QString&);
		void slotMailClicked(const QString&, const QString&);

	Q_SIGNALS:
//		void updateNickname(const QString &);
		void closing();
};

#endif

