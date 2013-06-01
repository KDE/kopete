/*
    googletalk.cpp - Google Talk and Google libjingle support

    Copyright (c) 2009 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "googletalk.h"
#include "googletalkcalldialog.h"

#include <QPointer>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QProcess>
//#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QEventLoop>

#define isRunning() callProcess->state() == QProcess::Running
#define callExe "googletalk-call"

#define RESTART_INTERVAL 100000
#define QUIT_INTERVAL 10000

GoogleTalk::GoogleTalk(const QString &jid, const QString &password) {

//	qDebug() << "GoogleTalk::GoogleTalk";
	callProcess = new QProcess;
	callDialog = new GoogleTalkCallDialog;
	timer = new QTimer;

	support = true;
	c = false;
	activeCall = false;

	callProcess->start(callExe);
	callProcess->waitForStarted();

	if ( callProcess->error() == QProcess::FailedToStart ) {
		//Cant start process call
		support = false;
		QPointer <QMessageBox> error = new QMessageBox(QMessageBox::Critical, "Jabber Protocol", i18n("Cannot start process %1. Check your installation of Kopete.", QString(callExe)));
		error->exec();
		delete error;
		return;
	} else {
		//Call exist, quit it
		callProcess->kill();
		callProcess->waitForFinished();
	}

	this->jid = jid;
	this->password = password;

	connect( callDialog->muteButton, SIGNAL(toggled(bool)), this, SLOT(muteCall(bool)) );
	connect( callDialog->acceptButton, SIGNAL(pressed()), this, SLOT(acceptCall()) );
	connect( callDialog->hangupButton, SIGNAL(pressed()), this, SLOT(hangupCall()) );
	connect( callDialog->rejectButton, SIGNAL(pressed()), this, SLOT(rejectCall()) );
	connect( callDialog, SIGNAL(closed()), this, SLOT(cancelCall()) );

}

GoogleTalk::~GoogleTalk() {

//	qDebug() << "GoogleTalk::~GoogleTalk";
	logout();

	delete timer;
	delete callProcess;
	delete callDialog;

}

void GoogleTalk::setUser(const QString &jid, const QString &password) {

//	qDebug() << "GoogleTalk::setUser";
	if ( ! support )
		return;

	c = false;
	activeCall = false;

	this->jid = jid;
	this->password = password;

}

void GoogleTalk::login() {

//	qDebug() << "GoogleTalk::login";
	if ( ! support )
		return;

	if ( isRunning() || isConnected() )
		logout();

	usersOnline.clear();

	connect( callProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(read()) );
	connect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)) );

	c = false;
	activeCall = false;

	callProcess->start(callExe);
//	callProcess->waitForStarted();
//	qDebug() << "started";

	#warning Disabled periodic restart
	//Uncomment this 2 lines if periodic restart is needed
	//connect( timer, SIGNAL(timeout()), this, SLOT(restart()) );
	//timer->start(RESTART_INTERVAL);

}

void GoogleTalk::logout(const QString &res) {

//	qDebug() << "GoogleTalk::logout";
	if ( ! support )
		return;

	timer->stop();

	disconnect( timer, SIGNAL(timeout()), this, SLOT(restart()) );
	disconnect( callProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(read()) );
	disconnect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)) );

	usersOnline.clear();

	if ( activeCall ) {
		activeCall = false;
		closeCallDialog();
		callDialog->userName->setText("");
		callDialog->callStatus->setText("");
	}

	if ( isRunning() && c ) {

		if ( res.isEmpty() )
			emit(disconnected("logout"));
		else
			emit(disconnected(res));

		write("quit");

		c = false;

		QEventLoop * loop = new QEventLoop;
		QTimer * quitTimer = new QTimer;

		connect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), loop, SLOT(quit()) );
		connect( quitTimer, SIGNAL(timeout()), loop, SLOT(quit()) );

		quitTimer->start(QUIT_INTERVAL);
		loop->exec();

		disconnect( quitTimer, SIGNAL(timeout()), loop, SLOT(quit()) );
		disconnect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), loop, SLOT(quit()) );

		if ( isRunning() ) {

			callProcess->kill();

			connect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), loop, SLOT(quit()) );
			connect( quitTimer, SIGNAL(timeout()), loop, SLOT(quit()) );

			quitTimer->start(QUIT_INTERVAL);
			loop->exec();

			disconnect( quitTimer, SIGNAL(timeout()), loop, SLOT(quit()) );
			disconnect( callProcess, SIGNAL(finished(int,QProcess::ExitStatus)), loop, SLOT(quit()) );

			if ( isRunning() )
				callProcess->terminate();

		}

		delete quitTimer;
		delete loop;
		
	}

}

void GoogleTalk::restart() {

//	qDebug() << "GoogleTalk::restart";
	if ( ! activeCall && c ) {
		logout("Periodic restart");
		login();
	}

}

void GoogleTalk::finished(int, QProcess::ExitStatus exitStatus) {

//	qDebug() << "GoogleTalk::finished";
	logout();

	if ( exitStatus == QProcess::CrashExit ) {
//		qDebug() << "crashed - disconnected";
		emit(disconnected("crashed"));
		login();
	}

}

void GoogleTalk::openCallDialog() {

//	qDebug() << "GoogleTalk::openCallDialog";
	callDialog->show();

}

void GoogleTalk::closeCallDialog() {

//	qDebug() << "GoogleTalk::closeCallDialog";
	callDialog->hide();

}

void GoogleTalk::write(const QByteArray &line) {

//	qDebug() << "GoogleTalk::write" << line;
	if ( ! isRunning() )
		return;

	callProcess->write(line);
	callProcess->write("\n");

}

void GoogleTalk::read() {

//	qDebug() << "GoogleTalk::read";
	QStringList input = QString::fromUtf8(callProcess->readAllStandardOutput()).split('\n');

	for ( int i=0; i<input.size(); ++i ) {

		QString line = input.at(i);
//		qDebug() << "line:" << line;

		if ( line.startsWith("JID:") ) {
			//Send user name
			write(jid.toUtf8());
//			qDebug() << "send user name";
		} else if ( line.startsWith("Password:") ) {
			//Send password
			write(password.toUtf8());
//			qDebug() << "send passwd";
		} else if ( line.startsWith("logged in...") ) {
			//We are connected
			c = true;
			emit(connected());
//			qDebug() << "connected";
		} else if ( line.startsWith("logged out...") ) {
			//We are logouted - bad username or passwd
			QString res = line.section("logged out...", -1).trimmed();
			emit(disconnected(res));
//			qDebug() << "bad user";
		} else if ( line.startsWith("Invalid JID") ) {
			//Invalid JID
			emit(disconnected(line));
		} else if ( line.startsWith("Removing from roster:") ) {
			//User doesnt support google talk libjingle
			QString user = line.section(':', -1).section('/', 0, 0).trimmed();
			QString resource = line.section(':', -1).section('/', -1).trimmed();
//			qDebug() << "user offline" << user << resource;
			emit(userOffline(user, resource));
			usersOnline.remove(user, resource);
		} else if ( line.startsWith("Adding to roster:") ) {
			//User support google talk libjingle
			QString user = line.section(':', -1).section('/', 0, 0).trimmed();
			QString resource = line.section(':', -1).section('/', -1).trimmed();
//			qDebug() << "user online" << user << resource;
			emit(userOnline(user, resource));
			if ( ! usersOnline.contains(user, resource) )
				usersOnline.insert(user, resource);
		} else if ( line.startsWith("Found online friend") ) {
			//User support google talk libjingle
			QString user = line.section('\'', 1, 1).section('/', 0, 0).trimmed();
			QString resource = line.section('\'', 1, 1).section('/', -1).trimmed();
//			qDebug() << "user online" << user << resource;
			emit(userOnline(user, resource));
			if ( ! usersOnline.contains(user, resource) )
				usersOnline.insert(user, resource);
		} else if ( line.startsWith("Incoming call from") ) {
			//Incoming call
			QString user = line.section('\'', 1, 1).section('/', 0, 0).trimmed();
			QString resource = line.section('\'', 1, 1).section('/', -1).trimmed();
//			qDebug() << "incoming call" << user << resource;
			activeCall = true;
			callDialog->acceptButton->setEnabled(true);
			callDialog->hangupButton->setEnabled(false);
			callDialog->rejectButton->setEnabled(true);
			callDialog->userName->setText(user);
			callDialog->callStatus->setText(i18n("Answer for incoming call"));
			openCallDialog();
			emit(incomingCall(user, resource));
		} else if ( line.startsWith("call answered") ) {
			//Accepted call
//			qDebug() << "accepted call";
			activeCall = true;
			callDialog->acceptButton->setEnabled(false);
			callDialog->hangupButton->setEnabled(true);
			callDialog->rejectButton->setEnabled(false);
			callDialog->callStatus->setText(i18n("Accepted"));
			emit(acceptedCall());
		} else if ( line.startsWith("calling...") ) {
			//Calling...
//			qDebug() << "calling...";
			activeCall = true;
			callDialog->acceptButton->setEnabled(false);
			callDialog->hangupButton->setEnabled(true);
			callDialog->rejectButton->setEnabled(false);
			callDialog->callStatus->setText(i18n("Calling..."));
			emit(callingCall());
		} else if ( line.startsWith("call not answered") ) {
			//Rejected call
//			qDebug() << "rejected call";
			callDialog->callStatus->setText(i18n("Rejected"));
			closeCallDialog();
			callDialog->userName->setText("");
			activeCall = false;
			emit(rejectedCall());
		} else if ( line.startsWith("call in progress") ) {
			//Call in progress
//			qDebug() << "Call in progress";
			activeCall = true;
			callDialog->acceptButton->setEnabled(false);
			callDialog->hangupButton->setEnabled(true);
			callDialog->rejectButton->setEnabled(false);
			callDialog->callStatus->setText(i18n("Call in progress"));
			emit(progressCall());
		} else if ( line.startsWith("other side hung up") ) {
			//Hanged up
//			qDebug() << "hanged up call";
			write("hangup"); //for correct
			callDialog->userName->setText("");
			callDialog->callStatus->setText(i18n("Other side hung up"));
			closeCallDialog();
			activeCall = false;
			emit(hangedupCall());
		} else if ( line.startsWith("call destroyed") ) {
			closeCallDialog();
			callDialog->callStatus->setText("");
			callDialog->userName->setText("");
			activeCall = false;
			emit(hangedupCall());
		}
	}

}

bool GoogleTalk::isOnline(const QString &user) {

//	qDebug() << "GoogleTalk::isOnline";
	if ( ! c )
		return false;

//	qDebug() << "Online are:" << usersOnline;
	return ( usersOnline.contains(user) && ! activeCall );
}

bool GoogleTalk::isConnected() {

//	qDebug() << "GoogleTalk::isConnected";
	return c;

}

void GoogleTalk::makeCall(const QString &user) {

//	qDebug() << "GoogleTalk::makeCall";
	if ( ! support )
		return;

	if ( ! isOnline(user) )
		return;

	write(QString("call %1").arg(user).toUtf8());

	callDialog->acceptButton->setEnabled(false);
	callDialog->hangupButton->setEnabled(true);
	callDialog->rejectButton->setEnabled(false);
	callDialog->userName->setText(user);
	callDialog->callStatus->setText(i18n("Waiting..."));

	openCallDialog();
	activeCall = true;

}

void GoogleTalk::acceptCall() {

//	qDebug() << "GoogleTalk::acceptCall";
	write("accept");
	activeCall = true;

}

void GoogleTalk::rejectCall() {

//	qDebug() << "GoogleTalk::rejectCall";
	write("reject");

	closeCallDialog();

	callDialog->userName->setText("");
	callDialog->callStatus->setText("");

	activeCall = false;

}

void GoogleTalk::hangupCall() {

//	qDebug() << "GoogleTalk::hangupCall";
	write("hangup");

	closeCallDialog();

	callDialog->userName->setText("");
	callDialog->callStatus->setText("");

	activeCall = false;

}

void GoogleTalk::cancelCall() {

//	qDebug() << "GoogleTalk::cancelCall";
	hangupCall();
	rejectCall();

}

void GoogleTalk::muteCall(bool b) {

//	qDebug() << "GoogleTalk::muteCall";
	if ( ! activeCall )
		return;

	if ( b )
		write("mute");
	else
		write("unmute");

}

#include "googletalk.moc"

