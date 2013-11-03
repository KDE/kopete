/*
    wlmcontact.h - Kopete Wlm Protocol

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

#ifndef WLMCONTACT_H
#define WLMCONTACT_H

#include <QMap>
#include <QList>
#include <kaction.h>
#include <kurl.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

#include "wlmchatsession.h"

class KAction;
class KActionCollection;
class KToggleAction;
namespace Kopete
{
    class Account;
}
namespace Kopete
{
    class ChatSession;
}
namespace Kopete
{
    class MetaContact;
}

class WlmAccount;

/**
@author Will Stephenson
*/
class WlmContact : public Kopete::Contact
{
    Q_OBJECT
public:
    WlmContact (Kopete::Account * _account, const QString & uniqueName,
                const QString & contactSerial, Kopete::MetaContact * parent);

    ~WlmContact ();

    virtual bool isReachable ();
    /**
	 * Serialize the contact's data into a key-value map
	 * suitable for writing to a file
	 */
    virtual void serialize (QMap < QString, QString > &serializedData,
               QMap < QString, QString > &addressBookData);
    /**
	 * Return the actions for this contact
	 */
    virtual QList <KAction *>*customContextMenuActions ();
    using Kopete::Contact::customContextMenuActions;
        /**
	 * Returns a Kopete::ChatSession associated with this contact
	 */
    virtual Kopete::ChatSession * manager (CanCreateFlags canCreate = CannotCreate);

    void setContactSerial (QString contactSerial) { m_contactSerial = contactSerial; }

    QString contactSerial () const { return m_contactSerial; }
    Kopete::Group* currentGroup () const { return m_currentGroup; }
    void setCurrentGroup (Kopete::Group *currentGroup) { m_currentGroup = currentGroup; }

	void setOnlineStatus(const Kopete::OnlineStatus&);

public slots:
    /**
	 * Transmits an outgoing message to the server 
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant Kopete::ChatSession signal)
	 */
    void sendMessage (Kopete::Message & message);
    /**
	 * Called when an incoming message arrived
	 * This displays it in the chatwindow
	 */
    void receivedMessage (const QString & message);

    QString getMsnObj () const { return m_msnobj; }

    void setMsnObj (QString msnobj) { m_msnobj = msnobj; }

    virtual void slotUserInfo();
    virtual void deleteContact ();

    virtual void sendFile (const KUrl & sourceURL = KUrl (),
              const QString & fileName = QString(), uint fileSize = 0L);

    void blockContact ( bool block );
    void slotShowProfile();
    void slotUpdateDisplayPicture();
    virtual void sync( unsigned int flags );
    bool isDisabled() const { return m_disabled; }
    void setDisabled( bool disabled, bool updateServer );
    void slotDontShowEmoticons(bool block);
    bool dontShowEmoticons();

protected slots:
    /**
	 * Show the settings dialog
	 */
    void showContactSettings ();
        /**
	 * Notify the contact that its current Kopete::ChatSession was
	 * destroyed - probably by the chatwindow being closed
	 */
    void slotChatSessionDestroyed ();

protected:
    WlmChatSession * m_msgManager;
    WlmAccount * m_account;
    KToggleAction* m_actionBlockContact;
    KToggleAction* m_actionDontShowEmoticons;
    KAction * m_actionShowProfile;
    KAction * m_actionUpdateDisplayPicture;
    KAction * m_actionPrefs;
    QString m_msnobj;
    QString m_contactSerial;
    Kopete::Group *m_currentGroup;
    bool m_disabled;
    bool m_dontShowEmoticons;
};

#endif
