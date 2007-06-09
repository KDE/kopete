/*
    meanwhilecontact.h - a contact

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MEANWHILECONTACT_H
#define MEANWHILECONTACT_H

#include <qmap.h>
#include "kopetecontact.h"
#include "kopetemessage.h"
#include "meanwhileaccount.h"

class KAction;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

class MeanwhileContact : public Kopete::Contact
{
    Q_OBJECT
public:

    MeanwhileContact(QString userId, QString nickname,
            MeanwhileAccount *account, Kopete::MetaContact *parent);
    ~MeanwhileContact();

    virtual bool isReachable();

    virtual void serialize(QMap<QString, QString> &serializedData,
            QMap<QString, QString> &addressBookData);

    virtual Kopete::ChatSession *manager(
            CanCreateFlags canCreate = CanCreate);

    QString meanwhileId() const;

        virtual void sync(unsigned int changed = 0xff);

public slots:

    void sendMessage( Kopete::Message &message );
    void receivedMessage( const QString &message );
    virtual void slotUserInfo();

protected slots:
    void showContactSettings();
    void slotChatSessionDestroyed();
    void slotSendTyping(bool isTyping);
	
private:
    QString m_meanwhileId;
    Kopete::ChatSession *m_msgManager;
};

#endif
