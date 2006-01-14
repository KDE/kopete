#include <qlabel.h>
#include <qpushbutton.h>

#include "voicecalldlg.h"
#include "voicecaller.h"

VoiceCallDlg::VoiceCallDlg(const Jid& jid, VoiceCaller* voiceCaller) : VoiceCallUI(0,0,false,WDestructiveClose), jid_(jid), voiceCaller_(voiceCaller)
{
	setCaption(QString(tr("Voice Call (%1)")).arg(jid.full()));
	
	// Voice Caller signals
	connect(voiceCaller_,SIGNAL(accepted(const Jid&)),SLOT(accepted(const Jid&)));
	connect(voiceCaller_,SIGNAL(rejected(const Jid&)),SLOT(rejected(const Jid&)));
	connect(voiceCaller_,SIGNAL(in_progress(const Jid&)),SLOT(in_progress(const Jid&)));
	connect(voiceCaller_,SIGNAL(terminated(const Jid&)),SLOT(terminated(const Jid&)));
	
	// Buttons
	pb_hangup->setEnabled(false);
	pb_accept->setEnabled(false);
	pb_reject->setEnabled(false);

	connect(pb_hangup,SIGNAL(clicked()),SLOT(terminate_call()));
	connect(pb_accept,SIGNAL(clicked()),SLOT(accept_call()));
	connect(pb_reject,SIGNAL(clicked()),SLOT(reject_call()));
	
}

void VoiceCallDlg::call()
{
	setStatus(Calling);
	voiceCaller_->call(jid_);
}

void VoiceCallDlg::accept_call()
{
	setStatus(Accepting);
	voiceCaller_->accept(jid_);
}

void VoiceCallDlg::reject_call()
{
	setStatus(Rejecting);
	voiceCaller_->reject(jid_);
	finalize();
	close();
}
	
void VoiceCallDlg::terminate_call()
{
	setStatus(Terminating);
	voiceCaller_->terminate(jid_);
	finalize();
	close();
}

void VoiceCallDlg::accepted(const Jid& j)
{
	if (jid_.compare(j)) {
		setStatus(Accepted);
	}
}
	
void VoiceCallDlg::rejected(const Jid& j)
{
	if (jid_.compare(j)) {
		setStatus(Rejected);
		finalize();
	}
}

void VoiceCallDlg::in_progress(const Jid& j)
{
	if (jid_.compare(j)) {
		setStatus(InProgress);
	}
}

void VoiceCallDlg::terminated(const Jid& j)
{
	if (jid_.compare(j)) {
		setStatus(Terminated);
		finalize();
	}
}
	

void VoiceCallDlg::incoming()
{
	setStatus(Incoming);
}

void VoiceCallDlg::setStatus(CallStatus s)
{
	status_ = s;
	switch (s) {
		case Calling:
			lb_status->setText(tr("Calling"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(true);
			break;

		case Accepting:
			lb_status->setText(tr("Accepting"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(true);
			break;

		case Rejecting:
			lb_status->setText(tr("Rejecting"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(false);
			break;

		case Terminating:
			lb_status->setText(tr("Hanging up"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(false);
			break;

		case Accepted:
			lb_status->setText(tr("Accepted"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(true);
			break;

		case Rejected:
			lb_status->setText(tr("Rejected"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(false);
			break;

		case InProgress:
			lb_status->setText(tr("In progress"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(true);
			break;

		case Terminated:
			lb_status->setText(tr("Terminated"));
			pb_accept->setEnabled(false);
			pb_reject->setEnabled(false);
			pb_hangup->setEnabled(false);
			break;
			
		case Incoming:
			lb_status->setText(tr("Incoming Call"));
			pb_accept->setEnabled(true);
			pb_reject->setEnabled(true);
			pb_hangup->setEnabled(false);
			break;

		default:
			break;
	}
}

void VoiceCallDlg::reject()
{
	finalize();
	QDialog::reject();
}

void VoiceCallDlg::finalize()
{
	// Close connection
	if (status_ == Incoming) {
		reject_call();
	}
	else if (status_ == InProgress || status_ == Calling || status_ == Accepting || status_ == Accepted) {
		terminate_call();
	}

	// Disconnect signals
	disconnect(voiceCaller_,SIGNAL(accepted(const Jid&)),this,SLOT(accepted(const Jid&)));
	disconnect(voiceCaller_,SIGNAL(rejected(const Jid&)),this,SLOT(rejected(const Jid&)));
	disconnect(voiceCaller_,SIGNAL(in_progress(const Jid&)),this,SLOT(in_progress(const Jid&)));
	disconnect(voiceCaller_,SIGNAL(terminated(const Jid&)),this,SLOT(terminated(const Jid&)));
}
