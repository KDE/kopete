/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlayout.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <kconfig.h>
#include <klocale.h>
#include <krestrictedline.h>
#include <kglobal.h>

#include "smspreferences.h"
#include "smsprefs.h"

SMSPreferences::SMSPreferences( const QString &pixmap, QObject *parent )
	: ConfigModule( i18n( "SMS Plugin" ), i18n( "Sending messages to cellphones" ), pixmap, parent )
{
	(new QBoxLayout(this, QBoxLayout::Down))->setAutoAdd(true);
	theUI = new SMSPrefsUI(this);

	KGlobal::config()->setGroup("SMS");
	theUI->subEnable->setChecked(KGlobal::config()->readBoolEntry("SubEnable", false));
	theUI->subCode->setText(KGlobal::config()->readEntry("SubCode", "+44"));
	theUI->msgAsk->setChecked(KGlobal::config()->readNumEntry("MsgAction", ACT_ASK) == ACT_ASK);
	theUI->msgCancel->setChecked(KGlobal::config()->readNumEntry("MsgAction", ACT_ASK) == ACT_CANCEL);
	theUI->msgSplit->setChecked(KGlobal::config()->readNumEntry("MsgAction", ACT_ASK) == ACT_SPLIT);
}

SMSPreferences::~SMSPreferences()
{
}

void SMSPreferences::save()
{
	KGlobal::config()->setGroup("SMS");
	KGlobal::config()->writeEntry("SubEnable", theUI->subEnable->isChecked());
	KGlobal::config()->writeEntry("SubCode", theUI->subCode->text());
	KGlobal::config()->writeEntry("MsgAction", int(theUI->msgAsk->isChecked() ? ACT_ASK : theUI->msgSplit->isChecked() ? ACT_SPLIT : ACT_CANCEL));
	KGlobal::config()->sync();
	emit saved();
}

#include "smspreferences.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

