/*
    libjingle.h - libjingle support

    Copyright (c) 2009-2014 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef Libjingle_H
#define Libjingle_H

#include <QtCore/QObject>
#include <QProcess>
#include <QHash>

class QTimer;
class LibjingleCallDialog;

/**
 * @author Pali Rohár
 * @short Class to use libjingle with external apps
 * This provide support for libjingle client. Now it support voice call using external libjingle example libjingle-call application
 * @todo Add support for libjingle file transfer
 * You must have installed "libjingle-call" application in PATH
 */
class Libjingle : public QObject
{

	Q_OBJECT

	public:

		/**
		 * Constructor
		 * It does not login automatically. For login use @see login()
		 * @param jid user name (jabber jid) for jabber account (default none)
		 * @param password password for jabber account (default none)
		 * @param host server host for jabber account (default google's one)
		 * @param port server port for jabber account (default google's one)
		 */
		Libjingle(const QString &jid = QString(), const QString &password = QString(), const QString &host = QString(), quint16 port = 0);

		/**
		 * Destructor
		 */
		~Libjingle();

		/**
		 * Set (or change) user name and password for jabber account
		 * Use it if you do not set up in constructor
		 * If you change, first logout @see logout()
		 * @param jid user name (jabber jid) for jabber account (default none)
		 * @param password password for jabber account (default none)
		 */
		void setUser(const QString &jid, const QString &password);

		/**
		 * Set (or change) server host and port for jabber account
		 * Use it if you do not set up in constructor
		 * If you change, first logout @see logout()
		 * @param host server host for jabber account (default google's one)
		 * @param port server port for jabber account (default google's one)
		 */
		void setServer(const QString &host, quint16 port);

		/**
		 * Check if user is online, support libjingle voice call and if no voice call is active
		 * @param user name of contact
		 * @return true if user support voice call
		 */
		bool isOnline(const QString &user);

		/**
		 * Check if we are connected
		 * @return true if we are connected
		 */
		bool isConnected();

	public slots:

		/**
		 * Start and login to jabber server using libjingle example libjingle-call application for voice call support
		 * Do not forget specify user name and password for jabber account
		 * @see setUser(const QString &jid, const QString &password) @see Libjingle(const QString &jid = QString(), const QString &password = QString())
		 */
		void login();

		/**
		 * Logout and quit from jabber server using libjingle example libjingle-call application for voice call support
		 * @param res Resolution why you are going to logout
		 * Resolution is only for signal @see disconnected()
		 */
		void logout(const QString &res = QString());

		/**
		 * It start voice call to user using external libjingle example libjingle-call application
		 * You must be connected if you want to start voice call @see login() @see connected()
		 * @param user Specify user for voice call
		 */
		void makeCall(const QString &user);

		/**
		 * Accept incoming call
		 */
		void acceptCall();

		/**
		 * Reject incoming call
		 */
		void rejectCall();

		/**
		 * Hang up active call
		 */
		void hangupCall();

		/**
		 * Call both hangupCall() and rejectCall()
		 */
		void cancelCall();

		/**
		 * Mute or unmute active call
		 * @param b true for mute, false for unmute
		 */
		void muteCall(bool b);

	private:

		/// libjingle example libjingle-call application process
		QProcess * callProcess;
		/// user name (jid) for jabber account
		QString jid;
		/// password for jabber account
		QString password;
		/// server host for jabber account
		QString host;
		/// server port for jabber account
		quint16 port;
		/// variable if we are connected
		bool c;
		/// variable if we are active voice call
		bool activeCall;
		/// variable if libjingle is supported (if libjingle-call exist in PATH)
		bool support;
		/// List of all online user, who support voice call
		QMultiHash <QString, QString> usersOnline;
		/// voice call dialog
		LibjingleCallDialog * callDialog;
		/// show voice call dialog
		void openCallDialog();
		/// hide voice call dialog
		void closeCallDialog();
		/// restart timer
		QTimer * timer;

	private slots:

		/// slot for read all available data from libjingle example libjingle-call application
		void read();
		/// slot for write line to libjingle example libjingle-call application
		void write(const QByteArray &line);
		/// slot called when libjingle example libjingle-call application failed to start
		void error(QProcess::ProcessError error);
		/// slot called when libjingle example libjingle-call application exit or crashed
		void finished(int, QProcess::ExitStatus exitStatus);
		/// slot for restart libjingle example libjingle-call application
		void restart();

	signals:

		/**
		 * This signal is emitted when we are succesfull login to jabber server
		 */
		void connected();

		/**
		 * This signal is emitted when we are disconnected or logouted from jabber server
		 * @param res Resolution why we are disconneced
		 */
		void disconnected(const QString &res);

		/**
		 * This signal is emitted when user go online and support voice call
		 * @param user name of who go online
		 * @param resource jabber resource of user
		 */
		void userOnline(const QString &user, const QString &resource);

		/**
		 * This signal is emitted when user go offline or not support voice call
		 * After succesfull login it is all user who are online, but dont support voice call
		 * @param user name of user who go offline
		 * @param resource jabber resource of user
		 */
		void userOffline(const QString &user, const QString &resourc);

		/**
		 * This signal is emitted when user call you
		 * @param user name of user who are call you
		 * @param resource jabber resource of user
		 */
		void incomingCall(const QString &user, const QString &resourc);

		/**
		 * This signal is emitted when you start call, but user from other side does not accept/reject call
		 */
		void callingCall();

		/**
		 * This signal is emitted when user from other side accept call
		 */
		void acceptedCall();

		/**
		 * This signal is emitted when user from other side reject call
		 */
		void rejectedCall();

		/**
		 * This signal is emitted when call is active in all side
		 */
		void progressCall();

		/**
		 * This signal is emitted when user from other side hang up call
		 */
		void hangedupCall();

};

#endif // Libjingle_H

