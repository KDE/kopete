/*
    jinglevoicesessiondialog.h - GUI for a voice session.

    Copyright (c) 2006      by Michaël Larouche     <larouche@kde.org>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLEVOICESESSIONDIALOG_H
#define JINGLEVOICESESSIONDIALOG_H

#include "ui_jinglevoicesessiondialogbase.h"

#include <im.h>
#include <xmpp.h>

using namespace XMPP;

class JabberContact;
class VoiceCaller;
class JingleVoiceSession;

class JingleVoiceSessionDialog : public QDialog
{
	Q_OBJECT
public:
	enum SessionState { Incoming, Waiting, Accepted, Declined, Started, Terminated };

	//JingleVoiceSessionDialog(const Jid &peerJid, VoiceCaller *caller, QWidget *parent = 0, const char *name = 0);
	JingleVoiceSessionDialog(JingleVoiceSession *session, QWidget *parent = 0);
	~JingleVoiceSessionDialog();

public slots:
	void start();

protected slots:
	void reject();

protected:
	void finalize();

private slots:
	void slotAcceptClicked();
	void slotDeclineClicked();
	void slotTerminateClicked();

	void sessionStarted();
	void sessionAccepted();
	void sessionDeclined();
	void sessionTerminated();

private:
	Ui::JingleVoiceSessionDialogBase Ui;
	void setContactInformation(JabberContact *contact);
	QWidget *m_parent;

	JingleVoiceSession *m_session;
	Jid m_peerJid;
	SessionState m_sessionState;
};

#endif
