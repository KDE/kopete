/*
  gsmlib.cpp  -  SMS Plugin

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

#include "gsmlib.h"
#ifdef INCLUDE_SMSGSM

#include <qcombobox.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qevent.h>
#include <qmutex.h>
#include <qthread.h>
#include <qcheckbox.h>
#include <QCustomEvent>
#include <Q3GridLayout>

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <k3process.h>
#include <kdebug.h>
#include <kconfigbase.h>

#include <unistd.h>
#include <gsmlib/gsm_me_ta.h>
#include <gsmlib/gsm_sms.h>
#include <gsmlib/gsm_util.h>
#include <gsmlib/gsm_error.h>
#include <kconfiggroup.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"

#include "gsmlibprefs.h"
#include "smsprotocol.h"
#include "smscontact.h"

#include "kopete_unix_serial.h"

/////////////////////////////////////////////////////////////////////
#define GSMLIB_EVENT_ID 245
GSMLibEvent::GSMLibEvent(SubType t) : QCustomEvent(QEvent::User+GSMLIB_EVENT_ID)
{
	setSubType(t);
}

GSMLibEvent::SubType GSMLibEvent::subType()
{
	return m_subType;
}

void GSMLibEvent::setSubType(GSMLibEvent::SubType t)
{
	m_subType = t;
}

/////////////////////////////////////////////////////////////////////
GSMLibThread::GSMLibThread(QString dev, GSMLib* parent)
{
	m_device = dev;
	m_parent = parent;
	m_run = true;
	m_MeTa = NULL;
}

GSMLibThread::~GSMLibThread()
{
	m_run = false;
}

void GSMLibThread::stop()
{
	m_run = false;
	kDebug( 14160 ) << "Waiting from GSMLibThread to die";
	if( wait(4000) == false )
		kWarning( 14160 ) << "GSMLibThread didn't exit!";
}
void GSMLibThread::run()
{
	if( doConnect() )
	{
		while( m_run )
		{
			pollForMessages();
			sendMessageQueue();
		}
	}

	delete m_MeTa;
	m_MeTa = NULL;
	QApplication::postEvent(m_parent, new GSMLibEvent(GSMLibEvent::DISCONNECTED));
	kDebug( 14160 ) << "GSMLibThread exited";
}

void GSMLibThread::send(const Kopete::Message& msg)
{
	if( m_MeTa )
	{
		m_outMessagesMutex.lock();
		m_outMessages.push_back(msg);
		m_outMessagesMutex.unlock();
	}
	else
	{
		GSMLibEvent* e = new GSMLibEvent( GSMLibEvent::MSG_NOT_SENT );
		e->Reason = QString("GSMLib: Not Connected");
		e->Message = msg;
		QApplication::postEvent(m_parent, e);
	}
}


bool GSMLibThread::doConnect()
{
	// open the port and ME/TA
	try
	{
		kDebug( 14160 ) << "Connecting to: '"<<m_device<<"'";

		gsmlib::Ref<gsmlib::Port> port = new gsmlib::KopeteUnixSerialPort(m_device.toLatin1(), 9600, gsmlib::DEFAULT_INIT_STRING, false);

		kDebug( 14160 ) << "Port created";

		m_MeTa = new gsmlib::MeTa(port);
		std::string dummy1, dummy2, receiveStoreName;
		m_MeTa->getSMSStore(dummy1, dummy2, receiveStoreName );
		m_MeTa->setSMSStore(receiveStoreName, 3);

		m_MeTa->setMessageService(1);

		// switch on SMS routing
		m_MeTa->setSMSRoutingToTA(true, false, false, true);

		m_MeTa->setEventHandler(this);
		QApplication::postEvent(m_parent, new GSMLibEvent(GSMLibEvent::CONNECTED));
		return true;
	}
	catch(gsmlib::GsmException &e)
	{
		kWarning( 14160 ) << e.what();
		m_run = false;
		return false;
	}
}

void GSMLibThread::SMSReception(gsmlib::SMSMessageRef newMessage, SMSMessageType messageType)
{
	try
	{
		IncomingMessage m;
		m.Type = messageType;
		m.Message = newMessage;

		m_newMessages.push_back(m);
	}
	catch(gsmlib::GsmException &e)
	{
		kWarning( 14160 ) << e.what();
		m_run = false;
	}
}

void GSMLibThread::SMSReceptionIndication(std::string storeName, unsigned int index, SMSMessageType messageType)
{
	kDebug( 14160 ) << "New Message in store: "<<storeName.c_str();

	try
	{
		if( messageType != gsmlib::GsmEvent::NormalSMS )
			return;

		IncomingMessage m;
		m.Index = index;
		m.StoreName = storeName.c_str();
		m.Type = messageType;
		m_newMessages.push_back(m);
	}
	catch(gsmlib::GsmException &e)
	{
		kWarning( 14160 ) << e.what();
		m_run = false;
	}
}

void GSMLibThread::pollForMessages( )
{
	if( m_MeTa == NULL )
		return;

	try
	{
		struct timeval timeoutVal;
		timeoutVal.tv_sec = 1;
		timeoutVal.tv_usec = 0;
		m_MeTa->waitEvent(&timeoutVal);

		MessageList::iterator it;
		for( it=m_newMessages.begin(); it!=m_newMessages.end(); it++)
		{
			IncomingMessage m = *it;

			// Do we need to fetch it from the ME?
			if( m.Message.isnull() )
			{
				gsmlib::SMSStoreRef store = m_MeTa->getSMSStore(m.StoreName.toLatin1());
				store->setCaching(false);

				m.Message = (*store.getptr())[m.Index].message();
				store->erase(store->begin() + m.Index);
			}

			GSMLibEvent* e = new GSMLibEvent( GSMLibEvent::NEW_MESSAGE );
			e->Text = m.Message->userData().c_str();
			e->Number = m.Message->address().toString().c_str();

			QApplication::postEvent(m_parent, e);

		}
		m_newMessages.clear();
	}
	catch(gsmlib::GsmException &e)
	{
		kWarning( 14160 ) << e.what();
		m_run = false;
	}
}

void GSMLibThread::sendMessageQueue()
{
	QMutexLocker _(&m_outMessagesMutex);

	if(m_outMessages.size() == 0 )
		return;

	KopeteMessageList::iterator it;
	for( it=m_outMessages.begin(); it!=m_outMessages.end(); it++)
		sendMessage(*it);
	m_outMessages.clear();
}

void GSMLibThread::sendMessage(const Kopete::Message& msg)
{
	QString reason;

	if (!m_MeTa)
	{
		GSMLibEvent* e = new GSMLibEvent( GSMLibEvent::MSG_NOT_SENT );
		e->Reason = QString("GSMLib: Not Connected");
		e->Message = msg;
		QApplication::postEvent(m_parent, e);
	}

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();

	// send SMS
	try
	{
		gsmlib::Ref<gsmlib::SMSSubmitMessage> submitSMS = new gsmlib::SMSSubmitMessage();
		gsmlib::Address destAddr( nr.toLatin1() );
		submitSMS->setDestinationAddress(destAddr);
		m_MeTa->sendSMSs(submitSMS, message.toLatin1(), true);

		GSMLibEvent* e = new GSMLibEvent( GSMLibEvent::MSG_SENT );
		e->Message = msg;
		QApplication::postEvent(m_parent, e);
	}
	catch(gsmlib::GsmException &e)
	{
		GSMLibEvent* ev = new GSMLibEvent( GSMLibEvent::MSG_NOT_SENT );
		ev->Reason = QString("GSMLib: ") + e.what();
		ev->Message = msg;
		QApplication::postEvent(m_parent, ev);
	}
}

/////////////////////////////////////////////////////////////////////

GSMLib::GSMLib(Kopete::Account* account)
	: SMSService(account)
{
	prefWidget = 0L;
	m_thread = NULL;

	loadConfig();
}

GSMLib::~GSMLib()
{
	disconnect();
}

void GSMLib::saveConfig()
{
	if( m_account != NULL )
	{
		KConfigGroup* c = m_account->configGroup();

		c->writeEntry(QString("%1:%2").arg("GSMLib").arg("Device"), m_device);
	}
}

void GSMLib::loadConfig()
{
	m_device = "/dev/bluetooth/rfcomm0";
	if( m_account != NULL )
	{
		QString temp;
		KConfigGroup* c = m_account->configGroup();

		temp = c->readEntry(QString("%1:%2").arg("GSMLib").arg("Device"), QString());
		if( !temp.isEmpty() )
			m_device = temp;
	}
}

void GSMLib::connect()
{

	m_thread = new GSMLibThread(m_device, this);
	m_thread->start();

}

void GSMLib::disconnect()
{
	kDebug( 14160 ) ;

	if( m_thread )
	{
		m_thread->stop();
		delete m_thread;
		m_thread = NULL;
		emit disconnected();
	}

}

void GSMLib::setWidgetContainer(QWidget* parent, Q3GridLayout* layout)
{
	m_parent = parent;
	m_layout = layout;
	QWidget *configWidget = configureWidget(parent);
	layout->addMultiCellWidget(configWidget, 0, 1, 0, 1);
	configWidget->show();
}

void GSMLib::send(const Kopete::Message& msg)
{
	m_thread->send(msg);
}

QWidget* GSMLib::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new GSMLibPrefsUI(parent);

	loadConfig();
	prefWidget->device->setUrl(m_device);

	return prefWidget;
}

void GSMLib::savePreferences()
{
	if( prefWidget )
	{
		m_device = prefWidget->device->url();
	}
	saveConfig();
}

int GSMLib::maxSize()
{
	return 160;
}

void GSMLib::customEvent(QCustomEvent* e)
{
	if( e->type() != QEvent::User+GSMLIB_EVENT_ID )
		return;

	if( m_account == NULL )
		return;

	GSMLibEvent* ge = (GSMLibEvent*)e;

	kDebug( 14160 ) << "Got event "<<ge->subType();
	switch( ge->subType() )
	{
		case GSMLibEvent::CONNECTED:
			emit connected();
			break;
		case GSMLibEvent::DISCONNECTED:
			disconnect();
			break;
		case GSMLibEvent::MSG_SENT:
			emit messageSent(ge->Message);
			break;
		case GSMLibEvent::MSG_NOT_SENT:
			emit messageNotSent(ge->Message, ge->Reason);
			break;
		case GSMLibEvent::NEW_MESSAGE:
		{
			QString nr = ge->Number;
			QString text = ge->Text;

			// Locate a contact
			SMSContact* contact = static_cast<SMSContact*>( m_account->contacts().value( nr ));
			if ( contact==NULL )
			{
				// No contact found, make a new one
				Kopete::MetaContact* metaContact = new Kopete::MetaContact ();
				metaContact->setTemporary ( true );
				contact = new SMSContact(m_account, nr, nr, metaContact );
				Kopete::ContactList::self ()->addMetaContact( metaContact );
				contact->setOnlineStatus( SMSProtocol::protocol()->SMSOnline );
			}

			// Deliver the msg
			Kopete::Message msg( contact, m_account->myself(), text, Kopete::Message::Inbound, Kopete::Message::RichText );
			contact->manager(Kopete::Contact::CanCreate)->appendMessage( msg );
			break;
		}
	}
}

const QString& GSMLib::description()
{
	QString url = "http://www.pxh.de/fs/gsmlib/";
	m_description = i18n("<qt>GSMLib is a library (and utilities) for sending SMS via a GSM device. The program can be found on <a href=\"%1\">%1</a></qt>", url);
	return m_description;
}

#include "gsmlib.moc"

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

