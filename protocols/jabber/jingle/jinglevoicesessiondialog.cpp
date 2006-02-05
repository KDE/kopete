/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

    Copyright (c) 2006      by Michaël Larouche     <michael.larouche@kdemail.net>

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
#include "jinglevoicesessiondialog.h"

// Qt includes
#include <qlabel.h>
#include <qpixmap.h>
#include <qimage.h>

// Jingle includes
// #include "jinglevoicesession.h"
// #include "jinglesessionmanager.h"
#include "voicecaller.h"

// KDE includes
#include <klocale.h>
#include <kpushbutton.h>

// Kopete includes
#include "jabberaccount.h"
#include "jabbercontact.h"
#include "jabbercontactpool.h"

#include "kopeteglobal.h"
#include "kopetemetacontact.h"

using namespace XMPP;

JingleVoiceSessionDialog::JingleVoiceSessionDialog(const Jid &peerJid, VoiceCaller *caller, QWidget *parent, const char *name)
 : JingleVoiceSessionDialogBase(parent, name), m_session(caller), m_peerJid(peerJid), m_sessionState(Incoming)
{
	QString contactJid = m_peerJid.full();
	setCaption( i18n("Voice session with %1").arg(contactJid) );

	connect(buttonAccept, SIGNAL(clicked()), this, SLOT(slotAcceptClicked()));
	connect(buttonDecline, SIGNAL(clicked()), this, SLOT(slotDeclineClicked()));
	connect(buttonTerminate, SIGNAL(clicked()), this, SLOT(slotTerminateClicked()));

// NOTE: Disabled for 0.12
#if 0
	connect(m_session, SIGNAL(sessionStarted()), this, SLOT(sessionStarted()));
	connect(m_session, SIGNAL(accepted()), this, SLOT(sessionAccepted()));
	connect(m_session, SIGNAL(declined()), this, SLOT(sessionDeclined()));
	connect(m_session, SIGNAL(terminated()), this, SLOT(sessionTerminated()));
#endif
	connect(m_session, SIGNAL(accepted( const Jid & )), this, SLOT( sessionAccepted(const Jid &) ));
	connect(m_session, SIGNAL(in_progress( const Jid & )), this, SLOT( sessionStarted(const Jid &) ));
	connect(m_session, SIGNAL(rejected( const Jid& )), this, SLOT( sessionDeclined(const Jid &) ));
	connect(m_session, SIGNAL(terminated( const Jid& )), this, SLOT( sessionTerminated(const Jid &) ));

	// Find JabberContact for the peer and fill this dialog with contact information.
	JabberContact *peerContact = static_cast<JabberContact*>( m_session->account()->contactPool()->findRelevantRecipient( m_peerJid ) );
	if( peerContact )
	{
		setContactInformation( peerContact );
	}
	
	labelSessionStatus->setText( i18n("Incoming Session...") );
	buttonAccept->setEnabled(true);
	buttonDecline->setEnabled(true);
}

JingleVoiceSessionDialog::~JingleVoiceSessionDialog()
{
	//m_session->account()->sessionManager()->removeSession(m_session);
}

void JingleVoiceSessionDialog::setContactInformation(JabberContact *contact)
{
	if( contact->metaContact() )
	{
		labelDisplayName->setText( contact->metaContact()->displayName() );
		labelContactPhoto->setPixmap( QPixmap(contact->metaContact()->photo()) );
	}
	else
	{
		labelDisplayName->setText( contact->nickName() );
		labelDisplayName->setPixmap( QPixmap(contact->property(Kopete::Global::Properties::self()->photo()).value().toString()) );
	}
}

void JingleVoiceSessionDialog::start()
{
	labelSessionStatus->setText( i18n("Waiting for other peer...") );
	buttonAccept->setEnabled(false);
	buttonDecline->setEnabled(false);
	buttonTerminate->setEnabled(true);
	//m_session->start();
	m_session->call( m_peerJid );
	m_sessionState = Waiting;
}

void JingleVoiceSessionDialog::slotAcceptClicked()
{
	labelSessionStatus->setText( i18n("Session accepted.") );
	buttonAccept->setEnabled(false);
	buttonDecline->setEnabled(false);
	buttonTerminate->setEnabled(true);
	
	//m_session->accept();
	m_session->accept( m_peerJid );
	m_sessionState = Accepted;
}

void JingleVoiceSessionDialog::slotDeclineClicked()
{
	labelSessionStatus->setText( i18n("Session declined.") );
	buttonAccept->setEnabled(false);
	buttonDecline->setEnabled(false);
	buttonTerminate->setEnabled(false);

	//m_session->decline();
	m_session->reject( m_peerJid );
	m_sessionState = Declined;
	finalize();
}

void JingleVoiceSessionDialog::slotTerminateClicked()
{
	labelSessionStatus->setText( i18n("Session terminated.") );
	buttonAccept->setEnabled(false);
	buttonDecline->setEnabled(false);
	buttonTerminate->setEnabled(false);

	//m_session->terminate();
	m_session->terminate( m_peerJid );
	m_sessionState = Terminated;
	finalize();
	close();
}

void JingleVoiceSessionDialog::sessionStarted(const Jid &jid)
{
	if( m_peerJid.compare(jid) )
	{
		labelSessionStatus->setText( i18n("Session in progress.") );
		buttonAccept->setEnabled(false);
		buttonDecline->setEnabled(false);
		buttonTerminate->setEnabled(true);
		m_sessionState = Started;
	}
}

void JingleVoiceSessionDialog::sessionAccepted(const Jid &jid)
{
	if( m_peerJid.compare(jid) )
	{
		labelSessionStatus->setText( i18n("Session accepted.") );
		buttonAccept->setEnabled(false);
		buttonDecline->setEnabled(false);
		buttonTerminate->setEnabled(true);
		m_sessionState = Accepted;
	}
}

void JingleVoiceSessionDialog::sessionDeclined(const Jid &jid)
{
	if( m_peerJid.compare(jid) )
	{
		labelSessionStatus->setText( i18n("Session declined.") );
		buttonAccept->setEnabled(false);
		buttonDecline->setEnabled(false);
		buttonTerminate->setEnabled(false);
		m_sessionState = Declined;
	}
}

void JingleVoiceSessionDialog::sessionTerminated(const Jid &jid)
{
	if( m_peerJid.compare(jid) )
	{
		labelSessionStatus->setText( i18n("Session terminated.") );
		buttonAccept->setEnabled(false);
		buttonDecline->setEnabled(false);
		buttonTerminate->setEnabled(false);
		m_sessionState = Terminated;
	}
}

void JingleVoiceSessionDialog::reject()
{
	finalize();
	QDialog::reject();
}

void JingleVoiceSessionDialog::finalize()
{
	disconnect(m_session, SIGNAL(accepted( const Jid & )), this, SLOT( sessionAccepted(const Jid &) ));
	disconnect(m_session, SIGNAL(in_progress( const Jid & )), this, SLOT( sessionStarted(const Jid &) ));
	disconnect(m_session, SIGNAL(rejected( const Jid& )), this, SLOT( sessionDeclined(const Jid &) ));
	disconnect(m_session, SIGNAL(terminated( const Jid& )), this, SLOT( sessionTerminated(const Jid &) ));
}

#include "jinglevoicesessiondialog.moc"
