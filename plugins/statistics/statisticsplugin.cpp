/*
    statisticsplugin.cpp

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>


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
#include <qtimer.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdeversion.h>

#include "kopetechatsessionmanager.h"
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
      : DCOPObject("StatisticsDCOPIface"), 
        Kopete::Plugin( StatisticsPluginFactory::instance(), parent, name )
      

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
	connect(Kopete::ContactList::self(), SIGNAL(metaContactAdded(Kopete::MetaContact*)),
			this, SLOT(slotMetaContactAdded(Kopete::MetaContact*)));	
	connect(Kopete::ContactList::self(), SIGNAL(metaContactRemoved(Kopete::MetaContact*)),
			this, SLOT(slotMetaContactRemoved(Kopete::MetaContact*)));	

	setXMLFile("statisticsui.rc");

	/* Initialization reads the database, so it could be a bit time-consuming
	due to disk access. This should overcome the problem and makes it non-blocking. */
	QTimer::singleShot(0, this, SLOT(slotInitialize()));
}	

void StatisticsPlugin::slotInitialize()
{
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
	QMap<Kopete::MetaContact*, StatisticsContact*>::Iterator it;
        for ( it = statisticsMetaContactMap.begin(); it != statisticsMetaContactMap.end(); ++it )
	{
		delete it.data();
	}
	delete m_db;
}

void StatisticsPlugin::slotAboutToReceive(Kopete::Message& m)
{
	if ( statisticsMetaContactMap.contains(m.from()->metaContact()) )
		statisticsMetaContactMap[m.from()->metaContact()]->newMessageReceived(m);
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
		// If this contact is not in other chat sessions
		if (!it.current()->manager() && statisticsMetaContactMap.contains(it.current()->metaContact()))
			statisticsMetaContactMap[it.current()->metaContact()]->setIsChatWindowOpen(false);
	}
}

void StatisticsPlugin::slotViewStatistics()
{
	Kopete::MetaContact *mc=Kopete::ContactList::self()->selectedMetaContacts().first();
	
	kdDebug() << k_funcinfo << "statistics - dialog :"+ mc->displayName() << endl;
	
	if ( mc && statisticsMetaContactMap.contains(mc) )
	{
		(new StatisticsDialog(statisticsMetaContactMap[mc], db()))->show();
	}
}

void StatisticsPlugin::slotOnlineStatusChanged(Kopete::MetaContact *mc, Kopete::OnlineStatus::StatusType status)
{
	if ( statisticsMetaContactMap.contains(mc) )
		statisticsMetaContactMap[mc]->onlineStatusChanged(status);
}

void StatisticsPlugin::slotMetaContactAdded(Kopete::MetaContact *mc)
{
	statisticsMetaContactMap[mc] = new StatisticsContact(mc, db());
	
	QPtrList<Kopete::Contact> clist = mc->contacts();
	Kopete::Contact *contact;
	
	// we need to call slotContactAdded if MetaContact allready have contacts
	for ( contact = clist.first(); contact; contact = clist.next() )
	{
		this->slotContactAdded(contact);
	}
	
	connect(mc, SIGNAL(onlineStatusChanged( Kopete::MetaContact *, Kopete::OnlineStatus::StatusType)), this,
					SLOT(slotOnlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)));
	connect(mc, SIGNAL(contactAdded( Kopete::Contact *)), this,
					SLOT(slotContactAdded( Kopete::Contact *)));
	connect(mc, SIGNAL(contactRemoved( Kopete::Contact *)), this,
					SLOT(slotContactRemoved( Kopete::Contact *)));
}

void StatisticsPlugin::slotMetaContactRemoved(Kopete::MetaContact *mc)
{
	if (statisticsMetaContactMap.contains(mc))
	{
		StatisticsContact *sc = statisticsMetaContactMap[mc];
		statisticsMetaContactMap.remove(mc);
		sc->removeFromDB();
		delete sc;
	}
}

void StatisticsPlugin::slotContactAdded( Kopete::Contact *c)
{
	if (statisticsMetaContactMap.contains(c->metaContact()))
	{
		StatisticsContact *sc = statisticsMetaContactMap[c->metaContact()];
		sc->contactAdded(c);
		statisticsContactMap[c->contactId()] = sc;
	}
}

void StatisticsPlugin::slotContactRemoved( Kopete::Contact *c)
{
	if (statisticsMetaContactMap.contains(c->metaContact()))
		statisticsMetaContactMap[c->metaContact()]->contactRemoved(c);
	
	statisticsContactMap.remove(c->contactId());
}

void StatisticsPlugin::dcopStatisticsDialog(QString id)
{
	kdDebug() << k_funcinfo << "statistics - DCOP dialog :" << id << endl;
	
	if (statisticsContactMap.contains(id))
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
	
	if (dateTime.isValid() && statisticsContactMap.contains(id))
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
	
	if (dt.isValid() && statisticsContactMap.contains(id))
	{
		return statisticsContactMap[id]->statusAt(dt);
	}
	
	return "";
}

QString StatisticsPlugin::dcopMainStatus(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);
	if (dt.isValid() && statisticsContactMap.contains(id))
	{
		return statisticsContactMap[id]->mainStatusDate(dt.date());
	}
	
	return "";
}
#include "statisticsplugin.moc"
