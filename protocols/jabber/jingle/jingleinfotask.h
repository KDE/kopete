
#ifdef JINGLEINFOTASK_H
#define JINGLEINFOTASK_H

namespace XMPP{
	Task;
}

/**
 * A struct representing a hostname and port.
 * May contain an IP address instead of unresolved hostname.
 */
struct PortAddress
{
	PortAddress(QString hn, quint16 p):hostname(hn),port(p);
	PortAddress(QHostAddress ad, quint16 p): address(ad), port(p);
	QString hostname;
	QHostAddress address;
	quint16 port;
}

class  JingleInfoTask : XMPP::Task
{
public:
	JingleInfoTask(XMPP::Task* parent);

	void updateInfo();
public signals:
	void signalJingoInfoUpdate(QString relayToken, QList<QHostAddress> relayHosts, QList<PortAddress> stunHosts);

};

#endif 
