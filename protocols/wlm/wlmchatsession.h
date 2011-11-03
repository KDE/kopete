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
#include <QWidgetAction>
#include <QTimer>

#include <kaction.h>
#include <kactionmenu.h>
#include <KMenu>

#include "kopetechatsession.h"
#include "wlmchatsessioninkaction.h"

#include <msn/msn.h>

#ifdef HAVE_MEDIASTREAMER
#include <mediastreamer2/msfilter.h>
#include <mediastreamer2/mssndcard.h>
#include <mediastreamer2/msticker.h>
#include <mediastreamer2/msfilerec.h>
#endif

class WlmContact;

class WlmChatSession: public Kopete::ChatSession
{
    Q_OBJECT
  public:
    WlmChatSession (Kopete::Protocol * protocol, const Kopete::Contact * user,
                    Kopete::ContactPtrList others,
                    MSN::SwitchboardServerConnection * conn = NULL);
    virtual ~WlmChatSession ();
    void setReady (bool value);
    bool isReady ();
    void addFileToRemove(QString path);
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
    QMap < QString, QString > emoticonsList;
    void convertToGif( const QPixmap & ink, QString filename);

  private slots:
    void slotMessageSent (Kopete::Message & message, Kopete::ChatSession * kmm);
    void sendTypingMsg (bool istyping);
    void sendNudge ();
    void switchboardConnectionTimeout ();
    void slotActionInviteAboutToShow ();
    void slotInviteContact (Kopete::Contact * contact);
    void slotSendInk ( const QPixmap &);
    void slotSendVoiceStartRec();
    void slotSendVoiceStopRec();
    void slotSendVoiceStopRecTimeout();
    void slotSendFile ();
    void sendKeepAlive ();
    void messageTimeout();

  private:
    MSN::Message parseMessage(Kopete::Message & msg);

    MSN::SwitchboardServerConnection * m_chatService;
    bool m_downloadDisplayPicture;
    bool m_sendNudge;
    bool m_chatServiceRequested;
    int m_tries;
    int m_oimid;
    int m_sessionID;
    QString m_lastMsnObj;
    QLinkedList < Kopete::Message > m_messagesQueue;
    QMap < unsigned int, Kopete::Message > m_messagesSentQueue;
    QLinkedList < int > m_messagesTimeoutQueue;
    QLinkedList < QString > m_pendingInvitations;
    QLinkedList < QString > m_pendingFiles;
    QLinkedList < QByteArray > m_pendingInks;
    KAction * m_actionNudge;
    WlmChatSessionInkAction * m_actionInk;
    KActionMenu * m_actionInvite;
    QList < KAction* > m_inviteactions;
    QTimer * m_keepalivetimer;
    QStringList m_filesToRemove;

#ifdef HAVE_MEDIASTREAMER
    KActionMenu * m_actionVoice;
    QString m_currentVoiceClipName;
    QTimer *m_voiceTimer;
    QLinkedList < QString > m_pendingVoices;

    MSFilter *m_voiceFilter;
    MSSndCard *m_voiceCardCapture;
    MSTicker *m_voiceTicker;
    MSFilter *m_voiceRecorder;
#endif

};

#endif

// vim: set noet ts=4 sts=4 tw=4:
