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
#include "config.h"
#ifdef INCLUDE_SMSGSM

#include <qcombobox.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kconfigbase.h>

#include <unistd.h>
#include <gsmlib/gsm_me_ta.h>
#include <gsmlib/gsm_sms.h>
#include <gsmlib/gsm_util.h>
#include <gsmlib/gsm_error.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"

#include "gsmlib.h"
#include "gsmlibprefs.h"
#include "smsprotocol.h"
#include "smscontact.h"

#include "kopete_unix_serial.h"

GSMLib::GSMLib(Kopete::Account* account)
	: SMSService(account)
{
	kdWarning( 14160 ) << k_funcinfo << endl;
	prefWidget = 0L;
	m_MeTa = NULL;
	
	loadConfig();
}

GSMLib::~GSMLib()
{
	disconnect();
}

void GSMLib::saveConfig()
{
	if( m_account == NULL )
	{
		KConfigGroup* c = m_account->configGroup();

		c->writeEntry(QString("%1:%2").arg("GSMLib").arg("Device"), m_device);
	}
}

void GSMLib::loadConfig()
{
	m_device = "/dev/bluetooth/rfcomm0";
	if( m_account == NULL )
	{
		QString temp;
		KConfigGroup* c = m_account->configGroup();
		
		temp = c->readEntry(QString("%1:%2").arg("GSMLib").arg("Device"), QString::null);
		if( temp != QString::null )
			m_device = temp;
	}
}

void GSMLib::connect()
{
			
	// open the port and ME/TA
	try
	{
		kdWarning( 14160 ) << "Connecting to: '"<<m_device<<"'"<<endl;
		
		gsmlib::Ref<gsmlib::Port> port = new gsmlib::KopeteUnixSerialPort(m_device.latin1(), 9600, gsmlib::DEFAULT_INIT_STRING, false);
		
		kdWarning( 14160 ) << "Port created"<<endl;
				
		m_MeTa = new gsmlib::MeTa(port);
		std::string dummy1, dummy2, receiveStoreName;
		m_MeTa->getSMSStore(dummy1, dummy2, receiveStoreName );
		m_MeTa->setSMSStore(receiveStoreName, 3);

		m_MeTa->setMessageService(1);

		// switch on SMS routing
		m_MeTa->setSMSRoutingToTA(true, false, false, true);

		m_MeTa->setEventHandler(this);
		killTimers();
		startTimer(1000);
		emit connected();
	}
	catch(gsmlib::GsmException &e)
	{
		kdWarning( 14160 ) << k_funcinfo<< e.what()<<endl;
		disconnect();
		return;
	}
}

void GSMLib::disconnect()
{
	killTimers();
	kdWarning( 14160 ) << k_funcinfo <<endl;
	delete m_MeTa;
	m_MeTa = NULL;

	emit disconnected();
}

void GSMLib::setWidgetContainer(QWidget* parent, QGridLayout* layout)
{
//	kdWarning( 14160 ) << k_funcinfo << "ml: " << layout << ", " << "mp: " << parent << endl;
	m_parent = parent;
	m_layout = layout;
	QWidget *configWidget = configureWidget(parent);
	layout->addMultiCellWidget(configWidget, 0, 1, 0, 1);
	configWidget->show();
}

void GSMLib::send(const Kopete::Message& msg)
{
//	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;
	
	QString reason;

	if (!m_account) return;
	if (!m_MeTa)
	{
		QString reason = QString("GSMLib: Not Connected");
		kdWarning( 14160 ) << k_funcinfo<< reason <<endl;
		emit messageNotSent(msg, reason);
		return;
	}

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();

	// send SMS
	try
	{
		gsmlib::Ref<gsmlib::SMSSubmitMessage> submitSMS = new gsmlib::SMSSubmitMessage();
		gsmlib::Address destAddr( nr.latin1() );
		submitSMS->setDestinationAddress(destAddr);
		kdWarning( 14160 ) << k_funcinfo << "before send"<<endl;
		m_MeTa->sendSMSs(submitSMS, message.latin1(), true);
		kdWarning( 14160 ) << k_funcinfo << "after send"<<endl;
		
		emit messageSent(msg);
	}
	catch(gsmlib::GsmException &e)
	{
		kdWarning( 14160 ) << k_funcinfo<< e.what()<<endl;
		QString reason = QString("GSMLib: ") + e.what();
		emit messageNotSent(msg, reason);
	}
}

QWidget* GSMLib::configureWidget(QWidget* parent)
{
//	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be ok if zero!!)" << endl;

	if (prefWidget == 0L)
		prefWidget = new GSMLibPrefsUI(parent);

	loadConfig();
	prefWidget->device->setURL(m_device);

	return prefWidget;
}

void GSMLib::savePreferences()
{
//	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be work if zero!!)" << endl;

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

void GSMLib::timerEvent( QTimerEvent * )
{
	if( m_MeTa == NULL )
		return;
	
	try
	{
		struct timeval timeoutVal;
		timeoutVal.tv_sec = 0;
		timeoutVal.tv_usec = 100;
		m_MeTa->waitEvent(&timeoutVal);
		
		MessageList::iterator it;
		for( it=m_newMessages.begin(); it!=m_newMessages.end(); it++)
		{
			kdWarning( 14160 ) << k_funcinfo <<endl;
			IncomingMessage m = *it;
			
			// Do we need to fetch it from the ME?
			if( m.Message.isnull() )
			{
				gsmlib::SMSStoreRef store = m_MeTa->getSMSStore(m.StoreName.latin1());
				store->setCaching(false);

				m.Message = (*store.getptr())[m.Index].message();
				store->erase(store->begin() + m.Index);
			}

			QString text = m.Message->userData().c_str();
			QString nr = m.Message->address().toString().c_str();
			kdWarning( 14160 ) << "Msg='"<<text <<"' addr='"<<nr<<"'"<<endl;
		
			// Locate a contact
			SMSContact* contact = static_cast<SMSContact*>( m_account->contacts().find( nr ));
			kdWarning( 14160 ) <<"contact="<<contact<<endl;
			if ( contact==NULL )
			{
				// No contact found, make a new one
				Kopete::MetaContact* metaContact = new Kopete::MetaContact ();
				metaContact->setTemporary ( true );
				contact = new SMSContact(m_account, nr, nr, metaContact );
				Kopete::ContactList::self ()->addMetaContact( metaContact );
			}
			
			// Deliver the msg
			Kopete::Message msg( contact, m_account->myself(), text, Kopete::Message::Inbound, Kopete::Message::RichText );
			kdWarning( 14160 ) <<"MARK1"<<endl;
			contact->manager(Kopete::Contact::CanCreate)->appendMessage( msg );
			kdWarning( 14160 ) <<"MARK2"<<endl;

		}
		m_newMessages.clear();
	}
	catch(gsmlib::GsmException &e)
	{
		kdWarning( 14160 ) << k_funcinfo<< e.what()<<endl;
		disconnect();
	}
}

void GSMLib::SMSReception(gsmlib::SMSMessageRef newMessage, SMSMessageType messageType)
{
	kdWarning( 14160 ) << k_funcinfo << "New Message" << endl;

	try
	{
		IncomingMessage m;
		m.Type = messageType;
		m.Message = newMessage;
		
		m_newMessages.push_back(m);
	}
	catch(gsmlib::GsmException &e)
	{
		kdWarning( 14160 ) << k_funcinfo<< e.what()<<endl;
	}
}

void GSMLib::SMSReceptionIndication(std::string storeName, unsigned int index, SMSMessageType messageType)
{
	kdWarning( 14160 ) << k_funcinfo << "New Message in store: "<<storeName.c_str() << endl;

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
		kdWarning( 14160 ) << k_funcinfo<< e.what()<<endl;
	}
}


const QString& GSMLib::description()
{
	QString url = "http://www.pxh.de/fs/gsmlib/";
	m_description = i18n("<qt>GSMLib is a library (and utilities) for sending SMS via a GSM device. The program can be found on <a href=\"%1\">%1</a></qt>").arg(url).arg(url);
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

