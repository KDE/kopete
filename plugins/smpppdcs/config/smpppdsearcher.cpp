/*
    smpppdsearcher.h
 
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

#include <qstringlist.h>
#include <qregexp.h>
#include <qfile.h>

#include <kextendedsocket.h>
#include <kprocess.h>
#include <kdebug.h>

#include "smpppdsearcher.h"

SMPPPDSearcher::SMPPPDSearcher()
        : m_procIfconfig(NULL),
        m_procNetstat(NULL),
m_sock(NULL) {

    m_sock = new KExtendedSocket();
}

SMPPPDSearcher::~SMPPPDSearcher() {
    delete m_sock;
    delete m_procIfconfig;
    delete m_procNetstat;
}

/*!
    \fn SMPPPDSearcher::searchNetwork() const
 */
void SMPPPDSearcher::searchNetwork() {
    // the first point to search is localhost
    if(!scan("127.0.0.1", "255.0.0.0")) {

        m_procNetstat  = new KProcess;
        m_procNetstat->setEnvironment("LANG", "C"); // we want to force english output

        *m_procNetstat << "/bin/netstat" << "-rn";
        connect(m_procNetstat, SIGNAL(receivedStdout(KProcess *,char *,int)), this, SLOT(slotStdoutReceivedNetstat(KProcess *,char *,int)));
        if(!m_procNetstat->start(KProcess::Block, KProcess::Stdout)) {
            kdDebug(0) << k_funcinfo << "Couldn't execute /sbin/netstat -rn" << endl << "Perhaps the package net-tools isn't installed." << endl;

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

    QRegExp rexGW(".*\\n0.0.0.0[ ]*([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}).*");
    QString myBuf = QString::fromLatin1(buf,len);

    if(!(rexGW.exactMatch(myBuf) && scan(rexGW.cap(1), "255.255.255.255"))) {
        // if netstat -r found no gateway we search the network
        m_procIfconfig = new KProcess;
        m_procIfconfig->setEnvironment("LANG", "C"); // we want to force english output

        *m_procIfconfig << "/sbin/ifconfig";
        connect(m_procIfconfig, SIGNAL(receivedStdout(KProcess *,char *,int)), this, SLOT(slotStdoutReceivedIfconfig(KProcess *,char *,int)));
        if(!m_procIfconfig->start(KProcess::Block, KProcess::Stdout)) {
            kdDebug(0) << k_funcinfo << "Couldn't execute /sbin/ifconfig" << endl << "Perhaps the package net-tools isn't installed." << endl;

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
    kdDebug(0) << k_funcinfo << "Scanning " << ip << "/" << mask << "..." << endl;

    if(ip == "127.0.0.1") { // localhost
        // we need only to check the existence of the socket file
        if(QFile::exists("/var/run/smpppd/control")) {
            emit smpppdFound("localhost");
            return true;
        }
    } else { // other interfaces

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
                    kdDebug(0) << k_funcinfo << "IP-Range: " << ipToks[0] << "." << ipToks[1] << "." <<  ipToks[2] << ".0 - " << ipToks[0] << "." << ipToks[1] << "." << ipToks[2] << ".255" << endl;
                    max_range = 255;
                } else if(lastWordMask == 255) {
                    min_range = max_range = lastWordIP;
                } else {
                    kdDebug(0) << k_funcinfo << "IP-Range: " << ipToks[0] << "." << ipToks[1] << "." <<  ipToks[2] << ".0 - " << ipToks[0] << "." << ipToks[1] << "." << ipToks[2] << "." << lastWordMask << endl;
                    max_range = lastWordMask;
                }
            }

            for(uint i = min_range; i <= max_range; i++) {
                if(scanIP(QString(ipToks[0] + "." + ipToks[1] + "." + ipToks[2] + "." + QString::number(i)))) {
                    return true;
                }
            }
        }
    }

    return false;
}

/*!
    \fn SMPPPDSearcher::scanIP(const QString& ip)
 */
bool SMPPPDSearcher::scanIP(const QString& ip) {
    kdDebug(0) << k_funcinfo << "Now scanning " << ip << "..." << endl;

    m_sock->reset();
    m_sock->setTimeout(0,500);
    m_sock->setAddress(ip,3185);

    if(!m_sock->connect()) {
        emit smpppdFound(ip);
        return true;
    }

    return false;
}

#include "smpppdsearcher.moc"
