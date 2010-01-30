/*
    wlmaccount.h - Kopete Wlm Protocol

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

#ifndef WLMACCOUNT_H
#define WLMACCOUNT_H

#include <ktemporaryfile.h>
#include <kopeteaccount.h>
#include "kopetechatsessionmanager.h"
#include "kopetepasswordedaccount.h"
#include "wlmserver.h"
#include "wlmchatsession.h"
#include "wlmtransfermanager.h"
#include "wlmchatmanager.h"

class KActionMenu;
namespace Kopete
{
    class Contact;
}
namespace Kopete
{
    class MetaContact;
}

class WlmContact;
class WlmProtocol;
class WlmServer;
class WlmTransferManager;
class WlmChatManager;

/**
 * This represents an account connected to the wlm
 * @author Will Stephenson
*/
class WlmAccount:public
    Kopete::PasswordedAccount
{
    Q_OBJECT
  public:
    WlmAccount (WlmProtocol * parent, const QString & accountID);
     ~
    WlmAccount ();
        /**
	 * Construct the context menu used for the status bar icon
	 */
    virtual void
    fillActionMenu (KActionMenu * actionMenu);

        /**
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplie
	 * Kopete::MetaContact
	 */
    virtual bool
    createContact (const QString & contactId,
                   Kopete::MetaContact * parentContact);
        /**
	 * Called when Kopete status is changed globally
	 */
    virtual void
    setOnlineStatus (const Kopete::OnlineStatus & status,
                     const Kopete::StatusMessage & reason = Kopete::StatusMessage (),
                     const OnlineStatusOptions& options = None);
    virtual void
    setStatusMessage (const Kopete::StatusMessage & statusMessage);
        /**
	 * 'Connect' to the wlm server.  Only sets myself() online.
	 */
//      virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus::OnlineStatus() );
        /**
	 * Disconnect from the server.  Only sets myself() offline.
	 */
    virtual void disconnect ();

    void logOff( Kopete::Account::DisconnectReason reason );

    /**
     * Return a reference to the server stub
     */
    virtual void connectWithPassword (const QString & password);

    QString serverName() const;
    uint serverPort() const;

    bool isProxyEnabled() const;
    QString proxyHost() const;
    uint proxyPort() const;
    QString proxyUsername() const;
    QString proxyPassword() const;
    uint proxyType() const;
    bool doNotRequestEmoticons() const;
    bool doNotSendEmoticons() const;

    WlmServer * server ();

    WlmChatManager * chatManager ()
    {
        return m_chatManager;
    }
    WlmTransferManager * transferManager ()
    {
        return m_transferManager;
    }

    // TODO: Check BPL
    bool blockUnknownUsers() const { return true; }

    bool isContactBlocked(const QString& passport) const;

    void blockContact(const QString& passport, bool block);

    bool isOnAllowList(const QString& passport) const { return m_allowList.contains( passport ); }

    bool isOnBlockList(const QString& passport) const { return m_blockList.contains( passport ); }

    bool isOnPendingList(const QString& passport) const { return m_pendingList.contains( passport ); }
    
    bool isOnReverseList(const QString& passport) const { return m_reverseList.contains( passport ); }
    
    // forward list (or also called address book)
    bool isOnServerSideList(const QString& passport) const { return m_serverSideContactsPassports.contains( passport ); }

    QSet<QString> allowList() const { return m_allowList; }

    QSet<QString> blockList() const { return m_blockList; }
    
    QSet<QString> pendingList() const { return m_pendingList; }

    QSet<QString> serverSideContacts() const { return m_serverSideContactsPassports; }
    
    QMap<QString, QString> groupToGroupId() const { return m_groupToGroupId; }

public slots:

    void error( int errCode );

    /**
     * Called by the server when it has a message for us.
     * This identifies the sending Kopete::Contact and passes it a Kopete::Message
     */
    void
    contactChangedStatus (const QString & buddy,
                          const QString & friendlyname,
                          const MSN::BuddyStatus & status,
                          const unsigned int &clientID,
                          const QString & msnobject);

    void
    contactDisconnected (const QString & buddy);

    void
    connectionCompleted ();

    void
    connectionFailed ();

    void
    changedStatus (MSN::BuddyStatus & state);

    void
    slotGlobalIdentityChanged (Kopete::PropertyContainer *,
                               const QString & key, const QVariant &,
                               const QVariant & newValue);

    void setPersonalMessage (const Kopete::StatusMessage & reason);

    void
    addressBookReceivedFromServer (std::map < std::string,
                                   MSN::Buddy * >&list);

    void
    groupListReceivedFromServer (std::map < std::string, MSN::Group > &list);

    void
    gotDisplayName (const QString & displayName);

    void
    gotDisplayPicture (const QString & contactId, const QString & filename);

    void
    gotNewContact (const MSN::ContactList & list, const QString & contact,
                   const QString & friendlyname);

    void gotRemovedContactFromList (const MSN::ContactList & list, const QString & contact);

    void
    receivedOIMList (std::vector < MSN::eachOIM > &oimlist);

    void
    receivedOIM (const QString & id, const QString & message);

    void
    gotContactPersonalInfo (const QString & fromPassport,
                            const MSN::personalInfo & pInfo);

    void
    NotificationServerConnectionTerminated (MSN::
                                            NotificationServerConnection *
                                            conn);

    void mainConnectionError (int errorCode);

    void
    scheduleConnect ();

    void gotAddedGroup (bool added,
                        const QString & groupName,
                        const QString & groupId);

    void gotRemovedGroup (bool removed,
                            const QString & groupId);

    void
    gotAddedContactToGroup (bool added,
                            const QString & groupId,
                            const QString & contactId);

    void
    gotRemovedContactFromGroup (bool removed,
                                const QString & groupId,
                                const QString & contactId);

    void
    gotAddedContactToAddressBook (bool added,
                                  const QString & passport,
                                  const QString & displayName,
                                  const QString & guid);

    void
    gotRemovedContactFromAddressBook (bool removed,
                                          const QString & passport,
                                          const QString & contactId);
    void
    deletedOIM(const QString& id, const bool deleted);

    void
    downloadPendingDisplayPicture();

    void
    slotInitialEmailNotification(const int unread_inbox);

    void
    slotNewEmailNotification(const QString from, const QString subject);

    void
    slotInboxUrl(MSN::hotmailInfo & info);

  protected:
        /**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
    void
    updateContactStatus ();

    WlmServer *
        m_server;

    WlmTransferManager *
        m_transferManager;

    WlmChatManager *
        m_chatManager;

    protected
        slots:
        /**
	 * Change the account's status.  Called by KActions and internally.
	 */
        void
    slotGoOnline ();
        /**
	 * Change the account's status.  Called by KActions and internally.
	 */
    void
    slotGoAway (const Kopete::OnlineStatus & status);
        /**
	 * Change the account's status.  Called by KActions and internally.
	 */
    void
    slotGoOffline ();

    void
    slotGoInvisible ();

    void
    disableInitialList ()
    {
        m_initialList = false;
    }

    void
    enableInitialList ()
    {
        m_initialList = true;
    }

    bool
    isInitialList ()
    {
        return m_initialList;
    }

private slots:
    void addedInfoEventActionActivated(uint actionId);
//     void slotStartChat();
    void slotOpenInbox();
    void slotChangePublicName();
    void slotOpenStatus();
    void slotRemoveTmpMailFile();
    void slotRemoveRecentDPRequests();

private:
    Kopete::OnlineStatus temporaryStatus;

    KAction *m_openInboxAction;
//     KAction *m_startChatAction;
    KAction *m_changeDNAction;
    KAction *m_openStatusAction;

    QString
        m_pictureFilename;

    bool
        m_initialList;

    uint
        clientid;

    QMap < QString, QString > m_oimList;

    //contacts waiting on their group to be added
    QMap<QString, QString> m_contactAddQueue;

    //group name to group id map
    QMap<QString, QString> m_groupToGroupId;

    // passport set of contacts which are stored on server
    QSet<QString> m_serverSideContactsPassports;

    // passport set of contacts which are on allow list
    QSet<QString> m_allowList;

    // passport set of contacts which are on block list
    QSet<QString> m_blockList;

    // passport set of contacts which are on pending list
    QSet<QString> m_pendingList;

    // passport set of contacts which are on reverse list
    QSet<QString> m_reverseList;

    // passport set of contacts which we do not have the display picture yet
    QSet<QString> m_pendingDisplayPictureList;

    QTimer * m_pendingDisplayPicturesTimer;

    KTemporaryFile * tmpMailFile;

    QTimer * m_tmpMailFileTimer;

    QStringList m_recentDPRequests;

    int m_lastMainConnectionError;
};

#endif
