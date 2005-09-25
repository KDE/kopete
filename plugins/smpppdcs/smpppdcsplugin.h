/*
    smpppdcsplugin.h
 
    Copyright (c) 2002-2003 by Chris Howells         <howells@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMPPPDCSPLUGIN_H
#define SMPPPDCSPLUGIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "kopeteplugin.h"
#include "kopeteaccount.h"
//Added by qt3to4:
#include <QByteArray>

#define SMPPPDCS_CONFIG_GROUP "SMPPPDCS Plugin"

class QTimer;
class KProcess;
class DCOPClient;
class KExtendedSocket;

/**
 * @brief Plugin for the detection of an internet connection
 *
 * This plugin inquires either the smpppd or netstat
 * for an existing internet connection and depending
 * on that connects or disconnects all accounts.
 *
 * Therefore it should be enabled on dial up network
 * connections.
 *
 * @author Chris Howells <howells@kde.org>, Heiko Schaefer <heiko@rangun.de>
 */
class SMPPPDCSPlugin : public Kopete::Plugin {
    Q_OBJECT
    SMPPPDCSPlugin(const SMPPPDCSPlugin&);
    SMPPPDCSPlugin& operator=(const SMPPPDCSPlugin&);

public:
    /**
     * @brief Creates an <code>SMPPPDCSPlugin</code> instance
     */
    SMPPPDCSPlugin( QObject *parent, const char *name, const QStringList &args );

    /**
     * @brief Destroys an <code>SMPPPDCSPlugin</code> instance
     */
    virtual ~SMPPPDCSPlugin();

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

protected:
    /**
     * @brief Should the smpppd be used for inquiring
     * @return <code>TRUE</code> for smpppd, <code>FALSE</code> for netstat
     */
    bool useSmpppd() const;

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
    void slotCheckStatus();
	void allPluginsLoaded();

    // Original cs-plugin code
    void slotProcessStdout( KProcess *process, char *buffer, int len );

    /**
     * Notify when the netstat process has exited
     */
    void slotProcessExited( KProcess *process );

private:
    void setConnectedStatus( bool newStatus );
	void netstatCheckStatus();
	
	void connectAllowed();
	void disconnectAllowed();
	
#ifdef USE_SMPPPD
    void smpppdCheckStatus();
    QStringList readSMPPPD();
    void writeSMPPPD(const char * cmd);
    void connectToSMPPPD();
#endif

private:

#ifdef USE_SMPPPD
	CommunicationState m_comState;
	KExtendedSocket   *m_sock;
    QStringList        m_ifcfgs;
	DCOPClient        *m_client;
    static QByteArray    m_kinternetApp;
#endif

    bool               m_pluginConnected;
	QTimer            *m_timer;
    KProcess          *m_process;
};

#endif /* SMPPPDCSPLUGIN_H */

// vim: set noet ts=4 sts=4 sw=4:

