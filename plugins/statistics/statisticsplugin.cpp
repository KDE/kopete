/*
    statisticsplugin.cpp

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>

   Copyright (c) 2007      by the Kopete Developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <math.h>

#include <qfile.h>
//Added by qt3to4:
#include <QList>
#include <qtimer.h>

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kactioncollection.h>
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
#include "kopetecontact.h"

#include "statisticscontact.h"
#include "statisticsdialog.h"
#include "statisticsplugin.h"
#include "statisticsadaptor.h"
#include "statisticsdb.h"

K_PLUGIN_FACTORY(StatisticsPluginFactory, registerPlugin<StatisticsPlugin>();)
K_EXPORT_PLUGIN(StatisticsPluginFactory( "kopete_statistics" ))

StatisticsPlugin::StatisticsPlugin( QObject *parent, const QVariantList &/*args*/ )
	: Kopete::Plugin( StatisticsPluginFactory::componentData(), parent )
{

	KAction *viewMetaContactStatistics = new KAction( KIcon("view-statistics"), i18n("View &Statistics" ),
		this );
	actionCollection()->addAction ( "viewMetaContactStatistics", viewMetaContactStatistics );
	connect(viewMetaContactStatistics, SIGNAL(triggered(bool)), this, SLOT(slotViewStatistics()));
	viewMetaContactStatistics->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count() == 1);

	connect(Kopete::ChatSessionManager::self(),SIGNAL(chatSessionCreated(Kopete::ChatSession*)),
				this, SLOT(slotViewCreated(Kopete::ChatSession*)));
	connect(Kopete::ChatSessionManager::self(),SIGNAL(aboutToReceive(Kopete::Message&)),
				this, SLOT(slotAboutToReceive(Kopete::Message&)));
		
	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)),
		viewMetaContactStatistics, SLOT(setEnabled(bool)));
	connect(Kopete::ContactList::self(), SIGNAL(metaContactAdded(Kopete::MetaContact*)),
			this, SLOT(slotMetaContactAdded(Kopete::MetaContact*)));	

	setXMLFile("statisticsui.rc");

	/* Initialization reads the database, so it could be a bit time-consuming
	due to disk access. This should overcome the problem and makes it non-blocking. */
	QTimer::singleShot(0, this, SLOT(slotInitialize()));
	
	new StatisticsAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/Statistics", this);
}	

void StatisticsPlugin::slotInitialize()
{
	// Initializes the database
	m_db = new StatisticsDB();
	
	QList<Kopete::MetaContact*> list = Kopete::ContactList::self()->metaContacts();
	foreach(Kopete::MetaContact *metaContact, list)
	{
		slotMetaContactAdded(metaContact);
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
	if (!m.from()->metaContact()->metaContactId().isEmpty() && statisticsContactMap[m.from()->metaContact()->metaContactId()])
		statisticsContactMap[m.from()->metaContact()->metaContactId()]->newMessageReceived(m);
}

void StatisticsPlugin::slotViewCreated(Kopete::ChatSession* session)
{
	connect(session, SIGNAL(closing(Kopete::ChatSession*)), this, SLOT(slotViewClosed(Kopete::ChatSession*)));
}

void StatisticsPlugin::slotViewClosed(Kopete::ChatSession* session)
{
	QList<Kopete::Contact*> list = session->members();
	foreach(Kopete::Contact *contact, list)
	{
		// If this contact is not in other chat sessions
		if (!contact->manager() && !contact->metaContact()->metaContactId().isEmpty()
				   && statisticsContactMap[contact->metaContact()->metaContactId()])
		statisticsContactMap[contact->metaContact()->metaContactId()]->setIsChatWindowOpen(false);
	}
}

void StatisticsPlugin::slotViewStatistics()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	
	kDebug(14315) << "statistics - dialog: " + m->displayName();
	
	if (m && !m->metaContactId().isEmpty())
	{
		StatisticsDialog* dialog = new StatisticsDialog(statisticsContactMap[m->metaContactId()], db());
		dialog->setObjectName( QLatin1String( "StatisticsDialog" ) );
		dialog->show();
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

void StatisticsPlugin::dbusStatisticsDialog(QString id)
{
	kDebug(14315) << "statistics - DBus dialog :" << id;
	
	if (statisticsContactMap[id])
	{
		StatisticsDialog* dialog = new StatisticsDialog(statisticsContactMap[id], db());
		dialog->setObjectName( QLatin1String("StatisticsDialog") );
		dialog->show();
	}	
}

bool StatisticsPlugin::dbusWasOnline(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dbusWasStatus(id, dt, Kopete::OnlineStatus::Online); 
}

bool StatisticsPlugin::dbusWasOnline(QString id, QString dateTime)
{
	return dbusWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Online);
}

bool StatisticsPlugin::dbusWasAway(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dbusWasStatus(id, dt, Kopete::OnlineStatus::Away); 
}

bool StatisticsPlugin::dbusWasAway(QString id, QString dateTime)
{
	return dbusWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Away);
}

bool StatisticsPlugin::dbusWasOffline(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);	
	return dbusWasStatus(id, dt, Kopete::OnlineStatus::Offline); 
}

bool StatisticsPlugin::dbusWasOffline(QString id, QString dateTime)
{
	return dbusWasStatus(id, QDateTime::fromString(dateTime), Kopete::OnlineStatus::Offline);
}

bool StatisticsPlugin::dbusWasStatus(QString id, QDateTime dateTime, Kopete::OnlineStatus::StatusType status)
{
	kDebug(14315) << "statistics - DBus wasOnline :" << id;
	
	if (dateTime.isValid() && statisticsContactMap[id])
	{
		return statisticsContactMap[id]->wasStatus(dateTime, status);
	}
	
	return false;	
}

QString StatisticsPlugin::dbusStatus(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);
	return dbusStatus(id, dt.toString());

}

QString StatisticsPlugin::dbusStatus(QString id, QString dateTime)
{
	QDateTime dt = QDateTime::fromString(dateTime);
	
	if (dt.isValid() && statisticsContactMap[id])
	{
		return statisticsContactMap[id]->statusAt(dt);
	}
	
	return "";
}

QString StatisticsPlugin::dbusMainStatus(QString id, int timeStamp)
{
	QDateTime dt;
	dt.setTime_t(timeStamp);
	if (dt.isValid() && statisticsContactMap[id])
	{
		return statisticsContactMap[id]->mainStatusDate(dt.date());
	}
	
	return "";
}
#include "statisticsplugin.moc"
