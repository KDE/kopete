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
#include <QtDBus>

#include <kdebug.h>
#include <klocale.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <qtimer.h>

typedef enum {
	cfConnected,
	cfNotConnected,
	cfNameSent,
	cfProtocolSent,
	cfWaitingStart
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
		QDBusConnection *conn;
		///This timer will keep trying until Skype starts
		QTimer *startTimer;
		///How much time rest? (until I say starting skype did not work)
		int timeRemaining;
		///Wait a while before we conect to just-started skype?
		int waitBeforeConnect;
};

SkypeConnection::SkypeConnection() {
	kDebug() << k_funcinfo << endl;//some debug info

	d = new SkypeConnectionPrivate;//create the d pointer
	d->fase = cfNotConnected;//not connected yet
	d->conn = 0L;//No connetion created
	d->startTimer = 0L;

	connect(this, SIGNAL(received(const QString&)), this, SLOT(parseMessage(const QString&)));//look into all messages
}

SkypeConnection::~SkypeConnection() {
	kDebug() << k_funcinfo << endl;//some debug info

	disconnectSkype();//disconnect before you leave
	delete d;//Remove the D pointer
}

void SkypeConnection::startLogOn() {
	kDebug() << k_funcinfo << endl;//some debug info

	if (d->startTimer) {
		d->startTimer->deleteLater();
		d->startTimer = 0L;
	}

	QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
	QDBusMessage reply = interface.call("Invoke", "Ping");

	QStringList replylist = reply.arguments().at(0).toStringList();
	QStringList::iterator it = replylist.begin();

	if (it == replylist.end()) {
		emit error(i18n("Could not ping Skype"));
		disconnectSkype(crLost);
		emit connectionDone(seNoSkype, 0);
		return;
	}

	d->fase = cfNameSent;
	send(QString("NAME %1").arg(d->appName));
}

void SkypeConnection::gotMessage(const QDBusMessage &message) {
	kDebug() << k_funcinfo << endl;//some debug info

	if (message.member() == "Ping") {//Skype wants to know if we are alive
		kDebug() << "Skype sent us ping, responding" << endl;
		QDBusMessage reply(message);//Create the reply
		reply << getpid();//respond with our PID
		d->conn->send(reply);//send it
		//d->conn->flush();
		return;//It is enough
	}

	if (message.member() == "Notify") {//Something importatnt?
		QStringList messagelist = message.arguments().at(0).toStringList();
		for (QStringList::iterator it = messagelist.begin(); it != messagelist.end(); ++it) {//run trough the whole message
			emit received((*it));//Take out the string and use it
		}

		if (message.isReplyRequired()) {
			kDebug() << "Message expects reply, sending a dummy one" << endl;
			QDBusMessage *reply = new QDBusMessage(message);//generate a reply for that message
			(*reply) << QString("ERROR 2");//write there seme error
			d->conn->send(*reply);//send the message
			//d->conn->flush();
		}
	}
}

void SkypeConnection::parseMessage(const QString &message) {
	kDebug() << k_funcinfo << QString("(message: %1)").arg(message) << endl;//some debug info

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
			if (message.contains(QString("PROTOCOL"), Qt::CaseSensitivity(false))) {//This one inform us what protocol do we use
				bool ok;
				int version = message.section(' ', 1, 1).trimmed().toInt(&ok, 0);//take out the protocol version and make it int
				if (!ok) {
					emit error(i18n("Skype API syntax error"));
					emit connectionDone(seUnknown, 0);
					disconnectSkype(crLost);//lost the connection
					return;//I have enough
				}
				d->protocolVer = version;//this will be the used version of protocol
				d->fase = cfConnected;
				emit connectionDone(seSuccess, version);//tell him that we are connected at last
			} else {//the API is not ready yet, try later
				emit error(i18n("Skype API not ready yet, wait a bit longer"));
				emit connectionDone(seUnknown, 0);
				disconnectSkype(crLost);
				return;
			}
			break;//Other messages are ignored, waiting for the protocol response
		}
	}
}

void SkypeConnection::connectSkype(const QString &start, const QString &appName, int protocolVer, int bus, bool startDBus, int launchTimeout, int waitBeforeConnect) {
	kDebug() << k_funcinfo << endl;//some debug info

	if (d->fase != cfNotConnected)
		return;

	d->appName = appName;
	d->protocolVer = protocolVer;
	d->conn = new QDBusConnection ("Skype");

	if (bus == 0)
		d->conn->connectToBus(QDBusConnection::SessionBus, "Skype");
	else
		d->conn->connectToBus(QDBusConnection::SystemBus, "Skype");

	///TODO: Port to kde4
	/*if ((!d->conn) || (!d->conn->isConnected())) {
		if ((bus == 0) && (startDBus)) {
			KProcess bus_launch;
			bus_launch << "dbus_launch";
			bus_launch << "--exit-with-session";
			connect(&bus_launch, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(setEnv(KProcess *, char*, int )));
			//bus_launch.start(KProcess::Block, KProcess::Stdout);
			bus_launch.start();
			connectSkype(start, appName, protocolVer, bus, false, launchTimeout, waitBeforeConnect);//try it once again, but if the dbus start did not work, it won't work that time ether, so do not cycle
			return;
		}
		emit error(i18n("Could not connect to DBus"));
		disconnectSkype(crLost);
		emit connectionDone(seNoDBus, 0);
		return;
	}*/

	///TODO: Port to kde4
	/*d->conn->registerObjectPath("org.kde.kopete.skype", "/com/Skype/Client");

	connect(d->conn, SIGNAL(messageArrived(const DBusQt::Message&)), this, SLOT(gotMessage(const DBusQt::Message &)));*/

	d->conn->registerObject("org.kde.kopete.skype" , this, QDBusConnection::ExportAllSlots );
	d->conn->registerService("/com/Skype/Client");


	{
		QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
		QDBusMessage reply = interface.call("Invoke", "Ping");

		QStringList replylist = reply.arguments().at(0).toStringList();
		QStringList::iterator it = replylist.begin();
		if ((it == replylist.end()) || (bool)((*it).toInt() != true)) {
			if (!start.isEmpty()) {//try starting Skype by the given command
				KProcess sk;
				QStringList args = start.split(' ');
				for (QStringList::iterator i = args.begin(); i != args.end(); ++i) {
					sk << (*i);
				}
				///TODO: Port to kde4
				/*if (!sk.start(KProcess::DontCare, KProcess::NoCommunication)) {
					emit error(i18n("Could not launch Skype"));
					disconnectSkype(crLost);
					emit connectionDone(seNoSkype, 0);
					return;
				}*/
				d->fase = cfWaitingStart;
				d->startTimer = new QTimer();
				connect(d->startTimer, SIGNAL(timeout()), this, SLOT(tryConnect()));
				d->startTimer->start(1000);
				d->timeRemaining = launchTimeout;
				d->waitBeforeConnect = waitBeforeConnect;
				return;
			}
			emit error(i18n("Could not find Skype"));
			disconnectSkype(crLost);
			emit connectionDone(seNoSkype, 0);
			return;
		}
	}

	startLogOn();
}

void SkypeConnection::disconnectSkype(skypeCloseReason reason) {
	kDebug() << k_funcinfo << endl;//some debug info

	if (!d->conn) //nothing to disconnect
		return;
	d->conn->disconnectFromBus(QString("Skype"));//close the connection
	///TODO: Port to kde4
	//delete d->conn;//destroy it
	d->conn = 0L;//andmark it as empty
	if (d->startTimer) {
		d->startTimer->stop();
		d->startTimer->deleteLater();
		d->startTimer = 0L;
	}

	d->fase = cfNotConnected;//No longer connected
	emit connectionDone(seCanceled, 0);
	emit connectionClosed(reason);//we disconnect
}

void SkypeConnection::send(const QString &message) {
	kDebug() << k_funcinfo << QString("(message: %1)").arg(message) << endl;//some debug info

	if (d->fase == cfNotConnected)
		return;//not connected, posibly because of earlier error, do not show it again

	QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
	QDBusMessage reply = interface.call("Invoke", message);

	//d->conn->flush();
	///TODO: Port to kde4
	/*if (d->conn->error()) {//There was some error
		emit error(i18n("Error while sending a message to skype (%1)").arg(d->conn->getError()));//say there was the error
		if (d->fase != cfConnected)
			emit connectionDone(seUnknown, 0);//Connection attempt finished with error
		disconnectSkype(crLost);//lost the connection

		return;//this is enough, no more errors please..
	}*/

	QStringList replylist = reply.arguments().at(0).toStringList();

	for (QStringList::iterator it = replylist.begin(); it != replylist.end(); ++it) {
		emit received((*it));
	}
//	d->conn->send(m);
}

bool SkypeConnection::connected() const {
	kDebug() << k_funcinfo << endl;//some debug info

	return d->fase == cfConnected;
}

int SkypeConnection::protocolVer() const {
	kDebug() << k_funcinfo << endl;//some debug info

	return d->protocolVer;//just give him the protocol version
}

SkypeConnection &SkypeConnection::operator <<(const QString &message) {
	send(message);//just send it
	return *this;//and return yourself
}

QString SkypeConnection::operator %(const QString &message) {
	kDebug() << k_funcinfo << "message: " << message << endl;//some debug info

	if (d->fase == cfNotConnected)
		return "";//not connected, posibly because of earlier error, do not show it again

	QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
	QDBusMessage reply = interface.call("Invoke", message);

	//d->conn->flush();
	///TODO: Port to kde4
	/*if (d->conn->error()) {//There was some error
		emit error(i18n("Error while sending a message to skype (%1)").arg(d->conn->getError()));//say there was the error
		if (d->fase != cfConnected)
			emit connectionDone(seUnknown, 0);//Connection attempt finished with error
		disconnectSkype(crLost);//lost the connection
		return "";//this is enough, no more errors please..
	}*/

	QStringList replylist = reply.arguments().at(0).toStringList();

	for (QStringList::iterator it = replylist.begin(); it != replylist.end(); ++it) {
		kDebug() << (*it) << endl;//show what we have received
		return (*it);//ok, just return it
	}

	return "";//the skype did not respond, which is unusual but what can I do..
}

void SkypeConnection::setEnv(KProcess *, char *buffer, int length) {
	kDebug() << k_funcinfo << endl;//some debug info

	char *myBuff = new char[length + 1];
	myBuff[length] = '\0';
	memcpy(myBuff, buffer, length);//copy the output from there

	char *next;

	for (char *c = myBuff; *c; c++) if (*c == '=') {
		*c = '\0';//Split the string
		next = c + 1;//This is the next one
		break;
	}

	if (strcmp(myBuff, "DBUS_SESSION_BUS_ADDRESS") != 0) {
		delete[] myBuff;
		return;//something I'm not interested in
	}

	//strip the apostrophes or quotes given by the dbus-launch command
	if ((next[0] == '\'') || (next[0] == '"')) ++next;
	int len = strlen(next);
	if ((next[len - 1] == '\'') || (next[len - 1] == '"')) next[len - 1] = '\0';

	setenv(myBuff, next, false);//and set the environment variable

	delete[] myBuff;
}

void SkypeConnection::tryConnect() {
	kDebug() << k_funcinfo << endl;//some debug info

	/*{
		QDBusMessage m = QDBusMessage::createMethodCall("com.Skype.API", "/com/Skype", "com.Skype.API", "Invoke");
		m << QString("Ping");
		//QDBusMessage m("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ServiceExists");
		//m << QString("com.Skype.API");
		m.setAutoActivation(true);

		QDBusMessage reply = d->conn->call(m);

		QDBusMessage::iterator it = reply.begin();
		if ((it == reply.end()) || ((*it).toBool() != true)) {
			if (--d->timeRemaining == 0) {
				d->startTimer->stop();
				d->startTimer->deleteLater();
				d->startTimer = 0L;
				emit error(i18n("Could not find Skype"));
				disconnectSkype(crLost);
				emit connectionDone(seNoSkype, 0);
				return;
			}
			return;//Maybe next time
		}
	}*/

	d->startTimer->stop();
	d->startTimer->deleteLater();
	d->startTimer = 0L;
	if (d->waitBeforeConnect) {
		QTimer::singleShot(1000 * d->waitBeforeConnect, this, SLOT(startLogOn()));
		//Skype does not like being bothered right after it's start, give it a while to breathe
	} else
		startLogOn();//OK, it's your choise
}

#include "skypeconnection.moc"
