/*
  smseditaccountwidget.h  -  SMS Plugin Edit Account Widget

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef SMSEDITACCOUNTWIDGET_H
#define SMSEDITACCOUNTWIDGET_H

#include <QWidget>
#include "editaccountwidget.h"

class SMSProtocol;
class SMSService;
class smsActPrefsUI;
namespace Kopete { class Account; }
class QGridLayout;

class SMSEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
	Q_OBJECT
public:
	SMSEditAccountWidget(SMSProtocol *protocol, Kopete::Account *theAccount, QWidget *parent = 0);
	~SMSEditAccountWidget();

	bool validateData();
	Kopete::Account* apply();
public slots:
	void setServicePreferences(const QString& serviceName);
	void showDescription();
protected:
	smsActPrefsUI *preferencesDialog;
	QWidget *configWidget;
	SMSService *service;
	SMSProtocol *m_protocol;
	QGridLayout *middleFrameLayout;

signals:
	void saved();
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

