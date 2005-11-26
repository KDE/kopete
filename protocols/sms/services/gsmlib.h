/*  *************************************************************************
    *   copyright: (C) 2005 Justin Huff <jjhuff@mspin.net>                  *
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

#ifndef GSMLIB_H_039562406
#define GSMLIB_H_039562406

#include "config.h"
#ifdef INCLUDE_SMSGSM

#include <unistd.h>

#include <gsmlib/gsm_unix_serial.h>
#include <gsmlib/gsm_sms.h>
#include <gsmlib/gsm_me_ta.h>
#include <gsmlib/gsm_util.h>
#include <gsmlib/gsm_event.h>

#include "smsservice.h"
#include "kopetemessage.h"

#include <qobject.h>
#include <qvaluelist.h>
#include <qstringlist.h>

class GSMLibPrefsUI;
class SMSContact;
class QListViewItem;
class KProcess;

class GSMLib : public SMSService, gsmlib::GsmEvent
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

//signals:
//	void messageSent(const Kopete::Message &);
protected:
    void timerEvent( QTimerEvent * );
    void SMSReception(gsmlib::SMSMessageRef newMessage, SMSMessageType messageType);
    void SMSReceptionIndication(std::string storeName, unsigned int index, SMSMessageType messageType);

private:
    QWidget* configureWidget(QWidget* parent);

    GSMLibPrefsUI* prefWidget;
    QStringList output;

    QString m_description;

    gsmlib::MeTa* m_MeTa;
    struct IncomingMessage
    {
        int Index;
        QString StoreName;
        gsmlib::SMSMessageRef Message;
        GsmEvent::SMSMessageType Type; 

        IncomingMessage() :   Index(-1)
        {}
    };

    typedef QValueList<IncomingMessage> MessageList;
    MessageList m_newMessages;
} ;
#endif
#endif //GSMLIB_H_039562406
