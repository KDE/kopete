#include "connectionmanageritem.h"
#include "telepathyprotocol.h"

//KDE includes
#include <kdebug.h>

//TelepathyQt4 includes
#include <TelepathyQt4/Client/PendingOperation>
#include <TelepathyQt4/Client/PendingReady>
#include <TelepathyQt4/Client/PendingReadyConnectionManager>

ConnectionManagerItem::ConnectionManagerItem(QString cmName, QTreeWidget *parent)
    : QTreeWidgetItem(parent)
{
	kDebug(TELEPATHY_DEBUG_AREA) ;
    m_connectionManager = new Telepathy::Client::ConnectionManager(cmName);

    setText(0, m_connectionManager->name());

    Telepathy::Client::PendingReady *op = m_connectionManager->becomeReady();
    QObject::connect(op,
		SIGNAL(finished(Telepathy::Client::PendingOperation *)),
        this,
		SLOT(setProtocolsSize(Telepathy::Client::PendingOperation *))
	);
}

Telepathy::Client::ProtocolInfoList ConnectionManagerItem::getProtocolInfoList() const
{
	return m_protocolInfoList;
}

QStringList ConnectionManagerItem::getSupportedProtocols() const
{
	return m_supportedProtocols;
}

void ConnectionManagerItem::setProtocolsSize(Telepathy::Client::PendingOperation *operation)
{
	kDebug(TELEPATHY_DEBUG_AREA) ;
	if(operation->isError())
	{
		kDebug(TELEPATHY_DEBUG_AREA) << operation->errorName() << operation->errorMessage();
		return;
	}
	
    int size = m_connectionManager->supportedProtocols().size();
	m_supportedProtocols = m_connectionManager->supportedProtocols();
	m_protocolInfoList = m_connectionManager->protocols();
    setText(1, QString::number(size));
}

#include "connectionmanageritem.moc"


