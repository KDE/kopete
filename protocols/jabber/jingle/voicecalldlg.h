#ifndef VOICECALLDLG
#define VOICECALLDLG

#include "ui_voicecall.h"
#include "im.h"

using namespace XMPP;

class VoiceCaller;

class VoiceCallDlg : public VoiceCallUI
{
	Q_OBJECT

public:
	enum CallStatus {
		Default,
		Calling, Accepting, Rejecting, Terminating, 
		Accepted, Rejected, InProgress, Terminated, Incoming
	};

	VoiceCallDlg(const Jid&, VoiceCaller*);

public slots:
	void incoming();
	void call();

	void terminate_call();
	void accept_call();
	void reject_call();

	void accepted(const Jid&);
	void rejected(const Jid&);
	void in_progress(const Jid&);
	void terminated(const Jid&);

protected slots:
	void reject();

protected:
	void finalize();
	void setStatus(CallStatus);

private:
	Jid jid_;
	CallStatus status_;
	VoiceCaller* voiceCaller_;
};

#endif
