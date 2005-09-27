/*
    detector.h
 
    Copyright (c) 2004-2005 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef DETECTOR_H
#define DETECTOR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qobject.h>
#include <qstringlist.h>

#define SMPPPDCS_CONFIG_GROUP "SMPPPDCS Plugin"

class KProcess;
class DCOPClient;
class IConnector;
class KExtendedSocket;

/**
 * @brief Detector to find out if there is a connection to the internet.
 *
 * Uses either the SuSE Meta PPP Daemon or netstat to inquire an existing
 * internet connection.
 *
 * This is useful on dial up connections on SuSE systems.
 *
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 *
 */

class Detector : protected QObject {
	Q_OBJECT

	Detector(const Detector&);
	Detector& operator=(const Detector&);

    /**
     * @brief Enumerates the different states of communication with the smpppd
     */
    enum CommunicationState {
        /// No connection to the smpppd is negotiated
        UNSETTLED,
        /// A connection to the smpppd is negotiated
        SMPPPDSETTLED,
        /// A challenge for the authentication is outspoken
        CHALLENGED,
        /// Ready for settling an smpppd connection
        READY,
        /// Listing of all interfaces requested
        LISTIFCFG,
        /// Status of all interfaces requested
        STATUSIFCFG
    };

public:
	/**
	 * @brief Creates an <code>Detector</code> instance.
	 *
	 * @param connector A connector to send feedback to the calling object
	 */
	Detector(IConnector * connector);

	/**
	 * @brief Destroys an <code>Detector</code> instance.
	 *
	 */
	virtual ~Detector();

	/**
	 * @brief Use netstat to get the status of an internet connection.
	 *
	 * Calls IConnector::setConnectedStatus of the IConnector given in
	 * the constructor.
	 *
	 * @see IConnector
	 *
	 */
	virtual void netstatCheckStatus();

#ifdef USE_SMPPPD
	/**
	 * @brief Use the smpppd to get the status of an internet connection.
	 *
	 * Calls IConnector::setConnectedStatus of the IConnector given in
	 * the constructor.
	 *
	 * @see IConnector
	 *
	 */
    	virtual void smpppdCheckStatus();
#endif

signals:
	void retryRequested();

private:
#ifdef USE_SMPPPD
	void connectToSMPPPD();
	QStringList readSMPPPD();
	void writeSMPPPD(const char * cmd);
#endif

	/**
	* @brief Makes an response for an challenge
	*
	* If the smpppd requests an authorization, it sends an challenge.
	* The password has to be appended to this challenge and the m5sum
	* in hex-display has to get responded to the smpppd.
	*
	* @param chex the challenge in hex display
	* @param password the passwort to authenticate
	* @return the reponse for the smpppd
	*/
	QString make_response(const QString& chex, const QString& password) const;

private slots:
    // Original cs-plugin code
    void slotProcessStdout( KProcess *process, char *buffer, int len );

    /**
     * Notify when the netstat process has exited
     */
    void slotProcessExited( KProcess *process );

private:
#ifdef USE_SMPPPD
	CommunicationState m_comState;
	DCOPClient        *m_client;
	static QCString    m_kinternetApp;
	KExtendedSocket   *m_sock;
	QStringList        m_ifcfgs;
#endif

	IConnector        *m_connector;
	KProcess          *m_process;
};

#endif
