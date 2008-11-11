/*
 * msntest.cpp
 * libmsn
 *
 * Created by Meredydd Luff.
 * Refactored by Tiago Salem Herrmann
 * Copyright (c) 2004 Meredydd Luff. All rights reserved.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <msn/msn.h>
#include <string>
#include <iostream>

#include <QObject>
#include <QApplication>
#include <QPushButton>
#include <QList>
#include <QEventLoop>
#include <QSslSocket>

#include "kopetemessage.h"
#include "kopetecontact.h"
#include "kopeteuiglobal.h"

#include "wlmlibmsn.h"
#include "wlmserver.h"
#include "wlmaccount.h"

void
Callbacks::registerSocket (void *s, int reading, int writing, bool isSSL)
{
    WlmSocket *a = (WlmSocket*)s;
    if (!a)
        return;

    if (reading)
    {
        QObject::disconnect(a, SIGNAL (readyRead ()),0,0);
        QObject::connect (a, SIGNAL (readyRead ()), a,
                  SLOT (incomingData ()));
    }
}

void
Callbacks::closeSocket (void *s)
{
    WlmSocket *a = (WlmSocket*)s;
    if (a)
    {
        a->close ();
        socketList.removeAll (a);
    }
}

void
Callbacks::unregisterSocket (void *s)
{
    WlmSocket *a = (WlmSocket*)s;
    if (a)
    {
        QObject::disconnect(a, SIGNAL (readyRead ()),0,0);
    }
}

void
Callbacks::gotFriendlyName (MSN::NotificationServerConnection * conn,
                            std::string friendlyname)
{
    myFriendlyName = friendlyname.c_str ();
    emit gotDisplayName (friendlyname.c_str ());
}

void
Callbacks::fileTransferInviteResponse (MSN::SwitchboardServerConnection * conn, 
                            unsigned int sessionID, bool response)
{
    emit slotfileTransferInviteResponse (conn, sessionID, response);
}

void
Callbacks::gotContactDisplayPicture (MSN::SwitchboardServerConnection * conn,
                                     MSN::Passport passport,
                                     std::string filename)
{
    emit gotDisplayPicture (passport.c_str (), filename.c_str ());
}

void
Callbacks::gotMessageSentACK (MSN::SwitchboardServerConnection * conn,
                              int trID)
{
    emit messageSentACK (conn, trID);
}

void
Callbacks::gotBuddyListInfo (MSN::NotificationServerConnection * conn,
                             MSN::ListSyncInfo * info)
{
    // IMPORTANT
    // Here you need to fill a vector with all your contacts
    // both received by the server and previous ones.
    // Next pass this vector to the function completeConnection()
    // if you dont call completeConnection(), the service will
    // not work.
    std::map < std::string, MSN::Buddy * >::iterator i =
        info->contactList.begin ();
    std::map < std::string, int >allContacts;

    for (; i != info->contactList.end (); ++i)
    {
        MSN::Buddy * contact = (*i).second;
        if (contact->lists & MSN::LST_AB)       // only if it is the address book
        {
            allContacts[contact->userName.c_str ()] = 0;
            allContacts[contact->userName.c_str ()] += 1;
            std::list < MSN::Buddy::PhoneNumber >::iterator pns =
                contact->phoneNumbers.begin ();
            std::list < MSN::Group * >::iterator g = contact->groups.begin ();
        }
        if (contact->lists & MSN::LST_AL)
        {
            allContacts[contact->userName.c_str ()] += 2;
        }

        if (contact->lists & MSN::LST_BL)
        {
            allContacts[contact->userName.c_str ()] += 4;
        }

        if (contact->lists & MSN::LST_RL)
        {
            //printf ("-RL %s \n", contact->userName.c_str ());
        }
        if (contact->lists & MSN::LST_PL)
        {
            //printf ("-PL %s \n", contact->userName.c_str ());
        }
    }
    //printf ("Available Groups:\n");
    std::map < std::string, MSN::Group >::iterator g = info->groups.begin ();

    for (; g != info->groups.end (); g++)
    {
        //printf ("    %s: %s\n", (*g).second.groupID.c_str (),
        //        (*g).second.name.c_str ());
    }

    // this will send the ADL command to the server
    // It is necessary. Dont forget to add *all* your contacts to allContacts,
    // (both Forward, allow and block lists) or you probably will
    // loose someone.
    // A contact cannot be present both on allow and block lists or the
    // server will return an error, so you need to let your application
    // choose the better list to put it in.
    m_server->m_account->groupListReceivedFromServer (info->groups);
    m_server->m_account->addressBookReceivedFromServer (info->contactList);
    conn->completeConnection (allContacts, info);
}

void
Callbacks::gotLatestListSerial (MSN::NotificationServerConnection * conn,
                                std::string lastChange)
{
}

void
Callbacks::gotGTC (MSN::NotificationServerConnection * conn, char c)
{
}

void
Callbacks::gotOIMDeleteConfirmation (MSN::NotificationServerConnection * conn,
                                     bool success, std::string id)
{
    if (success)
    {
        emit deletedOIM (id.c_str (), success);
        std::cout << "OIM " << id << " removed sucessfully." << std::endl;
    }
    else
        std::cout << "OIM " << id << " not removed sucessfully." << std::endl;

}

void
Callbacks::gotOIMSendConfirmation (MSN::NotificationServerConnection * conn,
                                   bool success, int id)
{
    if (success)
        std::cout << "OIM " << id << " sent sucessfully." << std::endl;
    else
        std::cout << "OIM " << id << " not sent sucessfully." << std::endl;
}

void
Callbacks::gotOIM (MSN::NotificationServerConnection * conn, bool success,
                   std::string id, std::string message)
{
    if (success)
        emit receivedOIM (id.c_str (), message.c_str ());
    else
        std::cout << "Error retreiving OIM " << id << std::endl;
}

void
Callbacks::gotOIMList (MSN::NotificationServerConnection * conn,
                       std::vector < MSN::eachOIM > OIMs)
{
    emit receivedOIMList (OIMs);
}

void
Callbacks::connectionReady (MSN::Connection * conn)
{
    emit connectionCompleted ();
}

void
Callbacks::gotBLP (MSN::NotificationServerConnection * conn, char c)
{
}

void
Callbacks::addedListEntry (MSN::NotificationServerConnection * conn,
                           MSN::ContactList list, MSN::Passport username,
                           std::string friendlyname)
{
    QString username1 (username.c_str ());
    QString friendlyname1 (friendlyname.c_str ());

    emit gotNewContact (list, username1, friendlyname1);
    // after adding the user you need to delete it from the pending list.
    // it will be added automatically by the msn service

    // on regular lists you'll never receive the contacts displayname
    // it is not needed anyway
}

void
Callbacks::removedListEntry (MSN::NotificationServerConnection * conn,
                             MSN::ContactList list, MSN::Passport username)
{
    // list is a number which matches with MSN::ContactList enum on util.h
}

void
Callbacks::addedGroup (MSN::NotificationServerConnection * conn, bool added,
                       std::string groupName, std::string groupID)
{
/*    if (added)
        printf ("A group named %s (%s) was added\n", groupName.c_str (),
                groupID.c_str ());
    else
        printf ("Group (%s) was NOT added\n", groupName.c_str ());
*/
    emit gotAddedGroup (added, QString(groupName.c_str()),
                        QString(groupID.c_str()));
}

void
Callbacks::removedGroup (MSN::NotificationServerConnection * conn,
                         bool removed, std::string groupID)
{
/*
    if (removed)
        printf ("A group with ID %s was removed\n", groupID.c_str ());
    else
        printf ("Group (%s) was NOT removed\n", groupID.c_str ());
*/
    emit gotRemovedGroup (removed, QString(groupID.c_str()));
}

void
Callbacks::renamedGroup (MSN::NotificationServerConnection * conn,
                         bool renamed, std::string newGroupName,
                         std::string groupID)
{
/*
    if (renamed)
        printf ("A group with ID %s was renamed to %s\n", groupID.c_str (),
                newGroupName.c_str ());
    else
        printf ("A group with ID %s was NOT renamed to %s\n",
                groupID.c_str (), newGroupName.c_str ());
*/
}

void
Callbacks::showError (MSN::Connection * conn, std::string msg)
{
    std::cout << "MSN: Error: " << msg.c_str () << std::endl;
    QString a = msg.c_str ();
    // FIXME
    if (a.contains ("Wrong Password"))
    {
        emit wrongPassword ();
    }
}

void
Callbacks::buddyChangedStatus (MSN::NotificationServerConnection * conn,
                               MSN::Passport buddy, std::string friendlyname,
                               MSN::BuddyStatus status, unsigned int clientID,
                               std::string msnobject)
{
    emit contactChangedStatus (buddy, QString(friendlyname.c_str()), status, clientID,
                               QString(msnobject.c_str()));
}

void
Callbacks::buddyOffline (MSN::NotificationServerConnection * conn,
                         MSN::Passport buddy)
{
    emit contactDisconnected (buddy);
}

void
Callbacks::gotSwitchboard (MSN::SwitchboardServerConnection * conn,
                           const void *tag)
{
    emit gotNewSwitchboard (dynamic_cast <
                            MSN::SwitchboardServerConnection * >(conn), tag);
}

void
Callbacks::buddyJoinedConversation (MSN::SwitchboardServerConnection * conn,
                                    MSN::Passport username,
                                    std::string friendlyname, int is_initial)
{
    QString a (username.c_str ());
    QString b (friendlyname.c_str ());
    emit joinedConversation (conn, a, b);
    const std::pair < std::string,
      std::string > *ctx = static_cast < const std::pair < std::string,
      std::string > *>(conn->auth.tag);
    if (ctx)
        delete ctx;
    conn->auth.tag = NULL;

/*    if (conn->auth.tag)
    {
        const std::pair<std::string, std::string> *ctx = static_cast<const std::pair<std::string, std::string> *>(conn->auth.tag);
	// Example of sending a custom emoticon
//	conn->myNotificationServer()->msnobj.addMSNObject("/tmp/emoticon.gif",2);
//	std::string obj;
//	conn->myNotificationServer()->msnobj.getMSNObjectXML("/tmp/emoticon.gif", 2, obj);
//	conn->sendEmoticon("(EMOTICON)", obj);

	conn->sendMessage(ctx->second);
        delete ctx;
        conn->auth.tag = NULL;

        //Example of sending a file
//	MSN::fileTransferInvite ft;
//	ft.filename = "/tmp/filetosend.txt";
//	ft.friendlyname = "filetosend2.txt";
//	ft.sessionId = sessionID++;
//	ft.type = MSN::FILE_TRANSFER_WITHOUT_PREVIEW;
//	conn->sendFile(ft);

//	conn->sendNudge();
//	conn->sendAction("Action message here");

	// Exemple of requesting a display picture.
//	std::string filename2("/tmp/displayPicture.bin"+MSN::toStr(sessionID));
	// lastObject is the msnobject received on each contact status change
	// you should generate a random sessionID
//	conn->requestFile(sessionID++, filename2, lastObject);

	// Example of sending a voice clip
//	conn->myNotificationServer()->msnobj.addMSNObject("/tmp/voiceclip.wav",11);
//	std::string obj;
//	conn->myNotificationServer()->msnobj.getMSNObjectXML("/tmp/voiceclip.wav", 11, obj);
//	conn->sendVoiceClip(obj);
	// exemple of sending an ink
//	std::string ink("base64 data here...");
//	conn->sendInk(ink);
    }
    */
}

void
Callbacks::buddyLeftConversation (MSN::SwitchboardServerConnection * conn,
                                  MSN::Passport username)
{
    QString a (username.c_str ());
    emit leftConversation (conn, a);

}

void
Callbacks::gotInstantMessage (MSN::SwitchboardServerConnection * conn,
                              MSN::Passport username,
                              std::string friendlyname, MSN::Message * msg)
{
    QString a = username.c_str ();
    Kopete::Message kmsg;
    kmsg.setPlainBody (msg->getBody ().c_str ());
    QFont font (msg->getFontName ().c_str ());
    if (msg->getFontEffects () & MSN::Message::BOLD_FONT)
        font.setBold (true);
    if (msg->getFontEffects () & MSN::Message::ITALIC_FONT)
        font.setItalic (true);
    if (msg->getFontEffects () & MSN::Message::UNDERLINE_FONT)
        font.setUnderline (true);
    if (msg->getFontEffects () & MSN::Message::STRIKETHROUGH_FONT)
        font.setStrikeOut (true);

    QColor color (msg->getColor ()[0], msg->getColor ()[1],
                  msg->getColor ()[2]);
    kmsg.setForegroundColor (color);

    kmsg.setFont (font);
    emit messageReceived (conn, a, kmsg);
}

void
Callbacks::gotEmoticonNotification (MSN::SwitchboardServerConnection * conn,
                                    MSN::Passport username, std::string alias,
                                    std::string msnobject)
{
    emit slotGotEmoticonNotification(conn, username, QString(alias.c_str()), QString(msnobject.c_str()));
}

void
Callbacks::failedSendingMessage (MSN::Connection * conn)
{
    //printf ("**************************************************\n");
    //printf ("ERROR:  Your last message failed to send correctly\n");
    //printf ("**************************************************\n");
}

void
Callbacks::buddyTyping (MSN::SwitchboardServerConnection * conn,
                        MSN::Passport username, std::string friendlyname)
{
    QString userid (username.c_str ());
    emit receivedTypingNotification (conn, userid);

}

void
Callbacks::gotNudge (MSN::SwitchboardServerConnection * conn,
                     MSN::Passport username)
{
    emit receivedNudge (conn, username.c_str ());
}

void
Callbacks::gotVoiceClipNotification (MSN::SwitchboardServerConnection * conn,
                         MSN::Passport username, std::string msnobject)
{
    emit slotGotVoiceClipNotification(conn, username, QString(msnobject.c_str()));
}

void
Callbacks::gotWinkNotification (MSN::SwitchboardServerConnection * conn,
                    MSN::Passport username, std::string msnobject)
{
    emit slotGotWinkNotification(conn, username, QString(msnobject.c_str()));
}

void
Callbacks::gotInk (MSN::SwitchboardServerConnection * conn,
                   MSN::Passport username, std::string image)
{
    emit slotGotInk(conn, username, QString(image.c_str()));
}

void
Callbacks::gotActionMessage (MSN::SwitchboardServerConnection * conn,
                             MSN::Passport username, std::string message)
{
}

void
Callbacks::gotInitialEmailNotification (MSN::NotificationServerConnection *
                                        conn, int msgs_inbox,
                                        int unread_inbox, int msgs_folders,
                                        int unread_folders)
{
/*
    if (unread_inbox > 0)
        printf ("You have %d new messages in your Inbox. Total: %d\n",
                unread_inbox, msgs_inbox);

    if (unread_folders > 0)
        printf ("You have %d new messages in other folders. Total: %d\n",
                unread_folders, msgs_folders);
*/
}

void
Callbacks::gotNewEmailNotification (MSN::NotificationServerConnection * conn,
                                    std::string from, std::string subject)
{
//    printf ("New e-mail has arrived from %s.\nSubject: %s\n", from.c_str (),
//            subject.c_str ());
}

void
Callbacks::fileTransferProgress (MSN::SwitchboardServerConnection * conn,
                                 unsigned int sessionID,
                                 unsigned long long transferred,
                                 unsigned long long total)
{
    emit gotFileTransferProgress (conn, sessionID, transferred);
}

void
Callbacks::fileTransferFailed (MSN::SwitchboardServerConnection * conn,
                               unsigned int sessionID, MSN::fileTransferError error)
{
    emit gotFileTransferFailed (conn, sessionID, error);
}

void
Callbacks::fileTransferSucceeded (MSN::SwitchboardServerConnection * conn,
                               unsigned int sessionID)
{
    //printf ("File transfer successfully completed. session: %d\n", sessionID);
    emit gotFileTransferSucceeded (conn, sessionID);
}

void
Callbacks::gotNewConnection (MSN::Connection * conn)
{
    if (dynamic_cast < MSN::NotificationServerConnection * >(conn))
        dynamic_cast <MSN::NotificationServerConnection *>(conn)->synchronizeContactList ();
}

void
Callbacks::buddyChangedPersonalInfo (MSN::NotificationServerConnection * conn,
                                     MSN::Passport fromPassport,
                                     MSN::personalInfo pInfo)
{
    // MSN::personalInfo shows all the data you can grab from the contact
    //printf ("User %s Personal Message: %s\n", fromPassport.c_str (),
    //        pInfo.PSM.c_str ());
    emit gotContactPersonalInfo (fromPassport, pInfo);
}

void
Callbacks::closingConnection (MSN::Connection * conn)
{
    if (dynamic_cast < MSN::SwitchboardServerConnection * >(conn))
        emit SwitchboardServerConnectionTerminated (
                dynamic_cast <MSN::SwitchboardServerConnection* >(conn));
    if (dynamic_cast < MSN::NotificationServerConnection * >(conn))
        emit NotificationServerConnectionTerminated (
                dynamic_cast <MSN::NotificationServerConnection* >(conn));
}

void
Callbacks::changedStatus (MSN::NotificationServerConnection * conn,
                          MSN::BuddyStatus state)
{
    //printf ("Your state is now: %s\n",
    //        MSN::buddyStatusToString (state).c_str ());
    emit changedStatus (state);
/*  MSN::personalInfo pInfo;
    pInfo.PSM="my personal message";
    pInfo.mediaType="Music";
    pInfo.mediaIsEnabled=1;
    pInfo.mediaFormat="{0} - {1}";
    pInfo.mediaLines.push_back("Artist");
    pInfo.mediaLines.push_back("Song");
    conn->setPersonalStatus(pInfo);
*/
}

size_t 
Callbacks::getDataFromSocket (void *sock, char *data, size_t size)
{
    WlmSocket *a = (WlmSocket*)sock;
    if (!a)
        return 0;

    return a->read(data, size);
}

size_t 
Callbacks::writeDataToSocket (void *sock, char *data, size_t size)
{
    WlmSocket *a = (WlmSocket*)sock;
    if (!a)
        return 0;

    return a->write(data, size);
}

void *
Callbacks::connectToServer (std::string hostname, int port, bool * connected, bool isSSL)
{
    WlmSocket *a = new WlmSocket (mainConnection, isSSL);
    if(!a)
        return NULL;

    connect( a, SIGNAL( sslErrors(const QList<QSslError> &) ), a, SLOT(
                      ignoreSslErrors() ) );

    if(!isSSL)
        a->connectToHost (hostname.c_str (), port);
    else
        a->connectToHostEncrypted (hostname.c_str (), port);

    *connected = false;
    socketList.append (a);
    return (void*)a;
}

int
Callbacks::listenOnPort (int port)
{
    int s;
    return s;
}

std::string Callbacks::getOurIP (void)
{
}

void
Callbacks::log (int i, const char *s)
{

}

int
Callbacks::getSocketFileDescriptor (void *sock)
{
    WlmSocket *a = (WlmSocket*)sock;
    if(!a)
        return -1;
    return a->socketDescriptor();
}

std::string Callbacks::getSecureHTTPProxy ()
{
    return "";
}

void
Callbacks::askFileTransfer (MSN::SwitchboardServerConnection * conn,
                            MSN::fileTransferInvite ft)
{
    emit incomingFileTransfer (conn, ft);
/*
	switch(ft.type)
	{
		case MSN::FILE_TRANSFER_BACKGROUND_SHARING:
			printf("User %s wants to share with you a background file named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
			break;
		case MSN::FILE_TRANSFER_BACKGROUND_SHARING_CUSTOM:
			printf("User %s wants to share with you a *custom background file named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
			break;
		case MSN::FILE_TRANSFER_WITH_PREVIEW:
			printf("User %s wants to send you a file *with preview named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
			// ft.preview has the base64 encoded png file
			break;
		case MSN::FILE_TRANSFER_WITHOUT_PREVIEW:
			printf("User %s wants to send you a file *without preview named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
			break;
		default:
			printf("Unknown filetransfer type from %s..\n", ft.userPassport.c_str());

	}
	conn->fileTransferResponse(ft.sessionId, filename2, true);
*/
}

void
Callbacks::addedContactToGroup (MSN::NotificationServerConnection * conn,
                                bool added, std::string groupId,
                                std::string contactId)
{
/*
    if (added)
        printf ("User Id (%s) added to group Id (%s)\n", contactId.c_str (),
                groupId.c_str ());
    else
        printf ("User Id (%s) NOT added to group Id (%s)\n",
                contactId.c_str (), groupId.c_str ());
*/
    emit gotAddedContactToGroup (added, QString(groupId.c_str()),
                                 QString(contactId.c_str()));
}

void
Callbacks::removedContactFromGroup (MSN::NotificationServerConnection * conn,
                                    bool removed, std::string groupId,
                                    std::string contactId)
{
/*
    if (removed)
        printf ("User Id (%s) removed from group Id (%s)\n",
                contactId.c_str (), groupId.c_str ());
    else
        printf ("User Id (%s) NOT removed from group Id (%s)\n",
                contactId.c_str (), groupId.c_str ());
*/
    emit gotRemovedContactFromGroup (removed, QString(groupId.c_str()),
                                     QString(contactId.c_str()));
}

void
Callbacks::addedContactToAddressBook (MSN::NotificationServerConnection *
                                      conn, bool added, std::string passport,
                                      std::string displayName,
                                      std::string guid)
{
/*
    if (added)
        printf ("User (%s - %s) added to AddressBook. Guid (%s)\n",
                passport.c_str (), displayName.c_str (), guid.c_str ());
    else
        printf ("User (%s - %s) NOT added to AddressBook.\n",
                passport.c_str (), displayName.c_str ());
*/
    emit gotAddedContactToAddressBook (added, QString(passport.c_str()), 
            QString(displayName.c_str()), QString(guid.c_str()));
}

void
Callbacks::removedContactFromAddressBook (MSN::NotificationServerConnection *
                                          conn, bool removed,
                                          std::string contactId,
                                          std::string passport)
{
/*
    if (removed)
        printf ("User %s removed from AddressBook. Guid (%s)\n",
                passport.c_str (), contactId.c_str ());
    else
        printf ("User %s NOT removed from AddressBook. Guid (%s)\n",
                passport.c_str (), contactId.c_str ());
*/
    emit gotRemovedContactFromAddressBook (removed, QString(passport.c_str()),
                                           QString(contactId.c_str()));
}

void
Callbacks::enabledContactOnAddressBook (MSN::NotificationServerConnection *
                                        conn, bool enabled,
                                        std::string contactId,
                                        std::string passport)
{
/*
    // this is used to enable a contact previously disabled from msn, but not fully removed
    if (enabled)
        printf ("User (%s) enabled on AddressBook. Guid (%s)\n",
                passport.c_str (), contactId.c_str ());
    else
        printf ("User (%s) NOT enabled on AddressBook. Guid (%s)\n",
                passport.c_str (), contactId.c_str ());
*/
}

void
Callbacks::disabledContactOnAddressBook (MSN::NotificationServerConnection *
                                         conn, bool disabled,
                                         std::string contactId)
{
    // this is used when you have disabled this user from msn, but not deleted from hotmail
    // I suggest to delete the contact instead of disable, since I haven't tested this too much yet
/*
    if (disabled)
        printf ("User disabled on AddressBook. Guid (%s)\n",
                contactId.c_str ());
    else
        printf ("User NOT disabled on AddressBook. Guid (%s)\n",
                contactId.c_str ());
*/
}

void Callbacks::gotVoiceClipFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file)
{
    emit slotGotVoiceClipFile(conn, sessionID, QString(file.c_str()));
}

void Callbacks::gotEmoticonFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string alias, std::string file)
{
    emit slotGotEmoticonFile(conn, sessionID, QString(alias.c_str()), QString(file.c_str()));
}

void Callbacks::gotWinkFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file)
{
    emit slotGotWinkFile(conn, sessionID, QString(file.c_str()));
}

#include "wlmlibmsn.moc"
