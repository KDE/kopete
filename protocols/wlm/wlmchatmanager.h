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

#ifndef WLMCHATMANAGER_H
#define WLMCHATMANAGER_H

#include <Q3ValueList>
#include <QMap>

#include "kopetechatsession.h"
#include "wlmchatsession.h"
#include "wlmaccount.h"
#include <msn/msn.h>

class WlmContact;

class KOPETE_EXPORT WlmChatManager:public QObject
{
  Q_OBJECT public:
    WlmChatManager (WlmAccount * account);
    ~WlmChatManager ();
    WlmAccount *account ()
    {
        return m_account;
    }
    QMap < MSN::SwitchboardServerConnection *, WlmChatSession * >chatSessions;
    void requestDisplayPicture (QString contactId);
    void createChat (MSN::SwitchboardServerConnection * conn);

    private slots:void receivedMessage (MSN::SwitchboardServerConnection *
                                        conn, const QString & from,
                                        const Kopete::Message & message);

    void joinedConversation (MSN::SwitchboardServerConnection * conn,
                             const QString & passport,
                             const QString & friendlyname);

    void leftConversation (MSN::SwitchboardServerConnection * conn,
                           const QString & passport);

    void removeChatSession (QObject * obj);

    void gotNewSwitchboard (MSN::SwitchboardServerConnection * conn,
                            const void *tag);

    void SwitchboardServerConnectionTerminated (MSN::
                                                SwitchboardServerConnection *
                                                conn);

    void messageSentACK (MSN::SwitchboardServerConnection * conn,
                         const unsigned int &trID);

    void receivedNudge (MSN::SwitchboardServerConnection * conn,
                        const QString & passport);

    void receivedTypingNotification (MSN::SwitchboardServerConnection * conn,
                                     const QString & contactId);


  private:
    WlmAccount * m_account;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:
