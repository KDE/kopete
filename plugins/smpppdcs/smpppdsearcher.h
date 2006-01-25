/*
    smpppdsearcher.h
 
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


#ifndef SMPPPDSEARCHER_H
#define SMPPPDSEARCHER_H

#include <kresolver.h>

class KProcess;

/**
 * @brief Searches a network for a smpppd
 *
 * @todo Use of the SLP to find the smpppd
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 */
class SMPPPDSearcher : public QObject {
    Q_OBJECT

    SMPPPDSearcher(const SMPPPDSearcher&);
    SMPPPDSearcher& operator=(const SMPPPDSearcher&);

public:
    /**
     * @brief Creates an <code>SMPPPDSearcher</code> instance
     */
    SMPPPDSearcher();
	
    /**
     * @brief Destroys an <code>SMPPPDSearcher</code> instance
     */
    ~SMPPPDSearcher();

    /**
     * @brief Triggers a network scan to find a smpppd
	 * @see smpppdFound
	 * @see smpppdNotFound
     */
    void searchNetwork();
	
	void cancelSearch();

protected:
    /**
     * @brief Scans a network for a smpppd
	 *
	 * Scans a network for a smpppd described by
	 * ip and mask.
	 *
     * @param ip   the ntwork ip
     * @param mask the network mask
     * @return <code>TRUE</code> if an smpppd was found
     */
    bool scan(const QString& ip, const QString& mask);

signals:
    /**
     * @brief A smppd was found
	 * 
     * @param host the host there the smpppd was found
     */
    void smpppdFound(const QString& host);
	
    /**
     * @brief No smpppd was found 
     */
    void smpppdNotFound();
	
	void scanStarted(uint total);
	void scanProgress(uint cur);
	void scanFinished();

protected slots:
    void slotStdoutReceivedIfconfig(KProcess * proc, char * buf, int len);
    void slotStdoutReceivedNetstat (KProcess * proc, char * buf, int len);

private:
	bool m_cancelSearchNow;
    KProcess * m_procIfconfig;
    KProcess * m_procNetstat;
};

inline void SMPPPDSearcher::cancelSearch() {
	m_cancelSearchNow = TRUE;
}

#endif

