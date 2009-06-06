/*
  smssend.h  -  SMS Plugin

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

#ifndef SMSSEND_H
#define SMSSEND_H

#include <QList>


#include <klineedit.h>

#include "smsservice.h"

class SMSSendProvider;
class SMSSendPrefsUI;
class QLabel;

class SMSSend : public SMSService
{
	Q_OBJECT
public:
	SMSSend(Kopete::Account* account);
	~SMSSend();

	virtual void setAccount(Kopete::Account* account);

	void send(const Kopete::Message& msg);
	void setWidgetContainer(QWidget* parent, QGridLayout* container);

	int maxSize();
	const QString& description();

public slots:
	void savePreferences();

private slots:
	void setOptions(const QString& name);
	void loadProviders(const QString& prefix);
//signals:
//	void messageSent(const Kopete::Message&);

private:
	SMSSendProvider* m_provider;
	SMSSendPrefsUI* prefWidget;
	QList<KLineEdit*> args;
	QList<QLabel*> labels;
	QString m_description;
} ;

#endif //SMSSEND_H
