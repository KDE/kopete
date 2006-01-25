/*
    smpppdsearcher.h
 
    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include <qregexp.h>
#include <qfile.h>

#include <kprocess.h>
#include <kdebug.h>

#include "smpppdclient.h"
#include "smpppdsearcher.h"

SMPPPDSearcher::SMPPPDSearcher()
        : m_cancelSearchNow(FALSE),
        m_procIfconfig(NULL),
m_procNetstat(NULL) {}

SMPPPDSearcher::~SMPPPDSearcher() {
    delete m_procIfconfig;
    delete m_procNetstat;
}

/*!
    \fn SMPPPDSearcher::searchNetwork() const
 */
void SMPPPDSearcher::searchNetwork() {
    kdDebug(14312) << k_funcinfo << endl;

    // the first point to search is localhost
    if(!scan("127.0.0.1", "255.0.0.0")) {

        m_procNetstat  = new KProcess;
        m_procNetstat->setEnvironment("LANG", "C"); // we want to force english output

        *m_procNetstat << "/bin/netstat" << "-rn";
        connect(m_procNetstat, SIGNAL(receivedStdout(KProcess *,char *,int)), this, SLOT(slotStdoutReceivedNetstat(KProcess *,char *,int)));
        if(!m_procNetstat->start(KProcess::Block, KProcess::Stdout)) {
            kdDebug(14312) << k_funcinfo << "Couldn't execute /sbin/netstat -rn" << endl << "Perhaps the package net-tools isn't installed." << endl;

            emit smpppdNotFound();
        }

        delete m_procNetstat;
        m_procNetstat = NULL;
    }
}

/*!
    \fn SMPPPDSearcher::slotStdoutReceived(KProcess * proc, char * buf, int len)
 */
void SMPPPDSearcher::slotStdoutReceivedIfconfig(KProcess * /* proc */, char * buf, int len) {
    kdDebug(14312) << k_funcinfo << endl;

    QString myBuf = QString::fromLatin1(buf,len);
    QRegExp rex("^[ ]{10}.*inet addr:([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}).*Mask:([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
    // tokenize the string into lines
    QStringList toks = QStringList::split("\n", myBuf);
    for(QStringList::size_type i = 0; i < toks.count(); i++) {
        if(rex.exactMatch(toks[i])) {
            if(scan(rex.cap(1), rex.cap(2))) {
                return;
            }
        }
    }

    emit smpppdNotFound();
}
void SMPPPDSearcher::slotStdoutReceivedNetstat(KProcess * /* proc */, char * buf, int len) {
    kdDebug(14312) << k_funcinfo << endl;

    QRegExp rexGW(".*\\n0.0.0.0[ ]*([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}).*");
    QString myBuf = QString::fromLatin1(buf,len);

    if(!(rexGW.exactMatch(myBuf) && scan(rexGW.cap(1), "255.255.255.255"))) {
        // if netstat -r found no gateway we search the network
        m_procIfconfig = new KProcess;
        m_procIfconfig->setEnvironment("LANG", "C"); // we want to force english output

        *m_procIfconfig << "/sbin/ifconfig";
        connect(m_procIfconfig, SIGNAL(receivedStdout(KProcess *,char *,int)), this, SLOT(slotStdoutReceivedIfconfig(KProcess *,char *,int)));
        if(!m_procIfconfig->start(KProcess::Block, KProcess::Stdout)) {
            kdDebug(14312) << k_funcinfo << "Couldn't execute /sbin/ifconfig" << endl << "Perhaps the package net-tools isn't installed." << endl;

            emit smpppdNotFound();
        }

        delete m_procIfconfig;
        m_procIfconfig = NULL;
    }
}

/*!
    \fn SMPPPDSearcher::scan() const
 */
bool SMPPPDSearcher::scan(const QString& ip, const QString& mask) {
    kdDebug(14312) << k_funcinfo << "Scanning " << ip << "/" << mask << "..." << endl;
	
	SMPPPD::Client client;
	
	if(ip == "127.0.0.1") { // if localhost, we only scan this one host
		if(client.connect(ip, 3185)) {
			client.disconnect();
			emit smpppdFound(ip);
			return true;
		}
		
		return false;
	}

    uint min_range = 0;
    uint max_range = 255;

    // calculate ip range (only last mask entry)
    QRegExp lastRex("([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})");
    if(lastRex.exactMatch(ip)) {

        uint lastWordIP = lastRex.cap(4).toUInt();

        QStringList ipToks;
        for(int i = 1; i < 5; i++) {
            ipToks.push_back(lastRex.cap(i));
        }

        if(lastRex.exactMatch(mask)) {
            uint lastWordMask = lastRex.cap(4).toUInt();

            if(lastWordMask == 0) {
                kdDebug(14312) << k_funcinfo << "IP-Range: " << ipToks[0] << "." << ipToks[1] << "." <<  ipToks[2] << ".0 - " << ipToks[0] << "." << ipToks[1] << "." << ipToks[2] << ".255" << endl;
                max_range = 255;
            } else if(lastWordMask == 255) {
                min_range = max_range = lastWordIP;
            } else {
                kdDebug(14312) << k_funcinfo << "IP-Range: " << ipToks[0] << "." << ipToks[1] << "." <<  ipToks[2] << ".0 - " << ipToks[0] << "." << ipToks[1] << "." << ipToks[2] << "." << lastWordMask << endl;
                max_range = lastWordMask;
            }
        }

        uint range = max_range - min_range;
        m_cancelSearchNow = FALSE;
        if(range > 1) {
            emit scanStarted(max_range);
        }
        for(uint i = min_range; i <= max_range; i++) {
            if(m_cancelSearchNow) {
                if(range > 1) {
                    emit scanFinished();
                }
                break;
            }
            if(range > 1) {
                emit scanProgress(i);
            }
			
			if(client.connect(QString(ipToks[0] + "." + ipToks[1] + "." + ipToks[2] + "." + QString::number(i)), 3185)) {
				client.disconnect();
				emit smpppdFound(ip);
                if(range > 1) {
                    emit scanFinished();
                }
                return true;
			} 
#ifndef NDEBUG
			else {
				kdDebug(14312) << k_funcinfo << "No smpppd found at " << QString(ipToks[0] + "." + ipToks[1] + "." + ipToks[2] + "." + QString::number(i)) << endl;
			}
#endif
        }
        if(range > 1) {
            emit scanFinished();
        }
    }

    return false;
}

#include "smpppdsearcher.moc"
