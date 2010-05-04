/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef AKONADIHISTORYLOGGER_H
#define AKONADIHISTORYLOGGER_H


#include "akonadihistoryplugin.h"
#include <akonadi/item.h>
#include <Akonadi/Collection>
#include <QDateTime>
#include <history/history.h>
#include <KJob>

#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"

class QDate;
class QTimer;

namespace Kopete {
class Message;
}
namespace Kopete {
class Contact;
}

class AkonadiHistoryPlugin;
 

class AkonadiHistoryLogger : public QObject
{
    Q_OBJECT
public:
  
    explicit AkonadiHistoryLogger(Kopete::Contact *c , QObject *parent = 0, QObject *hPlugin = 0 );
    ~AkonadiHistoryLogger();
    void appendMessage( const Kopete::Message &msg , const Kopete::Contact *c=0L  );
    
   
private:

    QPointer<AkonadiHistoryPlugin> m_hPlugin;
    
    Akonadi::Collection m_tosaveInCollection;
    Akonadi::Collection m_parentCollection;
    Akonadi::Collection m_kopeteChat;

    Akonadi::Item m_tosaveInItem;

    Kopete::Message m_message;

    const Kopete::Contact * m_contact;

    History m_history;
    bool itemFetched;
    bool itemModifiedOnce;

private slots:
   
    void appendMessage2();
    void pCollectionCreated(KJob*);
    void createCollection(KJob *);
    void createItem();
    void itemCreateDone(KJob*);
    void itemsReceivedDone(Akonadi::Item::List);
    void slotItemModified(KJob*);
    
};

#endif // AKONADIHISTORYLOGGER_H
