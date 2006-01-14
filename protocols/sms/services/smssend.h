/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
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

#include <qobject.h>
#include <qmap.h>
#include <qlabel.h>

#include <klineedit.h>

#include "smsservice.h"

class SMSSendProvider;
class SMSSendPrefsUI;
class QListViewItem;
class QGridLayout;

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
	QGridLayout *settingsBoxLayout;
	SMSSendProvider* m_provider;
	SMSSendPrefsUI* prefWidget;
	QPtrList<KLineEdit> args;
	QPtrList<QLabel> labels;
	QString m_description;
} ;

#endif //SMSSEND_H
