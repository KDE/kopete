/*  Skype Action Handler for Kopete
    Copyright (C) 2009 Pali Rohár <pali.rohar@gmail.com>

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

#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QtDBus>
#include <QProcess>
#include <QTimer>
#include <QEventLoop>
#include <QVariant>

int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);
	QString message = app.arguments().at(1);

	if ( message.isEmpty() ) {
		///Empty message
		return 1;
	}

	QString command;
	QString user;

	if ( message.startsWith("callto:", Qt::CaseInsensitive) ) {
		command = "call";
		user = message.remove("callto:", Qt::CaseInsensitive).remove("/").trimmed();
	} else if ( message.startsWith("tel:", Qt::CaseInsensitive) ) {
		command = "chat";
		user = message.remove("tel:", Qt::CaseInsensitive).remove("/").trimmed();
	} else if ( message.startsWith("skype:", Qt::CaseInsensitive) ) {
		command = message.section("?", 1, 1).trimmed();
		user = message.section(":", 1, 1).section("?", 0, 0).trimmed();
	} else {
		///Unknow message;
		return 1;
	}

	if ( command.isEmpty() || user.isEmpty() ) {
		///Unknow message;
		return 1;
	}

	if ( command == "add" ) {
		///Open add user dialog
	} else if ( command == "call" ) {
		///Start call with user
	} else if ( command == "chat" ) {
		///Open chat window
	} else if ( command == "sendfile" ) {
		///Open send file dialog
	} else if ( command == "userinfo" ) {
		///Open options dialog
	} else {
		///Unknow command
		return 1;
	}

	QDBusInterface skype("org.kde.kopete", "/SkypeActionHandler", "org.kde.Kopete");
	QDBusInterface kopete("org.kde.kopete", "/Kopete", "org.kde.Kopete");

	if ( ! kopete.isValid() ) {
		QProcess process;
		process.start("kopete");
		process.waitForStarted();
		if ( process.error() == QProcess::FailedToStart ) {
			///Cant start Kopete
			return 1;
		}
		process.waitForFinished();
		QDBusInterface kopete("org.kde.kopete", "/Kopete", "org.kde.Kopete");
		if ( ! kopete.isValid() ) {
			///Cant connect to Kopete
			return 1;
		}
	}

	int count = 60;
	QEventLoop * eventloop = new QEventLoop;
	QTimer * timer = new QTimer;

	QObject::connect( timer, SIGNAL(timeout()), eventloop, SLOT(quit()) );
	timer->start(500);

	bool ok = false;
	while ( count > 0 ) {
		eventloop->exec();
		qDebug() << "kopete call:" << kopete.call("isConnected", "Skype", "Skype").arguments().first().toBool() << endl;
		if ( ! kopete.call("isConnected", "Skype", "Skype").arguments().first().toBool() ) {
			kopete.call("connect", "Skype", "Skype");
			--count;
		} else {
			ok = true;
			break;
		}
	}

	timer->stop();
	delete timer;
	delete eventloop;

	if ( ! ok ) {
		///Cant login in Skype
		return 1;
	}

	if ( ! skype.isValid() ) {
		///Cant connect to Skype protocol
		return 1;
	}

	skype.call("SkypeActionHandler", message);

	return 0;
}
