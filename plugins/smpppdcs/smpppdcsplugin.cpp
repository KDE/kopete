/*
    smpppdcsplugin.cpp
 
    Copyright (c) 2002-2003 by Chris Howells         <howells@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004-2005 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "onlineinquiry.h"
#include "smpppdcsplugin.h"

#include <qtimer.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kgenericfactory.h>

#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteaccountmanager.h"

typedef KGenericFactory<SMPPPDCSPlugin> SMPPPDCSPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kopete_smpppdcs, SMPPPDCSPluginFactory("kopete_smpppdcs"))

SMPPPDCSPlugin::SMPPPDCSPlugin(QObject *parent, const char * name, const QStringList& /* args */)
 : DCOPObject("SMPPPDCSIface"), Kopete::Plugin(SMPPPDCSPluginFactory::instance(), parent, name),
   m_detector(NULL), m_timer(NULL), m_onlineInquiry(NULL)
    {
   
    /*if(useSmpppd()) {
        connectToSMPPPD();
    } */

    m_pluginConnected = false;

    // we wait for the allPluginsLoaded signal, to connect as early as possible after startup
    connect(Kopete::PluginManager::self(), SIGNAL(allPluginsLoaded()),
            this, SLOT(allPluginsLoaded()));

    m_onlineInquiry = new OnlineInquiry();
    m_detector      = new Detector(this);
    connect(m_detector, SIGNAL(retryRequested()), this, SLOT(slotCheckStatus()));
}

SMPPPDCSPlugin::~SMPPPDCSPlugin() {

    kdDebug( 0 ) << k_funcinfo << endl;

    disconnect(m_detector, SIGNAL(retryRequested()), this, SLOT(slotCheckStatus()));

    delete m_timer;
    delete m_detector;
    delete m_onlineInquiry;
}

void SMPPPDCSPlugin::allPluginsLoaded() {

    m_timer = new QTimer();
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( slotCheckStatus() ) );

    if(useSmpppd()) {
        m_timer->start(30000);
    } else {
        // we use 1 min interval, because it reflects the old connectionstatus plugin behaviour
        m_timer->start(60000);
    }

    slotCheckStatus();
}

bool SMPPPDCSPlugin::isOnline() {
	return m_onlineInquiry->isOnline(useSmpppd());
}

void SMPPPDCSPlugin::slotCheckStatus() {
    if(useSmpppd()) {
        m_detector->smpppdCheckStatus();
    } else {
        m_detector->netstatCheckStatus();
    }
}

void SMPPPDCSPlugin::setConnectedStatus( bool connected ) {
    kdDebug( 0 ) << k_funcinfo << endl;

    // We have to handle a few cases here. First is the machine is connected, and the plugin thinks
    // we're connected. Then we don't do anything. Next, we can have machine connected, but plugin thinks
    // we're disconnected. Also, machine disconnected, plugin disconnected -- we
    // don't do anything. Finally, we can have the machine disconnected, and the plugin thinks we're
    // connected. This mechanism is required so that we don't keep calling the connect/disconnect functions
    // constantly.

    if ( connected && !m_pluginConnected ) {
        // The machine is connected and plugin thinks we're disconnected
        kdDebug( 0 ) << k_funcinfo << "Setting m_pluginConnected to true" << endl;
        m_pluginConnected = true;
        //Kopete::AccountManager::self()->connectAll(true);
		connectAllowed();
        kdDebug( 0 ) << k_funcinfo << "We're connected" << endl;
    } else if ( !connected && m_pluginConnected ) {
        // The machine isn't connected and plugin thinks we're connected
        kdDebug( 0 ) << k_funcinfo << "Setting m_pluginConnected to false" << endl;
        m_pluginConnected = false;
        //Kopete::AccountManager::self()->disconnectAll();
	disconnectAllowed();
        kdDebug( 0 ) << k_funcinfo << "We're offline" << endl;
    }
}

void SMPPPDCSPlugin::connectAllowed()
{
	static KConfig *config = KGlobal::config();
	config->setGroup(SMPPPDCS_CONFIG_GROUP);
	QStringList list = config->readListEntry("ignoredAccounts");
	
	Kopete::AccountManager * m = Kopete::AccountManager::self();
	for(QPtrListIterator<Kopete::Account> it(m->accounts()); it.current(); ++it) {
		if(!list.contains(it.current()->protocol()->pluginId() + "_" + it.current()->accountId())) {
			it.current()->connect();
		}
	}
}

void SMPPPDCSPlugin::disconnectAllowed()
{
	static KConfig *config = KGlobal::config();
    config->setGroup(SMPPPDCS_CONFIG_GROUP);
	QStringList list = config->readListEntry("ignoredAccounts");
	
	Kopete::AccountManager * m = Kopete::AccountManager::self();
	for(QPtrListIterator<Kopete::Account> it(m->accounts()); it.current(); ++it) {
		if(!list.contains(it.current()->protocol()->pluginId() + "_" + it.current()->accountId())) {
			it.current()->disconnect();
		}
	}
}

/*!
    \fn SMPPPDCSPlugin::useSmpppd() const
 */
bool SMPPPDCSPlugin::useSmpppd() const {
    static KConfig *config = KGlobal::config();
    config->setGroup(SMPPPDCS_CONFIG_GROUP);
    return config->readBoolEntry("useSmpppd", false);
}

QString SMPPPDCSPlugin::detectionMethod() const {
    if(useSmpppd()) {
        return "smpppd";
    } else {
        return "netstat";
    }
}

#include "smpppdcsplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
