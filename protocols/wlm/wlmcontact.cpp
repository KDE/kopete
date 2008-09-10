/*
    wlmcontact.cpp - Kopete Wlm Protocol

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

#include "wlmcontact.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KUrl>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"

#include "wlmaccount.h"
#include "wlmprotocol.h"

WlmContact::WlmContact (Kopete::Account * _account,
                        const QString & uniqueName,
                        const QString & contactSerial,
                        const QString & displayName,
                        Kopete::MetaContact * parent):
Kopete::Contact (_account, uniqueName, parent)
{
    kDebug (14210) << k_funcinfo << " uniqueName: " << uniqueName <<
        ", displayName: " << displayName;
    m_msgManager = 0L;
    setFileCapable (true);
    setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    m_contactSerial = contactSerial;
}

WlmContact::~WlmContact ()
{
}

bool
WlmContact::isReachable ()
{
	// always true, as we can send offline messages
    return true;
}

void
WlmContact::sendFile (const KUrl & sourceURL, const QString & fileName,
                      uint fileSize)
{
    QString filePath;

    if (!sourceURL.isValid ())
        filePath =
            KFileDialog::getOpenFileName (KUrl (), "*", 0l,
                                          i18n ("Kopete File Transfer"));
    else
        filePath = sourceURL.path (KUrl::RemoveTrailingSlash);

    if (!filePath.isEmpty ())
    {
        quint32 fileSize = QFileInfo (filePath).size ();
        //Send the file
        static_cast <WlmChatSession *>
			(manager (Kopete::Contact::CanCreate))->sendFile (filePath,
                                                               fileSize);
    }
}

void
WlmContact::serialize (QMap < QString, QString > &serializedData,
                       QMap < QString, QString > & /* addressBookData */ )
{
    QString value;
    serializedData["contactType"] = value;
    serializedData["displayPicture"] =
        property (Kopete::Global::Properties::self ()->photo ()).value ().
        toString ();
    serializedData["contactSerial"] = m_contactSerial;
}

Kopete::ChatSession *
    WlmContact::manager (Kopete::Contact::CanCreateFlags canCreate)
{
    Kopete::ContactPtrList chatmembers;
    chatmembers.append (this);

    Kopete::ChatSession * _manager =
        Kopete::ChatSessionManager::self ()->
				findChatSession (account ()->myself (), chatmembers, protocol ());
    WlmChatSession *manager = dynamic_cast <WlmChatSession *>(_manager);
    if (!manager && canCreate == Kopete::Contact::CanCreate)
    {
        manager =
            new WlmChatSession (protocol (), account ()->myself (),
                                chatmembers);
    }
    return manager;
}

QList < KAction * >*WlmContact::customContextMenuActions ()     //OBSOLETE
{
    //FIXME!!!  this function is obsolete, we should use XMLGUI instead
    return 0L;
}

void
WlmContact::showContactSettings ()
{
    //WlmContactSettings* p = new WlmContactSettings( this );
    //p->show();
}

void
WlmContact::sendMessage (Kopete::Message & message)
{
    kDebug (14210) << k_funcinfo;
    // give it back to the manager to display
    manager ()->appendMessage (message);
    // tell the manager it was sent successfully
    manager ()->messageSucceeded ();
}

void
WlmContact::deleteContact ()
{
    if (account ()->isConnected ())
    {
        dynamic_cast <WlmAccount *>(account ())->server ()->mainConnection->
            delFromAddressBook (m_contactSerial.toLatin1 ().data (),
                                contactId ().toLatin1 ().data ());
        deleteLater ();
    }
    else
    {
        KMessageBox::error (Kopete::UI::Global::mainWidget (),
                            i18n
                            ("<qt>You need to go online to remove a contact from your contact list. This contact will appear again when you reconnect.</qt>"),
                            i18n ("WLM Plugin"));
    }
}

void
WlmContact::receivedMessage (const QString & message)
{
    // Create a Kopete::Message
    Kopete::ContactPtrList contactList;
    account ();
    contactList.append (account ()->myself ());
    Kopete::Message newMessage (this, contactList);
    newMessage.setPlainBody (message);
    newMessage.setDirection (Kopete::Message::Inbound);

    // Add it to the manager
    manager ()->appendMessage (newMessage);
}

void
WlmContact::slotChatSessionDestroyed ()
{
    //FIXME: the chat window was closed?  Take appropriate steps.
    m_msgManager = 0L;
}

#include "wlmcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
