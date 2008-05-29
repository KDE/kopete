/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

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
#include "jinglevoicesessiondialog.h"

// Qt includes
#include <qlabel.h>
#include <qpixmap.h>
#include <qimage.h>

// Jingle includes
 #include "jinglevoicesession.h"
 #include "jinglesessionmanager.h"
//#include "voicecaller.h"

// KDE includes
#include <klocale.h>
#include <kpushbutton.h>

// Kopete includes
#include "jabberaccount.h"
#include "jabbercontact.h"
#include "jabbercontactpool.h"

#include "kopeteglobal.h"
#include "kopetemetacontact.h"

#include <kdebug.h>
#include "jabberprotocol.h"
#define qDebug( X )  kDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << X << endl

using namespace XMPP;

// JingleVoiceSessionDialog::JingleVoiceSessionDialog(const Jid &peerJid, VoiceCaller *caller, QWidget *parent, const char *name)
//  : Ui::JingleVoiceSessionDialogBase(),QDialog(parent), m_session(caller), m_peerJid(peerJid), m_sessionState(Incoming)
// {
// 	
// 	setupUi(this);
// 	
// 	QString contactJid = m_peerJid.full();
// 	setWindowTitle( i18n("Voice session with %1", contactJid) );
// 
// 	connect(buttonAccept, SIGNAL(clicked()), this, SLOT(slotAcceptClicked()));
// 	connect(buttonDecline, SIGNAL(clicked()), this, SLOT(slotDeclineClicked()));
// 	connect(buttonTerminate, SIGNAL(clicked()), this, SLOT(slotTerminateClicked()));
// 
// // NOTE: Disabled for 0.12
// #if 1
// 	connect(m_session, SIGNAL(sessionStarted()), this, SLOT(sessionStarted()));
// 	connect(m_session, SIGNAL(accepted()), this, SLOT(sessionAccepted()));
// 	connect(m_session, SIGNAL(declined()), this, SLOT(sessionDeclined()));
// 	connect(m_session, SIGNAL(terminated()), this, SLOT(sessionTerminated()));
// #endif
// 	//connect(m_session, SIGNAL(accepted( const Jid & )), this, SLOT( sessionAccepted(const Jid &) ));
// 	//connect(m_session, SIGNAL(in_progress( const Jid & )), this, SLOT( sessionStarted(const Jid &) ));
// 	//connect(m_session, SIGNAL(rejected( const Jid& )), this, SLOT( sessionDeclined(const Jid &) ));
// 	//connect(m_session, SIGNAL(terminated( const Jid& )), this, SLOT( sessionTerminated(const Jid &) ));
// 
// 	// Find JabberContact for the peer and fill this dialog with contact information.
// 	JabberContact *peerContact = static_cast<JabberContact*>( m_session->account()->contactPool()->findRelevantRecipient( m_peerJid ) );
// 	if( peerContact )
// 	{
// 		setContactInformation( peerContact );
// 	}
// 	
// 	labelSessionStatus->setText( i18n("Incoming Session...") );
// 	buttonAccept->setEnabled(true);
// 	buttonDecline->setEnabled(true);
// }

JingleVoiceSessionDialog::JingleVoiceSessionDialog(JingleVoiceSession *session, QWidget *parent)
 : m_parent(parent), m_session(session), m_peerJid(session->peers().first()), m_sessionState(Incoming)
{
	
	Ui.setupUi(this);
	
	QString contactJid = m_peerJid.full();
	setWindowTitle( i18n("Voice session with %1", contactJid) );

	connect(Ui.buttonAccept, SIGNAL(clicked()), this, SLOT(slotAcceptClicked()));
	connect(Ui.buttonDecline, SIGNAL(clicked()), this, SLOT(slotDeclineClicked()));
	connect(Ui.buttonTerminate, SIGNAL(clicked()), this, SLOT(slotTerminateClicked()));

// NOTE: Disabled for 0.12
#if 1
	connect(m_session, SIGNAL(sessionStarted()), this, SLOT(sessionStarted() ));
	connect(m_session, SIGNAL(accepted()), this, SLOT(sessionAccepted() ));
	connect(m_session, SIGNAL(declined()), this, SLOT(sessionDeclined() ));
	connect(m_session, SIGNAL(terminated()), this, SLOT(sessionTerminated() ));
#endif
	//connect(m_session, SIGNAL(accepted( const Jid & )), this, SLOT( sessionAccepted(const Jid &) ));
	//connect(m_session, SIGNAL(in_progress( const Jid & )), this, SLOT( sessionStarted(const Jid &) ));
	//connect(m_session, SIGNAL(rejected( const Jid& )), this, SLOT( sessionDeclined(const Jid &) ));
	//connect(m_session, SIGNAL(terminated( const Jid& )), this, SLOT( sessionTerminated(const Jid &) ));

	// Find JabberContact for the peer and fill this dialog with contact information.
	JabberContact *peerContact = static_cast<JabberContact*>( m_session->account()->contactPool()->findRelevantRecipient( m_peerJid ) );
	if( peerContact )
	{
		setContactInformation( peerContact );
	}
	
	Ui.labelSessionStatus->setText( i18n("Incoming Session...") );
	Ui.buttonAccept->setEnabled(true);
	Ui.buttonDecline->setEnabled(true);
}

JingleVoiceSessionDialog::~JingleVoiceSessionDialog()
{
	m_session->account()->sessionManager()->removeSession(m_session);
}

void JingleVoiceSessionDialog::setContactInformation(JabberContact *contact)
{
	if( contact->metaContact() )
	{
		Ui.labelDisplayName->setText( contact->metaContact()->displayName() );
		Ui.labelContactPhoto->setPixmap( QPixmap(contact->metaContact()->photo()) );
	}
	else
	{
		Ui.labelDisplayName->setText( contact->nickName() );
		Ui.labelDisplayName->setPixmap( QPixmap(contact->property(Kopete::Global::Properties::self()->photo()).value().toString()) );
	}
}

void JingleVoiceSessionDialog::start()
{
	Ui.labelSessionStatus->setText( i18n("Waiting for other peer...") );
	Ui.buttonAccept->setEnabled(false);
	Ui.buttonDecline->setEnabled(false);
	Ui.buttonTerminate->setEnabled(true);
	m_session->start();
	//m_session->call( m_peerJid );
	m_sessionState = Waiting;
}

void JingleVoiceSessionDialog::slotAcceptClicked()
{
	Ui.labelSessionStatus->setText( i18n("Session accepted.") );
	Ui.buttonAccept->setEnabled(false);
	Ui.buttonDecline->setEnabled(false);
	Ui.buttonTerminate->setEnabled(true);
	
	m_session->accept();
	//m_session->accept( m_peerJid );
	m_sessionState = Accepted;
}

void JingleVoiceSessionDialog::slotDeclineClicked()
{
	Ui.labelSessionStatus->setText( i18n("Session declined.") );
	Ui.buttonAccept->setEnabled(false);
	Ui.buttonDecline->setEnabled(false);
	Ui.buttonTerminate->setEnabled(false);

	m_session->decline();
	//session->reject( m_peerJid );
	m_sessionState = Declined;
	finalize();
}

void JingleVoiceSessionDialog::slotTerminateClicked()
{
	Ui.labelSessionStatus->setText( i18n("Session terminated.") );
	Ui.buttonAccept->setEnabled(false);
	Ui.buttonDecline->setEnabled(false);
	Ui.buttonTerminate->setEnabled(false);

	m_session->terminate();
	//m_session->terminate( m_peerJid );
			qDebug(QString("JingleVoiceCaller: Here"));
	m_sessionState = Terminated;
			qDebug(QString("JingleVoiceCaller: Here2"));
	finalize();
			qDebug(QString("JingleVoiceCaller: Here3"));
	close();
			qDebug(QString("JingleVoiceCaller: Here4"));
}

void JingleVoiceSessionDialog::sessionStarted()
{
	//if( m_peerJid.compare(jid) )
	{
		Ui.labelSessionStatus->setText( i18n("Session in progress.") );
		Ui.buttonAccept->setEnabled(false);
		Ui.buttonDecline->setEnabled(false);
		Ui.buttonTerminate->setEnabled(true);
		m_sessionState = Started;
	}
}

void JingleVoiceSessionDialog::sessionAccepted()
{
	//if( m_peerJid.compare(jid) )
	{
		Ui.labelSessionStatus->setText( i18n("Session accepted.") );
		Ui.buttonAccept->setEnabled(false);
		Ui.buttonDecline->setEnabled(false);
		Ui.buttonTerminate->setEnabled(true);
		m_sessionState = Accepted;
	}
}

void JingleVoiceSessionDialog::sessionDeclined()
{
	//if( m_peerJid.compare(jid) )
	{
		Ui.labelSessionStatus->setText( i18n("Session declined.") );
		Ui.buttonAccept->setEnabled(false);
		Ui.buttonDecline->setEnabled(false);
		Ui.buttonTerminate->setEnabled(false);
		m_sessionState = Declined;
	}
}

void JingleVoiceSessionDialog::sessionTerminated()
{
	//if( m_peerJid.compare(jid) )
	{
		Ui.labelSessionStatus->setText( i18n("Session terminated.") );
		Ui.buttonAccept->setEnabled(false);
		Ui.buttonDecline->setEnabled(false);
		Ui.buttonTerminate->setEnabled(false);
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
	disconnect(m_session, SIGNAL(accepted( )), this, SLOT( sessionAccepted() ));
	disconnect(m_session, SIGNAL(sessionStarted(  )), this, SLOT( sessionStarted() ));
	disconnect(m_session, SIGNAL(declined( )), this, SLOT( sessionDeclined(c) ));
	disconnect(m_session, SIGNAL(terminated(  )), this, SLOT( sessionTerminated() ));
}

#include "jinglevoicesessiondialog.moc"
