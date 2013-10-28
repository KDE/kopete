/*
  gsmlib.h  -  SMS Plugin

  Copyright (c) 2005      by Justin Huff        <jjhuff@mspin.net>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef GSMLIB_H_039562406
#define GSMLIB_H_039562406

#include <Q3GridLayout>
#include <QCustomEvent>
#ifdef INCLUDE_SMSGSM

#include <unistd.h>

#include <gsmlib/gsm_unix_serial.h>
#include <gsmlib/gsm_sms.h>
#include <gsmlib/gsm_me_ta.h>
#include <gsmlib/gsm_util.h>
#include <gsmlib/gsm_event.h>

#include "smsservice.h"
#include "kopetemessage.h"

#include <QThread>
#include <QMutex>
#include <QList>
#include <QStringList>

class GSMLibPrefsUI;
class QGridLayout;
class GSMLibThread;

class GSMLib : public SMSService
{
    Q_OBJECT
public:
    GSMLib(Kopete::Account* account);
    ~GSMLib();

    void send(const Kopete::Message& msg);
    void setWidgetContainer(QWidget* parent, QGridLayout* container);

    int maxSize();
    const QString& description();

public slots:
    void savePreferences();
	virtual void connect();
	virtual void disconnect();

//signals:
//	void messageSent(const Kopete::Message &);
protected:
	virtual void customEvent(QCustomEvent* e);

    QWidget* configureWidget(QWidget* parent);
	void saveConfig();
	void loadConfig();

    GSMLibPrefsUI* prefWidget;
    QStringList output;

	QString m_device;

    QString m_description;

	GSMLibThread* m_thread;

} ;


/// Custom event for async-events
class GSMLibEvent : public QCustomEvent
{
public:
	enum SubType { CONNECTED, DISCONNECTED, NEW_MESSAGE, MSG_SENT, MSG_NOT_SENT };

	GSMLibEvent(SubType t);

	SubType subType();
	void setSubType(SubType t);

	QString Text;
	QString Number;

	QString Reason;

	Kopete::Message Message;
protected:
	SubType m_subType;
};

/// Thread to deal with GsmLib's blocking
class GSMLibThread : public QThread, gsmlib::GsmEvent
{
public:
	GSMLibThread(QString dev, GSMLib* parent);
	virtual ~GSMLibThread();

	virtual void run();
	void stop();
	void send(const Kopete::Message& msg);
protected:
	bool doConnect();
	void pollForMessages();
	void sendMessageQueue();
	void sendMessage(const Kopete::Message& msg);
    void SMSReception(gsmlib::SMSMessageRef newMessage, SMSMessageType messageType);
    void SMSReceptionIndication(std::string storeName, unsigned int index, SMSMessageType messageType);

	GSMLib* m_parent;
	QString m_device;

    gsmlib::MeTa* m_MeTa;

	bool m_run;

    struct IncomingMessage
    {
        int Index;
        QString StoreName;
        gsmlib::SMSMessageRef Message;
        GsmEvent::SMSMessageType Type;

        IncomingMessage() :   Index(-1)
        {}
    };

    typedef QList<IncomingMessage> MessageList;
    MessageList m_newMessages;

	typedef QList<Kopete::Message> KopeteMessageList;
	KopeteMessageList m_outMessages;
	QMutex m_outMessagesMutex;
};

#endif
#endif //GSMLIB_H_039562406
