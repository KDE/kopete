//
// C++ Implementation: skypeconnection
//
// Description:
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#define DBUS_API_SUBJECT_TO_CHANGE
#include "skypeconnection.h"

///@todo When QT 4 is used, the signal-slot wrapper will be better, replace it
#include "connection.h"

#include <kdebug.h>
#include <klocale.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum {
	cfConnected,
	cfNotConnected,
	cfNameSent,
	cfProtocolSent
} connFase;

class SkypeConnectionPrivate {
	public:
		///Are we connected/connecting?
		connFase fase;
		///How will we be known to skype?
		QString appName;
		///What is the protocol version used (wanted if not connected yet)
		int protocolVer;
		///The connection to DBus
		DBusQt::Connection *conn;
};

SkypeConnection::SkypeConnection() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d = new SkypeConnectionPrivate;//create the d pointer
	d->fase = cfNotConnected;//not connected yet
	d->conn = 0L;//No connetion created

	connect(this, SIGNAL(received(const QString&)), this, SLOT(parseMessage(const QString&)));//look into all messages
}

SkypeConnection::~SkypeConnection() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	disconnectSkype();//disconnect before you leave
	delete d;//Remove the D pointer
}

void SkypeConnection::startLogOn() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	DBusQt::Message ping("com.Skype.API", "/com/Skype", "com.Skype.API", "Ping");//create a ping message
	ping.setAutoActivation(true);
	DBusQt::Message reply = d->conn->sendWithReplyAndBlock(ping);//send it there

	DBusQt::Message::iterator it = reply.begin();
	if (it == reply.end()) {
		emit error(i18n("Could not ping Skype"));
		disconnectSkype(crLost);
		emit connectionDone(seNoSkype, 0);
		return;
	}

	d->fase = cfNameSent;
	send(QString("NAME %1").arg(d->appName));
}

void SkypeConnection::gotMessage(const DBusQt::Message &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (message.member() == "Ping") {//Skype wants to know if we are alive
		kdDebug(14311) << "Skype sent us ping, responding" << endl;
		DBusQt::Message reply(message);//Create the reply
		reply << getpid();//respond with our PID
		d->conn->send(reply);//send it
		d->conn->flush();
		return;//It is enough
	}

	if (message.member() == "Notify") {//Something importatnt?
		for (DBusQt::Message::iterator it = message.begin(); it != message.end(); ++it) {//run trough the whole message
			emit received((*it).toString());//Take out the string and use it
		}

		if (message.expectReply()) {
			kdDebug(14311) << "Message expects reply, sending a dummy one" << endl;
			DBusQt::Message *reply = new DBusQt::Message(message);//generate a reply for that message
			(*reply) << QString("ERROR 2");//write there seme error
			d->conn->send(*reply);//send the message
			d->conn->flush();
		}
	}
}

void SkypeConnection::parseMessage(const QString &message) {
	kdDebug(14311) << k_funcinfo << QString("(message: %1)").arg(message) << endl;//some debug info

	switch (d->fase) {
		case cfNameSent: {

			if (message == "OK") {//Accepted by skype
				d->fase = cfProtocolSent;//Sending the protocol
				send(QString("PROTOCOL %1").arg(d->protocolVer));//send the protocol version
			} else {//Not accepted by skype
				emit error(i18n("Skype did not accept this application"));//say there is an error
				emit connectionDone(seAuthorization, 0);//Problem with authorization
				disconnectSkype(crLost);//Lost the connection
			}
			break;
		}
		case cfProtocolSent: {
			if (message.contains("PROTOCOL", false)) {//This one inform us what protocol do we use
				bool ok;
				int version = message.section(' ', 1, 1).stripWhiteSpace().toInt(&ok, 0);//take out the protocol version and make it int
				if (!ok) {
					emit error(i18n("Skype API syntax error"));
					emit connectionDone(seUnknown, 0);
					disconnectSkype(crLost);//lost the connection
					return;//I have enough
				}
				d->protocolVer = version;//this will be the used version of protocol
				d->fase = cfConnected;
				emit connectionDone(seSuccess, version);//tell him that we are connected at last
			}
			break;//Other messages are ignored, waiting for the protocol response
		}
	}
}

void SkypeConnection::connectSkype(bool start, const QString &appName, int protocolVer) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->fase != cfNotConnected)
		return;

	d->appName = appName;
	d->protocolVer = protocolVer;

	d->conn = new DBusQt::Connection(DBUS_BUS_SYSTEM, this);

	if ((!d->conn) || (!d->conn->isConnected())) {
		emit error(i18n("Could not connect to DBus"));
		disconnectSkype(crLost);
		emit connectionDone(seNoDBus, 0);
		return;
	}

	d->conn->registerObjectPath("org.kde.kopete.skype", "/com/Skype/Client");

	connect(d->conn, SIGNAL(messageArrived(const DBusQt::Message&)), this, SLOT(gotMessage(const DBusQt::Message &)));

	{
		DBusQt::Message m("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ServiceExists");
		m << QString("com.Skype.API");
		m.setAutoActivation(true);

		DBusQt::Message reply = d->conn->sendWithReplyAndBlock(m);

		DBusQt::Message::iterator it = reply.begin();
		if ((it == reply.end()) || ((*it).toBool() != true)) {
			emit error(i18n("Could not find Skype"));
			disconnectSkype(crLost);
			emit connectionDone(seNoSkype, 0);
			return;
		}
	}

	startLogOn();
}

void SkypeConnection::disconnectSkype(skypeCloseReason reason) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (!d->conn) //nothing to disconnect
		return;
	d->conn->close();//close the connection
	delete d->conn;//destroy it
	d->conn = 0L;//andmark it as empty

	d->fase = cfNotConnected;//No longer connected
	emit connectionClosed(reason);//we disconnect
}

void SkypeConnection::send(const QString &message) {
	kdDebug(14311) << k_funcinfo << QString("(message: %1)").arg(message) << endl;//some debug info

	if (d->fase == cfNotConnected)
		return;//not connected, posibly because of earlier error, do not show it again

	DBusQt::Message m("com.Skype.API", "/com/Skype", "com.Skype.API", "Invoke");
	m << message;
	m.setAutoActivation(true);
	DBusQt::Message reply = d->conn->sendWithReplyAndBlock(m);

	d->conn->flush();
	if (d->conn->error()) {//There was some error
		emit error(i18n("Error while sending a message to skype (%1)").arg(d->conn->getError()));//say there was the error
		if (d->fase != cfConnected)
			emit connectionDone(seUnknown, 0);//Connection attempt finished with error
		disconnectSkype(crLost);//lost the connection

		return;//this is enough, no more errors please..
	}

	for (DBusQt::Message::iterator it = reply.begin(); it != reply.end(); ++it) {
		emit received((*it).toString());//use the message
	}
//	d->conn->send(m);
}

bool SkypeConnection::connected() const {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	return d->fase == cfConnected;
}

int SkypeConnection::protocolVer() const {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	return d->protocolVer;//just give him the protocol version
}

SkypeConnection &SkypeConnection::operator <<(const QString &message) {
	send(message);//just send it
	return *this;//and return yourself
}

QString SkypeConnection::operator %(const QString &message) {
	kdDebug(14311) << k_funcinfo << "message: " << message << endl;//some debug info

	if (d->fase == cfNotConnected)
		return "";//not connected, posibly because of earlier error, do not show it again

	DBusQt::Message m("com.Skype.API", "/com/Skype", "com.Skype.API", "Invoke");
	m << message;
	m.setAutoActivation(true);
	DBusQt::Message reply = d->conn->sendWithReplyAndBlock(m);

	d->conn->flush();
	if (d->conn->error()) {//There was some error
		emit error(i18n("Error while sending a message to skype (%1)").arg(d->conn->getError()));//say there was the error
		if (d->fase != cfConnected)
			emit connectionDone(seUnknown, 0);//Connection attempt finished with error
		disconnectSkype(crLost);//lost the connection
		return "";//this is enough, no more errors please..
	}

	for (DBusQt::Message::iterator it = reply.begin(); it != reply.end(); ++it) {
		kdDebug(14311) << (*it).toString() << endl;//show what we have received
		return (*it).toString();//ok, just return it
	}

	return "";//the skype did not respond, wich is unusual but what can I do..
}

#include "skypeconnection.moc"
