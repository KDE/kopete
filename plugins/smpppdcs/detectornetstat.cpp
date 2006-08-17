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
#include <kprocess.h>

#include "iconnector.h"
#include "detectornetstat.h"

DetectorNetstat::DetectorNetstat(IConnector* connector)
        : Detector(connector), m_buffer(QString::null), m_process(NULL) {}

DetectorNetstat::~DetectorNetstat() {
    delete m_process;
}

void DetectorNetstat::checkStatus() const {
    kdDebug(14312) << k_funcinfo << endl;

    if(m_process) {
        kdWarning(14312) << k_funcinfo << "Previous netstat process is still running!" << endl
        << "Not starting new netstat. Perhaps your system is under heavy load?" << endl;

        return;
    }

    m_buffer = QString::null;

    // Use KProcess to run netstat -r. We'll then parse the output of
    // netstat -r in slotProcessStdout() to see if it mentions the
    // default gateway. If so, we're connected, if not, we're offline
    m_process = new KProcess;
    *m_process << "netstat" << "-r";

    connect(m_process, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotProcessStdout( KProcess *, char *, int)));
    connect(m_process, SIGNAL(processExited(KProcess *)), this, SLOT(slotProcessExited(KProcess *)));

    if(!m_process->start(KProcess::NotifyOnExit, KProcess::Stdout)) {
        kdWarning(14312) << k_funcinfo << "Unable to start netstat process!" << endl;

        delete m_process;
        m_process = 0L;
    }
}

void DetectorNetstat::slotProcessStdout(KProcess *, char *buffer, int buflen) {
    // Look for a default gateway
    kdDebug(14312) << k_funcinfo << endl;
    m_buffer += QString::fromLatin1(buffer, buflen);
    kdDebug(14312) << m_buffer << endl;
}

void DetectorNetstat::slotProcessExited(KProcess *process) {
    kdDebug(14312) << k_funcinfo << m_buffer << endl;
    if(process == m_process) {
        m_connector->setConnectedStatus(m_buffer.contains("default"));
        m_buffer = QString::null;
        delete m_process;
        m_process = 0L;
    }
}

#include "detectornetstat.moc"
