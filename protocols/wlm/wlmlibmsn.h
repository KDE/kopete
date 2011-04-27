/*
    wlmlibmsn.h - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef WLMLIBMSN_H
#define WLMLIBMSN_H

#include <QObject>
#include "wlmsocket.h"
#include "kopetemessage.h"

#include <msn/msn.h>

class WlmAccount;
class WlmServer;

namespace WlmUtils {
// MSN::Passport/std::string to QString encoding functions
QString passport(const MSN::Passport& pass);
QString latin1(const std::string& s);
QString utf8(const std::string& s);
}

class Callbacks:public QObject, public MSN::Callbacks
{
    Q_OBJECT

private:
    virtual void registerSocket (void *s, int read, int write, bool isSSL);

    virtual void unregisterSocket (void *s);

    virtual void closeSocket (void *s);

    virtual void showError (MSN::Connection * conn, std::string msg);

    virtual void buddyChangedStatus (MSN::NotificationServerConnection * conn,
                                     MSN::Passport buddy, std::string friendlyname,
                                     MSN::BuddyStatus state, unsigned int clientID,
                                     std::string msnobject);

    virtual void buddyOffline (MSN::NotificationServerConnection * conn, MSN::Passport buddy);

    virtual void log (int writing, const char *buf);

    virtual void buddyChangedPersonalInfo (MSN::NotificationServerConnection * conn,
                                           MSN::Passport fromPassport, MSN::personalInfo);

    virtual void gotMessageSentACK (MSN::SwitchboardServerConnection * conn, int trID);

    virtual void gotFriendlyName (MSN::NotificationServerConnection * conn, std::string friendlyname);

    virtual void gotBuddyListInfo (MSN::NotificationServerConnection * conn, MSN::ListSyncInfo * data);

    virtual void gotLatestListSerial (MSN::NotificationServerConnection * conn, std::string lastChange);

    virtual void gotGTC (MSN::NotificationServerConnection * conn, char c);

    virtual void gotBLP (MSN::NotificationServerConnection * conn, char c);

    virtual void gotContactDisplayPicture (MSN::SwitchboardServerConnection *,
                                           MSN::Passport, std::string);

    virtual void addedListEntry (MSN::NotificationServerConnection * conn,
                                 MSN::ContactList list, MSN::Passport buddy,
                                 std::string friendlyname);

    virtual void removedListEntry (MSN::NotificationServerConnection * conn,
                                   MSN::ContactList list, MSN::Passport buddy);

    virtual void addedGroup (MSN::NotificationServerConnection * conn, bool added,
                             std::string groupName, std::string groupID);

    virtual void removedGroup (MSN::NotificationServerConnection * conn, bool removed,
                               std::string groupID);

    virtual void renamedGroup (MSN::NotificationServerConnection * conn, bool renamed,
                               std::string newGroupName, std::string groupID);

    virtual void addedContactToGroup (MSN::NotificationServerConnection * conn, bool added,
                                      std::string groupId, std::string contactId);

    virtual void removedContactFromGroup (MSN::NotificationServerConnection * conn,
                                          bool removed, std::string groupId,
                                          std::string contactId);

    virtual void addedContactToAddressBook (MSN::NotificationServerConnection * conn,
                                            bool added, std::string passport,
                                            std::string displayName, std::string guid);

    virtual void removedContactFromAddressBook (MSN::NotificationServerConnection * conn,
                                                bool removed, std::string contactId,
                                                std::string passport);

    virtual void fileTransferInviteResponse (MSN::SwitchboardServerConnection * conn,
                                             unsigned int sessionID, bool response);

    virtual void enabledContactOnAddressBook (MSN::NotificationServerConnection * conn,
                                              bool enabled, std::string contactId,
                                              std::string passport);

    virtual void disabledContactOnAddressBook (MSN::NotificationServerConnection * conn,
                                               bool disabled, std::string contactId);

    virtual void gotSwitchboard (MSN::SwitchboardServerConnection * conn, const void *tag);

    virtual void buddyJoinedConversation (MSN::SwitchboardServerConnection * conn,
                                          MSN::Passport buddy, std::string friendlyname,
                                          int is_initial);

    virtual void buddyLeftConversation (MSN::SwitchboardServerConnection * conn,
                                        MSN::Passport buddy);

    virtual void gotInstantMessage (MSN::SwitchboardServerConnection * conn,
                                    MSN::Passport buddy, std::string friendlyname,
                                    MSN::Message * msg);

    virtual void gotEmoticonNotification (MSN::SwitchboardServerConnection * conn,
                                          MSN::Passport buddy, std::string alias,
                                          std::string msnobject);

    virtual void failedSendingMessage (MSN::Connection * conn);

    virtual void buddyTyping (MSN::SwitchboardServerConnection * conn, MSN::Passport buddy,
                              std::string friendlyname);

    virtual void gotNudge (MSN::SwitchboardServerConnection * conn, MSN::Passport from);

    virtual void gotVoiceClipNotification (MSN::SwitchboardServerConnection * conn, MSN::Passport from,
                                           std::string msnobject);

    virtual void gotWinkNotification (MSN::SwitchboardServerConnection * conn, MSN::Passport from,
                                      std::string msnobject);

    virtual void gotInk (MSN::SwitchboardServerConnection * conn, MSN::Passport from,
                         std::string image);

    virtual void gotActionMessage (MSN::SwitchboardServerConnection * conn,
                                   MSN::Passport username, std::string message);

    virtual void gotInitialEmailNotification (MSN::NotificationServerConnection * conn,
                                              int msgs_inbox, int unread_inbox,
                                              int msgs_folders, int unread_folders);

    virtual void gotNewEmailNotification (MSN::NotificationServerConnection * conn,
                                          std::string from, std::string subject);

    virtual void fileTransferProgress (MSN::SwitchboardServerConnection * conn,
                                       unsigned int sessionID,
                                       long long unsigned transferred,
                                       long long unsigned total);

    virtual void fileTransferFailed (MSN::SwitchboardServerConnection * conn,
                                     unsigned int sessionID, MSN::fileTransferError error);

    virtual void fileTransferSucceeded (MSN::SwitchboardServerConnection * conn,
                                        unsigned int sessionID);

    virtual void gotNewConnection (MSN::Connection * conn);

    virtual void gotOIMList (MSN::NotificationServerConnection * conn,
                             std::vector < MSN::eachOIM > OIMs);

    virtual void gotOIM (MSN::NotificationServerConnection * conn, bool success,
                         std::string id, std::string message);

    virtual void gotOIMSendConfirmation (MSN::NotificationServerConnection * conn,
                                         bool success, int id);

    virtual void gotOIMDeleteConfirmation (MSN::NotificationServerConnection * conn,
                                           bool success, std::string id);

    virtual void closingConnection (MSN::Connection * conn);

    virtual void changedStatus (MSN::NotificationServerConnection * conn,
                                MSN::BuddyStatus state);

    virtual void gotVoiceClipFile(MSN::SwitchboardServerConnection * conn,
                                  unsigned int sessionID,
                                  std::string file);

    virtual void gotEmoticonFile(MSN::SwitchboardServerConnection * conn,
                                 unsigned int sessionID,
                                 std::string alias,
                                 std::string file);

    virtual void gotWinkFile(MSN::SwitchboardServerConnection * conn,
                             unsigned int sessionID,
                             std::string file);

    virtual void* connectToServer (std::string server, int port, bool * connected, bool isSSL);

    virtual size_t getDataFromSocket (void *sock, char *data, size_t size);

    virtual size_t writeDataToSocket (void *sock, char *data, size_t size);

    virtual void connectionReady (MSN::Connection * conn);

    virtual void askFileTransfer (MSN::SwitchboardServerConnection * conn,
                                  MSN::fileTransferInvite ft);

    virtual int listenOnPort (int port);

    int getSocketFileDescriptor (void *sock);

    virtual std::string getOurIP ();

    virtual std::string getSecureHTTPProxy ();

    virtual void gotInboxUrl (MSN::NotificationServerConnection *conn, MSN::hotmailInfo info);

  public:
    WlmServer * m_server;
    QList <WlmSocket*> socketList;
    MSN::NotificationServerConnection * mainConnection;
    enum MSNErrorCode {
        NoError = 0,
        WrongPassword,
        OtherClient,
        Unknown
    };

  signals:
    void messageReceived (MSN::SwitchboardServerConnection * conn,
                          const QString & from, const Kopete::Message & message);
    void joinedConversation (MSN::SwitchboardServerConnection * conn,
                             const QString & user, const QString & friendlyname);
    void leftConversation (MSN::SwitchboardServerConnection * conn,
                           const QString & user);
    void gotNewSwitchboard (MSN::SwitchboardServerConnection * conn,
                            const void *tag);
    void SwitchboardServerConnectionTerminated (MSN::SwitchboardServerConnection *conn);
    void NotificationServerConnectionTerminated (MSN::NotificationServerConnection *conn);
    void messageSentACK (MSN::SwitchboardServerConnection * conn, const unsigned int &trID);
    void incomingFileTransfer (MSN::SwitchboardServerConnection * conn, const MSN::fileTransferInvite & ft);
    void gotFileTransferProgress (MSN::SwitchboardServerConnection * conn, const unsigned int &sessionID,
                                  const unsigned long long &transferred);
    void gotFileTransferFailed (MSN::SwitchboardServerConnection * conn,
                                const unsigned int &sessionID,
                                const MSN::fileTransferError & error);

    void gotFileTransferSucceeded (MSN::SwitchboardServerConnection * conn,
                                   const unsigned int &sessionID);

    void gotDisplayName (const QString & displayName);

    void gotDisplayPicture (const QString & contact, const QString & filename);

    void gotNewContact (const MSN::ContactList & list, const QString & contact,
                        const QString & friendlyname);

    void gotRemovedContactFromList (const MSN::ContactList & list, const QString & contact);

    void gotAddedGroup (bool added, const QString & groupName, const QString & groupId);

    void gotRemovedGroup (bool removed, const QString & groupId);

    void gotAddedContactToGroup (bool added, const QString & groupId, const QString & contactId);

    void gotRemovedContactFromGroup (bool removed, const QString & groupId, const QString & contactId);

    void gotAddedContactToAddressBook (bool added, const QString & passport, const QString & displayName,
                                       const QString & guid);

    void gotRemovedContactFromAddressBook (bool removed, const QString & passport, const QString & contactId);

    void receivedNudge (MSN::SwitchboardServerConnection * conn, const QString & contactId);

    void receivedTypingNotification (MSN::SwitchboardServerConnection * conn, const QString & contactId);

    void gotContactPersonalInfo (const QString & fromPassport, const MSN::personalInfo & pInfo);

    void receivedOIMList (std::vector < MSN::eachOIM > &oimlist);

    void receivedOIM (const QString & id, const QString & message);

    void deletedOIM (const QString & id, const bool & deleted);

    void contactChangedStatus (const QString & buddy, const QString & friendlyname,
                               const MSN::BuddyStatus & status, const unsigned int &clientID, const QString & msnobject);

    void contactDisconnected (const QString & buddy);

    void connectionCompleted ();

    void connectionFailed ();

    void changedStatus (MSN::BuddyStatus & state);

    void slotfileTransferInviteResponse (MSN::SwitchboardServerConnection * conn,
                                         const unsigned int &sessionID,
                                         const bool & response);
    void slotGotEmoticonNotification (MSN::SwitchboardServerConnection * conn,
                                      const QString & buddy,
                                      const QString & alias,
                                      const QString & msnobject);

    void slotGotVoiceClipNotification (MSN::SwitchboardServerConnection * conn,
                                       const QString & from,
                                       const QString & msnobject);

    void slotGotWinkNotification (MSN::SwitchboardServerConnection * conn,
                                  const QString & from,
                                  const QString & msnobject);

    void slotGotInk (MSN::SwitchboardServerConnection * conn,
                     const QString & from,
                     const QByteArray & image);

    void slotGotVoiceClipFile(MSN::SwitchboardServerConnection * conn,
                              const unsigned int & sessionID,
                              const QString & file);

    void slotGotEmoticonFile(MSN::SwitchboardServerConnection * conn,
                             const unsigned int & sessionID,
                             const QString & alias,
                             const QString & file);

    void slotGotWinkFile(MSN::SwitchboardServerConnection * conn,
                         const unsigned int & sessionID,
                         const QString & file);

    void mainConnectionError( int error );

    void socketError( int error );

    void initialEmailNotification(const int unread_inbox);

    void newEmailNotification(const QString from, const QString subject);

    void inboxUrl(MSN::hotmailInfo & info);

private slots:
    void emitSocketError( QAbstractSocket::SocketError error );
};

#endif
