/*  This file is part of the KDE project
    Copyright (C) 2005 Kopete Developers <kopete-devel@kde.org>
    Copyright (C) 2008 Pali Rohár <pali.rohar@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#include "skypeconnection.h"
#include "clientadaptor.h"

#include <QtDBus>
#include <qstringlist.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <unistd.h>

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

	QDBusMessage reply = QDBusInterface("com.Skype.API", "/com/Skype", "com.Skype.API").call("Invoke", "PING");
	QStringList replylist = reply.arguments().at(0).toStringList();

	if ( replylist.isEmpty() || replylist.at(0) != "PONG"){
		emit error(i18n("Could not ping Skype"));
		disconnectSkype(crLost);
		emit connectionDone(seNoSkype, 0);
		return;
	}

	d->fase = cfNameSent;
	send(QString("NAME %1").arg(d->appName));
}

void SkypeConnection::parseMessage(const QString &message) {
	kDebug() << k_funcinfo << endl;//some debug info

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

void SkypeConnection::Notify(const QString &message){
	kDebug() << k_funcinfo << "Got message:" << message << endl;//show what we have got
	emit received(message);
}

void SkypeConnection::connectSkype(const QString &start, const QString &appName, int protocolVer, int launchTimeout, int waitBeforeConnect, const QString &name, const QString &pass) {
	kDebug() << k_funcinfo << endl;//some debug info

	if (d->fase != cfNotConnected)
		return;

	d->appName = appName;
	d->protocolVer = protocolVer;

	new ClientAdaptor(this);
	QDBusConnection::sessionBus().registerObject("/com/Skype/Client", this); //Register skype client to dbus for receiving messages to slot Notify

	{
		QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
		QDBusMessage reply = interface.call("Invoke", "PING");
		QStringList replylist = reply.arguments().at(0).toStringList();

		bool started = interface.isValid();
		bool loggedin = ! replylist.isEmpty() && replylist.at(0) == "PONG";

		if ( ! started || ! loggedin ){
			if ( ! started && ! start.isEmpty() ) {//try starting Skype by the given command
				QProcess * skype_process = new QProcess;
				QStringList args = start.split(' ');
				QString skypeBin = args.takeFirst();
				if ( !name.isEmpty() && !pass.isEmpty() ){
					args << "--pipelogin";
				}
				kDebug() << k_funcinfo << "Starting skype process" << skypeBin << "with parms" << args << endl;
				skype_process->start(skypeBin, args);
				if ( !name.isEmpty() && !pass.isEmpty() ){
					kDebug() << k_funcinfo << "Sending login name:" << name << endl;
					skype_process->write(name.trimmed().toLocal8Bit());
					skype_process->write(" ");
					kDebug() << k_funcinfo << "Sending password" << endl;
					skype_process->write(pass.trimmed().toLocal8Bit());
					skype_process->closeWriteChannel();
				}
				skype_process->waitForStarted();
				kDebug() << k_funcinfo << "Skype process state:" << skype_process->state() << "Skype process error:" << skype_process->error() << endl;
				if (skype_process->state() != QProcess::Running || skype_process->error() == QProcess::FailedToStart) {
					emit error(i18n("Could not launch Skype"));
					disconnectSkype(crLost);
					emit connectionDone(seNoSkype, 0);
					return;
				}
			} else {
				if ( start.isEmpty() ){
					emit error(i18n("Could not find Skype"));
					disconnectSkype(crLost);
					emit connectionDone(seNoSkype, 0);
					return;
				}
			}
			d->fase = cfWaitingStart;
			d->startTimer = new QTimer();
			connect(d->startTimer, SIGNAL(timeout()), this, SLOT(tryConnect()));
			d->startTimer->start(1000);
			d->timeRemaining = launchTimeout;
			d->waitBeforeConnect = waitBeforeConnect;
			return;
		}
	}

	startLogOn();
}

void SkypeConnection::disconnectSkype(skypeCloseReason reason) {
	kDebug() << k_funcinfo << endl;//some debug info

	if (d->startTimer) {
		d->startTimer->stop();
		d->startTimer->deleteLater();
		d->startTimer = 0L;
	}

	d->fase = cfNotConnected;//No longer connected
	emit connectionDone(seCanceled, 0);
	emit connectionClosed(reason);//we disconnect
}

QString SkypeConnection::operator %(const QString &message) {
	kDebug() << k_funcinfo << "Send message:" << message << endl;//some debug info

	if (d->fase == cfNotConnected)
		return "";//not connected, posibly because of earlier error, do not show it again

	QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
	QDBusMessage reply = interface.call("Invoke", message);

	if ( interface.lastError().type() != QDBusError::NoError && interface.lastError().type() != QDBusError::Other ){//There was some error
		if ( message == "PING" ){
			emit error(i18n("Could not ping Skype"));
			emit connectionDone(seNoSkype, 0);
			return "";//this is enough, no more errors please..
		}
		emit error(i18n("Error while sending a message to skype (%1)").arg(QDBusError::errorString(interface.lastError().type())));//say there was the error
		if (d->fase != cfConnected)
			emit connectionDone(seUnknown, 0);//Connection attempt finished with error
		disconnectSkype(crLost);//lost the connection
		return "";//this is enough, no more errors please..
	}

	QStringList replylist = reply.arguments().at(0).toStringList();

	if ( message == "PING" && ( replylist.isEmpty() || replylist.at(0) != "PONG" ) ){
		emit error(i18n("Could not ping Skype"));
		emit connectionDone(seNoSkype, 0);
		return "";//this is enough, no more errors please..
	}

	for (QStringList::iterator it = replylist.begin(); it != replylist.end(); ++it) {
		kDebug() << "Reply message:" << (*it) << endl;//show what we have received
		return (*it);//ok, just return it
	}

	return "";//the skype did not respond, which is unusual but what can I do..
}

void SkypeConnection::send(const QString &message) {
	kDebug() << k_funcinfo << endl;//some debug info

	QString reply = *this % message;
	if ( ! reply.isEmpty() && reply != "" )
		emit received(reply);
}

SkypeConnection &SkypeConnection::operator <<(const QString &message) {
	send(message);//just send it
	return *this;//and return yourself
}

bool SkypeConnection::connected() const {
	kDebug() << k_funcinfo << endl;//some debug info

	return d->fase == cfConnected;
}

int SkypeConnection::protocolVer() const {
	kDebug() << k_funcinfo << endl;//some debug info

	return d->protocolVer;//just give him the protocol version
}

void SkypeConnection::tryConnect() {
	kDebug() << k_funcinfo << endl;//some debug info

	{
		QDBusInterface interface("com.Skype.API", "/com/Skype", "com.Skype.API");
		QDBusMessage reply = interface.call("Invoke", "PING");
		QStringList replylist = reply.arguments().at(0).toStringList();

		bool started = interface.isValid();
		bool loggedin = ! replylist.isEmpty() && replylist.at(0) == "PONG";

		if ( ! started || ! loggedin ){
			if (--d->timeRemaining == 0) {
				d->startTimer->stop();
				d->startTimer->deleteLater();
				d->startTimer = 0L;
				if ( !started )
					emit error(i18n("Could not find Skype"));
				else
					emit error(i18n("User could not login to Skype"));
				disconnectSkype(crLost);
				emit connectionDone(seNoSkype, 0);
				return;
			}
			return;//Maybe next time
		}
	}

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
