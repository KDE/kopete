/*
    statisticsplugin.cpp

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@yahoo.fr>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qfile.h>
#include <qdict.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdeversion.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteview.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"
#include "kopetemessageevent.h"
#include "kopeteonlinestatus.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

#include "statisticscontact.h"
#include "statisticsdialog.h"
#include "statisticsplugin.h"
#include "statisticsdb.h"

typedef KGenericFactory<StatisticsPlugin> StatisticsPluginFactory;


static const KAboutData aboutdata("kopete_statistics", I18N_NOOP("Statistics") , "0.1" );
K_EXPORT_COMPONENT_FACTORY( kopete_statistics, StatisticsPluginFactory( &aboutdata )  )

StatisticsPlugin::StatisticsPlugin( QObject *parent, const char *name, const QStringList &)
	: Kopete::Plugin( StatisticsPluginFactory::instance(), parent, name ),
      DCOPObject("StatisticsDCOPIface")

{
	KAction *viewMetaContactStatistics = new KAction( i18n("View &Statistics" ),
		QString::fromLatin1( "log" ), 0, this, SLOT(slotViewStatistics()),
		actionCollection(), "viewMetaContactStatistics" );
	viewMetaContactStatistics->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count() == 1);

	connect(Kopete::ChatSessionManager::self(),SIGNAL(chatSessionCreated(Kopete::ChatSession*)),
				this, SLOT(slotViewCreated(Kopete::ChatSession*)));
	connect(Kopete::ChatSessionManager::self(),SIGNAL(aboutToReceive(Kopete::Message&)),
				this, SLOT(slotAboutToReceive(Kopete::Message&)));
		
	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)),
		viewMetaContactStatistics, SLOT(setEnabled(bool)));
	
	connect(Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account *)),
				this, SLOT(slotListenAccount(Kopete::Account *)));
	connect(Kopete::ContactList::self(), SIGNAL(metaContactAdded(Kopete::MetaContact*)),
			this, SLOT(slotMetaContactAdded(Kopete::MetaContact*)));	

	setXMLFile("statisticsui.rc");

	// Initializes the database
	m_db = new StatisticsDB();
	

	QPtrList<Kopete::MetaContact> list = Kopete::ContactList::self()->metaContacts();
	QPtrListIterator<Kopete::MetaContact> it( list );
	for (; it.current(); ++it)
	{
		slotMetaContactAdded(it.current());
	}
}	

StatisticsPlugin::~StatisticsPlugin()
{
	map<QString, StatisticsContact*>::iterator it;
	for (it = statisticsContactMap.begin(); it != statisticsContactMap.end(); ++it)
	{
		delete it->second;
		it->second = 0;
	}
}

void StatisticsPlugin::slotAboutToReceive(Kopete::Message& m)
{
	if (!m.from()->metaContact()->metaContactId().isEmpty())
		statisticsContactMap[m.from()->metaContact()->metaContactId()]->newMessageReceived(m);
}

void StatisticsPlugin::slotViewCreated(Kopete::ChatSession* session)
{
	connect(session, SIGNAL(closing(Kopete::ChatSession*)), this, SLOT(slotViewClosed(Kopete::ChatSession*)));
}

void StatisticsPlugin::slotViewClosed(Kopete::ChatSession* session)
{
	QPtrList<Kopete::Contact> list = session->members();
	QPtrListIterator<Kopete::Contact> it( list );
	
	for (; it.current(); ++it)
	{
		if (!it.current()->manager() && !it.current()->metaContact()->metaContactId().isEmpty()) // If this contact is not in other chat sessions
			statisticsContactMap[it.current()->metaContact()->metaContactId()]->setIsChatWindowOpen(false);
	}
}

void StatisticsPlugin::slotViewStatistics()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	
	kdDebug() << k_funcinfo << "statistics - dialog :"+ m->displayName() << endl;
	
	if (m && !m->metaContactId().isEmpty())
	{
		(new StatisticsDialog(statisticsContactMap[m->metaContactId()], db()))->show();
	}
}

void StatisticsPlugin::slotOnlineStatusChanged(Kopete::MetaContact *contact, Kopete::OnlineStatus::StatusType status)
{
	if (statisticsContactMap[contact->metaContactId()])
			statisticsContactMap[contact->metaContactId()]->onlineStatusChanged(status);
}

void StatisticsPlugin::slotMetaContactAdded(Kopete::MetaContact *mc)
{
	connect(mc, SIGNAL(onlineStatusChanged( Kopete::MetaContact *, Kopete::OnlineStatus::StatusType)), this, 		
					SLOT(slotOnlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)));
	
	if (!mc->metaContactId().isEmpty())
		statisticsContactMap[mc->metaContactId()] = new StatisticsContact(mc, db());
}

void StatisticsPlugin::dcopStatisticsDialog(QString id)
{
	kdDebug() << k_funcinfo << "statistics - DCOP dialog :" << id << endl;
	
	if (statisticsContactMap[id])
	{
		(new StatisticsDialog(statisticsContactMap[id], db()))->show();
	}	
}

bool StatisticsPlugin::dcopWasOnline(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dcopWasStatus(id, dt, Kopete::OnlineStatus::Online); 
}

bool StatisticsPlugin::dcopWasOnline(QString id, QString dateTime)
{
	return dcopWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Online);
}

bool StatisticsPlugin::dcopWasAway(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dcopWasStatus(id, dt, Kopete::OnlineStatus::Away); 
}

bool StatisticsPlugin::dcopWasAway(QString id, QString dateTime)
{
	return dcopWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Away);
}

bool StatisticsPlugin::dcopWasOffline(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dcopWasStatus(id, dt, Kopete::OnlineStatus::Offline); 
}

bool StatisticsPlugin::dcopWasOffline(QString id, QString dateTime)
{
	return dcopWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Offline);
}

bool StatisticsPlugin::dcopWasStatus(QString id, QDateTime dateTime, Kopete::OnlineStatus::StatusType status)
{
	kdDebug() << k_funcinfo << "statistics - DCOP wasOnline :" << id << endl;
	
	if (dateTime.isValid() && statisticsContactMap[id])
	{
		return statisticsContactMap[id]->wasStatus(dateTime, status);
	}
	
	return false;	
}

QString StatisticsPlugin::dcopStatus(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);
	return dcopStatus(id, dt.toString());

}

QString StatisticsPlugin::dcopStatus(QString id, QString dateTime)
{
	QDateTime dt = QDateTime::fromString(dateTime);
	
	if (dt.isValid() && statisticsContactMap[id])
	{
		return statisticsContactMap[id]->statusAt(dt);
	}
	
	return "";
}


#include "statisticsplugin.moc"
