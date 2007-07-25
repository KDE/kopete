
#ifndef JINGLECONNECTIONCANDIDATE_H
#define JINGLECONNECTIONCANDIDATE_H

#include <QDomElement>

namespace Solid{
	class NetworkInterface;
}

/**
 * A Jingle connection candidate has a
 * <ul><li>protocol: tcp ssltcp, or udp</li>
 * <li>network interface (a Solid::NetworkInterface)</li>
 * <li>socket (a QAbstractSocket)</li>
 * <li>relative quality</li></ul>
 *
 */
class JingleConnectionCandidate
{
public:
	JingleConnectionCandidate(QAbstractSocket socket, Solid::NetworkInterface nic, float qual, QString type):
		socket_(socket),netIFace(nic),quality(qual),type_(type);
	//NOTE const?
	const QAbstractSocket getSocket(){return socket_;}
	const Solid::NetworkInterface getNIC(){return netIface;}
	float getQuality(){return quality;}
	const QString type(){return type_;}

	void setSocket(QAbstractSocket socket){socket_= socket;}
	void setNIC(Solid::NetworkInterface nic){ netIface = nic; }
	void setQuality(float qual){quality = qual);

	const QString LOCAL_TYPE="local";
	const QString RELAY_TYPE="relay";
	const QString STUN_TYPE="stun";

	virtual QDomElement getCandidateElement();

	/**
	 * Returns true if this candidate is a working connection
	 */
	virtual bool isUseful();

private:
	QAbstractSocket socket_;
	Solid::NetworkInterface netIface;
	float quality;
	QString type_;

	QString ip_;
	int port_;


};

#endif
 
