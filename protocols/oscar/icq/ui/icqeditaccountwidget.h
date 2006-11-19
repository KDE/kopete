/*
    icqeditaccountwidget.h - ICQ Account Widget

    Copyright (c) 2003 by Chris TenHarmsel  <tenharmsel@staticmethod.net>
    Copyright (c) 2004-2005 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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
namespace Ui {
class ICQEditAccountUI;
}
class OscarPrivacyEngine;

class ICQEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT
	
public:
	ICQEditAccountWidget(ICQProtocol *, Kopete::Account *,
	                     QWidget *parent=0);
	~ICQEditAccountWidget();
	
	virtual bool validateData();
	virtual Kopete::Account *apply();
	
private slots:
	void slotOpenRegister();
	void slotChangePassword();
	
protected:
	ICQAccount *mAccount;
	ICQProtocol *mProtocol;
	Ui::ICQEditAccountUI *mAccountSettings;
	
private:
	OscarPrivacyEngine* m_visibleEngine;
	OscarPrivacyEngine* m_invisibleEngine;
	OscarPrivacyEngine* m_ignoreEngine;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode csands; space-indent off; replace-tabs off;
