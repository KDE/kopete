/*
    wlmchatsession.h - Wlm Message Manager

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef WLMCHATSESSION_H
#define WLMCHATSESSION_H

#include <QLinkedList>
#include <QMap>
#include <QTimer>

#include <kaction.h>
#include <kactionmenu.h>
#include <KMenu>

#include "kopetechatsession.h"
#include <msn/msn.h>

class WlmContact;

class KOPETE_EXPORT WlmChatSession: public Kopete::ChatSession
{
    Q_OBJECT
  public:
    WlmChatSession (Kopete::Protocol * protocol, const Kopete::Contact * user,
                    Kopete::ContactPtrList others,
                    MSN::SwitchboardServerConnection * conn = NULL);
     ~WlmChatSession ();
    void setReady (bool value);
    bool isReady ();
    void setChatService (MSN::SwitchboardServerConnection * conn);
	bool isConnecting();
    MSN::SwitchboardServerConnection * getChatService ()
    {
        return m_chatService;
    }
    void messageSentACK (unsigned int trID);
    bool requestChatService ();
    void requestDisplayPicture ();
    void
    setDownloadDisplayPicture (bool a)
    {
        m_downloadDisplayPicture = a;
    }
    bool
    isDownloadDisplayPicture ()
    {
        return m_downloadDisplayPicture;
    }
    void
    setSendNudge (bool a)
    {
        m_sendNudge = a;
    }
    bool
    isSendNudge ()
    {
        return m_sendNudge;
    }
    void receivedNudge (QString passport);
    void sendFile (const QString & fileLocation, long unsigned int fileSize);
    virtual void inviteContact (const QString &);
    void startSendKeepAlive();
    void stopSendKeepAlive();
    unsigned int generateSessionID();

  private slots:
    void slotMessageSent (Kopete::Message & message, Kopete::ChatSession * kmm);
    void sendTypingMsg (bool istyping);
    void sendNudge ();
    void switchboardConnectionTimeout ();
    void slotActionInviteAboutToShow ();
    void slotInviteContact (Kopete::Contact * contact);
    void slotSendFile ();
    void sendKeepAlive ();

  private:

    MSN::SwitchboardServerConnection * m_chatService;
    bool m_downloadDisplayPicture;
    bool m_sendNudge;
    int m_tries;
    int m_oimid;
    int m_sessionID;
    QString m_lastMsnObj;
    QLinkedList < Kopete::Message > m_messagesQueue;
    QMap < unsigned int, Kopete::Message > m_messagesSentQueue;
    QLinkedList < QString > m_pendingInvitations;
    QLinkedList < QString > m_pendingFiles;
    KAction * m_actionNudge;
    KAction * m_actionFile;
    KActionMenu * m_actionInvite;
    QList < KAction* > m_inviteactions;
    QTimer * m_keepalivetimer;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:
