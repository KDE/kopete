/*
    smpppdcsprefsimpl.cpp
 
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

#include <qgroupbox.h>
#include <qradiobutton.h>
 
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kpushbutton.h>
#include <kdebug.h>
 
#include "smpppdlocationwidget.h"
#include "smpppdcsprefsimpl.h"
#include "smpppdsearcher.h"

SMPPPDCSPrefs::SMPPPDCSPrefs(QWidget* parent, const char* name, WFlags fl)
        : SMPPPDCSPrefsBase(parent, name, fl) {

    // signals and slots connections
    connect(useNetstat, SIGNAL(toggled(bool)), this, SLOT(disableSMPPPDSettings()));
    connect(useSmpppd, SIGNAL(toggled(bool)), this, SLOT(enableSMPPPDSettings()));
    connect(autoCSTest, SIGNAL(clicked()), this, SLOT(determineCSType()));

	// if netstat is NOT available, disable the option and set to SMPPPD
	if(KStandardDirs::findExe("netstat") == QString::null) {
		autoCSTest->setEnabled(FALSE);
		useNetstat->setEnabled(FALSE);
		useNetstat->setChecked(FALSE);
		useSmpppd->setChecked(TRUE);
	}
}

SMPPPDCSPrefs::~SMPPPDCSPrefs() {}

void SMPPPDCSPrefs::enableSMPPPDSettings() {
    smpppdPrefs->setEnabled(true);
}

void SMPPPDCSPrefs::disableSMPPPDSettings() {
    smpppdPrefs->setEnabled(false);
}

void SMPPPDCSPrefs::determineCSType() {

    // while we search, we'll disable the button
    autoCSTest->setEnabled(false);
    qApp->processEvents();

    /* broadcast network for a smpppd.
       If one is available set to smpppd method */

    SMPPPDSearcher searcher;
    connect(&searcher, SIGNAL(smpppdFound(const QString&)), this, SLOT(smpppdFound(const QString&)));
    connect(&searcher, SIGNAL(smpppdNotFound()), this, SLOT(smpppdNotFound()));
    searcher.searchNetwork();
}

void SMPPPDCSPrefs::smpppdFound(const QString& host) {
    kdDebug(14312) << k_funcinfo << endl;
    SMPPPDLocation->setServer(host);
    useSmpppd->setChecked(true);
    autoCSTest->setEnabled(true);
}

void SMPPPDCSPrefs::smpppdNotFound() {
    kdDebug(14312) << k_funcinfo << endl;
    useNetstat->setChecked(true);
    autoCSTest->setEnabled(true);
}

#include "smpppdcsprefsimpl.moc"
