/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <qstring.h>
#include <kdebug.h>
#include <qlabel.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <kglobal.h>
#include <qdatetime.h>

#include "skypecalldialog.h"
#include "skypeaccount.h"

typedef enum {
	csNotRunning,
	csOnHold,
	csInProgress,
	csShuttingDown
} callStatus;

class SkypeCallDialogPrivate {
	public:
		///The call is done by some account
		SkypeAccount *account;
		///The other side
		QString userId;
		///Id of the call
		QString callId;
		///Was there some error?
		bool error;
		///The timer for updating call info
		QTimer *updater; 
		///The status of the call
		callStatus status;
		///The time the call is running or on hold (in halfes of seconds)
		int totalTime;
		///The time the call is actually running (in halfes of seconds)
		int callTime;
};

SkypeCallDialog::SkypeCallDialog(const QString &callId, const QString &userId, SkypeAccount *account) : SkypeCallDialogBase() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	//Initialize values
	d = new SkypeCallDialogPrivate();
	d->account = account;
	d->callId = callId;
	d->userId = userId;
	d->error = false;
	d->status = csNotRunning;
	d->totalTime = 0;
	d->callTime = 0; 

	d->updater = new QTimer();
	connect(d->updater, SIGNAL(timeout()), this, SLOT(updateCallInfo()));
	d->updater->start(500); 

	NameLabel->setText(account->getUserLabel(userId));

	//Show the window
	show();
}


SkypeCallDialog::~SkypeCallDialog(){
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	delete d->updater;
	delete d;
}

void SkypeCallDialog::updateStatus(const QString &callId, const QString &status) {
	kdDebug(14311) << k_funcinfo << "Status: " << status << endl;//some debug info

	if (callId == d->callId) {
		if (status == "CANCELLED") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("Canceled"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "BUSY") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("Other person is busy"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "REFUSED") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("Refused"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "MISSED") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("Missed"));
			d->status = csNotRunning;
		} else if (status == "FINISHED") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("Finished"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "LOCALHOLD") {
			HoldButton->setEnabled(true);
			HoldButton->setText(i18n("Resume"));
			HangButton->setEnabled(true);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("On hold (local)"));
			d->status = csOnHold;
		} else if (status == "ONHOLD") {
			HoldButton->setEnabled(false);
			HangButton->setEnabled(true);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("On hold"));
			d->status = csOnHold;
		} else if (status == "INPROGRESS") {
			HoldButton->setEnabled(true);
			HoldButton->setText(i18n("Hold"));
			HangButton->setEnabled(true);
			AcceptButton->setEnabled(false);
			StatusLabel->setText(i18n("In progress"));
			d->status=csInProgress;
		} else if (status == "RINGING") {
			HoldButton->setEnabled(false);
			AcceptButton->setEnabled(d->account->isCallIncoming(callId));
			HangButton->setEnabled(true);
			StatusLabel->setText(i18n("Ringing"));
			d->status = csNotRunning;
		} else if (status == "FAILED") {
			if (d->error) //This one is already handled
				return;
			HoldButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			HangButton->setEnabled(false);
			StatusLabel->setText(i18n("Failed"));
			d->status = csNotRunning;
		} else if (status == "ROUTING") {
			HoldButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			HangButton->setEnabled(true);
			StatusLabel->setText(i18n("Connecting"));
			d->status = csNotRunning;
		} else if (status == "EARLYMEDIA") {
			HoldButton->setEnabled(false);
			AcceptButton->setEnabled(false);
			HangButton->setEnabled(true);
			StatusLabel->setText(i18n("Early media (waitong for operator..)"));
			d->status = csNotRunning;
		} else if (status == "UNPLACED") {//Ups, whats that, how that call got here?
			deleteLater();//Just give up, this one is odd
		}
	}
}

void SkypeCallDialog::acceptCall() {
	emit acceptTheCall(d->callId);
}

void SkypeCallDialog::hangUp() {
	emit hangTheCall(d->callId);
}

void SkypeCallDialog::holdCall() {
	emit toggleHoldCall(d->callId);
}

void SkypeCallDialog::closeEvent(QCloseEvent *) {
	emit hangTheCall(d->callId);//Finish the call before you give up
	deleteLater();//some kind of suicide
}

void SkypeCallDialog::deathTimeout() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	deleteLater();//OK, the death is here :-)
}

void SkypeCallDialog::closeLater() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if ((d->account->closeCallWindowTimeout()) && (d->status != csShuttingDown)) {
		QTimer::singleShot(1000 * d->account->closeCallWindowTimeout(), this, SLOT(deathTimeout()));
		d->status = csShuttingDown;
	}
}

void SkypeCallDialog::updateError(const QString &callId, const QString &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	if (callId == d->callId) {
		AcceptButton->setEnabled(false);
		HangButton->setEnabled(false);
		HoldButton->setEnabled(false);
		StatusLabel->setText(i18n("Failed (%1)").arg(message));
		closeLater();
		d->error = true;
	}
}

void SkypeCallDialog::updateCallInfo() {
	switch (d->status) {
		case csInProgress:
			if (d->callTime % 20 == 0) 
				emit updateSkypeOut();//update the skype out
			++d->callTime; 
			//Do not break, do that as well
		case csOnHold:
			++d->totalTime;
		default:
			;//Get rid of that stupid warning about not handled value in switch
	}
	const QString &activeTime = KGlobal::locale()->formatTime(QTime().addSecs(d->callTime / 2), true, true);
	const QString &totalTime = KGlobal::locale()->formatTime(QTime().addSecs(d->totalTime / 2), true, true);
	TimeLabel->setText(i18n("%1 active\n%2 total").arg(activeTime).arg(totalTime));
}

void SkypeCallDialog::skypeOutInfo(int balance, const QString &currency) {
	float part;//How to change the balance before showing (multiply by this)
	QString symbol;//The symbol of the currency is
	int digits;
	if (currency == "EUR") {
		part = 0.01;//It's in cent's not in euros
		symbol = i18n("€");
		digits = 2;
	} else {
		CreditLabel->setText(i18n("Skypeout inactive"));	
		return;
	}
	float value = balance * part;
	CreditLabel->setText(KGlobal::locale()->formatMoney(value, symbol, digits));
}

#include "skypecalldialog.moc"
