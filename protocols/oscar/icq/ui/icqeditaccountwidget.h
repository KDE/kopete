/*
    icqeditaccountwidget.h - ICQ Account Widget

    Copyright (c) 2003 by Chris TenHarmsel  <tenharmsel@staticmethod.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQEDITACCOUNTWIDGET_H
#define ICQEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include <qdatetime.h>
#include "editaccountwidget.h"

namespace Kopete { class Account; }

class ICQAccount;
class ICQProtocol;
class ICQEditAccountUI;
class ICQUserInfoWidget;
class KJanusWidget;

class ICQEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
	Q_OBJECT

	public:
		ICQEditAccountWidget(ICQProtocol *, Kopete::Account *,
			QWidget *parent=0, const char *name=0);
//		virtual ~ICQEditAccountWidget();

		virtual bool validateData();
		virtual Kopete::Account *apply();

	private slots:
		void slotFetchInfo();
		void slotReadInfo();
		void slotSend();
		void slotModified();
		void slotRecalcAge(QDate);
		void slotCategory1Changed(int);
		void slotCategory2Changed(int);
		void slotCategory3Changed(int);
		void slotCategory4Changed(int);
		void slotOrganisation1Changed(int);
		void slotOrganisation2Changed(int);
		void slotOrganisation3Changed(int);
		void slotAffiliation1Changed(int);
		void slotAffiliation2Changed(int);
		void slotAffiliation3Changed(int);
		void slotOpenRegister();

	protected:
		ICQAccount *mAccount;
		ICQProtocol *mProtocol;

		/*
		 * GUI parts
		 */
		ICQEditAccountUI *mAccountSettings;
		ICQUserInfoWidget *mUserInfoSettings;
		KJanusWidget *mTop;
		bool mModified;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
