/*
  smsservice.h  -  SMS Plugin User Service

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

#ifndef SMSSERVICE_H
#define SMSSERVICE_H


#include <QObject>

#include "kopetemessage.h"

namespace Kopete { class Account; }
class QGridLayout;
class QWidget;

class SMSService : public QObject
{
	Q_OBJECT
public:
	SMSService(Kopete::Account* account = 0);
	virtual ~SMSService();

	/**
	 * Reimplement to do extra stuff when the account is dynamically changed
	 * (other than just changing m_account).
	 *
	 * Don't forget to call SMSService::setAccount(...) after you've finished.
	 */
	virtual void setAccount(Kopete::Account* account);

	/**
	 * Called when the settings widget has a place to be. @param parent is the
	 * settings widget's parent and @param layout is the 2xn grid layout it may
	 * use.
	 */
	virtual void setWidgetContainer(QWidget* parent, QGridLayout* layout) = 0;

	virtual void send(const Kopete::Message& msg) = 0;
	virtual int maxSize() = 0;
	virtual const QString& description() = 0;

public slots:
	virtual void savePreferences() = 0;
	virtual void connect();
	virtual void disconnect();

signals:
	void messageSent(const Kopete::Message &);
	void messageNotSent(const Kopete::Message &, const QString &);
	void connected();
	void disconnected();

protected:
	Kopete::Account* m_account;
	QGridLayout* m_layout;
	QWidget* m_parent;
};

#endif //SMSSERVICE_H
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

