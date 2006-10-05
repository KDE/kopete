/*
    AIMeditaccountwidget.h - AIM Account Widget

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


#ifndef AIMEDITACCOUNTWIDGET_H
#define AIMEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include "editaccountwidget.h"
/**
 * @author Chris TenHarmsel <tenharmsel@staticmethod.net>
 */

namespace Kopete
{
class Account;
}

class AIMAccount;
class AIMProtocol;
namespace Ui { class aimEditAccountUI; }
class OscarPrivacyEngine;

class AIMEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
Q_OBJECT

public:
	AIMEditAccountWidget(AIMProtocol *protocol, Kopete::Account *account,
	                     QWidget *parent=0);
	virtual ~AIMEditAccountWidget();
	
	virtual bool validateData();
	virtual Kopete::Account *apply();
	
private slots:
	void slotOpenRegister();
	
protected:
	AIMAccount *mAccount;
	AIMProtocol *mProtocol;
	Ui::aimEditAccountUI *mGui;

private:
	OscarPrivacyEngine* m_visibleEngine;
	OscarPrivacyEngine* m_invisibleEngine;
};
#endif
//kate: tab-width 4; indent-mode csands;
