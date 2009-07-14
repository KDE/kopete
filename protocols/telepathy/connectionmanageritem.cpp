#include "connectionmanageritem.h"
#include "telepathyprotocol.h"

//KDE includes
#include <kdebug.h>

//TelepathyQt4 includes
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

ConnectionManagerItem::ConnectionManagerItem(QString cmName, QTreeWidget *parent)
        : QTreeWidgetItem(parent)
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    m_connectionManager = Tp::ConnectionManager::create(cmName);

    setText(0, m_connectionManager->name());

    Tp::PendingReady *op = m_connectionManager->becomeReady();
    QObject::connect(op,
                     SIGNAL(finished(Tp::PendingOperation *)),
                     this,
                     SLOT(setProtocolsSize(Tp::PendingOperation *))
                    );
}

Tp::ProtocolInfoList ConnectionManagerItem::getProtocolInfoList() const
{
    return m_protocolInfoList;
}

QStringList ConnectionManagerItem::getSupportedProtocols() const
{
    return m_supportedProtocols;
}

void ConnectionManagerItem::setProtocolsSize(Tp::PendingOperation *operation)
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    if (operation->isError()) {
        kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
        return;
    }

    int size = m_connectionManager->supportedProtocols().size();
    m_supportedProtocols = m_connectionManager->supportedProtocols();
    m_protocolInfoList = m_connectionManager->protocols();
    setText(1, QString::number(size));
}


#include "connectionmanageritem.moc"

