/*
    detectornetstat.cpp
 
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

#include <kdebug.h>
#include <k3process.h>

#include "iconnector.h"
#include "detectornetstat.h"

DetectorNetstat::DetectorNetstat(IConnector* connector)
        : Detector(connector), m_buffer(QString()), m_process(NULL) {}

DetectorNetstat::~DetectorNetstat() {
    delete m_process;
}

void DetectorNetstat::checkStatus() const {
    kDebug(14312) ;

    if(m_process) {
        kWarning(14312) << "Previous netstat process is still running!" << endl
        << "Not starting new netstat. Perhaps your system is under heavy load?" << endl;

        return;
    }

    m_buffer.clear();

    // Use K3Process to run netstat -r. We'll then parse the output of
    // netstat -r in slotProcessStdout() to see if it mentions the
    // default gateway. If so, we're connected, if not, we're offline
    m_process = new K3Process;
    *m_process << "netstat" << "-r";

    connect(m_process, SIGNAL(receivedStdout(K3Process *, char *, int)), this, SLOT(slotProcessStdout( K3Process *, char *, int)));
    connect(m_process, SIGNAL(processExited(K3Process *)), this, SLOT(slotProcessExited(K3Process *)));

    if(!m_process->start(K3Process::NotifyOnExit, K3Process::Stdout)) {
        kWarning(14312) << "Unable to start netstat process!";

        delete m_process;
        m_process = 0L;
    }
}

void DetectorNetstat::slotProcessStdout(K3Process *, char *buffer, int buflen) {
    // Look for a default gateway
    kDebug(14312) ;
    m_buffer += QString::fromLatin1(buffer, buflen);
    kDebug(14312) << m_buffer;
}

void DetectorNetstat::slotProcessExited(K3Process *process) {
    kDebug(14312) << m_buffer;
    if(process == m_process) {
        m_connector->setConnectedStatus(m_buffer.contains("default"));
        m_buffer.clear();
        delete m_process;
        m_process = 0L;
    }
}

#include "detectornetstat.moc"
