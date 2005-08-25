/*
    smpppdcsplugin.cpp
 
    Copyright (c) 2002-2003 by Chris Howells         <howells@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "smpppdcsplugin.h"

#include <qtimer.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

#include <kdebug.h>
#include <kconfig.h>
#include <kextsock.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kgenericfactory.h>

#include <openssl/md5.h>

#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteaccountmanager.h"

Q3CString SMPPPDCSPlugin::m_kinternetApp = "";

typedef KGenericFactory<SMPPPDCSPlugin> SMPPPDCSPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kopete_smpppdcs, SMPPPDCSPluginFactory("kopete_smpppdcs"))

SMPPPDCSPlugin::SMPPPDCSPlugin(QObject *parent, const char * name, const QStringList& /* args */)
 : Kopete::Plugin(SMPPPDCSPluginFactory::instance(), parent, name), m_comState(READY), 
   m_sock(NULL), m_client(NULL), /* m_kinternetApp(""), */ m_timer(NULL), m_process(NULL) {
   
    /*if(useSmpppd()) {
        connectToSMPPPD();
    } */

    m_pluginConnected = false;

    // we wait for the allPluginsLoaded signal, to connect as early as possible after startup
    connect(Kopete::PluginManager::self(), SIGNAL(allPluginsLoaded()),
            this, SLOT(allPluginsLoaded()));

}

SMPPPDCSPlugin::~SMPPPDCSPlugin() {

    kdDebug( 0 ) << k_funcinfo << endl;

    if(m_sock) {
        m_sock->flush();
        m_sock->closeNow();
    }
	
    delete m_process;
    delete m_timer;
    delete m_sock;
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

void SMPPPDCSPlugin::slotCheckStatus() {
    if(useSmpppd()) {
        smpppdCheckStatus();
    } else {
        netstatCheckStatus();
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
	for(Q3PtrListIterator<Kopete::Account> it(m->accounts()); it.current(); ++it) {
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
	for(Q3PtrListIterator<Kopete::Account> it(m->accounts()); it.current(); ++it) {
		if(!list.contains(it.current()->protocol()->pluginId() + "_" + it.current()->accountId())) {
			it.current()->disconnect();
		}
	}
}

/*!
    \fn SMPPPDCSPlugin::readSMPPPD()
 */
QStringList SMPPPDCSPlugin::readSMPPPD() {
    QDataStream stream(m_sock);
    QStringList qsl;
    char s[1024];

    stream.readRawBytes(s, 1023);
    char *sp = s;

    for(int i = 0; i < 1024; i++) {
        if(s[i] == '\n') {
            s[i] = 0;
            qsl.push_back(sp);
            sp = &(s[i+1]);
        }
    }

    return qsl;
}

/*!
    \fn SMPPPDCSPlugin::writeSMPPPD(const QString& cmd)
 */
void SMPPPDCSPlugin::writeSMPPPD(const char * cmd) {
    QDataStream stream(m_sock);
    stream.writeRawBytes(cmd, strlen(cmd));
    stream.writeRawBytes("\n", strlen("\n"));
}

/*!
    \fn SMPPPDCSPlugin::connectToSMPPPD()
 */
void SMPPPDCSPlugin::connectToSMPPPD() {

    if(!m_sock ||
        m_sock->socketStatus() != KExtendedSocket::connected ||
        m_sock->socketStatus() != KExtendedSocket::connecting) {

        static KConfig *config = KGlobal::config();
        config->setGroup(SMPPPDCS_CONFIG_GROUP);
        unsigned int port = config->readUnsignedNumEntry("port", 3185);
        QString    server = config->readEntry("server", "localhost").utf8();

        delete m_sock;
        m_sock = NULL;
        m_comState = READY;
        m_sock = new KExtendedSocket(server, port, KExtendedSocket::inetSocket);

        kdDebug( 0 ) << k_funcinfo << "connect to smpppd \"" << server << ":" << port << "\"" << endl;

        switch(m_sock->connect()) {
        case  0:
            kdDebug( 0 ) << k_funcinfo << "connected to smpppd \"" << server << ":" << port << "\"" << endl;
            break;
        case -1:
            kdDebug( 0 ) << k_funcinfo << "system error" << endl;
            break;
        case -2:
            kdDebug( 0 ) << k_funcinfo << "this socket cannot connect(); this is a passiveSocket" << endl;
            break;
        case -3:
            kdDebug( 0 ) << k_funcinfo << "connection timed out" << endl;
            break;
        default:
            kdDebug( 0 ) << k_funcinfo << "unknown error" << endl;
            break;
        }
    }
}

void SMPPPDCSPlugin::slotProcessExited( KProcess *process ) {
    if ( process == m_process ) {
        delete m_process;
        m_process = 0L;
    }
}

void SMPPPDCSPlugin::slotProcessStdout( KProcess *, char *buffer, int buflen ) {
    // Look for a default gateway
    kdDebug( 0 ) << k_funcinfo << endl;
    QString qsBuffer = QString::fromLatin1( buffer, buflen );
    kdDebug( 0 ) << qsBuffer << endl;
    setConnectedStatus( qsBuffer.contains( "default" ) );
}

/*!
    \fn SMPPPDCSPlugin::smpppdCheckStatus()
 */
void SMPPPDCSPlugin::smpppdCheckStatus() {

	// get dcop client object and attach to it
	m_client = kapp->dcopClient();
	if(m_kinternetApp == "" && m_client && m_client->isAttached()) {
		// get all registered dcop apps and search for kinternet
		QCStringList apps = m_client->registeredApplications();
		QCStringList::iterator iter;
		for(iter = apps.begin(); iter != apps.end(); ++iter) {
			if((*iter).left(9) == "kinternet") {
				m_kinternetApp = *iter;
				break;
			}
		}
	}

    // we try to inquire an running kinternet
	if(m_kinternetApp != "" && m_client) {
		QByteArray data, replyData;
		Q3CString replyType;
		QDataStream arg(data, QIODevice::WriteOnly);
		
		kdDebug( 0 ) << k_funcinfo << "Start inquiring " << m_kinternetApp << " via DCOP" << endl;
		
		if(!m_client->call(m_kinternetApp, "KInternetIface", "isOnline()", data, replyType, replyData)) {
			kdDebug( 0 ) << k_funcinfo << "there was some error using DCOP." << endl;
		} else {
  			QDataStream reply(replyData, QIODevice::ReadOnly);
  			if(replyType == "bool") {
    			bool result;
    			reply >> result;
    			setConnectedStatus(result);
				return;
  			} else {
    			kdDebug( 0 ) << k_funcinfo << "isOnline() returned an unexpected type of reply!" << endl;
			}
		}
	}

    static KConfig *config = KGlobal::config();
    config->setGroup(SMPPPDCS_CONFIG_GROUP);
    QString pass = config->readEntry("Password", "").utf8();

    if(m_sock && 
	   m_sock->socketStatus() == KExtendedSocket::connected) {

        bool isConnected  = false;
        QString challenge = "";
        QRegExp ver("^SuSE Meta pppd \\(smpppd\\), Version (.*)$");

        while(m_comState != STATUSIFCFG &&
              m_comState != UNSETTLED) {
            switch(m_comState) {
            case READY: {

                    // the first string should be the version identifier of smpppd
                    QStringList stream = readSMPPPD();
                    QRegExp clg("^challenge = (.*)$");
                    if(ver.exactMatch(stream[0])) {
                        m_comState = SMPPPDSETTLED;
                        kdDebug( 0 ) << k_funcinfo << "Found smpppd Version " << ver.cap(1) << endl;
                    } else if(clg.exactMatch(stream[0])) {
                        kdDebug( 0 ) << k_funcinfo << "Authentication required: " << stream[0] << endl;
                        challenge  = clg.cap(1).stripWhiteSpace();
                        m_comState = CHALLENGED;
                    } else {
                        m_comState = UNSETTLED;
                        kdDebug( 0 ) << k_funcinfo << "anything but no smpppd answered" << endl;
                    }
                }
                break;
            case CHALLENGED: {
                    // write response to challenge
                    writeSMPPPD(QString("response = %1\n").arg(make_response(challenge, pass)).latin1());
                    // and then read the answer
                    QStringList stream = readSMPPPD();
                    kdDebug( 0 ) << k_funcinfo << "smpppd challenge ack: " << stream[0] << endl;
                    if(ver.exactMatch(stream[0])) {
                        m_comState = SMPPPDSETTLED;
                    } else  {
                        m_comState = UNSETTLED;
                    }
                }
                break;
            case SMPPPDSETTLED: {
                    // we want all ifcfgs
                    kdDebug( 0 ) << k_funcinfo << "smpppd req: list-ifcfgs" << endl;
                    writeSMPPPD("list-ifcfgs");
                    // and then the answer
                    QStringList stream = readSMPPPD();
                    kdDebug( 0 ) << k_funcinfo << "smpppd ack: " << stream[0] << endl;
                    if(stream[0].startsWith("ok")) {
                        // we have now a QStringList with all ifcfgs
                        // we extract them and put them in the global ifcfgs-list
                        // stream[1] tells us how many ifcfgs are coming next
                        QRegExp numIfcfgsRex("^BEGIN IFCFGS ([0-9]+).*");
                        if(numIfcfgsRex.exactMatch(stream[1])) {
                            int count_ifcfgs = numIfcfgsRex.cap(1).toInt();
                            kdDebug( 0 ) << k_funcinfo << "ifcfgs: " << count_ifcfgs << endl;

                            m_ifcfgs.clear();
                            for(int i = 0; i < count_ifcfgs; i++) {
                                QRegExp ifcfgRex("^i \"(ifcfg-[a-zA-Z]+[0-9]+)\".*");
                                if(ifcfgRex.exactMatch(stream[i+2])) {
                                    m_ifcfgs.push_back(ifcfgRex.cap(1));
                                }
                            }

                        } else {
                            kdDebug( 0 ) << k_funcinfo << "unexpected reply from smpppd" << endl;
                        }
                    } else {
                        kdDebug( 0 ) << k_funcinfo << "smpppd doesn't seem to understand me" << endl;
                    }
                    m_comState = LISTIFCFG;
                }
                break;
            case LISTIFCFG: {
                    for(unsigned int i = 0; i < m_ifcfgs.count(); i++) {
                        QString cmd = "list-status " + m_ifcfgs[i];
                        writeSMPPPD(cmd.latin1());
                        m_sock->waitForMore(0);
                        QStringList stream = readSMPPPD();
                        if(stream[0].startsWith("ok")) {
                            if(stream[2].startsWith("status connected")) {
                                isConnected = true;
                                break;
                            }
                        }
                    }

                    if(isConnected) {
                        kdDebug( 0 ) << k_funcinfo << "we are CONNECTED to the internet" << endl;
                    } else {
                        kdDebug( 0 ) << k_funcinfo << "we are DISCONNECTED from the internet" << endl;
                    }

                    m_comState = STATUSIFCFG;
                }
                break;
            default:
                break;
            }
        }

        // allow next dialog with smpppd
        if(m_comState != UNSETTLED) {
            m_comState = SMPPPDSETTLED;
            setConnectedStatus(isConnected);
        }

    } else {
        kdDebug( 0 ) << k_funcinfo << "not connected to smpppd => I try again" << endl;
        setConnectedStatus(false);
        connectToSMPPPD();
        slotCheckStatus();
    }
}

/*!
    \fn SMPPPDCSPlugin::netstatCheckStatus()
 */
void SMPPPDCSPlugin::netstatCheckStatus() {
    kdDebug( 0 ) << k_funcinfo << endl;

    if ( m_process ) {
        kdWarning( 0 ) << k_funcinfo << "Previous netstat process is still running!" << endl
        << "Not starting new netstat. Perhaps your system is under heavy load?" << endl;

        return;
    }

    // Use KProcess to run netstat -r. We'll then parse the output of
    // netstat -r in slotProcessStdout() to see if it mentions the
    // default gateway. If so, we're connected, if not, we're offline
    m_process = new KProcess;
    *m_process << "netstat" << "-r";

    connect( m_process, SIGNAL( receivedStdout( KProcess *, char *, int ) ), this, SLOT( slotProcessStdout( KProcess *, char *, int ) ) );
    connect( m_process, SIGNAL( processExited( KProcess * ) ), this, SLOT( slotProcessExited( KProcess * ) ) );

    if ( !m_process->start( KProcess::NotifyOnExit, KProcess::Stdout ) ) {
        kdWarning( 0 ) << k_funcinfo << "Unable to start netstat process!" << endl;

        delete m_process;
        m_process = 0L;
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

/*!
    \fn SMPPPDCSPlugin::make_response(const QString& chex) const
 */
QString SMPPPDCSPlugin::make_response(const QString& chex, const QString& password) const {

    int size = chex.length ();
    if (size & 1)
        return "error";
    size >>= 1;

    // convert challenge from hex to bin
    QString cbin;
    for (int i = 0; i < size; i++) {
        QString tmp = chex.mid (2 * i, 2);
        cbin.append ((char) strtol (tmp.ascii (), 0, 16));
    }

    // calculate response
    unsigned char rbin[MD5_DIGEST_LENGTH];
    MD5state_st md5;
    MD5_Init (&md5);
    MD5_Update (&md5, cbin.ascii (), size);
    MD5_Update (&md5, password.ascii(), password.length ());
    MD5_Final (rbin, &md5);

    // convert response from bin to hex
    QString rhex;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        char buffer[3];
        snprintf (buffer, 3, "%02x", rbin[i]);
        rhex.append (buffer);
    }

    return rhex;
}

#include "smpppdcsplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:
