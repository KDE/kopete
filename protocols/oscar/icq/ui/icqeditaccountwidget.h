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
#include "editaccountwidget.h"
/**
 * @author Chris TenHarmsel <tenharmsel@staticmethod.net>
 */

class KopeteAccount;

class ICQProtocol;
class OscarEditAccountUI;
class ICQUserInfoWidget;
// class QTabWidget;
class KJanusWidget;

class ICQEditAccountWidget : public QWidget, public EditAccountWidget
{
	Q_OBJECT

	public:
		ICQEditAccountWidget(ICQProtocol *, KopeteAccount *,
			QWidget *parent=0, const char *name=0);
		virtual ~ICQEditAccountWidget();

		virtual bool validateData();
		virtual KopeteAccount *apply();

	private slots:
		void slotFetchInfo();
		void slotReadInfo();

	protected:
		/** Our account we're editing */
		KopeteAccount *mAccount;
		/** The Protocol we're in */
		ICQProtocol *mProtocol;
		/** The GUI */
		OscarEditAccountUI *mAccountSettings;
		ICQUserInfoWidget *mUserInfoSettings;
		KJanusWidget *mTop;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
