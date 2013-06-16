/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008-2009 Pali Rohár <pali.rohar@gmail.com>

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

#include "skypecalldialog.h"
#include "skypewindow.h"

#include <qstring.h>
#include <kdebug.h>
#include <qlabel.h>
#include <klocale.h>
#include <kcurrencycode.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <kglobal.h>
#include <qdatetime.h>

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
		///Did I reported the ed of call already?
		bool callEnded;
		///Report that the call has ended, please
		void endCall() {
			if (!callEnded) {
				callEnded = true;
				account->endCall();
			}
		};
		///Skype window manager
		SkypeWindow * skypeWindow;
		///Widget for receiving video
		///TODO: move it to Ui::SkypeCallDialogBase
		QWidget * receivedVideo;
};

SkypeCallDialog::SkypeCallDialog(const QString &callId, const QString &userId, SkypeAccount *account) : KDialog() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	setButtons( KDialog::None ); //do not add any buttons
	setDefaultButton( KDialog::None );

	QWidget *w = new QWidget( this );
	dialog = new Ui::SkypeCallDialogBase();
	dialog->setupUi( w );
	setMainWidget( w );

	//Initialize values
	d = new SkypeCallDialogPrivate();
	d->account = account;
	d->callId = callId;
	d->userId = userId;
	d->error = false;
	d->status = csNotRunning;
	d->totalTime = 0;
	d->callTime = 0;
	d->callEnded = false;
	d->skypeWindow = new SkypeWindow;
	d->receivedVideo = new QWidget;

	d->updater = new QTimer();
	connect(d->updater, SIGNAL(timeout()), this, SLOT(updateCallInfo()));
	d->updater->start(500);

	dialog->NameLabel->setText(account->getUserLabel(userId));
	setCaption(i18n("Call with %1", account->getUserLabel(userId)));

	connect(dialog->AcceptButton, SIGNAL(clicked()), this, SLOT(acceptCall()));
	connect(dialog->HangButton, SIGNAL(clicked()), this, SLOT(hangUp()));
	connect(dialog->HoldButton, SIGNAL(clicked()), this, SLOT(holdCall()));
	connect(dialog->ChatButton, SIGNAL(clicked()), this, SLOT(chatUser()));
	connect(dialog->VideoButton, SIGNAL(toggled(bool)), this, SLOT(videoAction(bool)));

	QTimer::singleShot(0, this, SLOT(hideSkype())); //We need immediately connect signal-slot in account, start this as slot
}


SkypeCallDialog::~SkypeCallDialog(){
	kDebug(SKYPE_DEBUG_GLOBAL);

	emit callFinished(d->callId);
	d->endCall();
	d->skypeWindow->deleteCallDialog(d->userId);

	delete d->skypeWindow;
	delete d->updater;
	delete d;
	delete dialog;
}

void SkypeCallDialog::updateStatus(const QString &callId, const QString &status) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "Status: " << status;

	if (callId == d->callId) {
		if (status == "CANCELLED") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("Canceled"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "BUSY") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("Other person is busy"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "REFUSED") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("Refused"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "MISSED") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(true);
			dialog->AcceptButton->setText(i18n("Call Back"));
			dialog->StatusLabel->setText(i18n("Missed"));
			d->status = csNotRunning;
			disconnect(dialog->AcceptButton, SIGNAL(clicked()), this, SLOT(acceptCall()));
			connect(dialog->AcceptButton, SIGNAL(clicked()), this, SLOT(callBack()));
		} else if (status == "FINISHED") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("Finished"));
			closeLater();
			d->status = csNotRunning;
		} else if (status == "LOCALHOLD") {
			dialog->HoldButton->setEnabled(true);
			dialog->HoldButton->setText(i18n("Resume"));
			dialog->HangButton->setEnabled(true);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("On hold (local)"));
			d->status = csOnHold;
		} else if (status == "REMOTEHOLD") {
			dialog->HoldButton->setEnabled(false);
			dialog->HangButton->setEnabled(true);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("On hold (remote)"));
			d->status = csOnHold;
		} else if (status == "ONHOLD") {
			dialog->HoldButton->setEnabled(true);
			dialog->HangButton->setEnabled(true);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("On hold"));
			d->status = csOnHold;
		} else if (status == "INPROGRESS") {
			dialog->HoldButton->setEnabled(true);
			dialog->HoldButton->setText(i18n("Hold"));
			dialog->HangButton->setEnabled(true);
			dialog->AcceptButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("In progress"));
			d->status=csInProgress;
		} else if (status == "RINGING") {
			dialog->HoldButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(d->account->isCallIncoming(callId));
			dialog->HangButton->setEnabled(true);
			dialog->StatusLabel->setText(i18n("Ringing"));
			d->status = csNotRunning;
		} else if (status == "FAILED") {
			if (d->error) //This one is already handled
				return;
			dialog->HoldButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->HangButton->setEnabled(false);
			dialog->StatusLabel->setText(i18n("Failed"));
			d->status = csNotRunning;
		} else if (status == "ROUTING") {
			dialog->HoldButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->HangButton->setEnabled(true);
			dialog->StatusLabel->setText(i18n("Connecting"));
			d->status = csNotRunning;
		} else if (status == "EARLYMEDIA") {
			dialog->HoldButton->setEnabled(false);
			dialog->AcceptButton->setEnabled(false);
			dialog->HangButton->setEnabled(true);
			dialog->StatusLabel->setText(i18nc("Early media means the media played before the call is established. For example it can be a calling tone or a waiting message such as all operators are busy.", "Early media (waiting for operator...)"));
			d->status = csNotRunning;
		} else if (status == "UNPLACED") {//Ups, whats that, how that call got here?
			//TODO: What here? Skype sometimes send message unplaced when call is active
			//temporary disabled, do nothing in active call

			//deleteLater();//Just give up, this one is odd
			//dialog->StatusLabel->setText(i18n("Unplaced (please wait...)"));
			//it is when user create call after hangup, so do not close dialog and wait
		}
	}
}

void SkypeCallDialog::acceptCall() {
	d->account->startCall();
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
	kDebug(SKYPE_DEBUG_GLOBAL);

	deleteLater();//OK, the death is here :-)
}

void SkypeCallDialog::closeLater() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->endCall();
	d->skypeWindow->deleteCallDialog(d->userId);

	if ((d->account->closeCallWindowTimeout()) && (d->status != csShuttingDown)) {
		QTimer::singleShot(1000 * d->account->closeCallWindowTimeout(), this, SLOT(deathTimeout()));
		d->status = csShuttingDown;
	}
}

void SkypeCallDialog::updateError(const QString &callId, const QString &message) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	if (callId == d->callId) {
		dialog->AcceptButton->setEnabled(false);
		dialog->HangButton->setEnabled(false);
		dialog->HoldButton->setEnabled(false);
		dialog->StatusLabel->setText(i18n("Failed (%1)", message));
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
	dialog->TimeLabel->setText(i18n("%1 active\n%2 total", activeTime, totalTime));
}

void SkypeCallDialog::skypeOutInfo(int balance, const QString &currency) {
	KCurrencyCode currencyCode(currency);
	if (!currencyCode.isValid())	{
		dialog->CreditLabel->setText(i18n("Skypeout inactive"));
		return;
	}
	// NOTE: As per the Skype API docs, the precision of the balance is fixed at 2 decimal places, regardless of currency, etc.
	// If that ever changes, this might need to be altered.
	double value = static_cast<double>(balance) / 100.;
	dialog->CreditLabel->setText(KGlobal::locale()->formatMoney(value, currencyCode.defaultSymbol(), 2));
}

void SkypeCallDialog::chatUser() {
	d->account->chatUser(d->userId);
}

void SkypeCallDialog::callBack() {
	deleteLater();//close this window

	d->account->makeCall(d->userId);
}

void SkypeCallDialog::hideSkype() {
	d->skypeWindow->hideCallDialog(d->userId);
}

void SkypeCallDialog::videoAction(bool b) {
	kDebug(SKYPE_DEBUG_GLOBAL) << b;
	if ( b )
		d->account->startSendingVideo(d->callId);
	else
		d->account->stopSendingVideo(d->callId);
}

void SkypeCallDialog::startReceivingVideo(const QString &callId) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	if ( callId != d->callId )
		return;
	d->skypeWindow->moveWebcamWidget(d->userId, d->receivedVideo->winId(), 2, 24);
	d->receivedVideo->resize(320+2, 240+24); //Skype has only support for 320x240 video size
	d->receivedVideo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)); //TODO: fix this - do not resize this window!!!
	d->receivedVideo->show();
	d->receivedVideo->setFocus();
}

void SkypeCallDialog::stopReceivingVideo(const QString &callId) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	if ( callId != d->callId )
		return;
	d->receivedVideo->hide();
	d->skypeWindow->revertWebcamWidget(d->userId);
}

#include "skypecalldialog.moc"
