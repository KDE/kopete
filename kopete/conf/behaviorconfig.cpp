/*
    behaviorconfig.cpp  -  Kopete Look Feel Config

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "behaviorconfig.h"

#include <qcheckbox.h>
//#include <qdir.h>
#include <qlayout.h>
//#include <qhbuttongroup.h>
#include <qspinbox.h>
//#include <qslider.h>

#include <kdebug.h>
#include <klocale.h>
/*
#include <klineedit.h>
#include <kcolorcombo.h>
#include <kcolorbutton.h>
#include <klineeditdlg.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kmessagebox.h>
#include <knotifydialog.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kurlrequesterdlg.h>
#include <kpushbutton.h>
#include <kfontdialog.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kiconview.h>
*/
#include "kopeteprefs.h"
#include "kopeteaway.h"
#include "kopeteawayconfigui.h"

#include <qtabwidget.h>

BehaviorConfig::BehaviorConfig(QWidget * parent) :
	ConfigModule (
		i18n("Behavior"),
		i18n("Here You Can Personalize Kopete"),
		"appearance",
		parent )
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	mBehaviorTabCtl = new QTabWidget(this, "mBehaviorTabCtl");

	// "Away" TAB ========================================================
	mAwayConfigUI = new KopeteAwayConfigUI(mBehaviorTabCtl);
	mBehaviorTabCtl->addTab(mAwayConfigUI, i18n("&Away Settings"));
}

void BehaviorConfig::save()
{
	kdDebug(14000) << k_funcinfo << "called." << endl;

	KopetePrefs *p = KopetePrefs::prefs();
	KopeteAway *ka = KopeteAway::getInstance();

	p->setNotifyAway( mAwayConfigUI->mNotifyAway->isChecked());
	ka->setUseAutoAway(mAwayConfigUI->mUseAutoAway->isChecked());
	ka->setAutoAwayTimeout(mAwayConfigUI->mAutoAwayTimeout->value() * 60);
	ka->setGoAvailable(mAwayConfigUI->mGoAvailable->isChecked());
	ka->save();

	// disconnect or else we will end up in an endless loop
	p->save();
}

void BehaviorConfig::reopen()
{
	kdDebug(14000) << k_funcinfo << "called" << endl;

	KopetePrefs *p = KopetePrefs::prefs();

	mAwayConfigUI->updateView();
	mAwayConfigUI->mNotifyAway->setChecked( p->notifyAway() );
}

#include "behaviorconfig.moc"
// vim: set noet ts=4 sts=4 sw=4:
