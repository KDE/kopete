/*
    smpppdcsplugin.h
 
    Copyright (c) 2002-2003 by Chris Howells         <howells@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
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

#include "detector.h"
#include "iconnector.h"
#include "smpppdcsiface.h"

#include "kopeteplugin.h"
#include "kopeteaccount.h"

class QTimer;
class Detector;
class OnlineInquiry;

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
 * @author Chris Howells <howells@kde.org>, Heiko Sch&auml;fer <heiko@rangun.de>
 */
class SMPPPDCSPlugin : public Kopete::Plugin, public IConnector, virtual public SMPPPDCSIFace {
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

    // Implementation of DCOP iface
    /**
     * @brief Checks if we are online.
     * @note This method is reserved for future use. Do not use at the moment!
     * @return <code>TRUE</code> if online, otherwise <code>FALSE</code>
     */
    virtual bool isOnline() const;

    /**
     * @brief Sets the status in all allowed accounts.
     * Allowed accounts are set in the config dialog of the plugin.
     *
     * @see SMPPPDCSPrefs
     */
    virtual void setConnectedStatus( bool newStatus );

    virtual QString detectionMethod() const;

    virtual void aboutToUnload();

public slots:
    void smpppdServerChanged(const QString& server);

private slots:
    void slotCheckStatus();
    void allPluginsLoaded();

private:
    
	void connectAllowed();
    void disconnectAllowed();

private:

    Detector      * m_detectorSMPPPD;
    Detector      * m_detectorNetstat;
    Detector      * m_detectorNetworkStatus;
    bool            m_pluginConnected;
    QTimer        * m_timer;
    OnlineInquiry * m_onlineInquiry;
};

#endif /* SMPPPDCSPLUGIN_H */

// vim: set noet ts=4 sts=4 sw=4:
