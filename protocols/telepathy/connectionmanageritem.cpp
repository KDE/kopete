#include "connectionmanageritem.h"
#include "telepathyprotocol.h"

//KDE includes
#include <kdebug.h>

//TelepathyQt4 includes
#include <TelepathyQt4/Client/ConnectionManager>
#include <TelepathyQt4/Client/PendingOperation>

ConnectionManagerItem::ConnectionManagerItem(QString cmName, QTreeWidget *parent)
    : QTreeWidgetItem(parent)
{
    m_connectionManager = new Telepathy::Client::ConnectionManager(cmName);

    setText(0, m_connectionManager->name());

    Telepathy::Client::PendingOperation *op = m_connectionManager->becomeReady();
    connect(op, SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this, SLOT(setProtocolsSize(Telepathy::Client::PendingOperation *)));
}

Telepathy::Client::ConnectionManager *ConnectionManagerItem::connectionManager()
{
    return m_connectionManager;
}

void ConnectionManagerItem::setProtocolsSize(Telepathy::Client::PendingOperation *p)
{
    int size = 0;
    if(!p->isError())
    {
        size = m_connectionManager->supportedProtocols().size();
    }
    else
    {
        kDebug(TELEPATHY_DEBUG_AREA) << p->errorName() << ": " << p->errorMessage();
    }
    setText(1, QString::number(size));
}

#include "connectionmanageritem.moc"


