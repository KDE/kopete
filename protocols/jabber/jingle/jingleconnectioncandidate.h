/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

    Copyright (c) 2007      by Joshua Hodosh     <josh.hodosh@gmail.com>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLECONNECTIONCANDIDATE_H
#define JINGLECONNECTIONCANDIDATE_H

#include <QDomElement>
#include <QObject>

//#include <solid/networkinterface.h>
//namespace Solid{
//	class NetworkInterface;
//}

/**
 * A Jingle connection candidate has a
 * <ul><li>protocol: tcp ssltcp, or udp</li>
 * <li>network interface (a Solid::NetworkInterface)</li>
 * <li>socket (a QAbstractSocket)</li>
 * <li>relative quality</li></ul>
 *
 */
class JingleConnectionCandidate : public QObject
{
	Q_OBJECT
public:
//	JingleConnectionCandidate(QAbstractSocket socket, Solid::NetworkInterface nic, float qual):
//		socket_(socket),netIface(nic),quality(qual);
JingleConnectionCandidate();
JingleConnectionCandidate(QString ip, int port){ ip_=(ip); port_=(port);}
JingleConnectionCandidate(const JingleConnectionCandidate &c);
JingleConnectionCandidate &operator=(const JingleConnectionCandidate &other);

virtual ~JingleConnectionCandidate();
	//NOTE const?
	//const QAbstractSocket getSocket(){return socket_;}
	//const Solid::NetworkInterface getNIC(){return netIface;}
	float getQuality(){return quality;}
	//const QString type(){return type_;}

	//void setSocket(QAbstractSocket socket){socket_= socket;}
	//void setNIC(Solid::NetworkInterface nic){ netIface = nic; }
	void setQuality(float qual){quality = qual;}

	//static const QString LOCAL_TYPE="local";
	//static const QString RELAY_TYPE="relay";
	//static const QString STUN_TYPE="stun";

	virtual QDomElement getCandidateElement();

	/**
	 * Returns true if this candidate is a working connection
	 */
	virtual bool isUseful();

protected:
	//QAbstractSocket socket_;
	//Solid::NetworkInterface netIface;
	float quality;
	//QString type_;

	QString ip_;
	int port_;


};

#endif
 
